# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized milestones below:
> 1. **v0.3.0 (Architecture)**: The Klang Editor (TKE), Codebase Cruft Purge, & v0.2.0 Parity Audit.
> 2. **v0.4.0 (Sound & Chaos)**: New Effects Expansion, Dual Sample Players, Parameter Randomization, & Typography Engine.
> 3. **v0.5.0 (Pro Workflow)**: JSON Preset Browser, WAV Render, & Instant DAW Drag 'n' Drop.
> 4. **v0.6.0 (Hardware & Embedded)**: The Klang Seed (TKS) & TBD-16 Multi-Page FX Controller.
> 5. **Spin-Offs**: The Klang Mill (TKM) Standalone FX Rack.


> [!TIP]
> **CODE QUALITY STANDARD**: The C++ codebase currently maintains an A+ standard for defensive programming, descriptive `camelCase` variable naming, and explicit algorithmic comments (e.g., documenting DSP math curves directly above the function). All future contributions must rigidly match this level of in-line documentation and readability!

---

## 🚀 Milestone: v0.3.0 "The Architecture Update"
*Focus: Tooling, Data-Driven Architecture, and 1:1 Legacy Parity.*

### 1. Standalone JSON Data & Theme Editor (TheKlangEditor)
*Detailed Plan: [`docs/json_editor_tool_plan.md`](json_editor_tool_plan.md)*
Create a dedicated JUCE GUI application using a **Unified Tabbed Layout** (`[THEME]` vs `[CONTROLS]`) to cleanly separate visual styling from DSP parameter calibration, sharing a Live UI Preview and Raw JSON text editor pane.
*Note: Ensure the UI can handle the split between TKF (The Klang Farmer) and TKP (The Klang Planter) cards if they have different parameter groupings or coloring needs.*
*Text/Localization Extraction*: Extract all hardcoded C++ UI strings (slider names, tooltips, special formatting) into their respective module JSON files (using the **Component Model** architecture to explicitly keep text out of structural layout files). Global text (button labels, plugin names) will live in a new `global_ui.json` file. All text will be edited directly alongside numerical bounds inside the `[CONTROLS]` tab.
*Automated Build Tracking*: Use CMake to inject the Git Commit Count and Hash into the C++ preprocessor, displaying it in the Editor's title bar (e.g., `v0.3.0 (Build 171 - 155333f)`) while strictly avoiding timestamps to preserve Reproducible Builds.

### 2. Automated GUI Test Harness (Guardrail)
*Detailed Plan: [`docs/gui_test_harness_plan.md`](gui_test_harness_plan.md)*
Implement a functional state C++ testing harness (`gui_tests`) to simulate clicks and verify APVTS bindings. Integrated directly into the `/build-validate` skill to act as an industry-standard development guardrail (can be bypassed with `--skip-gui`).

### 3. Automated C++ Linting & Formatting (`clang-format`)
*Goal: Enforce the project's A+ code quality standards automatically.*
- Generate a `.clang-format` file matching the existing 4-space indent and camelCase style rules.
- Integrate into the local build process so code is mechanically standardized before compilation.

### 4. Comprehensive Codebase Cruft Purge
*Goal: Remove all orphaned UI components, unused classes, and hardcoded variables rendered obsolete by the JSON transition.*
- **Hard Deletion**: Aggressively delete all dead code from the C++ source files (relying entirely on Git for the archive).
- **Sequencing**: Must be executed *before* the Parity Audit to mathematically prove the purged cruft was not structurally load-bearing.

### 5. Automated v0.2.0 Parity Audit
*Detailed Plan: [`docs/v020_parity_audit_plan.md`](v020_parity_audit_plan.md)*
Run an automated Python script to extract legacy v0.2.0 C++ parameters, string formatters, and hex colors, and cross-reference them against the new JSON architecture to ensure 1:1 user parity. Outputs a highlighted HTML/PDF report with embedded card screenshots.

