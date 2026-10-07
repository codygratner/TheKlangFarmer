# Architecture Plan: Planter Header Interactions & Two-Line Status Bar (v0.3.1 - Tasks 4 & 5) — ✅ COMPLETED

**Goal:** Implement interactive Planter header controls (VU meter panic flush with visual flash, and right-click Limiter CalloutBox) and build a unified, two-line 36px bottom `StatusBarComponent` across both The Klang Farmer and The Klang Planter providing permanent value readouts, mouse shortcut badges, and a full-width tooltip feed.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High). Rapid execution speed and extensive internal reasoning for JUCE component layout and unit tests.

---

## Phase 1: Planter Engine Panic & Header Interactions (Task 4)
- [x] **Engine Panic Support (`source/PlanterEngine.h` & `source/PlanterProcessor.cpp`)**:
  - Add `PlanterDrumEngine::panic()`:
    - Real-time safe: Zero heap allocation, zero blocking locks.
    - Reset envelope trigger states: set `ampEnv`, `filterEnv`, `pitchEnv`, and `noise` timings to finished state (`timeSinceTrigger = 1000.0f`).
    - Reset `djFilter` state and clear master scope and peak meters (`peakL = 0.0f`, `peakR = 0.0f`).
  - Expose `panic()` on `TheKlangPlanterAudioProcessor`.
- [x] **Planter Visualizer Targets & Flash Animation (`source/PlanterEditor.cpp`)**:
  - In `PlanterHeaderVisualizer`:
    - Split bounds into `scopeArea` (Left), `limitArea` (Center, 38px), and `meterArea` (Right).
    - Handle `mouseDown(const juce::MouseEvent& e)`:
      - If click is in `meterArea`: invoke `audioProcessor.panic()`, set a 150ms visual flash state on the meter bars, and trigger immediate repaint.
      - If click is in `limitArea` and is right-click (`e.mods.isPopupMenu()`): launch `PlanterLimiterCalloutComponent`.
- [x] **Limiter Mini-Card CalloutBox (`source/PlanterEditor.h` & `source/PlanterEditor.cpp`)**:
  - Create `PlanterLimiterCalloutComponent : public juce::Component`:
    - Styled as a sleek mini Doepfer card with dark chassis and red accent (`0xffe53935`).
    - Houses an Enable toggle (`planter_limiter_enable`) and 3 mini `RotaryKnobSlider` controls:
      - `planter_limiter_gain` ("Gain")
      - `planter_limiter_thresh` ("Ceiling")
      - `planter_limiter_release` ("Release")
    - Attach controls via `SliderAttachment` and `ButtonAttachment` (or `ComboBoxAttachment`).
  - Anchor and launch via `juce::CalloutBox::launchAsynchronously` pointing to `limitArea`.

---

## Phase 2: Reusable Two-Line `StatusBarComponent` (Task 5)
- [x] **Define `StatusBarComponent` in `source/UIComponents.h` / `source/UIComponents.cpp`**:
  - Height: Fixed ~34–36px footer component.
  - **Line 1 (Top Bar - Permanent)**:
    - Left side: Bolder, high-contrast label displaying control name and live formatted value (e.g. `Carrier 1: Pitch  +12.0 st [440 Hz]`).
    - Right side: Subtle pill badges displaying contextual mouse shortcuts (e.g. `🖱️ Right-Click: Snap Points` | `2x-Click: Default (0.5)`).
    - Line 1 remains visible and active even when tooltips are disabled!
  - **Line 2 (Bottom Bar - Tooltip Feed)**:
    - Full-width label displaying parameter description and functional explanation.
    - If `tooltipsEnabled == false`, Line 2 displays subtle engine status or goes quiet.
  - **API**:
    - `setHoveredControl(const juce::String& name, const juce::String& value, const juce::String& desc, const juce::String& rightClickHint, const juce::String& doubleClickHint)`
    - `clearHoveredControl()`
    - `setTooltipsEnabled(bool enabled)`

---

## Phase 3: Integration into The Klang Farmer & The Klang Planter
- [x] **Farmer Integration (`source/FarmerEditor.h` & `source/FarmerEditor.cpp`)**:
  - Instantiate `StatusBarComponent statusBar;` as a child component.
  - In `resized()`: Reserve 36px from bottom (`auto statusArea = bounds.removeFromBottom(36); statusBar.setBounds(statusArea);`).
  - Wire all sliders' `mouseEnter` and `mouseExit` callbacks to update `statusBar`.
  - Wire `tooltipsButton` toggle to update `statusBar.setTooltipsEnabled()`.
- [x] **Planter Integration (`source/PlanterEditor.h` & `source/PlanterEditor.cpp`)**:
  - Instantiate `StatusBarComponent statusBar;`.
  - In `resized()`: Reserve 36px from bottom.
  - Wire all sliders, selectors, and `tooltipsButton` toggle.

---

## Phase 4: Automated GUI Test Verification (`test/gui_tests.cpp`)
- [x] **Test Planter Header Interactions**:
  - Simulate click on `headerViz` meter area $\to$ verify `panic()` flushes engine without exceptions.
  - Simulate right-click on `limitArea` $\to$ verify `CalloutBox` launches.
- [x] **Test 100% of Limiter Callout Controls (`PlanterLimiterCalloutComponent`)**:
  - Instantiate `PlanterLimiterCalloutComponent` and verify all 4 controls bind to APVTS:
    1. `planter_limiter_enable`: Toggle between Off and On $\to$ assert APVTS parameter synchronizes.
    2. `planter_limiter_gain`: Set value $\to$ assert `planter_limiter_gain` APVTS updates.
    3. `planter_limiter_thresh`: Set value $\to$ assert `planter_limiter_thresh` APVTS updates.
    4. `planter_limiter_release`: Set value $\to$ assert `planter_limiter_release` APVTS updates.
  - Assert that all 3 knobs display valid non-empty tooltips.
  - Verify clean destruction of callout and attachments with zero memory or timer leaks.
- [x] **Test Status Bar Integrity**:
  - Simulate hovering a slider in Farmer and Planter $\to$ assert `statusBar` receives control name and shortcut badges.
  - Verify toggling `tooltipsButton` toggles Line 2 visibility.

---

## Phase 5: Verification, Archival & Handoff
- [x] Run full build and test suite (`gui_tests` + `dsp_tests`).
- [x] Verify 100% of assertions pass with zero regressions (164/164 gui_tests, all dsp_tests).
- [x] Archive `PLAN.md` to `docs/completed_plans/<date>_planter_header_and_status_bar.md`.
- [x] Reset `PLAN.md` to `# No Active Plan`.
- [x] Update `docs/communique/build_to_plan.md` to `Status: COMPLETE ✅`.
- [x] Commit all changes cleanly to `0.3.1-dev`.
