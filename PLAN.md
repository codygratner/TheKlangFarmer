# Implementation Plan: SQA Automation Hardening

> **Status:** READY_FOR_EXECUTION  
> **Target Version:** v0.4.0 (Pre-Flight Testing Infrastructure)  
> **Authority:** New Klang City (Ivory Tower)  
> **Executor:** Klang Industries (Factory Floor)  
> **Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  

---

## 1. Overview & Objectives

Implement the senior SQA recommendations (derived from the consultation with Tom) across `gui_tests` and `test/GuiTestHelpers.h`:
1. **Dual Timeout Architecture:** A background watchdog thread (`std::jthread`) enforcing a 5-minute global process limit and a 30-second local test heartbeat to prevent deadlocks from hanging CI or dev sessions.
2. **The "Wait-Fail" Component Locator:** A resilient polling locator (`waitForComponent<T>`) that pumps the JUCE message thread during async transitions (modals, callouts, page navigation) before timing out.
3. **Automated Deduplicated Failure Snapshots:** Captures high-res offscreen PNGs to `test_screenshots/` with per-suite/10-second deduplication to prevent disk flooding. Outputs clickable `file:///` links for instant inspection.
4. **Failure-First Reporting & Execution Profiling:** High-visibility failure summaries printed at the end of runs, accompanied by a "Slowest Tests" execution leaderboard.
5. **Seeded & Replayable Chaos Monkey (`--chaos`):** On-demand stress suite injecting pseudo-random click/drag/resize bursts with deterministic seed logging for reproducible crash debugging.

---

## 2. Target Files

- `test/GuiTestHelpers.h` (Watchdog, wait-fail locator, deduplicator, profiling reporter)
- `test/gui_tests.cpp` (Watchdog startup, CLI argument parsing, failure summary hook)
- `test/ChaosMonkeySuite.h` (New seeded pseudo-random UI stress suite)
- `CMakeLists.txt` (Ensure `test/ChaosMonkeySuite.h` is included in test target)
- `.gitignore` (Add `test_screenshots/`)

---

## 3. Detailed Phased Execution Plan

### Phase 1: Dual Timeout Watchdog Architecture
**Target Files:** `test/GuiTestHelpers.h`, `test/gui_tests.cpp`

- [ ] **1.1: Add Atomic Heartbeat & Watchdog in `test/GuiTestHelpers.h`:**
  ```cpp
  namespace GuiTestHelpers {
      struct Watchdog {
          static inline std::atomic<bool> isRunning{ false };
          static inline std::atomic<uint64_t> lastHeartbeatMs{ 0 };
          static inline std::atomic<uint64_t> suiteStartMs{ 0 };
          static inline std::atomic<bool> testFailedDueToTimeout{ false };
          static inline std::unique_ptr<std::jthread> workerThread;

          static void start(int globalTimeoutMinutes = 5, int stepTimeoutSeconds = 30);
          static void stop();
          static void pingHeartbeat();
      };
  }
  ```
  - Watchdog thread sleeps in 500ms intervals.
  - If `now - suiteStartMs > 5 * 60 * 1000`: Logs `[FATAL GLOBAL TIMEOUT: 5m exceeded]`, calls `std::exit(1)`.
  - If `now - lastHeartbeatMs > 30 * 1000`: Logs `[LOCAL STEP TIMEOUT: 30s exceeded]`, flags failure, captures emergency snapshot, and invokes `juce::MessageManager::getInstance()->stopDispatchLoop()`.

- [ ] **1.2: Wire Watchdog into `test/gui_tests.cpp`:**
  - Call `GuiTestHelpers::Watchdog::start(5, 30)` in `main()` before running suites.
  - Call `GuiTestHelpers::Watchdog::stop()` at normal program termination.

---

### Phase 2: The "Wait-Fail" Component Locator Pattern
**Target Files:** `test/GuiTestHelpers.h`

- [ ] **2.1: Implement `waitForComponent<T>` in `test/GuiTestHelpers.h`:**
  ```cpp
  template <typename T>
  inline T* waitForComponent(juce::Component* parent, 
                             const juce::String& identifier, 
                             int timeoutMs = 30000, 
                             int pollIntervalMs = 20, 
                             juce::Component* rootToSnapshot = nullptr,
                             TestReporter* reporter = nullptr) 
  {
      auto start = juce::Time::getMillisecondCounter();
      while (juce::Time::getMillisecondCounter() - start < static_cast<juce::uint32>(timeoutMs)) {
          Watchdog::pingHeartbeat();
          
          // Pump message loop for pollIntervalMs
          juce::Timer::callAfterDelay(pollIntervalMs, []{ 
              if (auto* mm = juce::MessageManager::getInstanceWithoutCreating()) 
                  mm->stopDispatchLoop(); 
          });
          juce::MessageManager::getInstance()->runDispatchLoop();

          // Search tree
          if (auto* target = ComponentFinder::findByType<T>(parent)) {
              if (identifier.isEmpty() || target->getName() == identifier || target->getComponentID() == identifier)
                  return target;
          }
      }

      // Timeout expired
      if (reporter) {
          reporter->expect(false, "waitForComponent timed out waiting for: " + identifier, 
                           rootToSnapshot ? rootToSnapshot : parent, "wait_timeout");
      }
      return nullptr;
  }
  ```

