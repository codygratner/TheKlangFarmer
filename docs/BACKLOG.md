# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized milestones below:
> 1. **v0.3.0 (Architecture)**: The Klang Editor (TKE), Codebase Cruft Purge, & v0.2.0 Parity Audit.
> 2. **v0.4.0 (Sound & Chaos)**: 26-Effects Catalog & Browser Modal, Dual Sample Players, Parameter Randomization, & Typography Engine.
> 3. **v0.5.0 (Pro Workflow)**: JSON Preset Browser & Sound Design Library, WAV Render / SF2 Export, GitHub Version Checker, & Linux Headless CI.
> 4. **v0.6.0 (Hardware & Embedded)**: The Klang Seed (TKS Teensy/Daisy), TBD-16 16-Encoder Controller, & Zynthian V5 Port.
> 5. **Spin-Offs**: The Klang Mill (TKM 1x6 Pedalboard Rack).

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

### 1. Parameter Randomization Engine (d6)
Add a fully JSON-driven contextual randomization system:
- **d6 Icon UI**: Placed on the top-right of every Card (randomizes card), every Page (randomizes page), the global Header (randomizes synth), and the FX selection card (randomizes FX selectors).
- **Right-Click Modal**: Sets the 'Depth' (5%, 15%, 25%, 50%, 75%, 100%).
- **JSON Driven**: The 6 depth strings and 6 visual colors (e.g. Green to Red gradient) are explicitly stored in `global_ui.json` under `"global_strings"` and `"global_colors"`.
- **Randomization Logic**:
  - **Discrete Selectors**: Probability Flip (Depth percentage defines the literal chance that the selector randomly flips to a new choice).
  - **Continuous Sliders**: Incremental Jitter (Slider randomly shifts up to $\pm$Depth% away from its *current* position).

### 2. New Effects Processors Catalog Expansion (Effects 14–26), Universal Mix, & 5-Column Browser Modal
*Detailed Plan: [`docs/new_effects_plan.md`](new_effects_plan.md)*  
Expand the FX catalog from 13 to 26 algorithms (appended as indices 14–26 for 100% backward preset compatibility) and bundle the Kilohearts-style 5-column categorized modal browser:
- **Phase 1: Universal Dual-Mode Mix Helper & Core Enums**:
  - Implement shared `computeDualModeMix(float normParam, float& dryGain, float& wetGain)` in `source/DSPBlock.h` (-100% wet crossfade $\to$ 0% pure dry $\to$ +100% parallel additive blend).
  - Update `createFXBlock()` factory and `BlockType` enum with: TransientShaper (14), CustomWaveshaper (15), ChannelMixer (16), StereoEnhancer (17), HaasDelay (18), GatedReverb (19), JunoChorus (20), WaveguideResonator (21), SubGenerator (22), TapeWarmth (23), DynamicFilter (24), PitchTransposer (25), and StutterGate (26).
- **Phase 2: DSP Implementations (`source/ModularBlocks.h`)**:
  - TransientShaperBlock (14), CustomWaveshaperBlock (15), ChannelMixerBlock (16), StereoEnhancerBlock (17), HaasDelayBlock (18), GatedReverbBlock (19), JunoChorusBlock (20), WaveguideResonatorBlock (21), SubGeneratorBlock (22), TapeWarmthBlock (23), DynamicFilterBlock (24), PitchTransposerBlock (25), and StutterGateBlock (26).
- **Phase 3: Standardize Existing FX Mix Knobs**:
  - Migrate Chorus, Comb, Flanger, Phaser, Tempo Delay, and Drive to use `computeDualModeMix`.
- **Phase 4: Categorized FX Selection Modal (Kilohearts-Style Browser)**:
  - 5-Column Categorized Modal: Dynamics & Gain, Filters & Tone, Modulation & Pitch, Delay & Space, Lo-Fi & Character.
  - Left-click on FX card header launches browser modal; `<` / `>` steppers continue to cycle sequentially.
- **Phase 5: Automated DSP Unit Tests (`test/dsp_tests.cpp`)**:
  - Zero-allocation verification suite testing dual-mode mix curves and stereo imaging.

### 3. Dual Sample Players for Noise Transient Page (Plugin Only)
- Add two dedicated sample player modules to the Transients page (desktop plugin specific).
- **Controls per Player**:
  1. **File Picker**: File browser / drag-and-drop audio file loader.
  2. **Play Speed**: Bipolar playback speed with reverse: -400% $\to$ 0% $\to$ +400%.
  3. **Decay Time**: Percussive sample amplitude decay envelope.
  4. **Level**: Output gain level.
