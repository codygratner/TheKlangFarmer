# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized items below:
> 1. **Priority #1 (UX / Modal & Tooltips)**: Quick-Snap Presets & Tooltips in Edit Modal (`SliderCalloutComponent`).
> 2. **Priority #2 (Refactoring / Architecture)**: Extract Tooltips & Text into JSON (`StringManager` & CMake `juce_add_binary_data`).
> 3. **Priority #3 (Export & DAW Integration)**: WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17).
> 4. **Priority #4 (DSP / FX Expansion)**: New Effects Processors Catalog Expansion (Effects 14–19) & Universal Mix Standard (`docs/new_effects_plan.md`).
> 5. **Priority #5 (Transient Sampling)**: Dual Sample Players for Noise Transient Page (Plugin Only).

---

## 📌 Top Priorities for Upcoming Sessions

### 1. [PRIORITY #1] Quick-Snap Presets & Tooltips in Edit Modal (`SliderCalloutComponent`)
Inject dynamic parameter tooltips and contextual quick-snap preset buttons directly into the right-click `SliderCalloutComponent` modal:
- **Phase 1: Header Additions (`source/UIComponents.h`)**:
  - Add state variables to hold tooltip string (`tooltipText`) and text layout (`tooltipLayout` for height measurement).
  - Add `QuickPreset` struct (`juce::String label`, `juce::String valueStr`) and active presets vector (`std::vector<QuickPreset> activePresets`).
  - Add custom `PresetButtonLookAndFeel` for sleek preset "pills" (`drawButtonBackground`, `drawButtonText`).
  - Add button storage: `PresetButtonLookAndFeel presetBtnLaf; juce::OwnedArray<juce::TextButton> presetButtons;`.
- **Phase 2: Contextual Presets & Layout (`source/UIComponents.cpp`)**:
  - Dynamically populate `activePresets` based on target slider/parameter:
    - **Waveshape**: Sine (`0.0`), Triangle (`0.25`), Saw (`0.5`), Square (`0.75` / `1.0`)
    - **Pitch / Semitones / Coarse Tune**: `-24`, `-12`, `-7`, `0`, `+7`, `+12`, `+24`
    - **Dual-Mode FX Mix Knobs**: `-100% (Wet)`, `0% (Dry)`, `+100% (Parallel)`
    - **Filter Cutoff**: `60 Hz`, `250 Hz`, `1 kHz`, `3.5 kHz`, `10 kHz`
    - **Filter Resonance / Q**: `0.5`, `0.707`, `1.414`, `4.0`, `10.0`
    - **Envelopes (Decay / Release)**: `10 ms`, `60 ms`, `150 ms`, `600 ms`, `2.0 s`
    - **Tempo Delay & Reverb Gate Times**: `1/16`, `1/8`, `1/8D`, `1/8T`, `1/4`
    - **Stereo Enhancer / Width**: `0% (Mono)`, `100% (Normal)`, `200% (Wide)`, `400% (Hyper-Wide)`
  - Layout: Dynamically compute height accommodating the tooltip, preset pill button bar, slider, and text editor.
  - Clicking a preset pill updates the slider and active text box immediately.

---

### 2. [PRIORITY #2] Extract Tooltips & Text into JSON (`StringManager` & CMake `juce_add_binary_data`)
*Detailed Plan: [`docs/extract_tooltips_text_into_json_plan.md`](extract_tooltips_text_into_json_plan.md)*  
Extract all hardcoded UI text, tooltips, and Quickstart Guide copywriting into a centralized JSON dictionary:
- **Phase 1: Asset Creation & JSON Structure**:
  - Create `assets/en_strings.json` as the single source of truth for copywriting (Quickstart guide, navigation pages, FX algorithms, FX knobs, LED selectors, parameter descriptions).
- **Phase 2: CMake Binary Data Generation**:
  - Configure `juce_add_binary_data(TkfAssets ...)` in `CMakeLists.txt` to bake `en_strings.json` directly into the binary.
  - Link `TkfAssets` to `TheKlangFarmer`, `TheKlangPlanter`, and test binaries.
- **Phase 3: Runtime String Manager**:
  - Implement `StringManager` singleton (`source/StringManager.h`, `source/StringManager.cpp`) parsing JSON on startup and querying strings via dot-delimited key paths (`getString`, `getStringArray`, `getFxKnob`, `getVar`).
- **Phase 4: Refactoring UI Components**:
  - Refactor `TooltipHelper` in `source/UIComponents.cpp` to pull FX knob descriptions, algorithm descriptions, and LED selector tips dynamically from `StringManager`.
