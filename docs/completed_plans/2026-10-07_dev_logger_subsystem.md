# Implementation Plan: Developer Logging Subsystem (`TKS_LOG`) & Diagnostics Engine

**Active Milestone:** v0.3.2 "Agent Infrastructure & Editor Upgrades"  
**Branch:** `0.3.2-dev`  
**Execution Target:** Klang Industries (Factory Floor)  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** COMPLETED ✅

---

## 1. Architectural Blueprint & Requirements

### 1.1 Core Logging Subsystem (`source/DevLogger.h`)
- Create a header-only utility `source/DevLogger.h` in namespace `RlyehSound`.
- **Dual-Mode Output**:
  - In Debug builds (`JUCE_DEBUG`), logs are formatted with timestamp, level tag, file/line context, and written simultaneously to:
    - The OS/IDE debugger stream (`OutputDebugString` on Windows / `DBG`).
    - A local rotating file: `%LOCALAPPDATA%/TheKlangSuite/dev.log` managed by `juce::FileLogger::createDefaultAppLogger("TheKlangSuite", "dev.log", "=== The Klang Suite Dev Session ===", 5 * 1024 * 1024)`.
  - In Release builds (`!JUCE_DEBUG` / `NDEBUG`), all macros compile to `do {} while (false)` for zero binary strings, zero allocations, and zero CPU cycles.
- **Audio Thread Invariant Guard**:
  - `DevLogger` tracks the real-time audio thread ID via `std::atomic<std::thread::id> audioThreadId`.
  - Processors register their audio thread in `prepareToPlay()`.
  - If `log()` is ever invoked on the audio thread:
    - Debug build trips `jassert(!isAudioThread())` to alert the developer immediately.
    - Safe early return: `if (isAudioThread()) return;` before any heap allocation or file lock occurs.

### 1.2 Macro Interface
```cpp
#if JUCE_DEBUG
  #define TKS_LOG_INFO(msg)  ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Info,  (msg), __FILE__, __LINE__)
  #define TKS_LOG_WARN(msg)  ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Warn,  (msg), __FILE__, __LINE__)
  #define TKS_LOG_ERROR(msg) ::RlyehSound::DevLogger::getInstance().log(::RlyehSound::DevLogger::Level::Error, (msg), __FILE__, __LINE__)
  #define TKS_LOG(msg)       TKS_LOG_INFO(msg)
#else
  #define TKS_LOG_INFO(msg)  do {} while (false)
  #define TKS_LOG_WARN(msg)  do {} while (false)
  #define TKS_LOG_ERROR(msg) do {} while (false)
  #define TKS_LOG(msg)       do {} while (false)
#endif
```

### 1.3 Strategic Integration Points
- **ParameterManager**: Log asset loading metrics (number of controls and strings parsed) and warn on missing optional properties.
- **FarmerProcessor / PlanterProcessor**: Register audio thread ID and log `prepareToPlay` / `releaseResources` lifecycle events.
- **TheKlangEditor**: Replace scattered raw `juce::Logger::writeToLog` calls with structured `TKS_LOG_INFO`.

---

## 2. Phased Implementation Steps

### Phase 1: Core Logger Implementation (`source/DevLogger.h` & `CMakeLists.txt`)
- [x] 1. Create `source/DevLogger.h` implementing `DevLogger` with `Level` enum (`Info`, `Warn`, `Error`), `createDefaultAppLogger`, audio thread tracking, and preprocessor macros.
- [x] 2. Add `source/DevLogger.h` to `CMakeLists.txt` under `TheKlangFarmer`, `TheKlangPlanter`, and `TheKlangEditor` target sources.

### Phase 2: Engine & Editor Integration
- [x] 1. In `source/FarmerProcessor.cpp` and `source/PlanterProcessor.cpp`:
   - In `prepareToPlay`: call `DevLogger::getInstance().registerAudioThread(std::this_thread::get_id());` and log sample rate and block size.
   - In `releaseResources`: log resource cleanup.
- [x] 2. In `source/ParameterManager.cpp`:
   - Add `TKS_LOG_INFO` logging resource counts during `ParameterManager::ParameterManager()` and `reloadFromJson()`.
- [x] 3. In `tools/editor/Main.cpp`, `tools/editor/MainComponent.cpp`, and `source/UIComponents.cpp`:
   - Replace legacy `juce::Logger::writeToLog` calls with `TKS_LOG_INFO`.

### Phase 3: Static Guardrail Update (`audiothread-guard`)
- [x] 1. Static Guardrail Verification: Verified zero runtime logging violations in processing loops; delegated `.agents/skills/audiothread-guard/SKILL.md` update to New Klang City per Strict Guardrail & Skills Governance Gate.

### Phase 4: Test Suite Expansion (`test/DevLoggerTest.h`)
- [x] 1. Create `test/DevLoggerTest.h` with test suite `DevLoggerTest`:
   - Stage 1: Verify INFO, WARN, ERROR logs format cleanly without throwing.
   - Stage 2: Register a mock audio thread ID, call `TKS_LOG_INFO` from that mock thread, and assert that `isAudioThread()` returns true and execution safely aborts without writing or deadlocking.
   - Stage 3: Verify the log file `%LOCALAPPDATA%/TheKlangSuite/dev.log` is created and writable.
- [x] 2. Register `DevLoggerTest::runSuite(reporter)` in `test/gui_tests.cpp`.

### Phase 5: Build, Validate & Deploy
- [x] 1. Run `/audiothread-guard` on modified files to verify zero violations in audio loops.
- [x] 2. Run full test suite: `dsp_tests.exe` (100% pass) and `gui_tests.exe` (294/294 pass in both Debug and Release).
- [x] 3. Run `deploy.ps1` to sync artifacts to `current_build/` and system directories.
- [x] 4. Conclude with dual communique closure and "JOB'S DONE" handoff.
