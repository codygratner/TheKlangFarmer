# Implementation Plan: Top-Level Callout Parameter Controls in The Klang Editor

## Goal
Ensure that clicking `[Callout] Master Limiter` (or any callout) at the top level in The Klang Editor's tree immediately displays all of its bound parameter controls in the right-hand property panel, exactly like regular surface cards (Cards 1–8), without requiring the user to drill down into individual child parameters.

---

## Technical Details & Changes

### 1. Document Loading in `MainComponent::onTreeItemSelected`
Location: `tools/editor/MainComponent.cpp` (~lines 1182–1195)
- When `currentProductId == "callouts"`:
  - After extracting `cardJson` from `calloutsObj->getProperty(currentCardId)`, check if `cardJson.hasProperty("parameters")`.
  - If `"parameters"` is an array, iterate through the parameter IDs and populate `controlsObj` and `paramToFileMap` from `assets/controls/*.json` (identical to how regular cards are populated at lines 1215–1236).
  - Set `originalControlsJson = juce::JSON::toString(juce::var(controlsObj.get()));`.

### 2. Form Editor Population in `MainComponent::updateUIFromState`
Location: `tools/editor/MainComponent.cpp` (~line 1854)
- Change:
  ```cpp
  // Before:
  if (!isCallout && showParams && parsed.isObject()) {
  
  // After:
  if (showParams && parsed.isObject()) {
  ```
- Because callouts now declare `"parameters": [ ... ]`, removing the `!isCallout` gate allows `formEditor` to append the parameter property rows directly beneath the "Callout Style" section when the top-level callout is selected in `showParams` mode (or show just the targeted parameter if a child node was selected).

### 3. Automated Test Verification
Location: `test/EditorTestSuite.h`
- Add Stage 11:
  - Simulate selecting `[Callout] Master Limiter` top-level item in the tree.
  - Assert that `formEditor` contains property components for `planter_limiter_gain`, `planter_limiter_thresh`, `planter_limiter_release`, and `planter_limiter_enable`.

### 4. Build, Regression Test & Deploy
1. Build targets in Release:
   ```powershell
   cmake --build build --config Release --target TheKlangEditor gui_tests dsp_tests
   ```
2. Verify all tests pass:
   - `build/Release/dsp_tests.exe` (100% pass)
   - `build/Release/gui_tests.exe` (100% pass, exit code 0)
3. Run `deploy.ps1`.
4. Stage and commit:
   ```powershell
   git add -A
   git commit -m "feat(editor): populate parameter controls for top-level callout selection in property panel"
   ```
5. Report completion in `docs/communique/build_to_plan.md` and chime "JOB'S DONE!".

---

## Acceptance Criteria
- [x] Selecting `[Callout] Master Limiter` top-level node in the tree immediately renders both Callout Style and the 4 limiter parameter controls in `formEditor`.
- [x] Editing parameter values in the callout form persists changes cleanly into `assets/controls/planter.json`.
- [x] Stage 11 in `EditorTestSuite` passes.
- [x] 100% of `gui_tests` and `dsp_tests` pass with exit code 0.
- [x] Fresh executables deployed via `deploy.ps1`.
