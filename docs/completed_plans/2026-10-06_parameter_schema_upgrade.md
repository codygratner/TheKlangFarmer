# Architecture Plan: Comprehensive Parameter Data Schema Upgrade (v0.3.1 - Task 2)

**Goal:** Complete the parameter data-driven architecture by wiring `ControlDef` (`doubleClickValue`, `snapPoints`, `skew`) directly into `FarmerEditor` and `PlanterEditor` slider bindings, populating 100% of the parameter JSON schemas in `assets/controls/`, and adding an automated test suite verifying parameter schema compliance.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.1 Pro (Medium) or Gemini 3.8 Flash (High). Balanced reasoning for C++ UI wiring, thorough JSON data entry, and test assertions.

---

## Phase 1: C++ UI Slider Binding & Snap Support
- [ ] **Inspect & Verify `ParameterManager.cpp`**:
  - Confirm `double_click`, `snap_points`, `range` (`min`, `max`, `step`, `skew`), and `format` are parsed safely with fallbacks.
  - If `double_click` is absent in JSON, fall back to `defaultFloat`.
- [ ] **Wire `FarmerEditor::bindSlider` & `PlanterEditor::bindSlider`**:
  - In `FarmerEditor.cpp` (around line 2031) and `PlanterEditor.cpp` (around line 586):
  - Retrieve `const auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);`
  - If `def != nullptr`:
    - Configure slider double-click reset: `slider.setDoubleClickReturnValue(true, def->doubleClickValue);`
    - Configure default value getter: `slider.getDefaultValue = [v = def->doubleClickValue]() { return v; };`
    - If `!def->snapPoints.empty()`, pass snap points to `slider` (or configure tick marks/magnetism).
- [ ] **Verify Build**: Run quick compile to ensure zero compilation or linker errors.

---

## Phase 2: Schema Population — Filters, Envelopes & Modulators
- [ ] **`assets/controls/filters.json`**:
  - Audit all cutoff, resonance, and drive parameters (`filter1_cutoff`, `filter1_resonance`, `filter2_cutoff`, `filter2_resonance`, `filter3_cutoff`, `filter3_resonance`, `drive_filter`, etc.).
  - Ensure every float parameter defines:
    - `"range": { "min": ..., "max": ..., "step": ..., "skew": ... }`
    - `"default": ...`
    - `"double_click": ...`
    - `"snap_points": [ ... ]` (where applicable, e.g. center or key frequencies)
    - `"format": ...` (e.g. `"hertz"`, `"percent"`, `"bipolar"`)
- [ ] **`assets/controls/envelopes.json`**:
  - Audit all pitch, filter, amp, and mod envelope parameters (`slope`, `depth`, `decay`).
  - Add explicit `"range"`, `"default"`, `"double_click"`, and `"format"` to all entries.
- [ ] **`assets/controls/modulators.json`**:
  - Audit all LFO, FM depth, and pitch modulation controls.
  - Add explicit `"range"`, `"default"`, `"double_click"`, and `"format"` to all entries.

---

## Phase 3: Schema Population — Mixer, FX, Global & Planter
- [ ] **`assets/controls/mixer.json`**:
  - Audit channel levels, pans, and mutes/solos.
  - Ensure levels have appropriate skews (e.g. logarithmic amplitude curve) and double-click defaults (e.g. 0.0 dB / 0.8 norm).
  - Ensure pans have center snap point (`{ "value": 0.5, "label": "Center" }`) and double-click `0.5`.
- [ ] **`assets/controls/fx.json`**:
  - Audit all parameters for the 13 built-in FX algorithms.
  - Ensure mix knobs have default/double-click values and proper ranges.
- [ ] **`assets/controls/global.json` & `planter.json`**:
  - Audit master volume, tempo, tuning, and Planter sequence parameters.
  - Populate explicit ranges, defaults, and double-clicks.

---

## Phase 4: Automated Parameter Schema Compliance Test Suite
- [ ] **Add `ParameterSchemaAuditTest` in `test/gui_tests.cpp`**:
  - Instantiate `TheKlangFarmerAudioProcessor` and iterate through all registered parameters in its `apvts`.
  - For each parameter ID, look up `RlyehSound::ParameterManager::getInstance().getControlDef(paramId)`.
  - Assert that:
    1. `def != nullptr` (Every APVTS parameter MUST have a corresponding JSON definition).
    2. `def->min < def->max` (Valid non-inverted ranges).
    3. `def->defaultFloat >= def->min && def->defaultFloat <= def->max` (Default inside bounds).
    4. `def->doubleClickValue >= def->min && def->doubleClickValue <= def->max` (Double-click inside bounds).
    5. `def->skew > 0.0f` (Valid non-zero skew).
- [ ] **Execute `gui_tests`**:
  - Run the test suite via terminal or `build-validate`.
  - Verify 100% of test assertions pass.

---

## Phase 5: Verification, Archival & Handoff
- [ ] Run full build and test suite (`gui_tests` and `dsp_tests`).
- [ ] Archive `PLAN.md` to `docs/completed_plans/<date>_parameter_schema_upgrade.md`.
- [ ] Reset `PLAN.md` to `# No Active Plan`.
- [ ] Update `docs/communique/build_to_plan.md` to `Status: COMPLETE ✅`.
- [ ] Commit all changes cleanly to `0.3.1-dev`.
