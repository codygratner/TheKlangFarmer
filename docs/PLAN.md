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
  - **Stage 1 (Lifecycle & Layout Bounds)**: Headless instantiation of `TheKlangFarmerAudioProcessor` and editor; verify dimensions (1000x750) and resize limits (800x600 to 1200x900); assert child components are non-null.
  - **Stage 2 (All 7 Pages Navigation Transitions)**: Cycle sequentially through all 7 pages via `NavigationCardComponent`; verify visibility of active page cards and invisibility of inactive cards.
  - **Stage 3 (APVTS 2-Way Parameter Sync & Formatters)**:
    - Unipolar slider (`carrier1_pitch`): simulate upward drag, assert APVTS value increases.
    - Double-click default reset: simulate double-click, assert APVTS value resets to JSON default.
    - APVTS $\to$ UI: mutate APVTS parameter, pump message loop, assert slider visual value updates.
    - Bipolar slider (`carrier1_depth`) and shaped decay curve (`pitchenv1_decay`).
    - LED Selector (`carrier1TrackingSelector`): simulate click on "Freq", assert APVTS `carrier1_tracking` updates and text formatting switches to Hz.
  - **Stage 4 (Full 13 FX Algorithms Dynamic Reconfiguration Sweep)**:
    - In Pre-Amp FX page, sweep through all 13 FX algorithms across dynamic FX slots.
    - Assert `FXSlotCardComponent` updates active knob count (0-4), labels, and accents for every algorithm without leaks or asserts.
  - **Stage 5 (Right-Click Callout Popups & Snap-Point Clicks)**:
    - Simulate right-click on `carrier1_pitch`; assert `SliderCalloutComponent` opens with snap-point buttons.
    - Simulate clicking a snap-point button; assert slider snaps to value and callout closes.
  - **Stage 6 (Master Header Controls & Modals)**:
    - Verify audition Trigger hit fires.
    - Verify Init button resets patch to defaults.
    - Verify Quickstart Guide modal open and close.
    - Verify Visualizer Lock and Off button clicks.
  - **Stage 7 (Audio-to-Visualizer Data Pipeline)**:
    - Process audio block through `processor.processBlock()`.
    - Trigger `timerCallback()`; assert `MiniOscilloscopeComponent` buffer receives waveform points and peak meters register activity.
  - **Stage 8 (Preset / State Save & Restore Roundtrip)**:
    - Serialize patch state via `processor.getStateInformation()`.
    - Alter multiple UI controls; restore state via `processor.setStateInformation()`.
    - Assert all UI sliders and selectors update to match restored state.

## Phase 4: The Klang Planter Functional Test Suite
- [ ] **Step 4.1**: Create `test/PlanterTestSuite.h`:
  - **Stage 1 (Lifecycle & 4x2 Layout Bounds)**: Headless instantiation of `TheKlangPlanterAudioProcessor` and editor; verify dimensions (1040x740); verify 8 module cards bounds are valid and positioned in 4x2 matrix.
  - **Stage 2 (Context-Sensitive Text Formatter Switching)**:
    - Switch `carrierTrackingSelector` between MIDI (-24 to +24 st), Freq (Hz), and Note; verify pitch slider format text changes.
    - Switch `modTypeSelector` between Osc, Cyclic, and Noise; verify shape slider formatting switches between unipolar and bipolar %.
  - **Stage 3 (APVTS 2-Way Parameter Bindings)**: Test continuous slider (`planter_carrier_pitch`), bipolar slider (`planter_carrier_depth`), discrete selector (`planter_carrier_tracking`), and limiter toggle (`planter_limiter_enable`).
  - **Stage 4 (Audio Overload Limiter Badge & Peak Meters Response)**:
    - Render high-gain audio buffer through `processor.processBlock()`.
    - Pump timer callback; assert `PlanterHeaderVisualizer` registers peak meter activity and `LIMIT` warning badge lights up.
  - **Stage 5 (Audition Trigger & Init Reset)**: Assert audition trigger hit fires; assert `INIT` button resets APVTS cleanly.
  - **Stage 6 (Preset / State Save & Restore Roundtrip)**: Serialize state, alter controls, restore state, assert UI components synchronize.

