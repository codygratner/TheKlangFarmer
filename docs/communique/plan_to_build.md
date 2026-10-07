# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.3.1 (Tooling & Data Schema Hardening)  
**Task Name:** Top-Level Callout Parameter Controls in The Klang Editor  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** READY_FOR_EXECUTION  

---

## Strategic Objective
Ensure that clicking `[Callout] Master Limiter` (or any callout) at the top level in The Klang Editor's tree immediately displays all of its bound parameter controls in the right-hand property panel, exactly like Cards 1–8, without requiring the user to drill down into child parameter nodes.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev`.
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly:
   - Step 1: In `MainComponent.cpp` (`onTreeItemSelected`), populate `controlsObj` and `paramToFileMap` from `cardJson`'s `"parameters"` array when `currentProductId == "callouts"`.
   - Step 2: In `MainComponent.cpp` (`updateUIFromState`), change `if (!isCallout && showParams && parsed.isObject())` to `if (showParams && parsed.isObject())` so callout parameter properties populate the form editor.
   - Step 3: Add Stage 11 in `test/EditorTestSuite.h` verifying that top-level callout selection populates parameter controls in `formEditor`.
   - Step 4: Build Release targets (`TheKlangEditor`, `gui_tests`, `dsp_tests`), run tests (asserting 0 failures, exit code 0), run `deploy.ps1`, commit changes with `feat(editor): ...`, update `build_to_plan.md` to `COMPLETE`, and chime "JOB'S DONE!".
4. **Validation**: Confirm `build/Release/gui_tests.exe` and `build/Release/dsp_tests.exe` pass 100% with exit code 0.
5. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
