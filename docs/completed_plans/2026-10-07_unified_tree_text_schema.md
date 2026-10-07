# Implementation Plan: Unified Filterable Master Tree, Dedicated Text Schema, & DSP Block File Renaming

**Active Milestone:** v0.3.2 "Agent Infrastructure & Editor Upgrades"  
**Branch:** `0.3.2-dev`  
**Execution Target:** Klang Industries (Factory Floor)  
**Status:** Completed & Deployed ✅

---

## 1. Architectural Blueprint & Requirements

### 1.1 DSP Block File Renaming (`assets/controls/*.json`)
- Rename the three legacy plural files in `assets/controls/`:
  - `modulators.json` &rarr; `modulator.json`
  - `filters.json` &rarr; `filter.json`
  - `envelopes.json` &rarr; `envelope.json`
- Ruthlessly delete old plural files.
- Retain singular files: `carrier.json`, `mixer.json`, `global.json`, `fx.json`, `planter.json`.
- CMake automatically globs `assets/controls/*.json`, so `TkfAssets` embeds the renamed files immediately.

### 1.2 Dedicated Text & Localization Schema (`assets/text/strings.json`)
- Create new directory `assets/text/` and file `assets/text/strings.json`.
- Add `file(GLOB TEXT_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/text/*.json")` to `CMakeLists.txt` and include in `TkfAssets`.
- Structure `strings.json` under clean i18n namespaces grouped by DSP module to prevent massive flat lists:
  - `"shared"`: Global UI labels, headers, and window strings.
  - `"farmer"`: Sub-grouped by module (`carrier`, `filter`, `envelope`, `modulator`, `mixer`, `global`, `fx`) containing parameter descriptions and choice tooltips.
  - `"planter"`: Sub-grouped by module containing parameter descriptions and choice tooltips.
- **Strict Data/UI Separation**: 
  - `assets/controls/*.json` retains `"name"` (as the immutable Host/DAW automation contract).
  - Strip 100% of `"description"` and `"choice_tooltips"` from `assets/controls/*.json`.
  - Extract `"global_strings"` out of `assets/themes/theme.json` into `strings.json` under `"shared"`.

### 1.3 Seamless Startup Merge (`source/ParameterManager.cpp`)
- Update `ParameterManager`:
  - Parse `strings.json` and store the text definitions.
  - In `ParameterManager()` constructor, after iterating all `TkfAssets` resources, seamlessly merge the `description` and `choiceTooltips` from `strings.json` into their respective `ControlDef` objects.
  - Guarantees 100% callsite compatibility across the C++ codebase.

### 1.4 The Klang Editor: Unified Tree & Simultaneous Property Editing (`tools/editor/`)
- Replace `NavTabbedComponent navigationTabs` and dual trees with:
  - A mini toolbar `filterBar` with exactly **3 toggle buttons**: `[Controls]`, `[Layout]`, `[Theme]`.
  - A single `juce::TreeView masterTree`.
- **Smart Minimum**: If toggling off a button leaves zero filters active, immediately re-check it so at least 1 layer is always visible.
- **Unified Editing UX**:
  - When the user selects a Parameter node under `[Controls]`, the Property Panel displays the DSP math fields (Min, Max, Step) **AND** the Text fields (Description, Choice Tooltips) in the same unified form.
  - When the user edits description text, it marks the Editor dirty.
  - **Smart Routing on Save**: When saving, the Editor writes the math properties to `assets/controls/*.json` and the text properties to `assets/text/strings.json`.
- **Where Used Navigation**:
  - Double-clicking an item in "Where Used" auto-enables the required layer filter (e.g., turns on `[Layout]`) if it's currently hidden, then selects the node.

---

## 2. Phased Implementation Verification

- [x] **Phase 1: DSP Block File Renaming & CMake Integration**
  - Git rename files in `assets/controls/` (`modulator.json`, `filter.json`, `envelope.json`).
  - Update `CMakeLists.txt` to embed `assets/text/*.json` into `TkfAssets`.
- [x] **Phase 2: Schema Extraction & `assets/text/strings.json`**
  - Extract `"description"` and `"choice_tooltips"` from `assets/controls/*.json`, group them by module name, and write to `assets/text/strings.json`.
  - Strip `"description"` and `"choice_tooltips"` from `assets/controls/*.json`.
  - Extract `"global_strings"` from `theme.json` to `strings.json`.
  - Extract legacy `ui_strings` from all control files into `"shared"` in `strings.json`.
- [x] **Phase 3: `ParameterManager` Startup Text Merge**
  - Add text definition mapping to `ParameterManager` and seamlessly merge it into `ControlDef` at startup.
- [x] **Phase 4: The Klang Editor Unified Tree & Simultaneous Editor**
  - Replace dual tabs with `masterTree` and 3-button `filterBar` (`[Controls]`, `[Layout]`, `[Theme]`).
  - Implement Smart Minimum toggle logic.
  - Update Property Form binding so Parameter nodes display both `ControlDef` DSP fields and Text fields.
  - Update `saveChanges()` to route text edits correctly to `strings.json` preserving module namespaces.
  - Fix tree node selection for headless unit testing (`currentSelectedItem`).
  - Safe null checks for `DynamicObject` in `buildTree()`, `onTreeItemSelected()`, and `syncJsonToPreview()`.
  - Add `stopTimer()` to `MainComponent::~MainComponent()` for timer hygiene.
- [x] **Phase 5: Test Suite Expansion**
  - `test/ParameterSchemaAuditTest.h`:
    - Assert singular files exist; plural files do not.
    - Assert strict separation: zero `"description"`, `"choice_tooltips"`, `"ui_strings"`, or visual styling inside `assets/controls/*.json`.
    - Assert `assets/text/strings.json` covers APVTS parameters and `ParameterManager` merges them into `ControlDef`.
  - `test/EditorTestSuite.h`:
    - Bounds assertions for `masterTree` and the 3 filter buttons.
    - Smart Minimum validation (cannot disable all 3).
    - Unified property form binding logic for DSP math and Text descriptions.
    - Stage 11 callout parameter and styling controls verification.
- [x] **Phase 6: Build, Validate & Deploy**
  - Real-time audio safety verified (zero allocations or locks in DSP loops).
  - 100% unit tests passing in `gui_tests` (275/275 assertions passed).
  - 100% unit tests passing in `dsp_tests` (all modular drum synth & Planter tests passed).
  - Release build compiled for `TheKlangFarmer_VST3`, `TheKlangPlanter_VST3`, and `TheKlangEditor`.
  - Artifacts deployed via `deploy.ps1` to `current_build/` and system VST3 directories.
