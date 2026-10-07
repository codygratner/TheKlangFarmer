# Implementation Plan: Automated VST3 Parameter Validation Suite & Headless pluginval Runner

## Goal
Build a comprehensive, in-engine 6-pillar VST3 parameter validation suite in `test/PluginIntensiveTestSuite.h` covering 100% of registered parameters across both **The Klang Farmer** and **The Klang Planter**, and provide a headless `pluginval` CLI test runner script in `tools/run_pluginval.ps1`.

## User Intent & Safety
- **Safe for Mobile Execution**: This feature purely expands test suites and validation scripts. It has zero impact on runtime audio synthesis algorithms, preset values, or UI layout code. The user can safely verify desktop visual sanity when they return.
- **Strict Role Compliance**: New Klang City authored this plan. Klang Industries will execute the C++ additions, compilation, and test runs.

---

## Architecture & Test Suite Design

### Phase 1: In-Engine 6-Pillar Parameter Validation Suite
Location: `test/PluginIntensiveTestSuite.h` -> `runVst3ParameterValidationSuite(TestReporter& reporter)`

Implement thorough automated validation across both `TheKlangFarmerAudioProcessor` and `TheKlangPlanterAudioProcessor`:

1. **Pillar 1: Dynamic Reflection & Identity**:
   - Sweep all parameters returned by `processor.getParameters()` for both processors.
   - Assert parameter count exceeds thresholds (Farmer >= 100, Planter >= 30).
   - Assert every parameter has a non-empty `paramID` and non-empty `name`.

2. **Pillar 2: Normalization Roundtrip (`convertTo0to1` & `convertFrom0to1`)**:
   - For every continuous float parameter, evaluate normalized test points: `[0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 1.0f]`.
   - Assert `convertTo0to1(convertFrom0to1(normValue))` matches `normValue` within step-aware tolerance.

3. **Pillar 3: Boundary Clamping & Safety**:
   - For every parameter, feed out-of-bounds normalized values: `-1.0f`, `-0.05f`, `1.05f`, `2.0f`.
   - Call `param->setValueNotifyingHost(outOfBounds)`.
   - Verify `param->getValue()` is strictly clamped to `[0.0f, 1.0f]` with zero NaNs or Infs.

4. **Pillar 4: Dual-Plugin JSON Schema Parity**:
   - Verify every APVTS parameter in both Farmer and Planter has a corresponding `ControlDef` in `ParameterManager` or `assets/controls/*.json`.
   - Verify default normalized values fall within `[0.0f, 1.0f]`.

5. **Pillar 5: APVTS State Serialization & Restoration Roundtrip**:
   - Capture clean baseline state with `processor.getStateInformation(memBlock)`.
   - Randomize/invert all parameter values to non-default positions.
   - Restore state with `processor.setStateInformation(memBlock.getData(), (int)memBlock.getSize())`.
   - Assert that 100% of parameters return to their pre-randomization values within floating-point tolerance.

6. **Pillar 6: DSP Audio Smoke Pass Under Automation**:
   - Configure audio processor with `prepareToPlay(44100.0, 512)`.
   - Run a 32-block loop processing stereo audio (`juce::AudioBuffer<float> buffer(2, 512)`).
   - Rapidly sweep all parameters across their full range during block rendering.
   - Inspect output buffers for NaNs, Infs, or silent lockups (`std::isnan`, `std::isinf`).

---

### Phase 2: Master Runner Hook & `gui_tests` Integration
Location: `test/PluginIntensiveTestSuite.h`
- [x] Add `runVst3ParameterValidationSuite(reporter);` into `PluginIntensiveTestSuite::runSuite(TestReporter& reporter)`.
- [x] Ensure clean reporting with informative diagnostic failure messages.

---

### Phase 3: Portable Headless `pluginval` CLI Runner
Location: `tools/run_pluginval.ps1`
- [x] PowerShell runner script checking for `pluginval` in PATH or `tools/pluginval.exe`.
- [x] Validates deployed VST3 binaries:
  - `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3`
  - `C:\Program Files\Common Files\VST3\The Klang Planter.vst3`
- [x] Configurable strictness level (default: 5) with clean error capture.
- [x] If binary is absent, prints clear download instructions: `Invoke-WebRequest -Uri https://github.com/Tracktion/pluginval/releases/download/...`

---

### Phase 4: Build, Validation & Clean Deployment
- [x] Execute `cmake --build build --config Release --target gui_tests`.
- [x] Run `build/Release/gui_tests.exe` and assert 100% pass across all test suites with 0 failures (207/207 passed).
- [x] Run `dsp_tests.exe` and assert 0 failures.
- [x] Update `docs/communique/build_to_plan.md` with execution report.

---

## Acceptance Criteria
- [x] 100% of registered APVTS parameters in both plugins pass all 6 validation pillars.
- [x] `gui_tests` passes with zero failures (207/207).
- [x] `tools/run_pluginval.ps1` is authored and tested.
- [x] No regressions in existing unit tests (`dsp_tests` and `gui_tests`).