- **Phase 5: Refactoring Plugin Editor**:
  - Refactor `PluginEditor.cpp` to replace hardcoded strings in navigation buttons, parameter descriptions (`getFarmerParamDescription`), and `QuickstartGuideModalComponent` panels with dynamic `StringManager` fetches.

---

### 3. [PRIORITY #3] WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)
*Detailed Plan: [`docs/wav_render_sf2_export_dragndrop_plan.md`](wav_render_sf2_export_dragndrop_plan.md)*  
Comprehensive offline audio bounce, multi-sample SoundFont 2 (`.sf2`) bank generation, and zero-friction DAW integration:
- **Phase 1: Offline Render Pipeline, SF2 Builder & Last-Note Tracking**:
  - Shared `OfflineRenderSettings`, `Sf2ZoneSampleEntry`, and `OfflineRenderUtils` (`getMaxRenderSamples`, `postProcessRenderedBuffer`, `writeBufferToWavFile`, `writeSf2BankFile`, `getKeyZoneRange`, `getVelocitySplitRange`) in `source/UIComponents.h`/`.cpp`.
  - Zero-dependency RIFF `sfbk` v2.01 binary builder with 46-sample zero guards, L/R linked sample headers, and multi-preset Round-Robin mapping.
  - Non-blocking atomic note/velocity tracking (`lastTriggeredNote`, `lastTriggeredVelocity`, `triggerGeneration`) and sub-chunk accurate modulation rendering in `PluginProcessor` and `PlanterProcessor`.
- **Phase 2: Instant DAW Drag 'n' Drop ("Tekno-Style" Header Badge)**:
  - `InstantDragBadgeComponent` in header displaying miniature waveform preview of the last hit.
  - Inherit `juce::DragAndDropContainer` in editors; trigger `juce::DragAndDropContainer::performExternalDragDropOfFiles` on mouse drag from local temp cache (`RlyehSound_DragCache`).
- **Phase 3: WAV Render, Multi-Sample & SF2 Export Modal Dialog**:
  - `RenderExportModalComponent` background threaded export dialog (`juce::Thread` + `juce::AsyncUpdater`).
  - Target mode selection: Last Auditioned Note vs Multi-Sample Range (note range, steps, velocity layers, round-robins).
  - Format selection: WAV files folder, SoundFont 2 (`.sf2`) bank, or both.
  - Audio formats: 44.1/48/96 kHz, 16/24/32-bit float, fixed/auto-silence tails (-60/-80 dB), peak normalization.
- **Phase 4: Header Integration & Automated Unit Tests**:
  - Add `renderButton` and `dragBadge` to `TheKlangFarmerAudioProcessorEditor` and `TheKlangPlanterAudioProcessorEditor` header layout.
  - Multi-sample rate, auto-silence trim, Slop PRNG uniqueness, and SF2 RIFF chunk validation unit tests in `test/dsp_tests.cpp`.

---

### 4. [PRIORITY #4] New Effects Processors Catalog Expansion (Effects 14–19) & Universal Mix Standard
*Detailed Plan: [`docs/new_effects_plan.md`](new_effects_plan.md)*  
Expand the FX catalog from 13 to 19 algorithms (appended as indices 14–19 for 100% backward preset compatibility) and standardize Knob 4 across all modulation/time-based FX to the Universal Dual-Mode Mix:
- **Phase 1: Universal Dual-Mode Mix Helper & Core Enums**:
  - Implement shared `computeDualModeMix(float normParam, float& dryGain, float& wetGain)` in `source/DSPBlock.h` (`-100%` wet crossfade ↔ `0%` pure dry ↔ `+100%` parallel additive blend).
  - Update `createFXBlock()` factory and `BlockType` enum in `source/ModularBlocks.h` with `TransientShaper` (14), `CustomWaveshaper` (15), `ChannelMixer` (16), `StereoEnhancer` (17), `HaasDelay` (18), and `GatedReverb` (19).
  - Register algorithms in `PluginProcessor.cpp` and `PlanterProcessor.cpp` `fxChoices` list.
- **Phase 2: DSP Implementations (`source/ModularBlocks.h`)**:
  - `TransientShaperBlock`: Bipolar Attack ($\pm 100\%$), Pump ($0..100\%$), Sustain ($\pm 100\%$), Speed ($0.5..350\,\text{ms}$) with stereo-linked envelope detector.
  - `CustomWaveshaperBlock`: Morphing transfer function (Sine → Tri → Saw → Square → PWM), Drive ($1..20\times$), Pre-DJ Filter tilt, Universal Mix.
  - `ChannelMixerBlock`: 4-quadrant matrix mixer ($L \to L, R \to L, L \to R, R \to R$) with unity defaults $\{1.0, 0.5, 0.5, 1.0\}$.
  - `StereoEnhancerBlock`: Mid/Side balance, piecewise width ($0..100\%$ on $0..0.5$, $100..600\%$ on $0.5..1.0$), Pan, and per-trigger analog Slop drift.
  - `HaasDelayBlock`: Bipolar circular delay ($\pm 100\,\text{ms}$), Tone 6 dB/oct tilt, cross-feedback, and parallel blend phase protection ($0.0$ wet on undelayed channel).
  - `GatedReverbBlock`: 8-tap diffuser, 12-bit lo-fi damping, deterministic note-trigger sample countdown gate (1/64 to 1/2 note) with $3\,\text{ms}$ raised-cosine micro-fade, Universal Mix.
