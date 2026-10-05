# Implementation Plan: The Klang Mill (Standalone VST)

## Goal Description
Create a third standalone VST3 plugin in the ecosystem named **The Klang Mill**. This plugin strips away all synthesizer, noise, and envelope generation, serving purely as a serial multi-effects rack for external audio. 

It reuses the exact DSP algorithms and UI components from `TheKlangFarmer` but runs them in a streamlined 1x6 layout.

## User Review Required
> [!IMPORTANT] 
> **Base Class Dependency**
> This plugin will be built *after* the "Core Architecture Base Class Refactor" (Priority #1) is complete. `TheKlangMill` will inherit directly from `KlangCoreProcessor` and `KlangCoreEditor`. This means it automatically gets the JSON preset browser, standard LookAndFeel, and preset serialization for free!

## Proposed Changes

### 1. CMake Integration
Add a new VST3 target in the root CMake file that links to the shared core and effects DSP, but omits the synth engine.
#### [MODIFY] `CMakeLists.txt`
```cmake
juce_add_plugin(TheKlangMill
    PLUGIN_MANUFACTURER_CODE "Tbd!"
    PLUGIN_CODE "Tkm1"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "The Klang Mill")

target_sources(TheKlangMill PRIVATE
    source/EffectsProcessor.cpp
    source/EffectsEditor.cpp
    # Shared Core sources
    source/KlangCoreProcessor.cpp 
    source/KlangCoreEditor.cpp
)
```

### 2. DSP Processor
#### [NEW] `source/EffectsProcessor.h` & `.cpp`
- Inherits from `KlangCoreProcessor`.
- Contains an APVTS with parameters for 4 FX slots and 1 Master Limiter.
- `processBlock(buffer, midiMessages)`: 
  1. Takes external DAW audio buffer.
  2. Processes through `FXSlot 1 -> 2 -> 3 -> 4`.
  3. Processes through the Master Limiter.
  4. Outputs buffer.
- **Init Override**: When the standard `init` preset is called, it explicitly forces the APVTS FX selector parameters to `0` (Empty) and `limiter_on` to `false`.

### 3. UI Layout
#### [NEW] `source/EffectsEditor.h` & `.cpp`
- Inherits from `KlangCoreEditor`.
- **Window Size**: 1 Row, 6 Columns. Fixed width of ~1200px (6 cards * 200px) and height of ~350px.
- **Layout Definitions**:
  - **Card 1**: Global FX Routing. Contains the 4 dropdown selectors to pick the algorithm for Slots 1-4.
  - **Card 2..5**: The active UI knobs for the 4 selected effects (dynamically rendered just like TKF).
  - **Card 6**: Master Out & Limiter. Contains a `juce::ToggleButton` mapped to `limiter_on` and sliders for Fold, Bias, and Filter.

## Verification Plan

### Automated Tests
1. `cmake --build build --target TheKlangMill_VST3` compiles cleanly.

### Manual Verification
1. Load `The Klang Mill.vst3` into a DAW on an audio track.
2. Verify the GUI draws a clean 1x6 grid of cards.
3. Pass audio (like a drum loop) into the plugin.
4. Select "Overdrive" on Slot 1 and increase Drive. Verify audio is distorted.
5. Click the "Init" button in the preset bar. Verify all 4 slots reset to "Empty", the audio passes through cleanly, and the Limiter toggle switches off.
