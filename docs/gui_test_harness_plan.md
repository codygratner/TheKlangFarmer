# Architectural Plan: Universal Automated GUI Test Harness (`gui_tests`)

## 1. Executive Summary
The goal of this architectural milestone is to implement a unified, zero-dependency, automated C++ functional GUI test harness (`gui_tests`) for the entire Klang repository ecosystem:
1. **The Klang Farmer** (`TheKlangFarmerAudioProcessor` + `TheKlangFarmerAudioProcessorEditor`)
2. **The Klang Planter** (`TheKlangPlanterAudioProcessor` + `TheKlangPlanterAudioProcessorEditor`)
3. **The Klang Editor** (`MainComponent` standalone JSON theme & controls editor)

The harness acts as a strict regression guardrail during continuous development and CI. It verifies headless instantiation, layout bounding hygiene, synthetic event simulation (mouse drag, click, double-click reset), two-way APVTS parameter synchronization, page navigation transitions, dynamic FX slot reconfigurations, and paint smoke rendering without graphics assertions.

---

## 2. Key Design Decisions (from Architectural Interview)
- **Target Architecture**: True single binary (`gui_tests.exe`) testing all three programs sequentially, with CLI flags (`--all`, `--farmer`, `--planter`, `--editor`, `--smoke-only`).
- **JUCE Linker Conflict Resolution**: In `FarmerProcessor.cpp` and `PlanterProcessor.cpp`, guard the JUCE plugin factory function `createPluginFilter()` with `#ifndef TKF_GUI_TESTS`. When building `gui_tests`, CMake defines `TKF_GUI_TESTS=1`, preventing `LNK2005` duplicate symbol collisions.
- **Testing Depth**: Functional & State Verification.
  - Headless lifecycle & layout bounds safety.
  - Synthetic mouse event simulation (`juce::MouseEvent` down/drag/up/doubleClick).
  - Two-way APVTS binding verification (UI $\to$ APVTS and APVTS $\to$ UI).
  - Page navigation and tab transitions.
  - Paint smoke checks (offscreen software rendering to `juce::Image` to catch render assertions).
- **Diagnostics & Failure Handling**: Full test run execution with comprehensive summary reporting. Upon any assertion failure, automatically save an offscreen PNG snapshot of the failing component hierarchy to `test_artifacts/gui/<testName>_<timestamp>.png` and log exact diagnostic details (Expected vs Actual, Parameter ID, Component path).
- **Event Pumping**: Micro message-loop pumping helper `pumpMessageLoop(int maxIterations = 20, int timeoutMs = 50)` to deterministically flush JUCE async notifications, ComboBox dispatches, and timer callbacks between simulated user actions.

---

## 3. Directory & File Structure
```
TheKlangFarmer/
├── source/
│   ├── FarmerProcessor.cpp             # Guard createPluginFilter with #ifndef TKF_GUI_TESTS
│   └── PlanterProcessor.cpp            # Guard createPluginFilter with #ifndef TKF_GUI_TESTS
├── test/
│   ├── GuiTestHelpers.h                # Event simulator, component finder, failure snapshotter, message pump
│   ├── FarmerTestSuite.h               # Farmer functional test cases (Pages 0-6, APVTS bindings, FX dynamic slots)
│   ├── PlanterTestSuite.h              # Planter functional test cases (4x2 cards, APVTS bindings, meters, audition)
│   ├── EditorTestSuite.h               # Editor functional test cases (Tabs, Tree loading, Preview component)
│   ├── SmokePaintSuite.h               # Offscreen software rendering smoke checks for all GUIs
│   └── gui_tests.cpp                   # Main runner entrypoint & CLI argument parser
├── test_artifacts/
│   └── gui/                            # Auto-generated failure screenshots & visual logs
└── CMakeLists.txt                      # Configure gui_tests target with TKF_GUI_TESTS=1
```

---

## 4. Test Suite Specifications

### 4.1 Suite 1: The Klang Farmer (`FarmerTestSuite`)
- **Stage 1 (Lifecycle, Resizing & Layout Bounds)**:
  - Instantiate headless `TheKlangFarmerAudioProcessor` and editor.
  - Set size `(1000, 750)` and resize to `(800, 600)` and `(1200, 900)`.
  - Assert width and height are valid, child components are non-null and correctly positioned.