- **Phase 3: Standardize Existing FX Mix Knobs**:
  - Migrate Chorus, Comb, Flanger, Phaser, Tempo Delay, and Drive (replacing DJ Filter on Drive) to use `computeDualModeMix`.
- **Phase 4: UI / UX Integration (`source/UIComponents.cpp`)**:
  - Add parameter labels, units, and ranges in `TooltipHelper::getKnobParamInfo()`.
  - Add algorithm descriptions in `TooltipHelper::getFXBlockTooltip()`.
  - Add custom quick-snap presets for all 6 new effects in `SliderCalloutComponent`.
- **Phase 5: Automated DSP Unit Tests (`test/dsp_tests.cpp`)**:
  - Verification suite testing zero-allocation rendering, dual-mode mix curve math, stereo image preservation, matrix pass-through, and gate silence transitions.

---

### 5. [PRIORITY #5] Dual Sample Players for Noise Transient Page (Plugin Only)
- Add two dedicated sample player modules to the Transients page (desktop plugin specific; not constrained to TBD-16 4-control limits).
- **Controls per Player**:
  1. **File Picker**: File browser / drag-and-drop audio file loader.
  2. **Play Speed**: Bipolar playback speed with reverse: `-400%` ↔ `0%` ↔ `+400%` (defaults and double-clicks to `+100%`).
  3. **Decay Time**: Percussive sample amplitude decay envelope.
  4. **Level**: Output gain level.
- **Choke / Split Modal**: Modal dialog to configure split/choke groups (e.g. allowing one player to be an open hi-hat and the other a closed hi-hat that chokes the open sound).

---

