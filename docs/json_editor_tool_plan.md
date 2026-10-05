# Standalone JSON Data & Theme Editor

## Goal Description
Build a dedicated, standalone GUI application (`TheKlangEditor`) to rapidly author and preview the data-driven JSON architecture (`assets/controls/*.json` and `theme.json`). 

Instead of manually editing JSON text files and recompiling the synth to see visual changes, this tool will provide a 3-pane live-sync interface:
1. **Controls & Theme Form**: A property inspector with UI controls (color pickers, sliders, text boxes) for init values, double-click values, ranges, and layout assignments.
2. **Live Visual Preview**: A rendering canvas that instantiates the exact C++ UI components used by the synth, ensuring 1:1 visual parity.
3. **Raw JSON Editor**: A text editor pane with syntax highlighting for manual code adjustments.

Editing any of these three panes will instantly sync and reflect across the others.

## User Review Required
> [!IMPORTANT] 
> **App Framework Choice**
> By building this as a **JUCE Standalone GUI App** (added as a new target in `CMakeLists.txt`), we can `#include` the exact same `UIComponents.h` and `KlangCoreEditor.h` code from the main synth. This guarantees the preview pane looks *exactly* like the final VST3 plugin. Building it in a web framework (like React/Electron) would provide better text editing, but we would have to fake/re-create the JUCE UI in HTML/CSS, which defeats the purpose of an accurate preview.

## Proposed Changes

### 1. CMake Integration
Add a new executable target that links to the shared GUI code but explicitly excludes the heavy DSP (`PluginProcessor.cpp`).
#### [MODIFY] `CMakeLists.txt`
```cmake
# New target: TheKlangEditor (GUI App only)
juce_add_gui_app(TheKlangEditor
    PRODUCT_NAME "The Klang Editor"
    VERSION ${PROJECT_VERSION}
)
target_sources(TheKlangEditor PRIVATE
    tools/editor/Main.cpp
    tools/editor/MainComponent.cpp
    # Shared sources
    source/UIComponents.cpp
    source/ParameterManager.cpp
)
```

### 2. Standalone App Scaffolding
Create the standard JUCE GUI app entry point.
#### [NEW] `tools/editor/Main.cpp`
- Standard `juce::JUCEApplication` subclass.
- Creates a `juce::DocumentWindow` containing `MainComponent`.

### 3. The 3-Pane Interface
#### [NEW] `tools/editor/MainComponent.h` & `.cpp`
- **Layout**: Uses `juce::StretchableLayoutManager` for resizable vertical splitters.
- **Top Bar**: `juce::ComboBox` for selecting the active JSON module (e.g., `carrier.json`, `theme.json`).
- **Pane 1 (Form Editor)**: `juce::PropertyPanel`. Dynamically populates `juce::TextPropertyComponent`, `juce::SliderPropertyComponent`, and `juce::ColourPropertyComponent` based on the loaded JSON keys.
- **Pane 2 (Visual Preview)**: A wrapper `juce::Component`. Whenever the JSON updates, this destroys and re-instantiates the specific `SynthCardComponent` (e.g., the Carrier Card) using the new layout/theme data.
- **Pane 3 (Raw Code)**: `juce::CodeEditorComponent` attached to a `juce::CodeDocument`. 

### 4. Live Synchronization Engine
A central state manager to handle bidirectional updates without infinite loops.
- `juce::var activeJsonState;`
- **Form to JSON**: User picks a color -> `activeJsonState` is updated -> `CodeDocument` text is regenerated -> Preview is `repaint()`'d.
- **Text to Form**: User types in `CodeEditorComponent` -> triggers a delayed async parse (e.g., 500ms after typing stops) -> If JSON is valid, updates `activeJsonState` -> Updates Form values -> Preview is re-rendered.

---

## Verification Plan

### Automated Tests
1. **Compilation Check**: Run `/build-validate TheKlangEditor` to ensure the tool builds cleanly on macOS and Windows.

### Manual Verification
1. Launch `TheKlangEditor.exe`.
2. Select `carrier.json` from the dropdown.
3. Change the `"default"` init value of `carrier1_shape` in the text box pane.
4. Verify the visual slider in the Preview pane instantly snaps to the new default value.
5. Change a hex code in the `theme.json` Color Picker and verify the Preview background instantly changes.
6. Type a manual change in the Raw JSON pane and ensure the Form text boxes update automatically.