- **Choke / Split Modal**: Modal dialog to configure split/choke groups (e.g. allowing one player to be an open hi-hat and the other a closed hi-hat that chokes the open sound).

### 4. Advanced Typography Engine (JUCE 9)
*Detailed Plan: [`docs/typography_engine_plan.md`](typography_engine_plan.md)*
Implement a JSON-driven, CSS-class style typography system utilizing JUCE 9's advanced text rendering pipeline.
- **Embedded Binary Assets**: `.ttf`/`.otf` files are baked into `BinaryData` for 100% cross-platform consistency.
- **CSS-Style JSON Classes**: Define global text profiles (e.g., `HeaderStyle`, `TooltipStyle`) in the layout JSON, exposing Font Family, Size, Weight, Tracking (letter-spacing), and Justification.
- **Editor Integration**: The JSON Editor tool provides sliders/fields to instantly visualize tracking and weight changes across the UI.

### 5. Continuous Fuzz Testing (DSP Stability)
*Goal: Guarantee absolute DSP stability during extreme generative parameter changes.*
- Implement an automated fuzzing harness that blasts the `processBlock` and `apvts` with randomized, out-of-bounds, and extreme NaN garbage data to mathematically ensure the synth will never crash a host DAW.

### 6. Selector Right-Click Callout 'Reset to Default' Action
*Goal: Provide instant, discoverable default reset capability for discrete selector buttons.*
- Add a top/bottom action button `[Default: <Preset/Mode>]` inside `SelectorCalloutComponent` / right-click menu.
- Clicking the button instantly restores the selector parameter to its JSON-defined default value.

---

## 🚀 Milestone: v0.5.0 "The Pro Workflow Update"
*Focus: Professional DAW Integration, File Management, Preset Library, and Export.*

### 1. JSON Preset Browser, Tagging & State Migration
*Detailed Plan: [`docs/preset_system_plan.md`](preset_system_plan.md)*  
Implement a professional, tag-based preset management system utilizing JSON files for storage.
- **Phase 1: JSON Schema & StateMigrator (`source/PresetManager.h`, `source/StateMigrator.h`)**:
  - Background scanner to instantly build a database from metadata headers without loading full state.
  - Intercept older patches via StateMigrator to inject missing default values.
- **Phase 2: UI Browser Overlay (`source/PresetBrowserComponent.h`)**:
  - Dual-column UI (Tags on Left, Results on Right) with fuzzy text search.
  - "Save As" modal with text inputs for name, author, and tokenized tags.
- **Phase 3: Header Integration & Automated Tests**:
  - LCD-style preset display and `<` `>` stepper buttons in the main header.

### 2. Curated Factory Preset Library & Sound Design Pack (64–128 Patches)
*Goal: Provide professional out-of-the-box sounds showcasing the expanded DSP and sample players.*
- Author 64–128 production-ready drum patches categorised across:
  - **Kicks**: Sub-heavy 808s, punchy acoustic-style kicks, hardstyle industrial distortion kicks.
  - **Snares & Claps**: Metallic FM snares, 80s gated reverb claps, organic transient layers.
  - **Toms & Percs**: Physical modeling resonant tubes, FM bells, alien zaps, and cowbells.
  - **Hi-Hats & Cymbals**: Transient sample-layered hats, choked pairs, and FM metallic cymbal washes.

### 3. One-Click Preset Bank Sharing (`.tkfbank` / `.zip` Import/Export)
*Goal: Zero-friction sharing of user presets and community expansion packs.*
- **Export Bank**: Bundles selected presets, tags, and custom transient sample files into a single compressed `.tkfbank` archive.
- **Import Bank**: Drag-and-drop `.tkfbank` file onto the preset browser to automatically install, categorize, and rebuild the tag cache.

### 4. WAV Render, Multi-Sample & SF2 Export Dialog + Instant DAW Drag 'n' Drop (Features #16 & #17)
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

### 5. Automated GitHub Release Version Checker & Settings Modal
*Goal: Provide seamless, non-intrusive notification of new releases directly inside the plugin.*
- **Non-Blocking Background Worker**: Async background thread querying GitHub release API on plugin load.
- **Header Notification Badge**: Subtle, glowing 'Update Available' tag next to version text. Clicking opens release page.
- **Settings & About Modal**: Gear icon in header exposing 'Check for updates on launch' toggle, manual 'Check Now' button, and build metadata.

