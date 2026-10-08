# Implementation Plan: v0.4.0 Interaction & UI Polish

## Overview
Address first-round desktop feedback on The Klang Farmer:
1. Fix text truncation in custom-rendered meter sliders and card headers.
2. Fix double-click interaction (double-click opens text entry; Alt-click resets to default).
3. Replace text "d6" button with a vector-rendered 6-sided die icon with pips.
4. Replace OS-style right-click popup menu with sleek `juce::CallOutBox` popovers on parameters and module cards.
5. Fix Audition `TRIGGER` button by wiring an atomic audio-thread trigger queue in `FarmerProcessor` with verified audio output.
6. Expand automated test suites with regression tests for all 5 fixes in both Debug and Release modes.

---

### Phase 1: Interaction Scheme Remediation (Double-Click vs Alt-Click & Audition Trigger)
- [x] Fix `RotaryKnobSlider` double-click behavior in `source/UIComponents.cpp`:
  - In `RotaryKnobSlider::mouseDoubleClick`: Remove `setValue(getDefaultValue())`. Call strictly `openHoveringEditor()`.
  - In `RotaryKnobSlider::mouseDown`: Verify Alt-click (`e.mods.isAltDown()`) cleanly resets to default value without initiating drag or text entry.
  - In `DiagramSliderLabel`: Forward `mouseDoubleClick` directly to `slider.openHoveringEditor()`.
- [x] Fix Audition `TRIGGER` button thread safety & audio rendering:
  - In `source/FarmerProcessor.h`: Add thread-safe audition trigger atomics:
    ```cpp
    std::atomic<bool> auditionTriggerRequested { false };
    std::atomic<float> auditionVelocity { 1.0f };
    std::atomic<int> auditionNote { 36 };
    void triggerAudition(float velocity = 1.0f, int midiNote = 36);
    ```
  - In `source/FarmerProcessor.cpp`:
    - Implement `triggerAudition`:
      ```cpp
      void TheKlangFarmerAudioProcessor::triggerAudition(float velocity, int midiNote) {
          auditionVelocity.store(velocity, std::memory_order_relaxed);
          auditionNote.store(midiNote, std::memory_order_relaxed);
          auditionTriggerRequested.store(true, std::memory_order_release);
      }
      ```
    - In `processBlock`: At the start of rendering, check `auditionTriggerRequested.exchange(false, std::memory_order_acq_rel)`. If true, set `engine.setMidiPitch(auditionNote.load())`, `engine.trigger(auditionVelocity.load())`, and update modulation targets.
  - In `source/FarmerEditor.cpp`: Wire `triggerButton.onClick = [this] { audioProcessor.triggerAudition(1.0f, 36); };`.

### Phase 2: Vector 6-Sided Die Icon & Popover Callout Layer
- [x] Implement `DiceButton` in `source/UIComponents.h` and `source/UIComponents.cpp`:
  - Custom component inheriting `juce::Button`.
  - Renders a sleek vector 6-sided die:
    - Rounded square body with hairline outline (`0xff283141` default, accent highlight on hover).
    - Vector circular pips (dots) representing a die face (e.g. 5 or 6 pips) in high-contrast cyan/accent color.
    - Tactile 1px press offset on click.
  - Replace `juce::TextButton diceButton { "d6" }` in `ModuleCardComponent` with `DiceButton`.
  - Replace `juce::TextButton globalDiceButton { "d6" }` in `FarmerEditor` with `DiceButton`.
- [x] Replace OS-style PopupMenu with sleek Popover CalloutBox on right-click:
  - In `RotaryKnobSlider::mouseDown`: When `e.mods.isRightButtonDown() || e.mods.isPopupMenu()`, open `SliderCalloutComponent` inside `juce::CallOutBox` directly at the control's bounds instead of `showContextMenu()` OS popup.
  - In `ModuleCardComponent::mouseDown`: On right-click anywhere on the card header/body, launch a `CardInspectorPopover` in `juce::CallOutBox` anchored to the card, exposing card-level secondary parameters and quick actions.

