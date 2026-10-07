# Implementation Plan: Schema Separation of Concerns & Cross-Reference "Where Used" Inspector

## Goal
Enforce a clean architectural separation between parameter data and visual styling, make all 9 Planter modules (including the Master Limiter) discoverable in The Klang Planter tree, and introduce a pre-indexed, instantaneous Cross-Reference ("Where Used") navigation panel in The Klang Editor.

---

## Technical Analysis & Requirements
1. **Schema Separation Leak**:
   - `assets/controls/` currently contains visual styling (`ui_colors` in 6 files, and `global_ui.json` defining `callout_styles` with pixel dimensions and colors).
   - Solution: Create `assets/themes/`. Move visual palettes into `theme.json` and callout styling into `callouts.json`. Leave `assets/controls/` 100% pure DSP parameter contracts.
2. **Missing Limiter in Planter Tree**:
   - `assets/layouts/tkp_layout.json` only contains the 8 surface cards. The Limiter parameters (`planter_limiter_enable`, `planter_limiter_gain`, `planter_limiter_thresh`, `planter_limiter_release`) are hidden from TKP's tree.
   - Solution: Add `Master Limiter` to `tkp_layout.json` with its 4 parameters.
3. **Callout Parameter Binding**:
   - In `assets/themes/callouts.json`, each callout explicitly declares its `"parameters"` array (e.g. `planter_limiter` lists the 4 limiter parameter IDs).
   - The Editor's `Callouts & Overlays` tree populates these as child parameter nodes, allowing direct inspection and two-way JSON editing.
4. **Cross-Reference ("Where Used") Panel**:
   - Pre-computes a bidirectional index (`< 2ms`) at startup mapping parameters <-> cards <-> callouts <-> products.
   - Adds a lower list panel beneath the Tree View in The Klang Editor displaying contextual associations for the selected item.
   - Double-clicking any item in the list automatically expands, selects, and navigates to that node in the tree and loads it into the property editor.

---

## Architecture & Implementation Phases

### Phase 1: Directory Setup & Strict Schema Migration
Location: `assets/themes/`, `assets/controls/`
1. Create directory `assets/themes/`.
2. Author `assets/themes/theme.json`:
   - Consolidate `"global_strings"`, `"global_colors"`, and `"module_colors"` (migrating `"ui_colors"` from `carrier.json`, `envelopes.json`, `filters.json`, `global.json`, `mixer.json`, `modulators.json`).
3. Author `assets/themes/callouts.json`:
   - Move `callout_styles` from `global_ui.json`.
   - Add `"parameters"` array to `"planter_limiter"`:
     ```json
     "planter_limiter": {
       "title": "Master Limiter",
       "product": "tkp",
       "width": 300,
       "height": 130,
       "background_colour": "0xff1e222b",
       "border_colour": "0xffe53935",
       "corner_radius": 6.0,
       "parameters": [
         "planter_limiter_enable",
         "planter_limiter_gain",
         "planter_limiter_thresh",
         "planter_limiter_release"
       ]
     }
     ```
   - Add `"parameters": []` to `"slider_modulation"`.
4. Strip `"ui_colors"` from `assets/controls/carrier.json`, `envelopes.json`, `filters.json`, `global.json`, `mixer.json`, `modulators.json`.
5. Remove `assets/controls/global_ui.json`.

---

### Phase 2: Build System & Asset Parsing Updates
Location: `CMakeLists.txt`, `source/ParameterManager.h`, `source/ParameterManager.cpp`
1. In `CMakeLists.txt`:
   - Update `TkfAssets` binary data sources to include `assets/themes/*.json` along with `assets/controls/*.json`.
2. In `source/ParameterManager.cpp`:
   - Parse `theme.json` for `"module_colors"` and `"global_colors"`.
   - Maintain backward-compatible getters (`getModuleColor`, `getGlobalColor`, `getGlobalString`).

---