### 6. Automated macOS Notarization & Code Signing Pipeline
*Goal: Prepare final binaries for public distribution.*
- Integrate Apple Developer ID code-signing and `notarytool` automated ticket stapling in CI so AU/VST3 binaries pass macOS Gatekeeper without warnings.
- Integrate Windows Authenticode code-signing for SmartScreen trust.

### 7. Headless Linux CLAP / VST3 Automated CI/CD Runner
*Goal: Ensure multi-platform stability and continuous validation for Linux audio.*
- Add an Ubuntu `aarch64` / `x86_64` container to GitHub Actions building headless Linux CLAP/VST3 binaries on every commit.

---

## 🚀 Milestone: v0.6.0 "The Hardware & Embedded Update"
*Focus: Standalone Hardware Synthesizer (The Klang Seed), dadamachines tbd-16 Integration, and Zynthian V5 Linux Port.*

### 1. dadamachines tbd-16 Groovebox Integration
*Detailed Plan: [`docs/tbd16_klang_seed_effects_plan.md`](tbd16_klang_seed_effects_plan.md)*
Deploy the pure C++ DSP engine onto the open-source **dadamachines tbd-16** platform:
- **Architecture**: Dual-core **ESP32-P4 RISC-V @ 400 MHz** (Audio DSP) + **RP2350B @ 150 MHz** (UI, Sequencer, 2.4" OLED, 30 RGB buttons) + **ESP32-C6** (Wi-Fi/Ableton Link).
- **Native 4-Encoder Mapping**: The unit features **4 endless push-encoders**; each 4-knob card and 4-knob FX slot in our engine maps directly to one 4-encoder screen page on its 2.4" OLED!

### 2. The Klang Seed (TKS) Standalone Hardware Synthesizer
*Detailed Plan: [`docs/embedded_dsp_and_hardware_port_plan.md`](embedded_dsp_and_hardware_port_plan.md)*
Port the zero-dependency pure C++ DSP engine to dedicated DIY embedded platforms:
- **Daisy Seed (Electro-Smith)**: STM32H750 ARM Cortex-M7 @ 480 MHz with 64 MB onboard SDRAM and integrated AK4556 stereo codec. Purpose-built for plug-and-play stereo desktop units and Eurorack modules.
- **Teensy 4.1 (PJRC)**: NXP i.MX RT1062 ARM Cortex-M7 @ 600 MHz running 8 mono drum voices (~14.7% CPU load) with CS42448 8-channel DAC delivering **8 discrete physical 1/4" voice outputs** for studio outboard processing.

### 3. Zynthian V5 / V4 Standalone Hardware Port (TENTATIVE)
*Detailed Plan: [`docs/zynthian_port_plan.md`](zynthian_port_plan.md)*
Deploy headless Linux LV2 / CLAP plugins onto the open-source Zynthian hardware ecosystem:
- **Compute**: Raspberry Pi 5 (Quad-core ARM Cortex-A76 @ 2.4 GHz) running 64-bit ZynthianOS.
- **Zero GUI Overhead**: Pure headless real-time DSP without X11/OpenGL overhead.
- **1:1 4-Encoder Page Mapping**: Maps 1:1 onto Zynthian V5's 4 physical optical push-encoders and 800x480 touchscreen.

---

## 🚀 Spin-Off Products & Explorations

### 1. The Klang Mill (Standalone VST) — Industrial 1x6 Multi-FX Pedalboard Rack
*Detailed Plan: [`docs/the_klang_mill_plan.md`](the_klang_mill_plan.md)*
Create a standalone multi-effects VST3 plugin styled after vintage studio rackmounts and boutique pedalboards (e.g., Soundtoys Effect Rack):
- **1x6 Horizontal Chassis**: Input/Slop $\to$ 4 Serial Multi-FX Pedal Slots (26 algorithms) $\to$ Master Limiter & Output.
- **Immediate & Tactile**: Zero routing matrices or drag-and-drop clutter; dedicated stomp bypasses per slot.
- **Global Slop**: Injects organic, non-linear analog drift across all 4 pedals for instant vintage character.
- **Codebase Integration**: Built as a sibling build target (`TheKlangMill_VST3`) inheriting directly from `KlangCoreProcessor` and `KlangCoreEditor`.

---

## 📁 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in `docs/completed_plans/`)*
