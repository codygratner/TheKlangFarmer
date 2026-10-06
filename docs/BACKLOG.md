# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized milestones below:
> 1. **v0.3.0 (Architecture)**: GUI Test Harness, The Klang Editor (TKE) & Snapshots, macOS .pkg Pipeline, GitHub Version Checker, Cruft Purge, & Parity Audit.
> 2. **v0.4.0 (Sound & Chaos)**: 26-Effects Catalog & Browser Modal, Dual Sample Players, Parameter Randomization, Gated Bass Engine, & Typography Engine.
> 3. **v0.5.0 (Pro Workflow)**: JSON Preset Browser & Sound Design Library, WAV Render / SF2 Export, & Linux Headless CI.
> 4. **v1.0.0 (General Availability)**: Multi-Platform Installers, Comprehensive User Manual, & Launch Demo Reel.
> 5. **v1.1.0 (Hardware Universe)**: dadamachines tbd-16, TKS-Daisy (Stereo), TKS-8 (Teensy Multi-Out), & Zynthian V5.
> 6. **Spin-Offs**: The Klang Mill (TKM 1x6 Pedalboard Rack).

> [!TIP]
> **CODE QUALITY STANDARD**: The C++ codebase currently maintains an A+ standard for defensive programming, descriptive `camelCase` variable naming, and explicit algorithmic comments (e.g., documenting DSP math curves directly above the function). All future contributions must rigidly match this level of in-line documentation and readability!

---

## 🚀 Milestone: v0.3.0 "The Architecture Update"
*Focus: Tooling, Data-Driven Architecture, and 1:1 Legacy Parity.*

### 1. Automated GUI Test Harness (Guardrail)
*Detailed Plan: [`docs/gui_test_harness_plan.md`](gui_test_harness_plan.md)*
Implement a comprehensive, single-binary C++ functional GUI testing harness (`gui_tests`) to test all three applications across the repository: The Klang Farmer, The Klang Planter, and The Klang Editor. Features synthetic mouse event simulation (drag, click, double-click), two-way APVTS parameter synchronization, page navigation transitions (Pages 0–6), dynamic FX slot reconfiguration, modal guide handling, offscreen smoke paint checks, and automated failure PNG snapshot capture in `test_artifacts/gui/`. Integrated directly into the `/build-validate` skill as an automated development guardrail (bypassed with `--skip-gui`). Now prioritized as **Item #1** to lock down test coverage and protect ongoing Editor work!

### 2. Standalone JSON Data & Theme Editor (TheKlangEditor) — Phase 2: Controls, Typography & Snapshots
*Detailed Plan: [`docs/json_editor_tool_plan.md`](json_editor_tool_plan.md)*
Continue development of the dedicated JUCE GUI editor with card preview and editing:
- **Unified Tabbed Layout**: `[THEME]` (Color Wheel, hex inputs) vs `[CONTROLS]` (parameter bounds, labels, tooltips).
- **Text/Localization Extraction**: Extract all remaining hardcoded C++ UI strings into their respective module JSON files and `global_ui.json`.
- **Automated Build Tracking**: CMake injection of Git Commit Count & Hash in the title bar.
- **Consolidated JSON Snapshot & Factory Restore**:
  - `[ Export Snapshot ]`: Bundles all modular control/layout JSONs into a single timestamped `.json` archive.
  - `[ Import Snapshot ]`: Restores full UI state from an external snapshot file.
  - `[ Restore Factory Defaults ]`: One-click button reverting all on-disk JSONs to `assets/factory_defaults_snapshot.json` if a setting is borked.
  - **Bugfix — Card Tree Parameter Population**: Fix issue where drilling down on cards from the plugins in the tree view fails to populate parameters in the property panel.

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


### 7. Zero-Cost macOS FOSS Distribution Pipeline (.pkg + Quarantine Stripper + Visual Guide)
*Goal: Ensure the v0.3.0 Mac release installs and upgrades with zero friction or Gatekeeper blocks.*
- **Automated `.pkg` Installer Generator**: GitHub Actions runner uses macOS native `pkgbuild` & `productbuild` to generate a standard installer.
- **Automated Post-Install Quarantine Stripper**: Installer runs an automated `postinstall` script (`xattr -rd com.apple.quarantine /Library/Audio/Plug-Ins/...`) that strips the internet quarantine flag so DAWs scan the VST3/AU immediately with zero Gatekeeper warnings!
- **In-Place Seamless Upgrades**: Overwrites older `v0.2.0` bundles cleanly while preserving all user presets and DAW project compatibility.
- **Manual Portable DMG & Helper Script**:
  - Packages a stylized `.dmg` with drag-and-drop symlinks to `/Library/Audio/Plug-Ins/`.
  - Includes a double-clickable `Fix_Mac_Permissions.command` helper script.
  - Includes an illustrated `macOS_Install_Guide.html` showing the 2-step bypass in System Settings -> Privacy & Security.

