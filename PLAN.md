# Architecture Plan: Extract Hardcoded C++ Parameter Metadata into JSON (v0.3.1 - Task 2)

**Goal:** Eliminate all hardcoded C++ parameter descriptions and settings in `FarmerEditor.cpp` by migrating them into the data-driven `assets/controls/*.json` files, bringing `FarmerEditor` up to parity with `PlanterEditor`'s `ControlDef` binding pattern, while maintaining 100% backward behavioral and preset parity. (Interactive parameter curve/snap re-tuning is strictly deferred to v0.4.0).

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.1 Pro (Medium) or Gemini 3.8 Flash (High). Clean C++ refactoring and accurate JSON data migration.

---

## Phase 1: Audit & Migrate Hardcoded Descriptions into JSON
- [ ] **Extract `getFarmerParamDescription()` strings**:
  - Read lines 1912–2030 in `source/FarmerEditor.cpp`.
  - For every parameter ID checked in `getFarmerParamDescription()`:
    - Locate the matching entry in `assets/controls/*.json` (e.g. `carrier.json`, `modulators.json`, `envelopes.json`, `filters.json`, `mixer.json`, `fx.json`).
    - Set `"description"` to the exact string currently hardcoded in C++.
    - Set `"is_bipolar": true` if `isBipolar = true` was set in C++ (otherwise `false`).
- [ ] **Preserve Existing Values**:
  - Do NOT invent or alter existing numerical ranges, defaults, or slopes. Keep exact parity with the current codebase.

## Phase 2: Modernize `FarmerEditor::bindSlider`
- [ ] **Align with `PlanterEditor` Pattern**:
  - In `source/FarmerEditor.cpp` (around line 2031):
    - Replace the hardcoded `getFarmerParamDescription` call with `ParameterManager::getInstance().getControlDef(paramId)`.
    - If `def != nullptr`:
      - Set `slider.setDoubleClickReturnValue(true, def->doubleClickValue);`
      - Set `slider.getDefaultValue = [v = def->doubleClickValue]() { return v; };`
      - Populate `slider.snapValues` from `def->snapPoints` if present.
      - Use `def->description` and `def->isBipolar` when configuring `TooltipHelper::makeKnobTooltipFromParam`.
- [ ] **Delete Legacy C++ Boilerplate**:
  - Safely remove the monolithic 120-line `getFarmerParamDescription` static helper from `source/FarmerEditor.cpp`.

## Phase 3: Parity & Regression Verification
- [ ] **Compile & Test**:
  - Build the project and run `gui_tests` and `dsp_tests`.
  - Verify that 100% of tooltips, bipolar slider indicators, and parameters continue to display and operate identically.
  - Verify zero compiler warnings or broken bindings.

## Phase 4: Archival & Handoff
- [ ] Archive `PLAN.md` to `docs/completed_plans/<date>_parameter_metadata_extraction.md`.
- [ ] Reset `PLAN.md` to `# No Active Plan`.
- [ ] Update `docs/communique/build_to_plan.md` to `Status: COMPLETE ✅`.
- [ ] Commit all changes cleanly to `0.3.1-dev`.