### Phase 3: TKP Layout Card Binding
Location: `assets/layouts/tkp_layout.json`
- Add `Master Limiter` card:
  ```json
  "Master Limiter": {
    "color": "0xFFE53935",
    "style": "StandardDark",
    "is_callout": true,
    "tooltip": "LIMITER: Master brickwall peak limiter with auto soft-knee release and gain boosting.",
    "parameters": [
      "planter_limiter_enable",
      "planter_limiter_gain",
      "planter_limiter_thresh",
      "planter_limiter_release"
    ]
  }
  ```

---

### Phase 4: The Klang Editor "Where Used" Panel & Callout Child Parameters
Location: `tools/editor/MainComponent.h`, `tools/editor/MainComponent.cpp`
1. **Pre-Computed References Index**:
   - In `MainComponent`, create `struct ReferenceItem { juce::String label; juce::String targetType; juce::String targetId; juce::String extra; };`.
   - Map: `std::map<juce::String, juce::Array<ReferenceItem>> referencesMap;`.
   - Method: `void buildReferencesIndex();` executed once at startup (`< 2ms`).
2. **Where-Used Lower Sidebar Panel**:
   - Add `juce::ListBox whereUsedListBox` and custom `ListBoxModel` beneath the Tree View in the left sidebar.
   - Header label: `"WHERE USED & ASSOCIATIONS"`.
   - When a tree item is selected, lookup `referencesMap` and update the list.
   - Handle double-click: programmatic tree search, expand ancestors, select node, and trigger `onTreeItemSelected()`.
3. **Callout Child Parameter Nodes in Tree**:
   - In `buildTree()`, parse `callouts.json`. For each callout, add child nodes for its `"parameters"` array (`1: planter_limiter_enable`, etc.).
   - Clicking a child parameter opens it in the parameter form editor for editing.

---

### Phase 5: Automated Test Coverage & Schema Audit
Location: `test/ParameterSchemaAuditTest.h`, `test/EditorTestSuite.h`
1. In `test/ParameterSchemaAuditTest.h`:
   - Audit `assets/controls/*.json`: assert 0 files contain `"ui_colors"`, `"width"`, `"height"`, or `"callout_styles"`.
   - Audit `assets/themes/theme.json` and `callouts.json`: assert valid JSON and required keys.
2. In `test/EditorTestSuite.h`:
   - Stage 8: Verify `Master Limiter` exists under `The Klang Planter` in the tree.
   - Stage 9: Verify `Planter Master Limiter` callout has 4 child parameter tree items.
   - Stage 10: Verify `whereUsedListBox` populates references on parameter/callout selection and double-click navigates to the target node.

---

### Phase 6: Validation, Deployment & Retagging
1. Compile Debug & Release:
   ```powershell
   cmake --build build --config Debug --target dsp_tests gui_tests
   cmake --build build --config Release --target TheKlangFarmer_Standalone TheKlangPlanter_Standalone TheKlangFarmer_VST3 TheKlangPlanter_VST3 TheKlangEditor dsp_tests gui_tests
   ```
2. Run test suites:
   - `build/Release/dsp_tests.exe` (100% pass).
   - `build/Release/gui_tests.exe` (100% pass, exit code 0).
3. Execute `deploy.ps1`.
4. Update local tag:
   ```powershell
   git add -A
   git commit -m "feat(editor): schema separation of concerns, callout parameter bindings, and Where-Used inspector"
   git tag -f -a "v0.3.1" -m "Release v0.3.1"
   ```

---

## Acceptance Criteria
- [ ] `assets/controls/*.json` contains strictly parameter definitions (0 visual colors, 0 dimensions).
- [ ] `assets/themes/` houses `theme.json` and `callouts.json`.
- [ ] `Master Limiter` appears under The Klang Planter in the tree with its 4 APVTS parameters.
- [ ] `Callouts & Overlays -> Planter Master Limiter` displays its 4 child parameters in the tree.
- [ ] The lower left sidebar contains the 'Where Used & Associations' panel.
- [ ] Double-clicking a reference item navigates directly to that node in the tree.
- [ ] All automated tests pass with 0 failures and exit code 0.
- [ ] Local tag `v0.3.1` updated and verified.
