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
- **Lifecycle & Layout**:
  - Instantiate headless `TheKlangFarmerAudioProcessor`.
  - Instantiate `TheKlangFarmerAudioProcessorEditor`.
  - Set size `(1000, 750)` and resize to `(800, 600)` and `(1200, 900)`.
  - Assert width and height are valid, child components are non-null.
- **APVTS Two-Way Binding**:
  - Locate `carrier1_pitch` slider via `GuiTestHelpers::findSliderByParamId`.
  - Simulate mouse drag upwards: assert APVTS parameter increases.
  - Simulate double-click: assert APVTS parameter resets to default value.
  - Programmatically update APVTS parameter: pump message loop, assert slider value reflects change.
  - Verify bipolar knob (`carrier1_depth`) and non-linear decay curve knob (`pitchenv1_decay`).
  - Locate `carrier1TrackingSelector` (`LedSelectorComponent`), simulate click on Item 1 ("Freq"), verify APVTS `carrier1_tracking` updates and slider formatting changes.
- **Page Navigation**:
  - Locate `NavigationCardComponent` in Slot 1.
  - Sequentially switch through all 7 pages (Voice 1, Voice 2, Transients, Pre-Amp FX, Amplifier, Post-Amp FX, Modulations).
  - After each transition, pump message loop and assert that the expected module cards are visible and other pages' cards are hidden.
- **Dynamic FX Slot Reconfiguration**:
  - Navigate to Pre-Amp FX page.
  - Change FX Slot 1 ComboBox selection across algorithms (e.g. Waveshaper, Delay, Reverb).
  - Verify that `FXSlotCardComponent` dynamically updates knob labels, active count, and color accents.
- **Modal Reference & Header Actions**:
  - Toggle Quickstart Guide modal; assert modal is visible.
  - Simulate click on close button; assert modal is hidden.
  - Simulate click on audition trigger button; verify audio audition fires without audio thread allocations or GUI stalls.
  - Simulate click on `INIT` button; verify all parameters reset to patch defaults.

### 4.2 Suite 2: The Klang Planter (`PlanterTestSuite`)
- **Lifecycle & Layout**:
  - Instantiate headless `TheKlangPlanterAudioProcessor`.
  - Instantiate `TheKlangPlanterAudioProcessorEditor`.
  - Set size `(1040, 740)`.
  - Verify 8 module cards bounds are non-empty and arranged in a 4x2 matrix.
- **APVTS Two-Way Binding**:
  - Test continuous slider (`planter_carrier_pitch`), bipolar slider (`planter_carrier_depth`), discrete selector (`planter_carrier_tracking`), limiter toggle (`planter_limiter_enable`).
- **Master Header & Visualizer**:
  - Fire audition trigger hit; pump message loop; assert engine trigger executes and header visualizer buffer updates.
  - Click `INIT` button; verify APVTS resets cleanly.

### 4.3 Suite 3: The Klang Editor (`EditorTestSuite`)
- **Lifecycle & Tabs**:
  - Instantiate `MainComponent`.
  - Set size `(1200, 800)`.
  - Switch tabs between `[THEME]` and `[CONTROLS]`; verify active tab index updates.
- **Tree Population & Preview Loading**:
  - Verify `layoutsTree` and `controlsTree` items populate from asset JSON files.
  - Simulate selecting a module item from the tree; verify `previewWrapper` is populated with a live card component without throwing parse exceptions or memory corruption.

### 4.4 Suite 4: Smoke Paint (`SmokePaintSuite`)
- Render offscreen software snapshots (`createComponentSnapshot()`) for Farmer, Planter, and Editor.
- Verify generated `juce::Image` is non-null, valid dimensions, non-empty pixel data, and triggers 0 JUCE graphics assertions.

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