### 8. Automated GitHub Release Version Checker & Settings Modal
*Goal: Provide seamless, non-intrusive notification of new releases directly inside the plugin so v0.3.0 users automatically know when v0.4.0 and beyond drop.*
- **Non-Blocking Background Worker**: Async background thread querying GitHub release API on plugin load.
- **Header Notification Badge**: Subtle, glowing 'Update Available' tag next to version text. Clicking opens release page.
- **Settings & About Modal**: Gear icon in header exposing 'Check for updates on launch' toggle, manual 'Check Now' button, and build metadata.

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

### 7. Voice & Articulation Engine: Gated Staccato Bass, Note-Off Release & Portamento Glide
*Detailed Plan: [`docs/gated_bass_note_off_plan.md`](gated_bass_note_off_plan.md)*  
Transform the dual-FM drum synthesizer into a dual-threat drum and bass machine capable of tight, punchy, articulate staccato basslines, sustained drones, and fluid portamento slides:
- **Header Front-Panel Badge (`VOICE / ARTICULATION`)**:
  - `ONE-SHOT` (Default): Traditional drum-machine behavior; ignores MIDI Note-Offs so envelopes decay naturally.
  - `GATED`: MIDI Note-Off immediately cuts the voice with a smooth, pop-free release ramp.
  - `GLIDE ~`: Shows animated glide indicator when portamento is active (`GLIDE: LEGATO` or `GLIDE: ALWAYS`).
  - Left-click toggles One-Shot vs Gated mode; right-click launches `VoiceArticulationCalloutComponent`.
- **Unified Modal Callout (`VoiceArticulationCalloutComponent`)**:
  - **Trigger Mode**: `[One-Shot]` / `[Gated]`.
  - **Note-Off Release**: `1.0 ms` to `30.0 ms` (default `5.0 ms`, logarithmic skew).
  - **Glide Mode**: `[Off]` / `[Legato]` (glides on overlapping notes) / `[Always]` (glides between all notes).
  - **Glide Time / Sync**: Free milliseconds (`5.0 ms` to `2000.0 ms`) vs Tempo Sync (`1/64` to `1/2 bar`).
  - **Glide Slope**: Slew curve control: `Exponential (0.0)` (analog RC curve) &rarr; `Linear (0.5)` &rarr; `Logarithmic (1.0)`.
  - **Legato Retrigger**: `Off (Continuous)` for fluid acid slides vs `On (Punchy)` for modern trap 808 re-striking slides.
- **Click-Free Semitone-Space Pitch Slew & Release DSP**:
  - Slews pitch in musical semitone space so 1-octave bass slides match 1-octave lead slides identically.
  - Exponential amplitude release ramp via `TbdAudio::FastMath::fastExp`.
  - Zero heap allocations, zero mutexes, and zero DC pops on the audio thread.

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

### 5. Headless Linux CLAP / VST3 Automated CI/CD Runner
*Goal: Ensure multi-platform stability and continuous validation for Linux audio.*
- Add an Ubuntu `aarch64` / `x86_64` container to GitHub Actions building headless Linux CLAP/VST3 binaries on every commit.

---

## 🚀 Milestone: v0.6.0 "The Visual Polish & UI Mastery Update"
*Focus: Professional Boutique Aesthetics, High-DPI Scaling, 60 FPS Visualizers, and Tactile Industrial Hardware Styling.*

### 1. Dynamic UI Scaling (100% to 200%) & High-DPI Vector Crispness
*Goal: Guarantee razor-sharp visuals on 4K, 5K, and Retina displays.*
- **Interactive Free Resizing**: Bottom-right corner drag handle allowing smooth proportional scaling with fixed aspect ratio.
- **Header Zoom Presets**: Stepped zoom selector (100%, 125%, 150%, 175%, 200%) persisted in global config JSON.
- **100% Vector Rendering**: Replace any remaining raster graphics with pure JUCE vector paths and procedural glyphs so lines never blur at high scaling factors.

