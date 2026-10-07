# Implementation Plan: Fix Release Teardown Crash (0xC0000005) in VersionChecker

## Goal
Eliminate the process teardown access violation (`0xC0000005` in `ntdll.dll!RtlFreeHeap`) when running `gui_tests.exe` in the Release configuration by making `VersionChecker` inherit from `juce::DeletedAtShutdown`, managing its singleton via an explicit pointer, and invoking `VersionChecker::teardown()` prior to `guiContext` destruction in `main()`.

---

## Technical Analysis & Root Cause
1. In `test/gui_tests.cpp`, `GuiTestHelpers::ScopedGuiContext guiContext;` is allocated on the stack in `main()`.
2. When `main()` returns, `guiContext` is destructed, invoking `juce::shutdownJuce_GUI()`. This tears down JUCE's `MessageManager`, `Desktop`, and event dispatch loop.
3. Afterward, C++ runtime static destruction executes. `VersionChecker::getInstance()` stored its singleton in a static function-local variable (`static VersionChecker instance;`).
4. Its destructor called `stopThread(3000)` and invoked `juce::ChangeBroadcaster` and `juce::Thread` cleanup *after* the JUCE thread/message subsystem was already dead, causing an access violation in `ntdll.dll!RtlFreeHeap` during CRT shutdown.
5. In addition, `VersionChecker` was not registered with `juce::DeletedAtShutdown`, so it was not cleaned up during `shutdownJuce_GUI()`.
6. Furthermore, in `test/PluginIntensiveTestSuite.h`, `CallOutBox::launchAsynchronously` was called with `box.dismiss()`, scheduling an asynchronous modal cleanup via `MessageManager::callAsync` that triggered a double-free against `editor` child destruction upon test suite return. Replacing this with deterministic `std::make_unique<juce::CallOutBox>(testCallout, area, editor.get())` resolved the modal teardown crash completely.

---

## Architecture & Implementation Phases

### Phase 1: VersionChecker Header Update
Location: `source/VersionChecker.h`
- Inherit from `juce::DeletedAtShutdown`:
  ```cpp
  class VersionChecker : public juce::Thread,
                         public juce::ChangeBroadcaster,
                         public juce::DeletedAtShutdown
  ```
- Expose a public static teardown method:
  ```cpp
  static void teardown();
  ```

---

### Phase 2: VersionChecker Singleton Teardown Implementation
Location: `source/VersionChecker.cpp`
- Replace function-local static with managed pointer and clean `teardown()` / destructor reset:
  ```cpp
  static VersionChecker* activeVersionCheckerInstance = nullptr;

  VersionChecker& VersionChecker::getInstance()
  {
      if (activeVersionCheckerInstance == nullptr)
          activeVersionCheckerInstance = new VersionChecker();
      return *activeVersionCheckerInstance;
  }

  void VersionChecker::teardown()
  {
      if (activeVersionCheckerInstance != nullptr)
      {
          delete activeVersionCheckerInstance;
          activeVersionCheckerInstance = nullptr;
      }
  }

  VersionChecker::VersionChecker()
      : juce::Thread("TKF_VersionChecker")
  {
  }

  VersionChecker::~VersionChecker()
  {
      stopThread(3000);
      activeVersionCheckerInstance = nullptr;
  }
  ```

---

### Phase 3: Test Harness Teardown Hook
Location: `test/gui_tests.cpp`, `test/PluginIntensiveTestSuite.h`
- In `main()`, replace `VersionChecker::getInstance().stopThread(2000);` with `VersionChecker::teardown();` before `guiContext` is destructed:
  ```cpp
      reporter.printSummary();

      VersionChecker::teardown();
      GuiTestHelpers::pumpMessageLoop(20, 10);

      return (reporter.failed == 0) ? 0 : 1;
  ```
- In `test/PluginIntensiveTestSuite.h`, use deterministic `std::make_unique<juce::CallOutBox>(testCallout, area, editor.get())` and synchronous reset.

---

### Phase 4: Validation & Verification
- Compile `gui_tests` in Release mode:
  ```powershell
  cmake --build build --config Release --target gui_tests
  ```
- Run `build/Release/gui_tests.exe` and check exit code:
  ```powershell
  .\build\Release\gui_tests.exe ; "EXIT CODE: $LASTEXITCODE"
  ```
- Confirm:
  - 215 / 215 tests pass.
  - Process exits cleanly with `EXIT CODE: 0` (zero `ntdll.dll` / `0xC0000005` errors).
- Run `build/Release/dsp_tests.exe` to guarantee zero regressions.

---

## Acceptance Criteria
- [x] `VersionChecker` inherits `juce::DeletedAtShutdown`.
- [x] `VersionChecker::teardown()` safely deletes the active instance and nulls the pointer.
- [x] `test/gui_tests.cpp` invokes `VersionChecker::teardown()` before `guiContext` destruction.
- [x] `build\Release\gui_tests.exe` exits cleanly with code `0`.
- [x] All 215 GUI unit tests pass.
- [x] `/cut-release` can resume cleanly.