### Phase 3: The Extended Theme Engine & Settings Modal Quick-Change Swatches
- [x] Define 7 curated themes in `assets/themes/theme.json`:
  1. `cyberpunk` (Default / High Contrast): Electric cyan `#38bdf8`, solar amber `#f59e0b`, acid lime `#10b981`, hot magenta `#ec4899`, dark anthracite `#0d1117`.
  2. `cykranosh` (Creator's Signature / Studio Anti-Fatigue): Deep cosmic slate `#18202c`, ghostly teal `#4ebeb1`, arctic ice blue `#7dd3fc`, starlight silver `#94a3b8`.
  3. `dexciyan` (Abyssal Underwater): Trench black `#050c18`, deep ocean blue `#0b192e`, bioluminescent cyan `#00f0ff`, kelp seafoam `#00bfa5`, coral `#ff7043`.
  4. `boring` (Minimalist Console / Dieter Rams): Matte graphite `#1a1a1a`, concrete cards `#242424`, monochromatic signal amber `#e5a93b`, neutral light grey `#d4d4d4`.
  5. `matrix_green` (P1 Phosphor CRT): True black `#040804`, phosphor shadow cards `#0a140a`, radioactive green `#22c55e` across all accents.
  6. `amber_crt` (P3 Phosphor CRT): Warm black `#080502`, dark amber cards `#170e05`, glowing amber `#f59e0b` across all accents.
  7. `tracker_ft2` (FastTracker II Classic): Tracker midnight blue `#040a1c`, pattern blue `#0c1938`, gold `#facc15` and pale cyan `#38bdf8`.
- [x] Implement live theme switching in `ParameterManager`:
  - Add `void loadTheme(const juce::String& themeId)`.
  - Add `void applyCustomTint(juce::Colour bg, juce::Colour accent)`.
- [x] Add Theme Selector & Quick-Change Swatches in `source/SettingsModal.h` & `source/SettingsModal.cpp`:
  - `juce::ComboBox themeBox` listing all 7 themes with instant live switching.
  - Quick-Change Accent Swatch Row: Clickable colored pills (`[Cyan]`, `[Amber]`, `[Emerald]`, `[Magenta]`, `[Coral]`, `[Gold]`, `[Ice]`) to quickly override the master accent color without restarting.
  - Quick-Change Background Swatch Row: `[Pure Black]`, `[Deep Slate]`, `[Abyssal Navy]`, `[Concrete]`.
  - Repaints parent editor and all sub-components synchronously on change.

### Phase 4: Rendered Text Bounds Audit & Truncation Elimination
- [x] Diagnose and eliminate text clipping in custom-drawn components:
  - In `DiagramSlider` / `RotaryKnobSlider`: Audit label layout and value text formatting. Ensure parameter names and formatted value strings have adequate horizontal padding and do not collide with meter troughs.
  - In `ModuleCardComponent`: Ensure card header titles and status labels have at least 8px right padding before dice/lock buttons.
  - In `StatusBarComponent`: Verify text bounding boxes do not clip on long parameter descriptions across 100%, 125%, 150%, and 200% display scaling.
- [x] Expand `TextTruncationAuditSuite` in `test/HardeningSuites.h`:
  - Add measurement checks for custom-rendered text components (`DiagramSlider`, `ModuleCardComponent` titles, `HeaderMeterComponent`).
  - Verify that across 800x600, 1040x740, and 1400x900 window sizes, zero text elements clip or get truncated.

### Phase 5: Automated Verification & Dual-Configuration Gauntlet
- [x] Author automated regression unit tests in `test/FarmerTestSuite.h`:
  - `testDoubleClickOpensEditorNotReset`: Asserts double-clicking a slider leaves its value unchanged and opens the callout editor.
  - `testAltClickResetsToDefault`: Asserts Alt-clicking a slider cleanly resets its value to default.
  - `testAuditionTriggerProducesAudio`: Calls `triggerAudition()` on the processor and asserts that `processBlock()` renders non-silent audio energy (> 0.05 peak).
  - `testDiceButtonVectorRendering`: Verifies `DiceButton` paints correctly without error.
- [x] Run full dual-configuration validation:
  - **Debug Build**: `cmake -B build -DCMAKE_BUILD_TYPE=Debug` & run `dsp_tests.exe` and `gui_tests.exe`.
  - **Release Build**: `cmake --build build --config Release` & run `dsp_tests.exe` and `gui_tests.exe`.
  - Execute `deploy.ps1` to place fresh binaries into `current_build/` and system VST3 directory.

---

## Acceptance Criteria
- [x] Double-clicking any slider or knob opens the Smart Value text entry editor without resetting the parameter value.
- [x] Alt-clicking any slider or knob resets it to its default value.
- [x] The "d6" text button is replaced with a sleek vector 6-sided die icon with pips on both module cards and the header.
- [x] Right-clicking any slider opens the floating `juce::CallOutBox` popover instead of an OS-style popup menu.
- [x] Right-clicking a module card opens the Card Inspector Popover.
- [x] Clicking the `TRIGGER` button generates an audition drum hit with audible sound and peak meter activity.
- [x] Zero text clipping or truncation across all cards, sliders, and headers across all DPI scales.
- [x] 100% pass across all unit tests in both Debug and Release configurations.