- **Stage 2 (All 7 Pages Navigation Transitions)**:
  - Locate `NavigationCardComponent` in Slot 1.
  - Sequentially traverse Pages 0–6: Voice 1, Voice 2, Transients, Pre-Amp FX, Amplifier, Post-Amp FX, Modulations.
  - After each page transition, pump message loop and assert that expected module cards are visible and inactive cards are hidden.
- **Stage 3 (APVTS 2-Way Parameter Sync & Formatters)**:
  - Continuous unipolar slider (`carrier1_pitch`): simulate upward drag, assert APVTS value increases.
  - Double-click default reset: simulate double-click, assert APVTS value resets to JSON default.
  - APVTS $\to$ UI: programmatically mutate APVTS parameter, pump message loop, assert slider visual value updates.
  - Bipolar slider (`carrier1_depth`) and non-linear decay curve slider (`pitchenv1_decay`).
  - Discrete LED Selector (`carrier1TrackingSelector`): simulate click on "Freq", assert APVTS `carrier1_tracking` updates and text formatting switches from semitones to Hz.
- **Stage 3.5 (Dynamic Reflection Sweep — 100% Parameter Coverage Guarantee)**:
  - Dynamically introspects `processor.apvts.getParameterTree()` across all parameter groups.
  - For every single registered APVTS parameter:
    1. Asserts a corresponding UI component or slider exists within the editor's component hierarchy.
    2. Asserts a valid JSON definition is loaded in `ParameterManager`.
    3. Programmatically mutates the parameter from min to max; asserts the UI component updates accurately without stalling.
  - Zero-orphan guarantee: if an engineer adds an APVTS parameter without wiring a UI control, or creates a rogue UI slider with no APVTS binding, `gui_tests` hard-fails immediately with a descriptive error.
- **Stage 4 (Full 13 FX Algorithms Dynamic Reconfiguration Sweep)**:
  - Navigate to Pre-Amp FX page.
  - Sweep through all 13 FX algorithms across dynamic FX slots:
    `Chorus`, `CombFilter`, `Flanger`, `Phaser`, `TempoDelay`, `Bitcrusher`, `Drive`, `PitchShifter`, `RingMod`, `AutoWah`, `Compressor`, `Reverb`, `Decimator`.
  - For each algorithm, verify `FXSlotCardComponent` dynamically updates active knob count (0 to 4), knob labels, and color accents without memory leaks or graphics asserts.
- **Stage 5 (Right-Click Callout Popups & Snap-Point Clicks)**:
  - Simulate right-click on `carrier1_pitch` slider; verify `SliderCalloutComponent` opens with POI snap-point buttons.
  - Simulate clicking a snap-point button; verify slider value snaps to preset value and callout dismisses.
- **Stage 6 (Master Header Controls, Modals & Visualizer Toggles)**:
  - Simulate audition trigger button click; verify drum hit fires.
  - Simulate `INIT` button click; verify all parameters reset to patch defaults.
  - Toggle Quickstart Guide modal; assert visible; simulate close button click, assert hidden.
  - Simulate clicks on Visualizer Lock and Off icons; assert `isLocked` and `isOff` states toggle.
- **Stage 7 (Audio-to-Visualizer Data Pipeline)**:
  - Process an audio buffer block through `processor.processBlock()`.
  - Trigger `timerCallback()`; assert `MiniOscilloscopeComponent` buffer receives waveform points and peak meters register activity.
- **Stage 8 (Preset / State Save & Restore Roundtrip)**:
  - Call `processor.getStateInformation()` to serialize patch state to XML memory block.
  - Alter multiple UI sliders and selectors away from initial values.
  - Call `processor.setStateInformation()` to restore original state.
  - Pump message loop; assert all UI sliders and selectors update to match the restored state.

### 4.2 Suite 2: The Klang Planter (`PlanterTestSuite`)
- **Stage 1 (Lifecycle & 4x2 Matrix Bounds)**:
  - Instantiate headless `TheKlangPlanterAudioProcessor` and editor.
  - Set size `(1040, 740)`.
  - Assert all 8 module cards (Carrier, Modulator, Pitch Env, Noise, Filter, Filter Env, Amp, Limiter) have non-empty bounds and are arranged in a 4x2 matrix.