### 6. GitHub CLI Integration & Repository Tagging
*Goal: Improve repository discoverability for audio-plugin developers and the vibe coding community.*
- Have the AI install the GitHub CLI (`gh`) via `winget` and authenticate.
- Automatically apply curated repository tags (Topics) covering Audio Plugin/JUCE (`vst3`, `juce-framework`), FM Drum Synthesis (`drum-machine`, `fm-synthesis`), and the Vibe Coding (`vibe-coding`, `agentic-coding`) communities.

---

## 🚀 Milestone: v0.4.0 "The Sound & Chaos Update"
*Focus: Sonic Expansion, Workflow Disruption, and Modulation.*

### 3. Parameter Randomization Engine (d6)
Add a fully JSON-driven contextual randomization system:
- **d6 Icon UI**: Placed on the top-right of every Card (randomizes card), every Page (randomizes page), the global Header (randomizes synth), and the FX selection card (randomizes FX selectors).
- **Right-Click Modal**: Sets the 'Depth' (5%, 15%, 25%, 50%, 75%, 100%).
- **JSON Driven**: The 6 depth strings and 6 visual colors (e.g. Green to Red gradient) are explicitly stored in `global_ui.json` under `"global_strings"` and `"global_colors"`.
- **Randomization Logic**:
  - **Discrete Selectors**: Probability Flip (Depth percentage defines the literal chance that the selector randomly flips to a new choice).
  - **Continuous Sliders**: Incremental Jitter (Slider randomly shifts up to $\pm$Depth% away from its *current* position).

### 4. New Effects Processors Catalog Expansion (Effects 14–26) & Universal Mix Standard
*Detailed Plan: [`docs/new_effects_plan.md`](new_effects_plan.md)*  
Expand the FX catalog from 13 to 26 algorithms (appended as indices 14–26 for 100% backward preset compatibility) and standardize Knob 4 across all modulation/time-based FX to the Universal Dual-Mode Mix:
- **Phase 1: Universal Dual-Mode Mix Helper & Core Enums**:
  - Implement shared computeDualModeMix(float normParam, float& dryGain, float& wetGain) in source/DSPBlock.h (-100% wet crossfade -> 0% pure dry -> +100% parallel additive blend).
  - Update createFXBlock() factory and BlockType enum in source/ModularBlocks.h with TransientShaper (14), CustomWaveshaper (15), ChannelMixer (16), StereoEnhancer (17), HaasDelay (18), GatedReverb (19), JunoChorus (20), WaveguideResonator (21), SubGenerator (22), TapeWarmth (23), DynamicFilter (24), PitchTransposer (25), and StutterGate (26).
  - Register algorithms in PluginProcessor.cpp and PlanterProcessor.cpp fxChoices list.
- **Phase 2: DSP Implementations (source/ModularBlocks.h)**:
  - TransientShaperBlock: Bipolar Attack, Pump, Sustain, Speed with stereo-linked envelope detector.
  - CustomWaveshaperBlock: Morphing transfer function, Drive, Pre-DJ Filter tilt, Universal Mix.
  - ChannelMixerBlock: 4-quadrant matrix mixer with unity defaults.
  - StereoEnhancerBlock: Mid/Side balance, piecewise width, Pan, and per-trigger analog Slop drift.
  - HaasDelayBlock: Bipolar circular delay, Tone 6 dB/oct tilt, cross-feedback, and parallel blend phase protection.
  - GatedReverbBlock: 8-tap diffuser, 12-bit lo-fi damping, deterministic note-trigger sample countdown gate with micro-fade, Universal Mix.
- **Phase 3: Standardize Existing FX Mix Knobs**:
  - Migrate Chorus, Comb, Flanger, Phaser, Tempo Delay, and Drive to use computeDualModeMix.
- **Phase 4: UI / UX Integration (source/UIComponents.cpp)**:
  - Add parameter labels, units, and ranges in TooltipHelper.
  - Add custom quick-snap presets for all 6 new effects in SliderCalloutComponent.
- **Phase 5: Automated DSP Unit Tests (test/dsp_tests.cpp)**:
  - Verification suite testing zero-allocation rendering, dual-mode mix curve math, and stereo image preservation.

