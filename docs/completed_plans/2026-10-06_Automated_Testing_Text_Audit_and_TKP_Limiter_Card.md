# Execution Plan: Automated Testing, Text Audit & TKP Limiter Card

## Phase 0: Automated Crash Reproduction & Fix (Carrier 1)
**Goal:** Build an internal test harness to automatically and instantly reproduce the "Carrier 1" crash on startup, allowing the agent to repeatedly test code changes without user intervention.
- [x] **Step 0.1: Internal Test Harness**
  - Inject a startup hook into `MainComponent.cpp` that bypasses the saved  ppState XML and forcefully triggers `onTreeItemSelected` for `Carrier 1`.
  - Add debug trace logs to precisely pinpoint where the GUI thread is crashing (e.g., inside `DynamicObject` access, out-of-bounds array access, or JUCE component resizing).
- [x] **Step 0.2: Automated Loop Execution**
  - Run the Editor executable headlessly/silently using an automated agent loop (Start-Process -Wait).
  - Capture the exit code and `Editor.log`.
  - Continuously apply fixes and test until the exit code is exactly   (clean exit) and the Editor stays alive.
- [x] **Step 0.3: Fix & Remove Harness**
  - Once the root cause is resolved (e.g., UI component lifetime mismatch or JSON parsing bug), verify the fix.
  - Remove the test harness and return the Editor to normal operation.
## Phase 1: TKF/TKP Text Verification (Interactive Audit)
**Goal:** Ensure all strings (global titles, UI cards, tooltips, fallback text, and buttons) are pulled from JSON files (`global_ui.json`, `controls/*.json`). When migrating, the original hardcoded C++ string will take precedence over any existing JSON strings.

*Per the user's instructions, this phase will be executed interactively. I will pause and ask for confirmation after preparing each card/section's text.*

- [x] **Step 1.1: Global & Header Text**
  - Extract button tooltips (`INIT`, `TRIGGER`, `TOOLTIPS`, `QUICKSTART GUIDE`).
  - Extract navigation tab text and tooltips (`PAGES`, `LIMIT`, `OFF`).
  - *Action:* Move to `global_ui.json`, update C++ to use `getGlobalString()`. Present for confirmation.
- [x] **Step 1.2: TKF Module Cards Text**
  - Extract module card titles and tooltips for Farmer (e.g. `cardCarrier1->setTooltip(...)`, `cardMod1`, `cardMixer`, etc.).
  - Extract fallback descriptions in `makeKnobTooltipFromParam` calls.
  - *Action:* Move to respective module JSON files (`carrier.json`, `modulators.json`, `envelopes.json`, etc.). Present for confirmation.
- [x] **Step 1.3: TKP Module Cards Text**
  - Extract module card titles and tooltips for Planter (e.g. `cardCarrier`, `cardNoise`, `cardFilter`).
  - Extract fallback descriptions in `makeKnobTooltipFromParam` for TKP parameters.
  - *Action:* Move to `planter.json`. Present for confirmation.
- [x] **Step 1.4: Validation & Cleanup**
  - Scan `FarmerEditor.cpp`, `PlanterEditor.cpp`, and `UIComponents.cpp` to ensure absolutely zero hardcoded user-facing strings remain.

## Phase 2: TKP Limiter Editor Card (Code Reuse)
**Goal:** Expose an editor card for the "always on" limiter in The Klang Planter. It will reuse the true brickwall `LimiterBlock` logic from TKF instead of TKP's current permanent soft-saturation `tanh` approximation.

- [x] **Step 2.1: DSP Implementation (`PlanterEngine.h`)**
  - Remove the hardcoded `FastMath::fastTanh` from `PlanterAmpBlock`.
  - Import and instantiate the standard `TbdAudio::LimiterBlock` (used by TKF in `ModularBlocks.h`) into `PlanterDrumEngine` as the final output stage.
- [x] **Step 2.2: Parameter Definitions (`planter.json`)**
  - Define new `planter_limiter_enable`, `planter_limiter_gain`, `planter_limiter_thresh`, and `planter_limiter_release` parameters in JSON, matching TKF's ranges.
- [x] **Step 2.3: Audio Processor Boilerplate (`PlanterProcessor.cpp`)**
  - Wire the new APVTS parameters to the `PlanterDrumEngine` limiter block page parameters.
- [x] **Step 2.4: UI Card Implementation (`PlanterEditor.cpp`)**
  - Create a new `ModuleCardComponent` for the Limiter.
  - Add the knobs (Gain, Threshold, Release) and the Enable toggle to the editor layout.
  - Test UI responsiveness and DSP peak limiting behavior.