### 2. Tactile Industrial Chassis Shading & Depth (Baby Audio / Elektron Aesthetic)
*Goal: Transform flat 2D cards into a rich, tactile piece of boutique hardware.*
- **Subtle Drop Shadows & Recessed Bezels**: Procedural soft drop shadows behind cards and sunken, beveled card slots.
- **Chassis Texturing**: Powder-coated matte chassis background rendering with brushed aluminum card borders.
- **Tactile Button Physics**: Depressed click animations with shadow shifts on buttons and selector steppers.

### 3. 60 FPS Smooth Visualizers, Peak Meters & Realistic LED Glow
*Goal: Bring the plugin to life with fluid, responsive feedback.*
- **Tear-Free 60 FPS Oscilloscope**: Lock-free circular FIFO streaming audio waveform points to `MiniOscilloscopeComponent` rendering at a solid 60 FPS with zero audio-thread overhead.
- **Analog-Style Meter Ballistics**: Smooth peak meters with calibrated attack and gentle exponential decay.
- **Realistic LED Bloom**: Soft radial bloom/glow shaders for active LEDs, indicators, and visualizer traces.

### 4. Curated Theme Palette Presets (JSON Driven)
*Goal: Provide distinctive, switchable visual flavors for different studio moods.*
- Author curated, pre-made theme palettes in `assets/themes/`:
  - **Cyberpunk Neon (Default — High-Impact Showcase)**: Deep obsidian black chassis with glowing electric cyan, hot magenta, and radioactive green LEDs (engineered for maximum visual punch in videos, thumbnails, and screenshots).
  - **Cykranosh (Creator's Signature)**: Deep desaturated slate navy chassis (\#161B22\ / \#1A202C\), muted deep blue card surfaces, with eerie ghostly teal (\#4EBEB1\), ice blue, and starlight silver indicators (custom tuned for zero eye fatigue during live sets and marathon studio sessions).
  - **Industrial Carbon**: Dark matte charcoal chassis with warm amber & muted industrial orange accents.
  - **Vintage Hardware (80s Cream)**: Retro off-white chassis with classic slate blue & brick red hardware buttons.
  - **High-Contrast Dark Studio**: Ultra-clean monochrome palette for minimal distraction.
    - **The 'Vibe Coder' IDE Essentials Pack**:
      - **Dracula**: Iconic vampire dark purple background (`#282A36`) with bright cyan, hot pink, and lime green LED accents.
      - **Monokai Pro**: Classic code editor dark grey (`#2D2A2E`) with warm peach, yellow, and vibrant green controls.
      - **Nord**: Arctic frost blue-grey palette with icy cyan and pastel aurora highlights.
      - **Solarized Dark**: Precision teal-grey background with soft amber and cyan highlights.
      - **Solarized Light (The Cursed Mode)**: Warm cream/beige background with high-contrast slate text and warm pastel LEDs for daylight studio sessions.
- Instant, non-destructive live theme switching from the Settings & About modal without restarting the DAW.

---

## 🚀 Milestone: v1.0.0 "The General Availability Launch"
*Focus: Official Public FOSS (GPLv3) Release of The Klang Farmer & The Klang Planter.*

### 1. Multi-Platform Automated Installers & Distribution Packaging
*Goal: Provide zero-friction, professional installers across all major operating systems.*
- **Windows InnoSetup Installer**:
  - Automatically installs VST3 binaries to `C:\Program Files\Common Files\VST3\`.
  - Installs Standalone executables, factory preset library, and documentation.
  - Registers uninstaller in Windows Control Panel / Settings.
- **macOS Signed & Notarized `.pkg` Installer**:
  - Automatically deploys VST3 to `/Library/Audio/Plug-Ins/VST3/` and AU component to `/Library/Audio/Plug-Ins/Components/`.
  - Signed with Apple Developer ID and notarized via `notarytool`.

### 2. Comprehensive User Manual & Interactive Guide
*Goal: Empower sound designers and music producers to master the engine.*
- Author an illustrated, searchable HTML and PDF documentation manual:
  - Deep-dive diagrams explaining the dual FM carrier/modulator phase architecture.
  - Complete 4-parameter reference guide for all 26 DSP effects in the catalog.
  - Keyboard shortcuts, right-click quick-snap intervals, and MIDI CC mapping guide.

### 3. Launch Demo Reel & Audio Showcase
*Goal: Showcase the sonic versatility of the engine on GitHub and social media.*
- Produce high-fidelity audio stems and video demos across multiple musical genres:
  - Industrial Techno, Cyberpunk, 80s Gated Retro Synthwave, and Punchy Modern Trap/Hip-Hop.

---

## 🚀 Milestone: v1.1.0 "The Hardware Universe (Post-1.0)"
*Focus: Standalone Hardware Synthesizer (The Klang Seed), dadamachines tbd-16 Integration, and Zynthian V5 Linux Port.*

### 1. dadamachines tbd-16 Groovebox Integration
*Detailed Plan: [`docs/tbd16_klang_seed_effects_plan.md`](tbd16_klang_seed_effects_plan.md)*
Deploy the pure C++ DSP engine onto the open-source **dadamachines tbd-16** platform:
- **Architecture**: Dual-core **ESP32-P4 RISC-V @ 400 MHz** (Audio DSP) + **RP2350B @ 150 MHz** (UI, Sequencer, 2.4" OLED, 30 RGB buttons) + **ESP32-C6** (Wi-Fi/Ableton Link).
- **Native 4-Encoder Mapping**: The unit features **4 endless push-encoders**; each 4-knob card and 4-knob FX slot in our engine maps directly to one 4-encoder screen page on its 2.4" OLED!

### 2. The Klang Seed: Daisy Edition (TKS-D) — Stereo Desktop & Eurorack Hardware
*Detailed Plan: [`docs/embedded_dsp_and_hardware_port_plan.md`](embedded_dsp_and_hardware_port_plan.md)*
A self-contained, portable stereo FM drum synthesizer and Eurorack module built on the **Electro-Smith Daisy Seed**:
- **Processor & Memory**: STM32H750 ARM Cortex-M7 @ 480 MHz with **64 MB high-speed SDRAM** for immense reverb/delay buffers.
- **Onboard Codec**: Integrated AK4556 24-bit 96 kHz stereo audio DAC/ADC.
- **Hardware Build Complexity**: Low/Moderate. Simple breakout PCB housing Daisy Seed, 4 rotary encoders, 128x64 OLED screen, MIDI TRS/DIN, and 1/4" stereo outputs. Perfect for rapid hardware prototyping!

### 3. The Klang Seed: Studio Edition (TKS-8) — Teensy 4.1 8-Voice Multi-Output Drum Machine
*Detailed Plan: [`docs/embedded_dsp_and_hardware_port_plan.md`](embedded_dsp_and_hardware_port_plan.md)*
A flagship studio drum machine built on **PJRC Teensy 4.1** featuring discrete individual analog voice routing:
- **Processor**: NXP i.MX RT1062 ARM Cortex-M7 @ 600 MHz running 8 mono drum voices (~14.7% CPU load).
- **Multi-Channel DAC**: Cirrus Logic **CS42448 8-Channel 24-bit 192 kHz Codec** driven via TDM.
- **8 Discrete Analog Outputs**: 8 individual 1/4" phone jacks plus Master Stereo L/R. Switched normalled jacks automatically remove a voice from the master stereo mix when an external cable is plugged in, allowing each drum voice to be processed through separate outboard preamps, compressors, and mixing consoles!
- **Hardware Build Complexity**: Advanced. Custom PCB housing Teensy 4.1, CS42448 daughterboard, 10 switched phone jacks, 4 encoders, and OLED screen.

### 4. Zynthian V5 / V4 Standalone Hardware Port (TENTATIVE)
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

### 2. The Klang Boilerplate — Modern C++20 / JUCE 9 FOSS Plugin Starter Template
*Detailed Plan: [`docs/plugin_starter_template_repo_plan.md`](plugin_starter_template_repo_plan.md)*  
Extract a clean, standalone GitHub Template Repository incorporating all lessons learned from *The Klang Farmer* to accelerate new audio plugin development:
- **Zero-Allocation DSP Core**: Strict audio-thread invariants, vectorized `TbdAudio::FastMath`, and lock-free SPSC FIFO queues.
- **Data-Driven JSON Architecture**: APVTS parameter registration, quick-snap intervals, and declarative card/page layouts authored 100% in JSON (`assets/controls/`, `assets/layouts/`).
- **Automated Headless Reflection Testing (`gui_tests`)**: Sweeps 100% of APVTS parameters and JSON assets headlessly in CI without audio hardware or display servers.
- **Boutique UI & Theme System**: JSON theme palettes (*Cyberpunk Neon*, *Cykranosh*, *Dracula*, *Monokai Pro*), vector LookAndFeel, high-DPI scaling, and JUCE 9.0.3 timer hygiene.
- **Developer Onboarding CLI (`init_plugin.py`)**: One-command wizard to rename targets, bundle IDs, C++ namespaces, and parameter prefixes in seconds.

---

## 📁 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in `docs/completed_plans/`)*