### 5. Dual Sample Players for Noise Transient Page (Plugin Only)
- Add two dedicated sample player modules to the Transients page (desktop plugin specific).
- **Controls per Player**:
  1. **File Picker**: File browser / drag-and-drop audio file loader.
  2. **Play Speed**: Bipolar playback speed with reverse: -400% -> 0% -> +400%.
  3. **Decay Time**: Percussive sample amplitude decay envelope.
  4. **Level**: Output gain level.
- **Choke / Split Modal**: Modal dialog to configure split/choke groups (e.g. allowing one player to be an open hi-hat and the other a closed hi-hat that chokes the open sound).

### 6. Advanced Typography Engine (JUCE 9)
*Detailed Plan: [`docs/typography_engine_plan.md`](typography_engine_plan.md)*
Implement a JSON-driven, CSS-class style typography system utilizing JUCE 9's advanced text rendering pipeline.
- **Embedded Binary Assets**: `.ttf`/`.otf` files are baked into `BinaryData` for 100% cross-platform consistency.
- **CSS-Style JSON Classes**: Define global text profiles (e.g., `HeaderStyle`, `TooltipStyle`) in the layout JSON, exposing Font Family, Size, Weight, Tracking (letter-spacing), and Justification.
- **Editor Integration**: The JSON Editor tool provides sliders/fields to instantly visualize tracking and weight changes across the UI.


### 7. Continuous Fuzz Testing (DSP Stability)
*Goal: Guarantee absolute DSP stability during extreme generative parameter changes.*
- Implement an automated fuzzing harness that blasts the `processBlock` and `apvts` with randomized, out-of-bounds, and extreme NaN garbage data to mathematically ensure the synth will never crash a host DAW.


### 8. Selector Right-Click Callout 'Reset to Default' Action
*Goal: Provide instant, discoverable default reset capability for discrete selector buttons.*
- Add a top/bottom action button `[Default: <Preset/Mode>]` inside `SelectorCalloutComponent` / right-click menu.
- Clicking the button instantly restores the selector parameter to its JSON-defined default value.


### 9. Categorized FX Selection Modal (Kilohearts Snapin-Style Browser)
*Goal: Fast, visual selection across all 26 effects without scrolling linear dropdowns.*
- **Trigger Interaction**: Left-clicking the effect title on any FX Card opens the modal; `<` and `>` arrow steppers continue to cycle sequentially.
- **5-Column Categorized Layout (Alphabetical within columns)**:
  - **Dynamics & Gain (5)**: Channel Mixer, Drive, Stutter Gate, Sub Generator, Transient Shaper
  - **Filters & Tone (4)**: Bell EQ, Comb Filter, Dynamic Filter, Filter (SVF)
  - **Modulation & Pitch (6)**: Chorus, Flanger, Frequency Shifter, Juno Chorus, Phaser, Pitch Transposer
  - **Delay & Space (5)**: Gated Reverb, Haas Delay, Phase Smear, Tempo Delay, Waveguide Resonator
  - **Lo-Fi & Character (6)**: Custom Waveshaper, Grit FX, RingMod, Stereo Enhancer, Tape Warmth, Wave Folder
- **Visual Polish**: Keyboard escape to dismiss, mouse hover highlights with algorithm descriptions, and currently loaded effect highlighted with active indicator dot.

---

## 🚀 Milestone: v0.5.0 "The Pro Workflow Update"


*Focus: Professional DAW Integration, File Management, and Export.*

### 1. JSON Preset Browser, Tagging & State Migration
*Detailed Plan: [`docs/preset_system_plan.md`](preset_system_plan.md)*  
Implement a professional, tag-based preset management system utilizing JSON files for storage.
- **Phase 1: JSON Schema & StateMigrator (source/PresetManager.h, source/StateMigrator.h)**:
  - Background scanner to instantly build a database from metadata headers without loading full state.
  - Intercept older patches via StateMigrator to inject missing default values.
- **Phase 2: UI Browser Overlay (source/PresetBrowserComponent.h)**:
  - Dual-column UI (Tags on Left, Results on Right) with fuzzy text search.
  - "Save As" modal with text inputs for name, author, and tokenized tags.
- **Phase 3: Header Integration & Automated Tests**:
  - LCD-style preset display and < > stepper buttons in the main header.

