# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized items below:
> 1. **Priority #1 (Architecture)**: Core Architecture Base Class Refactor.
> 2. **Priority #2 (UI Architecture)**: Data-Driven UI Layout, Colors, & Groupings (theme.json).
> 3. **Priority #3 (Tooling)**: Standalone JSON Data & Theme Editor (`TheKlangEditor`).
> 4. **Priority #4 (Export & DAW Integration)**: WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17).
> 5. **Priority #5 (DSP / FX Expansion)**: New Effects Processors Catalog Expansion (Effects 14–19) & Universal Mix Standard (`docs/new_effects_plan.md`).
> 6. **Priority #6 (Transient Sampling)**: Dual Sample Players for Noise Transient Page (Plugin Only).

---

## 📌 Top Priorities for Upcoming Sessions

### 1. [PRIORITY #1] Core Architecture Base Class Refactor
*Detailed Plan: [`docs/core_architecture_plan.md`](core_architecture_plan.md)*
Refactor codebase to use a "Common Core" base-class architecture (`KlangCoreProcessor`, `KlangCoreEditor`) to share standard JUCE boilerplate, UI LookAndFeel, preset management, and APVTS loading/saving across `TheKlangFarmer`, `TheKlangPlanter`, and `The Klang Seed` (Hardware).

---

### 2. [PRIORITY #2] Data-Driven UI Layout & Colors (`theme.json`)
Extract the remaining hardcoded UI configuration out of C++ (`PluginEditor.cpp`) into JSON assets (`assets/controls/theme.json` or `layout.json`).
- Move all **Knob Colors** (e.g. `juce::Colour(0xff00d2ff)`) into JSON.
- Move **UI Coordinates & Sizes** (`setBounds(x,y,w,h)`) into JSON.
- Move **Card Groupings** (which knobs belong to which physical "Cards" on the screen) into JSON.
- Hook this up to `ParameterManager` so the UI can be fully skinned and reconfigured dynamically without recompiling C++.

---

### 3. [PRIORITY #3] Standalone JSON Data & Theme Editor (`TheKlangEditor`)
*Detailed Plan: [`docs/json_editor_tool_plan.md`](json_editor_tool_plan.md)*
Create a dedicated JUCE GUI application with a 3-pane live-sync interface (Form Property Panel, Raw JSON Editor, and Live UI Preview) to rapidly author module schemas, init/double-click values, and theme colors without recompiling the main synth.

---

### 4. [PRIORITY #4] WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)
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

### 5. [PRIORITY #5] New Effects Processors Catalog Expansion (Effects 14–19) & Universal Mix Standard
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

### 6. [PRIORITY #6] Dual Sample Players for Noise Transient Page (Plugin Only)
- Add two dedicated sample player modules to the Transients page (desktop plugin specific; not constrained to TBD-16 4-control limits).
- **Controls per Player**:
  1. **File Picker**: File browser / drag-and-drop audio file loader.
  2. **Play Speed**: Bipolar playback speed with reverse: `-400%` ↔ `0%` ↔ `+400%` (defaults and double-clicks to `+100%`).
  3. **Decay Time**: Percussive sample amplitude decay envelope.
  4. **Level**: Output gain level.
- **Choke / Split Modal**: Modal dialog to configure split/choke groups (e.g. allowing one player to be an open hi-hat and the other a closed hi-hat that chokes the open sound).

---

### 7. [PRIORITY #7] JSON Preset Browser, Tagging & State Migration
*Detailed Plan: [`docs/preset_system_plan.md`](preset_system_plan.md)*  
Implement a professional, tag-based preset management system utilizing JSON files for storage, complete with author metadata, tagging, and versioning.
- **Phase 1: JSON Schema & `StateMigrator` (`source/PresetManager.h`, `source/StateMigrator.h`)**:
  - Background `TimeSliceThread` scanner to instantly build a database from `metadata` headers without loading full state.
  - Intercept older patches via `StateMigrator` to inject missing default values, ensuring forward compatibility as the DSP evolves.
- **Phase 2: UI Browser Overlay (`source/PresetBrowserComponent.h`)**:
  - Dual-column UI (Tags on Left, Results on Right) with fuzzy text search.
  - "Save As" modal with text inputs for name, author, and tokenized tags.
- **Phase 3: Header Integration & Automated Tests**:
  - LCD-style preset display and `<` `>` stepper buttons in the main header.
  - Unit tests to verify `StateMigrator` accurately forces transparent defaults on legacy mock presets.

---

## 🖱️ UI / UX & Modal Enhancements (Backlog)

---

## 📐 Layout & System Architecture (Backlog)

### 14. 2x5 Eurorack Modular Layout Exploration
- Investigate moving from the current 2x4 (8-card) chassis to an expanded **2-row by 5-column (2x5, 10-card)** layout.

### 15. The Klang Seed (TKS) Standalone Synthesizer Port (BACK BURNER)
- Put on the back burner per user instruction, but fully architected for execution:
  - **Teensy 4.1 Hardware Drum Machine ("The Klang Seed 8-Voice Hardware")**:
    - NXP i.MX RT1062 ARM Cortex-M7 running at **600 MHz**.
    - 8 Mono Voices takes ~2,000 cycles/sample ($\approx 14.7\%$ CPU load).
    - Multi-channel audio output via Cirrus Logic CS42448 (8-channel 24-bit 192 kHz codec).
    - Switched jack normaling for individual voice outs vs. Master Stereo bus.
    - 8 independent MIDI channels with identical CC mappings.
  - **Daisy Seed (Electro-Smith)**: Alternative open hardware platform ($29, STM32H750 Cortex-M7 @ 480 MHz).
  - **Embedded Core Engine (`embedded/KlangPlanterEmbedded.h`)**: Single-file, zero-allocation DSP engine.

---

## 📦 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in [`docs/completed_plans/`](completed_plans/))*