- **Stage 2 (Context-Sensitive Text Formatter Switching)**:
  - Switch `carrierTrackingSelector` between MIDI (-24 to +24 st), Freq (Hz), and Note; verify pitch slider format text changes.
  - Switch `modTypeSelector` between Osc, Cyclic, and Noise; verify shape slider formatting switches between unipolar and bipolar %.
- **Stage 3 (APVTS 2-Way Parameter Bindings)**:
  - Test continuous slider (`planter_carrier_pitch`), bipolar slider (`planter_carrier_depth`), discrete selector (`planter_carrier_tracking`), and limiter toggle (`planter_limiter_enable`).
- **Stage 4 (Audio Overload Limiter Badge & Peak Meters Response)**:
  - Render a high-gain audio buffer through `processor.processBlock()` to trigger brickwall limiting.
  - Pump message loop / timer; assert `PlanterHeaderVisualizer` registers peak meter activity and `LIMIT` warning badge lights up.
- **Stage 5 (Audition Trigger & Init Reset)**:
  - Simulate trigger button click; assert audition hit fires.
  - Simulate `INIT` button click; assert APVTS resets cleanly.
- **Stage 6 (Preset / State Save & Restore Roundtrip)**:
  - Serialize state to memory block; mutate controls; restore state; pump message loop; assert UI components synchronize.

### 4.3 Suite 3: The Klang Editor (`EditorTestSuite`)
`MainComponent` grants `friend class EditorTestSuite;` to enable deep, deterministic inspection without polluting the production API.
- **Stage 1: Lifecycle & Layout Bounds**:
  - Headless instantiation of `MainComponent`; set size `(1200, 800)`.
  - Assert all top-level child components (`navigationTabs`, `layoutsTree`, `controlsTree`, `refreshButton`, `saveButton`, `toggleOriginalButton`, `formEditor`, `previewWrapper`, `jsonContainer`) have valid bounds $> 0$.
- **Stage 2: Tab Switching**:
  - Verify initial tab is index 0 (`CONTROLS`).
  - Switch to tab index 1 (`LAYOUTS`); assert `navigationTabs.getCurrentTabIndex() == 1`.
  - Assert `jsonSplitterLayout` recalculates dimensions (controls editor hidden, layout editor expanded).
  - Switch back to tab index 0 (`CONTROLS`).
- **Stage 3: Tree Hierarchy & Product Node Audit**:
  - Verify `layoutsTree.getRootItem()` and `controlsTree.getRootItem()` populate from asset JSONs.
  - Assert all expected product nodes exist: `tkf` (The Klang Farmer), `tkp` (The Klang Planter), `tkm` (The Klang Mill), `tks` (The Klang Seed), plus "Raw Layout JSONs" and "Raw Control JSONs".
- **Stage 4: Card Tree Selection & Preview Mounting**:
  - Drill down and select a card node in the tree (e.g. `tkf` $\to$ `Voice 1` $\to$ `carrier1`).
  - Pump message loop; assert `previewWrapper` contains a live `ModuleCardComponent`.
  - Assert `filePathDisplay` reflects `assets/layouts/tkf_layout.json`.
- **Stage 5: PropertyPanel Row Population (Bugfix Validation)**:
  - Assert `formEditor` (`PropertyPanel`) contains property rows (`getProperties().size() > 0`), mathematically verifying that card selection populates parameters in the property panel (guarding against the backlog parameter population bug).
- **Stage 6: Two-Way PropertyPanel to Preview Editing**:
  - Locate `ParamRowPropertyComponent` in `formEditor` for `carrier1_pitch`.
  - Read raw value label; trigger edit action/lambda with a new pitch offset.
  - Assert the preview slider in `previewWrapper` updates in sync with the property panel edit.
- **Stage 7: Live CodeDocument JSON Sync**:
  - Programmatically modify a parameter or color property string in `controlsJsonDocument` or `layoutJsonDocument`.
  - Pump message loop/timer (`startTimer(500)` debounced trigger $\to$ `timerCallback()` $\to$ `syncJsonToPreview()`).
  - Assert the preview card updates without throwing JSON parse exceptions or memory corruption.
- **Stage 8: Toolbar Actions (Toggle Original & Refresh)**:
  - Simulate click on `toggleOriginalButton`: assert `showingOriginal` flips to `true` and button text updates to "Show Edited".
  - Click again: assert state reverts to `false`.
  - Simulate click on `refreshButton`: assert `reloadFromJson` executes, tree openness and selection are restored cleanly.

