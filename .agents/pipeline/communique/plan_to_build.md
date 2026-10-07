# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.4.0 "The Interface & Experience Update"  
**Task Name:** SQA Automation Hardening  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** COMPLETED ✅  
**Source Plan:** [`c:\Dev\TheKlangSuite\.agents\pipeline\plans\completed\2026-10-07_sqa_automation_hardening.md`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/plans/completed/2026-10-07_sqa_automation_hardening.md)  

---

## Strategic Objective
Implement the senior SQA hardening recommendations across `gui_tests` and `test/GuiTestHelpers.h`:
1. **Dual Timeout Architecture:** A background watchdog thread (`std::jthread`) enforcing a 5-minute global process limit and a 30-second local test heartbeat to prevent deadlocks from hanging CI or dev sessions.
2. **The "Wait-Fail" Component Locator:** A resilient polling locator (`waitForComponent<T>`) that pumps the JUCE message thread during async transitions (modals, callouts, page navigation) before timing out.
3. **Automated Deduplicated Failure Snapshots:** Captures high-res offscreen PNGs to `test_screenshots/` with per-suite/10-second deduplication to prevent disk flooding. Outputs clickable `file:///` links for instant inspection.
4. **Failure-First Reporting & Execution Profiling:** High-visibility failure summaries printed at the end of runs, accompanied by a "Slowest Tests" execution leaderboard.
5. **Seeded & Replayable Chaos Monkey (`--chaos`):** On-demand stress suite injecting pseudo-random click/drag/resize bursts with deterministic seed logging for reproducible crash debugging.

---

## Acceptance Criteria & Execution Guardrails
1. **Target Files**:
   - `test/GuiTestHelpers.h` (Watchdog, wait-fail locator, deduplicator, profiling reporter)
   - `test/gui_tests.cpp` (Watchdog startup, CLI argument parsing, failure summary hook)
   - `test/ChaosMonkeySuite.h` (New seeded pseudo-random UI stress suite)
   - `CMakeLists.txt` (Ensure `test/ChaosMonkeySuite.h` is included in test target)
   - `.gitignore` (Add `test_screenshots/`)
2. **Clean Builds**:
   - Both Debug and Release builds compile cleanly with zero warnings.
3. **Deterministic Fast Runs**:
   - Normal `gui_tests.exe` must complete in < 5 seconds with zero flakiness.
   - Chaos monkey is strictly opt-in via `--chaos`.
4. **Job's Done Protocol**:
   - Check off all phases in `PLAN.md`.
   - Update `build_to_plan.md` to `Status: COMPLETE ✅`.
   - Update `plan_to_build.md` to `Status: COMPLETED ✅`.
   - Auto-archive `PLAN.md` to `.agents/pipeline/plans/completed/`.
   - Sound the Job's Done chime!