## 🎛️ New Effects Processors (Backlog)
*(Elevated to Active Priority #4: see [Top Priorities section](#4-priority-4-new-effects-processors-catalog-expansion-effects-1419--universal-mix-standard) and [`docs/new_effects_plan.md`](new_effects_plan.md))*

### 7. Transient Shaper Effect
- 4-knob envelope dynamic processor (Kilohearts style):
  1. **Attack**: Boost or attenuate initial transient impact.
  2. **Pump**: Sustained envelope swell and recovery.
  3. **Sustain**: Tail length and body amplification.
  4. **Speed**: Detection envelope attack/release tracking speed.

### 8. Custom Waveshaper Effect
- Uses the current crossfaded waveform morph (sine / tri / saw / square / PWM) as the non-linear transfer function / transform for waveshaping.
- **Controls**:
  1. **Waveshape**: Selects/morphs the shaping curve.
  2. **Drive**: Input pre-gain drive.
  3. **DJ Filter**: Bipolar tilt filter (0% flat center).
  4. **Mix**: Universal dual-mode mix.

### 9. Channel Mixer Effect
- 4-knob cross-channel matrix mixer (Kilohearts style):
  1. **L → L**: Incoming Left to Outgoing Left ($-100\%$ to $+100\%$).
  2. **R → L**: Incoming Right to Outgoing Left ($-100\%$ to $+100\%$).
  3. **L → R**: Incoming Right to Outgoing Right ($-100\%$ to $+100\%$).
  4. **R → R**: Incoming Right to Outgoing Right ($-100\%$ to $+100\%$).

### 10. Stereo Enhancer Effect
- Advanced stereo field shaper:
  1. **Mid**: Mid-channel gain ($0\%$ to $100\%$).
  2. **Width**: Side-channel expansion ($0\%$ to $100\%$ to $600\%$ super-wide).
  3. **Pan**: Stereo balance (Left ↔ Center ↔ Right).
  4. **Slop**: Random analog drift depth ($0\%$ to $100\%$) applied across Mid, Width, and Pan per trigger hit.

### 11. Haas Delay Effect
- Psychoacoustic spatial widener via the Haas effect:
  - **Haas Delay Knob**: Single bipolar delay slider:
    - `-100 ms`: Left channel delayed up to 100 ms (Right arrives first → sound localized Right).
    - `0 ms`: No delay on either channel (Center).
    - `+100 ms`: Right channel delayed up to 100 ms (Left arrives first → sound localized Left).

### 12. Lo-Fi Early Reflections Gated Reverb Effect
- Characterized by dense, vintage, "kinda crap sounding" retro early reflections specifically tailored for 80s-style gated snare and punchy drum sounds.
- **Controls**:
  1. **Time**: Reverb decay time / room size.
  2. **DJ Filter**: Bipolar tilt filter (0% flat center) for reflection high/low spectral shaping.
  3. **Mix**: Universal dual-mode mix (`-100%` pure wet crossfade ↔ `0%` dry ↔ `+100%` parallel additive).
  4. **Gate Time**: BPM-synced gate cutoff time in 64th note increments (1/64, 2/64, 3/64, etc.) to abruptly terminate the reverb tail in sync with the song tempo.

---

## 🖱️ UI / UX & Modal Enhancements (Backlog)

### 13. Right-Click Parameter Edit Modal: Quick-Snap Preset Buttons (Approved Spec)
- **Waveshape Edit Modal**:
  - Add quick preset buttons directly into the right-click edit modal: **Sine** (`0.0`), **Triangle** (`0.25`), **Saw** (`0.5`), and **Square** (`0.75` / `1.0`).
- **Contextual Quick-Snap Buttons Across the Synth**:
  1. **Pitch / Semitones / Coarse Tune**: `-24`, `-12`, `-7`, `0`, `+7`, `+12`, `+24`
  2. **Dual-Mode FX Mix Knobs**: `-100% (Wet)`, `0% (Dry)`, `+100% (Parallel)`
  3. **Filter Cutoff**: `60 Hz (Sub)`, `250 Hz (Warmth)`, `1 kHz (Body)`, `3.5 kHz (Edge)`, `10 kHz (Air)`
  4. **Filter Resonance / Q**: `0.5 (Gentle)`, `0.707 (Flat/Butterworth)`, `1.414 (Musical Peak)`, `4.0 (Ring)`, `10.0 (Self-Osc)`
  5. **Envelopes (Decay / Release)**: `10 ms (Click)`, `60 ms (Tight Snare)`, `150 ms (Punchy Kick)`, `600 ms (808 Boom)`, `2.0 s (Tail)`
  6. **Tempo Delay & Reverb Gate Times**: `1/16`, `1/8`, `1/8D (Dotted)`, `1/8T (Triplet)`, `1/4`
  7. **Stereo Enhancer / Width**: `0% (Mono)`, `100% (Normal)`, `200% (Wide)`, `400% (Hyper-Wide)`
  8. **Sample Playback Speed (Plugin Transient Players)**: `-100% (Reverse)`, `+50% (Half-Speed)`, `+100% (Normal)`, `+200% (Double-Speed)`

---

## 📐 Layout & System Architecture (Backlog)

### 14. 2x5 Eurorack Modular Layout Exploration
- Investigate moving from the current 2x4 (8-card) chassis to an expanded **2-row by 5-column (2x5, 10-card)** layout.

### 15. Hardware Standalone Synthesizer Port (BACK BURNER)
- Put on the back burner per user instruction, but fully architected for execution:
  - **Teensy 4.1 Hardware Drum Machine ("The Klang Planter 8-Voice Hardware")**:
    - NXP i.MX RT1062 ARM Cortex-M7 running at **600 MHz**.
    - 8 Mono Voices takes ~2,000 cycles/sample ($\approx 14.7\%$ CPU load).
    - Multi-channel audio output via Cirrus Logic CS42448 (8-channel 24-bit 192 kHz codec).
    - Switched jack normaling for individual voice outs vs. Master Stereo bus.
    - 8 independent MIDI channels with identical CC mappings.
  - **Daisy Seed (Electro-Smith)**: Alternative open hardware platform ($29, STM32H750 Cortex-M7 @ 480 MHz).
  - **Embedded Core Engine (`embedded/KlangPlanterEmbedded.h`)**: Single-file, zero-allocation DSP engine.

### 16. WAV Render & Multi-Sample Export Dialog
- *Elevated to Active Priority #3 (see [Top Priorities section](#3-priority-3-wav-render-multi-sample--sf2-export-dialog--instant-daw-drag-n-drop-features-16--17) and [`docs/wav_render_sf2_export_dragndrop_plan.md`](wav_render_sf2_export_dragndrop_plan.md))*.

### 17. Instant DAW Drag 'n' Drop (Tekno-Style)
- *Elevated to Active Priority #3 (see [Top Priorities section](#3-priority-3-wav-render-multi-sample--sf2-export-dialog--instant-daw-drag-n-drop-features-16--17) and [`docs/wav_render_sf2_export_dragndrop_plan.md`](wav_render_sf2_export_dragndrop_plan.md))*.

---

## 📦 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in [`docs/completed_plans/`](completed_plans/))*
