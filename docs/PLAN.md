# Implementation Plan: Universal Automated GUI Test Harness (`gui_tests`)

## Status & Overview
This plan implements a comprehensive, single-binary C++ functional GUI test harness (`gui_tests.exe`) covering all three GUIs in the repository:
1. **The Klang Farmer** (`TheKlangFarmerAudioProcessorEditor`)
2. **The Klang Planter** (`TheKlangPlanterAudioProcessorEditor`)
3. **The Klang Editor** (`TheKlangEditor` / `MainComponent`)

The harness validates headless instantiation, layout bounding hygiene, synthetic mouse events (click, drag, double-click), two-way APVTS parameter synchronization, page navigation transitions, dynamic FX slot reconfigurations, and paint smoke rendering without graphics assertions.

---

## Phase 1: Build Configuration & JUCE Symbol Isolation
- [ ] **Step 1.1**: Guard `createPluginFilter()` in `source/FarmerProcessor.cpp` with `#ifndef TKF_GUI_TESTS`.
- [ ] **Step 1.2**: Guard `createPluginFilter()` in `source/PlanterProcessor.cpp` with `#ifndef TKF_GUI_TESTS`.
- [ ] **Step 1.3**: Update `CMakeLists.txt` for `gui_tests`:
  - Define compile definition `TKF_GUI_TESTS=1`.
  - Include sources for Farmer, Planter, and Editor (`tools/editor/MainComponent.cpp`).
  - Link `TkfAssets`, `${TKF_JUCE_MODULES}`, and `juce::juce_gui_extra`.

## Phase 2: Core Test Framework & Event Simulation Helpers
- [ ] **Step 2.1**: Create `test/GuiTestHelpers.h`:
  - `ScopedGuiContext`: Safe headless GUI environment setup (`juce::ScopedJuceInitialiser_GUI`).
  - `pumpMessageLoop(int maxIterations = 20, int timeoutMs = 50)`: Micro message pump to deterministically flush async JUCE messages and timer callbacks.
  - `ComponentFinder`: Helpers to recursively locate child components by name, parameter ID (`RotaryKnobSlider::getParamId()`), and component type.
  - `EventSimulator`: Synthetic event dispatchers (`simulateClick`, `simulateDrag`, `simulateDoubleClick`) creating and sending valid `juce::MouseEvent`s.
  - `FailureCapture`: Automatic offscreen snapshot generator saving timestamped PNGs to `test_artifacts/gui/<testName>_<stepName>_<timestamp>.png` upon assertion failures.
  - `TestReporter`: Assertion counter, pass/fail tally, structured terminal report generator.

## Phase 3: The Klang Farmer Functional Test Suite
- [ ] **Step 3.1**: Create `test/FarmerTestSuite.h`:
  - **Lifecycle & Layout**: Headless instantiation of `TheKlangFarmerAudioProcessor` and editor; verify dimensions (1000x750) and resize limits; assert all children non-null.
  - **APVTS Two-Way Binding**:
    - Unipolar slider (`carrier1_pitch`): simulate vertical drag, assert APVTS value increases.
    - Default reset: simulate double-click, assert APVTS value resets to JSON default.
    - APVTS $\to$ UI: mutate APVTS parameter, pump message loop, assert slider visual value updates.
    - Bipolar slider (`carrier1_depth`) & shaped decay curve (`pitchenv1_decay`).
    - LED Selector (`carrier1TrackingSelector`): simulate click on "Freq", assert APVTS `carrier1_tracking` updates and text formatting switches to Hz.
  - **Page Navigation**: Cycle sequentially through all 7 pages via `NavigationCardComponent`; verify visibility of active page cards and invisibility of inactive cards.
  - **Dynamic FX Slots**: Navigate to Pre-Amp FX page; change FX Slot 1 ComboBox; assert `FXSlotCardComponent` updates active knob count, labels, and accents.
  - **Modals & Master Controls**: Verify Quickstart Guide modal open and close; simulate Audition Trigger hit; simulate Init button reset.

## Phase 4: The Klang Planter Functional Test Suite
- [ ] **Step 4.1**: Create `test/PlanterTestSuite.h`:
  - **Lifecycle & 4x2 Layout**: Headless instantiation of `TheKlangPlanterAudioProcessor` and editor; verify dimensions (1040x740); verify 8 module cards bounds are valid and positioned in 4x2 matrix.
  - **APVTS Two-Way Binding**: Test continuous slider (`planter_carrier_pitch`), discrete selector (`planter_carrier_tracking`), and limiter toggle (`planter_limiter_enable`).
  - **Master Visualizer & Audition**: Simulate audition hit; pump message loop; verify audio block executes and visualizer updates without allocation faults.

## Phase 5: The Klang Editor Functional Test Suite
- [ ] **Step 5.1**: Create `test/EditorTestSuite.h`:
  - **Lifecycle & Navigation Tabs**: Headless instantiation of `MainComponent`; verify dimensions (1200x800); simulate tab change between `[THEME]` and `[CONTROLS]`; verify active tab state.
  - **Tree View & Module Preview**: Assert tree view populates from JSON assets; simulate selecting a module tree item; verify `previewWrapper` hosts a live card component without throwing parse exceptions.

## Phase 6: Smoke Paint Suite & CLI Argument Runner
- [ ] **Step 6.1**: Create `test/SmokePaintSuite.h`:
  - Render offscreen software snapshots (`createComponentSnapshot()`) for Farmer, Planter, and Editor.
  - Assert images have valid dimensions, non-blank buffers, and trigger zero JUCE graphics assertions.
- [ ] **Step 6.2**: Update `test/gui_tests.cpp`:
  - Implement CLI argument parsing: `--all` (default), `--farmer`, `--planter`, `--editor`, `--smoke-only`, `--artifacts-dir <dir>`.
  - Wire up all suites and output the final test diagnostic summary.
  - Return exit code `0` on 100% pass, `1` on any failure.

## Phase 7: Build Validation & Guardrail Verification
- [ ] **Step 7.1**: Compile and link `gui_tests` using CMake in Release mode.
- [ ] **Step 7.2**: Execute `./build/Release/gui_tests.exe --all` and verify all tests pass.
- [ ] **Step 7.3**: Verify `build-validate` execution with and without `--skip-gui`.