## Phase 5: The Klang Editor Functional Test Suite
- [ ] **Step 5.1**: Add `friend class EditorTestSuite;` in `tools/editor/MainComponent.h` for clean test access without polluting public APIs.
- [ ] **Step 5.2**: Create `test/EditorTestSuite.h`:
  - **Stage 1 (Lifecycle & Layout Bounds)**: Headless instantiation of `MainComponent`; verify dimensions (1200x800); assert child components (`navigationTabs`, `layoutsTree`, `controlsTree`, `formEditor`, `previewWrapper`, `jsonContainer`) have valid bounds $> 0$.
  - **Stage 2 (Tab Switching)**: Toggle between `[CONTROLS]` (index 0) and `[LAYOUTS]` (index 1); assert `navigationTabs.getCurrentTabIndex()` updates; verify `jsonSplitterLayout` column sizing adapts.
  - **Stage 3 (Tree Hierarchy Audit)**: Verify `layoutsTree` and `controlsTree` roots populate product items (`tkf`, `tkp`, `tkm`, `tks`, `Raw Layout JSONs`, `Raw Control JSONs`).
  - **Stage 4 (Card Selection & Preview Mount)**: Simulate selecting `tkf` $\to$ `Voice 1` $\to$ `carrier1`; pump message loop; assert `previewWrapper` mounts a live `ModuleCardComponent`.
  - **Stage 5 (PropertyPanel Row Verification)**: Assert `formEditor.getProperties().size() > 0`, verifying that drilling down on cards populates parameters in the property panel (guarding against the backlog parameter population bug).
  - **Stage 6 (Two-Way Property-to-Preview Editing)**: Locate `ParamRowPropertyComponent` for `carrier1_pitch`; simulate editing raw value; verify preview slider in `previewWrapper` reflects change.
  - **Stage 7 (Live CodeDocument JSON Sync)**: Programmatically mutate `controlsJsonDocument` or `layoutJsonDocument`; pump debounced timer; assert preview component updates without parse exceptions.
  - **Stage 8 (Toolbar Actions)**: Simulate `toggleOriginalButton` click; verify `showingOriginal` state and button text toggle; simulate `refreshButton` click; verify clean reload and state restoration.

## Phase 6: Smoke Paint Suite & CLI Argument Runner
- [ ] **Step 6.1**: Create `test/SmokePaintSuite.h`:
  - Render offscreen software snapshots (`createComponentSnapshot()`) for Farmer, Planter, and Editor.
  - Assert images have valid dimensions, non-blank buffers, and trigger zero JUCE graphics assertions.
- [ ] **Step 6.2**: Create `test/ReflectionGuardrailSuite.h` (Strict Dynamic Guardrail):
  - Dynamically iterate 100% of APVTS parameters in Farmer and Planter: assert every registered parameter is bound to a valid UI component in the editor, and test min/max/default boundaries (0 NaN, 0 repaint crashes). Hard-fail if an unmapped parameter is discovered!
  - Dynamically iterate 100% of JSON layout and control files: assert every file is mountable in `TheKlangEditor` without syntax or memory faults. Hard-fail if an unmountable card is discovered!
- [ ] **Step 6.3**: Create `test/HardeningSuites.h` (Industry-Standard Resilience):
  - `LifecycleStressSuite`: Rapid 10x Editor instantiation/destruction loop across Farmer and Planter to guarantee JUCE 9.0.3 timer hygiene (#1696), zero dangling pointers, and zero memory leaks.
  - `DpiScaleSuite`: Test scaling transforms at 1.0x, 1.25x, 1.5x, and 2.0x Retina/4K to ensure proportional scaling without clipping or arithmetic overflows.
  - `TooltipCoverageAuditSuite`: 100% recursive audit of all interactive controls in Farmer and Planter, asserting non-empty JSON tooltip definitions.
- [ ] **Step 6.4**: Update `test/gui_tests.cpp`:
  - Implement CLI argument parsing: `--all` (default), `--farmer`, `--planter`, `--editor`, `--smoke-only`, `--reflection-only`, `--stress-only`, `--artifacts-dir <dir>`.
  - Wire up all suites and output the final test diagnostic summary.
  - Return exit code `0` on 100% pass, `1` on any failure.

## Phase 7: Build Validation & Guardrail Verification
- [x] **Step 7.1**: Compile and link `gui_tests` using CMake in Release mode.
- [x] **Step 7.2**: Execute `./build/Release/gui_tests.exe --all` and verify all tests pass.
- [x] **Step 7.3**: Verify `build-validate` execution with and without `--skip-gui`.
