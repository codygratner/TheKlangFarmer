# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-06  
**Active Milestone:** v0.4.0 (Fast-Tracked Non-Destructive Test Tooling)  
**Task Name:** Automated VST3 Parameter Validation Suite & Headless `pluginval` Runner  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  

---

## Strategic Objective
Implement the 6-pillar in-engine VST3 parameter validation suite in `test/PluginIntensiveTestSuite.h` covering 100% of parameters across both The Klang Farmer and The Klang Planter, and provide a headless `pluginval` runner script in `tools/run_pluginval.ps1`.

This task is 100% test suite and tooling code—it modifies zero runtime audio DSP math or UI components, making it completely safe to build while the user is away from desktop.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev` (or branch as needed).
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly across Phases 1 through 4.
4. **Validation**: Execute `gui_tests.exe` and confirm 100% assertions pass with 0 failures.
5. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
