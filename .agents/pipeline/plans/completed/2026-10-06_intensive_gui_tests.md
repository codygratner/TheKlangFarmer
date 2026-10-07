# Architecture Plan: Intensive GUI Test Suite for Plugins & Standalone (v0.3.1 - Task 3)

**Goal:** Expand the automated GUI test harness (`test/gui_tests.cpp`) to provide deep, intensive test coverage for both The Klang Farmer and The Klang Planter (in plugin and standalone contexts), modeled after The Klang Editor's headless verification architecture, complete with offscreen paint smoke passes, modal lifecycle checks, and coordinate-based fallback automation.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.1 Pro (Medium) or Gemini 3.8 Flash (High). Balanced reasoning for JUCE component hierarchy traversal, synthetic event dispatching, and offscreen paint validation.

---

## Phase 1: Headless Component & APVTS Binding Deep Sweep
- [x] **Target File**: `test/gui_tests.cpp`
- [x] **Implement `PluginComponentBindingTest`**:
  - Instantiate `TheKlangFarmerAudioProcessor` and create its editor (`createEditorIfNeeded()`).
  - Instantiate `TheKlangPlanterAudioProcessor` and create its editor.
  - Recursively sweep all child components (`juce::Component::getChildren()`):
    - Identify all `RotaryKnobSlider`s, `LedSelectorComponent`s, and `juce::ComboBox`es.
    - Check APVTS parameter linkage: verify that every interactive slider is attached or mapped to a valid APVTS parameter ID.
    - Assert that 100% of sliders have a non-empty, meaningful tooltip (zero empty tooltips).
    - Assert that `slider.getRange().getStart() < slider.getRange().getEnd()`.
- [x] **Verify Compile**: Run quick compile to verify syntax and test harness linkages.

---

## Phase 2: Page Navigation & Paint Smoke Passes
- [x] **Implement `PageNavigationAndPaintSmokeTest` in `test/gui_tests.cpp`**:
  - **For The Klang Farmer**:
    - Programmatically trigger page switches across all primary tabs (Synth / FX / Modulators / Global).
    - On each page transition:
      - Resize editor across standard (1000x700), minimum (800x600), and 4K scaled dimensions.
      - Render into an offscreen `juce::Image` buffer and execute `editor->paintEntireComponent(g, true)`.
      - Assert zero crashes, zero null-pointer dereferences, and zero division-by-zero errors.
  - **For The Klang Planter**:
    - Perform identical page switching and offscreen paint smoke passes.

---

## Phase 3: Modal Lifecycle & Dialog Smoke Tests
- [x] **Implement `ModalLifecycleSmokeTest` in `test/gui_tests.cpp`**:
  - Programmatically simulate opening and dismissing each modal overlay:
    - `SettingsModalComponent` (gear button).
    - `VersionChecker` notification badge.
    - Right-click popup menus on cards and sliders.
  - Assert that opening the modal creates a valid component, and dismissing it releases all resources without memory leaks or timer leaks (enforcing JUCE 9.0.3 timer hygiene).

---

## Phase 4: Synthetic Mouse & Coordinate Fallback Integration
- [x] **Implement Coordinate-Based Interaction Verification**:
  - Simulate synthetic `juce::MouseEvent` at slider coordinates:
    - Mouse drag up/down alters APVTS parameter value.
    - Mouse double-click restores slider value to `def->doubleClickValue`.
  - Document this as the primary diagnostic harness if headless checks pass but human testers report visual anomalies.

---

## Phase 5: Verification, Archival & Handoff
- [x] Run full build and test suite (`gui_tests` and `dsp_tests`).
- [x] Verify that all test suites pass with zero failures (100+ total assertions).
- [x] Archive `PLAN.md` to `docs/completed_plans/<date>_intensive_gui_tests.md`.
- [x] Reset `PLAN.md` to `# No Active Plan`.
- [x] Update `docs/communique/build_to_plan.md` to `Status: COMPLETE ✅`.
- [x] Commit all changes cleanly to `0.3.1-dev`.