---

### Phase 3: Automated Deduplicated Failure Snapshots
**Target Files:** `test/GuiTestHelpers.h`, `.gitignore`

- [ ] **3.1: Add `test_screenshots/` to `.gitignore`:**
  - Ensure local screenshot artifacts are never committed to Git.

- [ ] **3.2: Implement `SnapshotDeduplicator` in `test/GuiTestHelpers.h`:**
  ```cpp
  struct SnapshotDeduplicator {
      static inline juce::String lastFailedTest;
      static inline juce::String lastFailedComponentId;
      static inline uint64_t lastCaptureTimeMs = 0;

      static bool shouldCapture(const juce::String& testName, const juce::String& compId) {
          auto now = juce::Time::getMillisecondCounter();
          if (testName == lastFailedTest && compId == lastFailedComponentId && (now - lastCaptureTimeMs < 10000)) {
              return false; // Deduplicate within 10 seconds
          }
          lastFailedTest = testName;
          lastFailedComponentId = compId;
          lastCaptureTimeMs = now;
          return true;
      }
  };
  ```

- [ ] **3.3: Upgrade `captureFailureArtifact` in `test/GuiTestHelpers.h`:**
  - Save PNG to `test_screenshots/<TestName>_<StepName>_<Timestamp>.png`.
  - Check `SnapshotDeduplicator::shouldCapture()` before disk write.
  - Print full clickable absolute file URI:
    `std::cerr << "  [FAILURE SNAPSHOT]: file:///" << file.getFullPathName().replaceCharacter('\\', '/') << std::endl;`

---

### Phase 4: High-Visibility Reporting & Execution Profiling
**Target Files:** `test/GuiTestHelpers.h`, `test/gui_tests.cpp`

- [ ] **4.1: Expand `TestReporter` with Step Timing & Failure Cache:**
  ```cpp
  struct FailureRecord {
      juce::String testName;
      juce::String failureMessage;
      juce::String screenshotPath;
  };

  struct SuiteTiming {
      juce::String suiteName;
      double durationSeconds;
  };

  struct TestReporter {
      int passed = 0;
      int failed = 0;
      juce::String currentSuite;
      juce::String currentTest;
      juce::int64 currentTestStartTicks = 0;
      std::vector<FailureRecord> failures;
      std::vector<SuiteTiming> suiteTimings;

      void beginSuite(const juce::String& suiteName);
      void endSuite();
      void beginTest(const juce::String& testName);
      bool expect(bool condition, const juce::String& message, juce::Component* compToSnapshot = nullptr, const juce::String& stepName = "");
      void printSummary();
  };
  ```

- [ ] **4.2: Implement Elevated Reporting in `printSummary()`:**
  - If `failures.size() > 0`: Print high-visibility **🚨 CRITICAL FAILURE SUMMARY** box listing failure messages and clickable screenshot paths.
  - Sort `suiteTimings` descending and print **⏱️ EXECUTION PROFILING LEADERBOARD** showing top 5 slowest suites and total execution time.

---

### Phase 5: Seeded & Replayable Chaos Monkey Suite
**Target Files:** `test/ChaosMonkeySuite.h`, `test/gui_tests.cpp`, `CMakeLists.txt`

- [ ] **5.1: Create `test/ChaosMonkeySuite.h`:**
  - Implements `ChaosMonkeySuite::runSuite(TestReporter& reporter, uint32_t seed, int durationMs = 3000)`.
  - Uses `juce::Random` initialized with `seed` (defaults to random device if `seed == 0`, logs the seed!).
  - Loops over active Farmer and Planter editors, firing randomized mouse downs, drags, clicks, and window resized events.
  - If a crash or exception occurs, reports the exact reproduction seed: `gui_tests --chaos --seed=<SEED>`.

- [ ] **5.2: Wire CLI Flags in `test/gui_tests.cpp`:**
  - Add `--chaos` flag to trigger `ChaosMonkeySuite`.
  - Add `--seed=<N>` parameter parser.
  - Default `--all` keeps chaos off to maintain < 3s deterministic build-validate loop.

---

## 4. Verification & Acceptance Criteria

- [ ] Both Debug and Release builds compile cleanly with zero warnings (`cmake --build build --config Release --target gui_tests`).
- [ ] 100% of existing tests pass (`build/bin/Release/gui_tests.exe`) in under 5 seconds with execution profiling leaderboard displayed.
- [ ] Watchdog heartbeats verify that no hangs occur during standard sweeps.
- [ ] Running `gui_tests.exe --chaos` successfully executes a 3-second randomized stress run with seed logging.
- [ ] Snapshot deduplicator confirms rate limiting when consecutive assertions fail.
- [ ] Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1` to ensure telemetry reflects the test results.
