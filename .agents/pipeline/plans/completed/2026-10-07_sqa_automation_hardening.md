# Implementation Plan: SQA Automation Hardening

> **Status:** COMPLETED ✅  
> **Target Version:** v0.4.0 (Pre-Flight Testing Infrastructure)  
> **Authority:** New Klang City (Ivory Tower)  
> **Executor:** Klang Industries (Factory Floor)  
> **Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  

---

## 1. Overview & Objectives

Implement the senior SQA recommendations across `gui_tests` and `test/GuiTestHelpers.h`:
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
- `test/HardeningSuites.h` (Verification unit tests for Watchdog, waitForComponent, and deduplicator)
- `CMakeLists.txt` (Included `test/ChaosMonkeySuite.h` in test target)
- `.gitignore` (Added `test_screenshots/` and `test_artifacts/`)

---

## 3. Phased Execution Review

### Phase 1: Dual Timeout Watchdog Architecture
- [x] **1.1: Add Atomic Heartbeat & Watchdog in `test/GuiTestHelpers.h`:**
  - Implemented `GuiTestHelpers::Watchdog` with `isRunning`, `suiteStartMs`, `lastHeartbeatMs`, and `std::jthread`.
  - Enforced 5-minute global limit and 30-second local heartbeat timeout.
- [x] **1.2: Wire Watchdog into `test/gui_tests.cpp`:**
  - Initialized `Watchdog::start(5, 30)` in `main()`, gracefully stopped with `Watchdog::stop()`.

### Phase 2: The "Wait-Fail" Component Locator Pattern
- [x] **2.1: Implement `waitForComponent<T>` in `test/GuiTestHelpers.h`:**
  - Implemented resilient polling loop pumping the JUCE dispatch loop in 20ms slices.
  - Automatically pings `Watchdog::pingHeartbeat()` to keep watchdog alive during async transitions.

### Phase 3: Automated Deduplicated Failure Snapshots
- [x] **3.1: Add `test_screenshots/` to `.gitignore`:**
  - Added `test_screenshots/` and `test_artifacts/` to gitignore.
- [x] **3.2: Implement `SnapshotDeduplicator` in `test/GuiTestHelpers.h`:**
  - Implemented rate limiter suppressing redundant captures within 10 seconds for the same test/component.
- [x] **3.3: Upgrade `captureFailureArtifact` in `test/GuiTestHelpers.h`:**
  - Writes PNG offscreen snapshots to `test_screenshots/` and logs clickable `file:///` URIs.

### Phase 4: High-Visibility Reporting & Execution Profiling
- [x] **4.1: Expand `TestReporter` with Step Timing & Failure Cache:**
  - Cached `failures` with message and snapshot URI.
  - Profiled per-suite durations in high-resolution ticks.
- [x] **4.2: Implement Elevated Reporting in `printSummary()`:**
  - Formatted critical failure summary box and Execution Profiling Leaderboard.

### Phase 5: Seeded & Replayable Chaos Monkey Suite
- [x] **5.1: Create `test/ChaosMonkeySuite.h`:**
  - Seeded randomized event injection (clicks, drags, double-clicks, resizing) over 3000ms.
  - Prominently logs seed and reproduction CLI (`gui_tests --chaos --seed=<SEED>`).
- [x] **5.2: Wire CLI Flags in `test/gui_tests.cpp`:**
  - Added `--chaos` and `--seed=<N>` parsing.
  - 1,230,640 randomized events survived without crash.

---

## 4. Verification & Acceptance Criteria
- [x] Both Debug and Release builds compile cleanly with zero warnings.
- [x] 100% of existing tests pass (`gui_tests.exe`: 300/300 passed in Release in 6.90s).
- [x] Watchdog heartbeats verified.
- [x] `gui_tests.exe --chaos` successfully executed 3-second randomized stress run with seed logging.
- [x] Snapshot deduplicator confirms rate limiting.
- [x] Obsidian vault synced (`sync_obsidian_vault.ps1`).
