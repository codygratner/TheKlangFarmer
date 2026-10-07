# Data-Driven Parameter Architecture (Replacing Priorities #1 & #2)

## Goal Description
Shift `TheKlangFarmer` and `TheKlangPlanter` from hardcoded C++ parameter definitions to a modular, data-driven JSON architecture. 
This refactor merges the UI Tooltips JSON plan and the Quick-Snap Presets plan into a unified system. It will decouple parameter configuration (ID, name, ranges, default values, UI labels, and quick-snap points) from the APVTS initialization logic. 

**Key Benefits:**
1. **Modularity:** Parameters are defined in highly readable JSON files organized by module (e.g., `carrier.json`, `envelopes.json`, `fx_comb.json`).
2. **Dynamic UI Generation:** The right-click edit modal will automatically generate "Quick-Snap" preset pills by reading the `snap_points` array defined in the JSON.
3. **Painless Expansion:** Adding the 6 new effects (Priority #4) will simply require dropping a new JSON file into the folder.

## User Review Required
> [!WARNING] 
> **Backward Compatibility / APVTS IDs**
> The `id` keys in the JSON files *must* perfectly match the existing hardcoded APVTS string IDs (e.g., `"carrier1_pitch"`, `"mod1_type"`). If these IDs drift, all existing DAW sessions and saved user presets will break. The implementation will strictly port existing IDs.

> [!IMPORTANT]
> **Data Types**
> We will support two parameter types: `float` (continuous knobs/sliders) and `choice` (dropdowns/toggles).

## Open Questions
No immediate blockers. 

---

## Proposed Changes

### 0. Establish Architecture Guardrails (`GEMINI.md`) [x]
Append a strict rule to the AI guidelines file to permanently enforce the new architecture across all future chat sessions.
#### [MODIFY] `GEMINI.md`
- Add a new section `## Strict Data-Driven Architecture (CRITICAL)` with rules explicitly forbidding hardcoded APVTS parameters, default values, UI labels, and coordinates in C++.

### 1. JSON Asset Architecture & Schema [x]
Create a new directory `assets/controls/` to house individual module definitions.

#### [NEW] `assets/controls/carrier.json` (Example)
```json
{
  "carrier1_shape": {
    "type": "float",
    "name": "Carrier 1: Shape",
    "description": "Morph carrier waveform from sine through triangle and saw to pulse/square.",
    "is_bipolar": false,
    "range": { "min": 0.0, "max": 1.0, "step": 0.0005, "skew": 1.0 },
    "default": 0.0,
    "double_click": 0.0,
    "snap_points": [
      { "value": 0.0, "label": "Sine" },
      { "value": 0.25, "label": "Triangle" },
      { "value": 0.5, "label": "Sawtooth" },
      { "value": 0.75, "label": "Square" },
      { "value": 1.0, "label": "PWM" }
    ]
  },
  "carrier1_tracking": {
    "type": "choice",
    "name": "Carrier 1: Tracking",
    "description": "Dictates how Carrier 1 responds to incoming MIDI notes.",
    "choices": ["MIDI", "Freq", "Note"],
    "default": 0
  }
}
```

### 2. CMake Integration [x]
Bind the `assets/controls/` folder into the plugin binary to eliminate runtime disk I/O.

#### [MODIFY] `CMakeLists.txt`
```cmake
# Glob all JSON files in the controls directory
file(GLOB CONTROL_JSON_FILES "assets/controls/*.json")

juce_add_binary_data(TkfAssets
    HEADER_NAME "TkfAssets.h"
    NAMESPACE TkfAssets
    SOURCE_DIR assets
    ${CONTROL_JSON_FILES}
)

# Link TkfAssets to TheKlangFarmer, TheKlangPlanter, and Tests
```

### 3. C++ Runtime Parameter Manager [x]
Create a singleton or static manager to parse the binary JSON at startup.

#### [NEW] `source/ParameterManager.h` & `source/ParameterManager.cpp`
- **Responsibilities**: 
  - Loop over `TkfAssets::namedResourceList` during initialization.
  - Parse `juce::var` JSON blobs into a unified `std::unordered_map<juce::String, ControlDef>`.
  - Provide helper functions: `getTooltip(id)`, `getPointsOfInterest(id)`.

#### [MODIFY] `source/PluginProcessor.cpp` & `source/PlanterProcessor.cpp`
- Refactor `createParameterLayout()`:
  - Fetch the parsed dictionary from `ParameterManager`.
  - Iterate over the dictionary, pushing `std::make_unique<juce::AudioParameterFloat>` or `juce::AudioParameterChoice` into the APVTS `ParameterLayout` based on the JSON specs.

### 4. Dynamic UI Quick-Snaps & Tooltips [x]
Inject the parsed metadata directly into the frontend.

#### [MODIFY] `source/UIComponents.cpp` & `source/PluginEditor.cpp`
- [x] Remove hardcoded tooltips and replace them with `ParameterManager::getTooltip("carrier1_shape")`.
- [x] Refactor the right-click `SliderCalloutComponent` to automatically fetch `snap_points`. If they exist, dynamically instantiate and layout custom `PresetButton` pills above the slider. Clicking a pill fires `slider.setValue()`.

### 5. Piecemeal Execution Strategy
To isolate risk, this plan will be executed incrementally. The agent will pause for user compilation and manual testing after *every* phase before proceeding.
- **After Phase 2**: Compile check (Ensures CMake binary data works).
- **After Phase 3**: Plugin boot check (Ensures JSON parser doesn't crash on startup).
- **After Phase 4**: Audio & UI check (Ensures APVTS knobs still control audio and Quick-Snaps spawn).

---

## Verification Plan

### Automated Tests
1. **Compilation Check**: Run the `build-validate` skill between phases to ensure CMake successfully embeds the `TkfAssets` binary data.
2. **DSP Validation**: Run `pluginval` to verify that the APVTS tree is completely valid and parameters expose correct normalized ranges.

### Manual Verification
1. Open the plugin in the Standalone wrapper.
2. Verify all UI elements (knobs, sliders) accurately map to their parameters.
3. Right-click the **Carrier 1 Shape** slider and verify that the exact snap points ("Sine", "Triangle", "Sawtooth", "Square", "PWM") appear as clickable preset pills.
4. Hover over elements to ensure descriptions and tooltips have accurately propagated from the JSON files.
