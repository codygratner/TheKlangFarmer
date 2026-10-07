# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.3.1 (Architecture & Tooling Hardening)  
**Task Name:** Schema Separation of Concerns & Cross-Reference "Where Used" Inspector  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** READY_FOR_EXECUTION  

---

## Strategic Objective
1. Enforce strict schema separation of concerns: migrate all visual styling, palettes (`theme.json`), and callout geometry (`callouts.json`) into `assets/themes/`, stripping all colors and dimensions out of `assets/controls/`.
2. Bind the Master Limiter parameters to `assets/themes/callouts.json` and add `Master Limiter` to `assets/layouts/tkp_layout.json` so all 9 Planter modules and 100% of APVTS parameters are discoverable in the tree.
3. Add a pre-indexed (`< 2ms`), bidirectional Cross-Reference ("Where Used & Associations") panel to the lower left sidebar of The Klang Editor, enabling double-click tree navigation between parameters, cards, and callouts.
4. Update `ParameterSchemaAuditTest` and `EditorTestSuite`, run the full regression gauntlet, deploy fresh artifacts, and retag `v0.3.1` locally.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev`.
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly across Phases 1 through 6:
   - Phase 1: Create `assets/themes/`, author `theme.json` and `callouts.json`, strip `ui_colors` from `assets/controls/`, delete `assets/controls/global_ui.json`.
   - Phase 2: Update `CMakeLists.txt` (`TkfAssets`) and `ParameterManager.cpp` to parse `assets/themes/*.json`.
   - Phase 3: Bind Master Limiter to `assets/themes/callouts.json` and dynamically mount under TKP tree as `[Callout] Master Limiter` with popup preview rendering (leaving `tkp_layout.json` strictly to the 8 surface cards).
   - Phase 4: Implement `buildReferencesIndex()` and `whereUsedListBox` in `MainComponent`, along with child parameter nodes under `Callouts & Overlays`.
   - Phase 5: Update `test/ParameterSchemaAuditTest.h` (strict schema leak check) and `test/EditorTestSuite.h` (Where-Used verification).
   - Phase 6: Compile Release, run tests, run `deploy.ps1`, and retag `v0.3.1` locally (`git tag -f -a "v0.3.1" -m "Release v0.3.1"`).
4. **Validation**: Confirm `build/Release/gui_tests.exe` and `build/Release/dsp_tests.exe` pass 100% with exit code 0.
5. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
