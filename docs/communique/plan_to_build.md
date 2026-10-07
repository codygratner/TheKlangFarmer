# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.3.2 "Agent Infrastructure & Editor Upgrades"  
**Task Name:** Unified Filterable Master Tree, Dedicated Text Schema, & DSP Block File Renaming  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** COMPLETED ✅  
**Source Plan:** [`docs/completed_plans/2026-10-07_unified_tree_text_schema.md`](file:///c:/Dev/TheKlangSuite/docs/completed_plans/2026-10-07_unified_tree_text_schema.md)  

---

## Strategic Objective
Execute the full refactor of The Klang Editor's navigation tree into a unified Master Tree with a 3-button multi-state filter bar (`[Controls]`, `[Layout]`, `[Theme]`), establish a dedicated centralized text schema in `assets/text/strings.json` grouped cleanly by module namespace, achieve pure separation of concerns by extracting text from `assets/controls/*.json`, rename DSP block JSON files to consistent singular forms (`modulator.json`, `filter.json`, `envelope.json`), and unify the property panel UX so text and DSP limits are edited seamlessly in one place.

---

## Acceptance Criteria & Execution Guardrails
1. **DSP Block Renaming (`assets/controls/`)**:
   - `modulators.json` &rarr; `modulator.json`
   - `filters.json` &rarr; `filter.json`
   - `envelopes.json` &rarr; `envelope.json`
   - Zero legacy plural files remaining.
2. **Dedicated Text Schema (`assets/text/strings.json`)**:
   - `assets/text/strings.json` created with `"shared"`, `"farmer"`, and `"planter"` namespaces.
   - Inside `"farmer"`, parameters MUST be grouped by their module (e.g., `"carrier"`, `"filter"`) to avoid massive flat lists.
   - 100% of parameter `"description"` and `"choice_tooltips"` extracted out of `assets/controls/*.json`.
   - `"name"` remains in `assets/controls/` as the immutable DAW contract.
3. **Seamless Startup Text Merge (`source/ParameterManager.cpp`)**:
   - `ParameterManager` merges text definitions into `ControlDef` at startup so existing `def->description` callers experience zero regressions.
4. **The Klang Editor Unified Master Tree & Editor UX (`tools/editor/`)**:
   - Dual tabs replaced by single `masterTree` and a 3-button filter bar (`[Controls]`, `[Layout]`, `[Theme]`).
   - Smart Minimum enforced (cannot toggle off all 3 buttons).
   - Unified Editing: Selecting a parameter node under `[Controls]` allows editing DSP and Text fields simultaneously in the property panel.
   - Save routes text properties safely to `strings.json`.
5. **Test Suite & Verification**:
   - `ParameterSchemaAuditTest` and `EditorTestSuite` updated and passing 100%.
   - Full build validation passes (`dsp_tests.exe` and `gui_tests.exe` - 275/275 pass).
   - `deploy.ps1` executed to sync `.vst3` and standalone binaries.
