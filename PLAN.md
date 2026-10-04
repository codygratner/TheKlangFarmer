# Implementation Plan: Control Tooltips on Hover (THE-9)

**Branch:** `feature/THE-9-control-tooltips`  
**Linear Issue:** [THE-9](https://linear.app/the-klang-farmer/issue/THE-9/control-tooltips-on-hover-the-klang-farmer-and-the-klang-planter)

---

## Overview
Implement a unified, hardware-styled hover tooltip system across **The Klang Farmer** and **The Klang Planter** using JUCE's tooltip architecture (`juce::TooltipWindow` + custom `LookAndFeel` rendering + `juce::SettableTooltipClient` extensions). All tooltip assignments and string formatting execute strictly on the UI message thread with zero audio-thread allocations or lock contention.

---

## Phases

### Phase 1: Tooltip LookAndFeel & Custom Component Infrastructure — [x] COMPLETED
**Files:** `source/UIComponents.h`, `source/UIComponents.cpp`

- [x] **Step 1.1: Custom Industrial Tooltip Rendering in `RotaryKnobLookAndFeel`**
  - Override `getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)`:
    - Layout text using `juce::AttributedString` and `juce::TextLayout` with a bold `12.5f` font and a maximum wrap width of `280px`.
    - Add `20px` horizontal (`10px` per side) and `14px` vertical (`7px` per side) padding.
    - Clamp the returned rectangle inside `parentArea.reduced(6)` so tooltips on edge modules never clip outside the plugin window.
  - Override `drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)`:
    - Fill rounded background (`4.0f` radius) with deep slate `#101722` (`juce::Colour(0xff101722)`).
    - Draw a subtle outer/inner cyan glow (`juce::Colour(0xff00d4ff).withAlpha(0.18f)`, `2.5f` stroke) and a crisp `1.0f` border (`juce::Colour(0xff00d4ff).withAlpha(0.75f)`).
    - Render high-contrast crisp text (`juce::Colour(0xfff0f6fc)`) at `12.5f` bold using `juce::TextLayout`.
- [x] **Step 1.2: Extend `LedSelectorComponent` for Per-Item & Component Tooltips**
  - Inherit from `juce::SettableTooltipClient`.
  - Add per-item tooltip storage and API (`setItemTooltips`, `setItemTooltip`, `getTooltip() override`).
  - Track hovered cell index in `mouseMove` and clear in `mouseExit`. Return cell-specific tooltip if set, falling back to base `SettableTooltipClient::getTooltip()`.
- [x] **Step 1.3: Extend Custom Cards & Visualizers for Tooltip Support**
  - Check custom components handling direct mouse interactions (`VisualizationCardComponent`, `PlanterHeaderVisualizer`, `NavigationCardComponent`).
  - Inherit from `juce::SettableTooltipClient` or attach child tooltips.
  - Track hovered regions / buttons and return context-specific tooltips in `getTooltip()`.

---

### Phase 2: Centralized Tooltip Builder & Parameter Metadata Helper — [x] COMPLETED
**Files:** `source/UIComponents.h`, `source/UIComponents.cpp`

- [x] **Step 2.1: Tooltip Formatting Helper (`TooltipHelper`)**
  - Create `TooltipHelper` namespace in `UIComponents.h` / `UIComponents.cpp`.
  - Implement `makeKnobTooltip(title, description, defaultAndUnits, isBipolar)`.
  - Implement `makeKnobTooltipFromParam(apvts, paramId, description, isBipolar)` querying parameter name, default text, units, and bipolar flag.
  - Format as:
    ```text
    <Parameter Name>: <Musical/DSP Description>
    Default: <DefaultValue> <Units> | Double-click to reset
    ```
- [x] **Step 2.2: Standard LED Selector & Enum Tooltip Dictionaries**
  - Populate `TooltipHelper::getLedSelectorItemTooltips` for all shared hardware selector archetypes:
    - Routing Destinations (`Car`, `Mod`, `Both`, `Opp`)
    - Envelope / Filter Slopes (`6`, `12`, `18`, `24`, `30`, `36`)
    - Filter Types (`LPF`, `BPF`, `HPF`, `BRF`, `APF`)
    - Carrier Tracking Modes (`MIDI`, `Freq`, `Note`)
  - Populate `TooltipHelper::getFxAlgorithmTooltip(int fxIndex)` describing Bypass and all 13 modular FX blocks.

---

### Phase 3: The Klang Planter Tooltip Integration — [x] COMPLETED
**Files:** `source/PlanterEditor.h`, `source/PlanterEditor.cpp`

- [x] **Step 3.1: Instantiate `juce::TooltipWindow` in Planter**
  - Add `juce::TooltipWindow tooltipWindow { this, 300 };` to `TheKlangPlanterAudioProcessorEditor`.
  - Set look and feel to `knobLookAndFeel`, non-opaque, no mouse intercept.
  - Clean up look and feel in `~TheKlangPlanterAudioProcessorEditor()`.
- [x] **Step 3.2: Wire All Planter Controls**
  - Header controls (`initButton`, `triggerButton`, `headerViz`).
  - 8 Module Cards (Carrier, Modulator, Pitch Env, Transients/Noise, Filter, Filter Env, Amp, Amp Env).

---

### Phase 4: The Klang Farmer Tooltip Integration — [x] COMPLETED
**Files:** `source/PluginEditor.h`, `source/PluginEditor.cpp`, `source/UIComponents.cpp`

- [x] **Step 4.1: Instantiate `juce::TooltipWindow` in Farmer**
  - Add `juce::TooltipWindow tooltipWindow { this, 300 };` to `TheKlangFarmerAudioProcessorEditor`.
  - Set look and feel to `knobLookAndFeel`.
  - Clean up look and feel in `~TheKlangFarmerAudioProcessorEditor()`.
- [x] **Step 4.2: Wire All Farmer Controls Across All 7 Pages & Fixed Slots**
  - Header buttons (`guideButton`, `initButton`, `triggerButton`).
  - Slot 1 (`NavigationCardComponent` page tab tooltips).
  - Slot 8 (`VisualizationCardComponent` display and lock toggle).
  - Pages 1 & 2 (`Voice 1` & `Voice 2` knobs and selectors).
  - Page 3 (`Transients` noise, filter 3, filter env 3).
  - Pages 4 & 6 (`Pre-Amp FX` & `Post-Amp FX` slot cards with dynamic ComboBox and parameter descriptions).
  - Page 5 (`Amplifier` master controls, velocity, clap generator).
  - Page 7 (`Modulations` mod matrix, analog slop, LFO controls).

---

### Phase 5: UX Hygiene, Interaction Guards & Verification — [x] COMPLETED
**Files:** `source/UIComponents.cpp`, `source/PluginEditor.cpp`, `source/PlanterEditor.cpp`

- [x] **Step 5.1: Mouse Drag & Modal Suppression**
  - Suppress/dismiss tooltips during knob drag (`RotaryKnobSlider::mouseDown` and `mouseDrag`).
  - Ensure right-click popup / text-entry and combo menus are unobstructed.
- [x] **Step 5.2: Build & Verification Checklist**
  - Compile both targets (`TheKlangFarmer`, `TheKlangPlanter`) and test suite (`dsp_tests`).
  - Functional verification of 300ms hover delay, styling, dynamic FX updates, right-click modals.
  - Real-time audio-thread safety audit (`audiothread-guard`).
