# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-06  
**Active Milestone:** v0.3.1 (Hotfix & GUI Performance Tuning)  
**Task Name:** Optimize Planter GUI Rendering & Eliminate UI Thread Latency  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  

---

## Strategic Objective
Eliminate GUI input lag and sluggishness in The Klang Planter by setting `PlanterHeaderVisualizer` to opaque, reducing the timer frequency from 60 Hz to 30 Hz (matching Farmer), optimizing the oscilloscope spline path from 128 to 64 points, and caching static header strings to stop redundant parent window repaints.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev`.
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly across Phases 1 through 4.
4. **Validation**: Execute `gui_tests.exe` and confirm 100% assertions pass with 0 failures (215/215).
5. **Deployment**: Run `deploy.ps1` to update `current_build\Standalone\The Klang Planter.exe` and system VST3 binaries.
6. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