### 4.4 Suite 4: Smoke Paint (`SmokePaintSuite`)
- Render offscreen software snapshots (`createComponentSnapshot()`) for Farmer, Planter, and Editor.
- Verify generated `juce::Image` is non-null, valid dimensions, non-empty pixel data, and triggers 0 JUCE graphics assertions.

### 4.5 Suite 5: Dynamic Reflection & Orphan Feature Guardrail (`ReflectionGuardrailSuite`)
- **100% APVTS Parameter Audit**:
  - Dynamically query `audioProcessor.apvts.getParameterLayout()` for both The Klang Farmer and The Klang Planter.
  - Iterate through every single registered parameter.
  - Search the respective Editor for a bound UI slider or selector (`findSliderByParamId` / `findSelectorByParamId`).
  - **Hard Failure**: If any non-internal APVTS parameter lacks a corresponding bound UI component in the editor, the test immediately fails with an error: `FAILED: Orphan APVTS parameter '<id>' found without UI binding!`.
  - Perform boundary sanity sweeps: Set every parameter to its minimum ($0.0$), maximum ($1.0$), and default value; pump message loop; verify zero crashes, NaNs, or repaint exceptions.
- **100% JSON Card & Layout Mount Audit**:
  - Dynamically iterate over every JSON file in `assets/layouts/` and `assets/controls/`.
  - Verify every file parses validly into `TheKlangEditor` and can be mounted into `previewWrapper` without throwing JSON syntax or memory corruption errors.
  - **Hard Failure**: If an orphaned or unmountable card file is discovered, the test fails.

### 4.6 Suite 6: Rapid Lifecycle Stress & Timer Hygiene (`LifecycleStressSuite`)
- **Industry Standard DAW Window Open/Close Stress**:
  - Simulates rapid DAW host track selection and UI window toggling by repeatedly instantiating and destroying the Editor 10 times in a tight loop across both Farmer and Planter.
  - Pumping message loop on each cycle.
  - **JUCE 9.0.3 Timer Hygiene (#1696)**: Asserts that all timers are cleanly stopped on destruction, zero dangling LookAndFeel or AsyncUpdater pointers linger, and zero memory leaks occur (`JUCE_LEAK_DETECTOR`).

### 4.7 Suite 7: High-DPI Scaling & Transform Resilience (`DpiScaleSuite`)
- **Multi-DPI Desktop Verification**:
  - Applies scaling transforms across common desktop DPI scales: 100% (1.0x standard), 125% (1.25x), 150% (1.5x), and 200% (2.0x Retina / 4K).
  - Asserts that `setTransform(juce::AffineTransform::scale(scaleFactor))` executes cleanly, component bounds scale proportionally, and no layout clipping or division-by-zero occurs during paint.

### 4.8 Suite 8: 100% Tooltip & Localization Coverage Audit (`TooltipCoverageAuditSuite`)
- **Zero Missing Tooltips**:
  - Recursively traverses all child sliders, buttons, ComboBoxes, and cards across Farmer and Planter.
  - Asserts that every visible interactive control has a non-empty, descriptive tooltip string populated from JSON assets (`global_ui.json` or module control JSONs).
  - Hard-fails if any interactive control is missing a tooltip string.

---

## 5. Failure Artifact Capture
When an assertion fails in any suite:
```cpp
void captureFailureArtifact(juce::Component& comp, const juce::String& testName, const juce::String& stepName) {
    auto img = comp.createComponentSnapshot(comp.getLocalBounds());
    juce::File dir("test_artifacts/gui");
    dir.createDirectory();
    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    juce::File file = dir.getChildFile(testName + "_" + stepName + "_" + timestamp + ".png");
    juce::FileOutputStream stream(file);
    if (stream.openedOk()) {
        juce::PNGImageFormat png;
        png.writeImageToStream(img, stream);
        std::cerr << "  [SNAPSHOT SAVED]: " << file.getFullPathName() << std::endl;
    }
}
```

---

## 6. Build & CI Integration
- `gui_tests` is built alongside `dsp_tests`.
- The `/build-validate` skill automatically runs `gui_tests` unless bypassed via `--skip-gui`.
- CI runs `gui_tests` in headless mode on Windows, Linux, and macOS.
