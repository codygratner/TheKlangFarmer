# Implementation Plan: Optimize Planter GUI Rendering & Eliminate UI Thread Latency

## Goal
Resolve GUI sluggishness and input latency in **The Klang Planter** standalone and plugin by optimizing the header visualizer repaint cycle, making `PlanterHeaderVisualizer` opaque, reducing the timer frequency from 60 Hz to the 30 Hz standard, optimizing the oscilloscope path, and caching text lookups.

---

## Technical Analysis & Bottlenecks
1. **60 Hz Timer Frequency**: `startTimerHz(60)` in `PlanterEditor.cpp` runs at twice the rate of `FarmerEditor.cpp` (30 Hz), generating excessive repaint cycles on the message thread.
2. **Transparent Component Repaint Cascade**: `PlanterHeaderVisualizer` does not declare `setOpaque(true)`. Every time `headerViz.repaint()` is triggered, JUCE invalidates the parent editor background and top header bar, forcing expensive glyph width and JSON string lookups on every frame.
3. **128-Point Curved Spline Stroking**: `PlanterHeaderVisualizer::paint()` converts a 128-point path into cubic Bézier splines using `PathStrokeType::curved`, consuming high CPU time in JUCE software rendering.
4. **Per-Frame JSON String Lookups in `paint()`**: Calling `ParameterManager::getGlobalString("badge_limit", "LIMIT")` inside `paint()` introduces redundant map lookups.

---

## Architecture & Implementation Phases

### Phase 1: Header Visualizer Opacity & Path Optimization
Location: `source/PlanterEditor.h`, `source/PlanterEditor.cpp`
- In `PlanterHeaderVisualizer::PlanterHeaderVisualizer()`:
  - Call `setOpaque(true)`.
  - Cache `"LIMIT"` string: `limitText = RlyehSound::ParameterManager::getInstance().getGlobalString("badge_limit", "LIMIT");`.
- In `PlanterHeaderVisualizer::paint(juce::Graphics& g)`:
  - Ensure the entire bounds are filled with solid opaque background: `g.fillAll(juce::Colour(0xff151821));` (header bar chassis color).
  - Draw the visualizer card inside.
  - Use `limitText` cached string.
- In `PlanterHeaderVisualizer::updateData()`:
  - Reduce `targetPoints` from 128 to 64 (sufficient resolution for a 130px oscilloscope).
  - Use `juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)` on 64 points.

---

### Phase 2: Frame Rate Alignment (30 Hz) & Idle Throttling
Location: `source/PlanterEditor.cpp`
- In `TheKlangPlanterAudioProcessorEditor::TheKlangPlanterAudioProcessorEditor()`:
  - Change `startTimerHz(60);` to `startTimerHz(30);` (aligning with `FarmerEditor`).
- In `PlanterHeaderVisualizer::updateData()`:
  - Check if the visualizer is active: if `peakL < 0.001f && peakR < 0.001f && limiterActivity < 0.001f` and points are all near zero, check if points changed before calling `repaint()`. Avoid redundant paints when audio is completely silent.

---

### Phase 3: Parent Header String & Glyph Caching
Location: `source/PlanterEditor.h`, `source/PlanterEditor.cpp`
- In `TheKlangPlanterAudioProcessorEditor`:
  - Cache `titleText`, `subtitleText`, and `versionText` strings and their computed widths in member variables during initialization/resized.
  - In `paint(juce::Graphics& g)`, use the cached values instead of calculating `getStringWidthInt()` on every repaint.

---

### Phase 4: Validation & Benchmarking
- Build Release target: `cmake --build build --config Release --target TheKlangPlanter_Standalone TheKlangPlanter_VST3 gui_tests`.
- Run `gui_tests.exe` and confirm 100% assertions pass (215 / 215).
- Run `dsp_tests.exe` and confirm 100% pass.
- Execute `deploy.ps1` to update standalone and VST3 binaries.

---

## Acceptance Criteria
- [x] `PlanterHeaderVisualizer` is marked opaque (`setOpaque(true)`) and fills all bounds.
- [x] Timer frequency is 30 Hz (matching Farmer).
- [x] Oscilloscope resolution is 64 points with smooth line stroking.
- [x] No JSON map lookups or font width measurements in `paint()`.
- [x] All 215 GUI unit tests pass with zero failures.
- [x] Planter UI feels fluid and responsive with zero mouse drag lag.