### 2. WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)
*Detailed Plan: [`docs/wav_render_sf2_export_dragndrop_plan.md`](wav_render_sf2_export_dragndrop_plan.md)*  
Comprehensive offline audio bounce, multi-sample SoundFont 2 (.sf2) bank generation, and zero-friction DAW integration:
- **Phase 1: Offline Render Pipeline & SF2 Builder**:
  - Zero-dependency RIFF sfbk v2.01 binary builder with L/R linked sample headers.
  - Non-blocking atomic note/velocity tracking.
- **Phase 2: Instant DAW Drag 'n' Drop ("Tekno-Style" Header Badge)**:
  - InstantDragBadgeComponent in header displaying miniature waveform preview of the last hit.
- **Phase 3: WAV Render & Export Modal Dialog**:
  - Target mode selection: Last Auditioned Note vs Multi-Sample Range.
  - Format selection: WAV files folder, SoundFont 2 (.sf2) bank, or both.
- **Phase 4: Header Integration & Automated Unit Tests**:
  - Add renderButton and dragBadge to plugin headers.

### 3. Automated GitHub Release Version Checker & Settings Modal
*Goal: Provide seamless, non-intrusive notification of new releases directly inside the plugin.*
- **Non-Blocking Background Worker**: Uses an async background thread (`juce::Thread` / `juce::URL`) to query GitHub's latest release API (`https://api.github.com/repos/.../releases/latest`) on plugin load. Zero audio thread or UI stalling.
- **Header Notification Badge**: When a newer semver tag is detected, a subtle, glowing 'Update Available' badge appears next to the version text in the header. Clicking opens the release URL in the system browser.
- **Settings & About Modal**: 
  - Accessed via a new gear / info icon in the header.
  - Contains: 'Check for updates on launch' toggle (persisted in config JSON), manual 'Check for Updates Now' button, and current build metadata.

### 4. Automated macOS Notarization & Code Signing
*Goal: Prepare the final binaries for commercial distribution.*
- Integrate a code-signing and Apple Notarization pipeline so the VST3/AU binaries clear macOS Gatekeeper and Windows SmartScreen without throwing 'unidentified developer' warnings to users.

---

## 🚀 Spin-Off Products & Explorations

### 8. The Klang Mill (Standalone VST) — Snap Heap-Style 2x6 Multi-FX & Vital-Style Matrix
*Detailed Plan: [`docs/the_klang_mill_plan.md`](the_klang_mill_plan.md)*
Create a standalone multi-effects VST3 plugin with a 2-row modular chassis and comprehensive routing matrix:
- **Row 1 (Effects Path)**: 4 Multi-FX slots (26 algorithms) + Master Limiter + Input/Output routing.
- **Row 2 (Modulation Rack)**: Multi-wave LFO, Envelope Follower, Random/S&H, Macro controllers, and Transient Gate.
- **Drag-and-Drop Modulation**: Drag crosshairs onto effect knobs with animated colored sweep arcs.
- **Vital-Style `[ MATRIX ]` Tab**: Full-screen tabular routing page exposing Source, Bipolar Depth, Destination, Aux/Scale-By modulator, transfer curve, bypass, and delete.

### 9. 2x5 Eurorack Modular Layout Exploration
- Investigate moving from the current 2x4 (8-card) chassis to an expanded **2-row by 5-column (2x5, 10-card)** layout.




### 12. Zynthian V5 / V4 Standalone Hardware Port (TENTATIVE)
*Detailed Plan: [`docs/zynthian_port_plan.md`](zynthian_port_plan.md)*
Tentative exploration to deploy headless Linux LV2 / CLAP plugins onto the open-source Zynthian hardware ecosystem (Raspberry Pi 5 ARM64):
- **Zero GUI Overhead**: Compiles purely the audio processor and APVTS state tree with no desktop GUI overhead.
- **1:1 4-Encoder Page Mapping**: Each 4-knob card in our engine corresponds directly to one 4-encoder screen page on Zynthian V5's display, controlled by its 4 physical push-rotary optical encoders.
- **Native Zynthian Host**: Leverages ZynthianOS for native preset storage, MIDI routing, and snapshot management.

---

## 📁 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in `docs/completed_plans/`)*
