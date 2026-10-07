# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-06  
**Active Milestone:** v0.4.0 (Pre-Release Theming Harness)  
**Task Name:** The Klang Editor CalloutBox Preview & Live Theming Harness  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  

---

## Strategic Objective
Bring full visual inspection and live JSON-driven theming capability for modal CalloutBoxes (Planter Limiter and Slider Modulation) directly into **The Klang Editor**, backed by automated GUI test coverage in `test/EditorTestSuite.h`.

This ensures all floating popups and callouts can be visually calibrated, themed, and verified without needing to launch host DAWs or manual click sessions.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev`.
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly across Phases 1 through 4.
4. **Validation**: Execute `gui_tests.exe` and confirm 100% assertions pass with 0 failures.
5. **Deployment**: Run `deploy.ps1` to update `current_build\Editor\The Klang Editor.exe`.
6. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
