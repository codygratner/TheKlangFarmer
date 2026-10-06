# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized milestones below:
> 1. **v0.3.0 (Architecture)**: GUI Test Harness, The Klang Editor (TKE) & Snapshots, macOS .pkg Pipeline, GitHub Version Checker, Cruft Purge, & Parity Audit.
> 2. **v0.4.0 (Sound & Chaos)**: 26-Effects Catalog & Browser Modal, Dual Sample Players, Parameter Randomization, Gated Bass & Glide, Undo/Redo & A/B, Velocity & MIDI Learn, & Panic Switch.
> 3. **v0.5.0 (Pro Workflow)**: JSON Preset Browser & Sound Design Library, WAV Render / SF2 Export, 2x/4x Oversampling, 4 TBD-16 Macros, Zero-Server GitHub Crash Reporting, One-Time Quick Tour ("Right-Click is the Way"), & Linux Headless CI.
> 4. **v1.0.0 (General Availability)**: Multi-Platform Installers, Comprehensive User Manual, & Launch Demo Reel.
> 5. **v1.1.0 (Hardware Universe)**: dadamachines tbd-16, TKS-Daisy (Stereo), TKS-8 (Teensy Multi-Out), & Zynthian V5.
> 6. **Spin-Offs**: The Klang Mill (TKM 1x6 Pedalboard Rack), The Klang Boilerplate, & The Klang R1 (TKR-1).

> [!TIP]
> **CODE QUALITY STANDARD**: The C++ codebase currently maintains an A+ standard for defensive programming, descriptive `camelCase` variable naming, and explicit algorithmic comments (e.g., documenting DSP math curves directly above the function). All future contributions must rigidly match this level of in-line documentation and readability!

---

## 🚀 Milestone: v0.3.0 "The Architecture Update"
*Focus: Tooling, Data-Driven Architecture, and 1:1 Legacy Parity.*

### 1. Automated GUI Test Harness (Guardrail) — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_Universal_Automated_GUI_Test_Harness.md`](completed_plans/2026-10-06_Universal_Automated_GUI_Test_Harness.md)*
Comprehensive, single-binary C++ functional GUI testing harness (`gui_tests`) testing The Klang Farmer, The Klang Planter, and The Klang Editor with synthetic mouse event simulation, APVTS parameter sync, page navigation, offscreen smoke paint checks, and dynamic reflection audit.

### 2. Standalone JSON Data & Theme Editor (TheKlangEditor) — Phase 2: Controls, Typography & Snapshots — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-05_TheKlangEditor.md`](completed_plans/2026-10-05_TheKlangEditor.md)*
Dedicated JUCE GUI editor with card preview, controls inspector, JSON snapshot export/import/factory restore, and automated commit tracking.

### 3. Automated C++ Linting & Formatting (`clang-format`) — ✅ COMPLETED
`.clang-format` configured and active, enforcing 4-space indentation and clean C++ formatting.

### 4. Comprehensive Codebase Cruft Purge — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_Cruft_Purge_Execution_Plan.md`](completed_plans/2026-10-06_Cruft_Purge_Execution_Plan.md)*
Purged legacy standalone FX blocks, orphaned APVTS parameters, and bypassed routing in commit `4001296`, cleanly migrating all effects to the dynamic 8-slot multi-instance architecture.

### 5. Automated v0.2.0 Parity Audit — ✅ COMPLETED
*Detailed Plan: [`docs/v020_parity_audit_plan.md`](v020_parity_audit_plan.md)*
*Audit Report:* [`docs/parity_audit/tkf_parity_audit.html`](parity_audit/tkf_parity_audit.html) | [`docs/parity_audit/tkf_parity_audit.pdf`](parity_audit/tkf_parity_audit.pdf)
Comprehensive automated audit cross-referencing all 204 legacy v0.2.0 parameters against current v0.3.0 JSON controls across The Klang Farmer and The Klang Planter. All calibrated legacy defaults, string formatters, and colors verified with 0 discrepancies (160 exact matches, 44 intentional multi-instance FX slot migrations, and 30 safe additions). Slide-deck printable PDF report generated.

### 6. Zero-Cost macOS FOSS Distribution Pipeline (.pkg + Quarantine Stripper + Visual Guide) — ✅ COMPLETED
*Goal: Ensure the v0.3.0 Mac release installs and upgrades with zero friction or Gatekeeper blocks.*
- **Automated `.pkg` Installer Generator**: GitHub Actions runner uses macOS native `pkgbuild` & `productbuild` to generate a standard installer.
- **Automated Post-Install Quarantine Stripper**: Installer runs an automated `postinstall` script (`xattr -rd com.apple.quarantine /Library/Audio/Plug-Ins/...`) that strips the internet quarantine flag so DAWs scan the VST3/AU immediately with zero Gatekeeper warnings!
- **In-Place Seamless Upgrades**: Overwrites older `v0.2.0` bundles cleanly while preserving all user presets and DAW project compatibility.
- **Manual Portable DMG & Helper Script**:
  - Packages a stylized `.dmg` with drag-and-drop symlinks to `/Library/Audio/Plug-Ins/`.
  - Includes a double-clickable `Fix_Mac_Permissions.command` helper script.
  - Includes an illustrated `macOS_Install_Guide.html` showing the 2-step bypass in System Settings -> Privacy & Security.

### 7. Automated GitHub Release Version Checker & Settings Modal — ✅ COMPLETED
*Goal: Provide seamless, non-intrusive notification of new releases directly inside the plugin so v0.3.0 users automatically know when v0.4.0 and beyond drop.*
- **Non-Blocking Background Worker**: Async background thread querying GitHub release API on plugin load (`VersionChecker`).
- **Header Notification Badge**: Subtle, glowing 'Update Available' tag next to version text (`UpdateBadgeButton`). Clicking opens release page.
- **Settings & About Modal**: Gear icon in header (`GearButton`) exposing 'Check for updates on launch' toggle, manual 'Check Now' button, and build metadata (`SettingsModalComponent`).

### 8. GitHub CLI Integration & Repository Tagging — ✅ COMPLETED
*Goal: Improve repository discoverability for audio-plugin developers and the vibe coding community.*
- Installed GitHub CLI (`gh`) via `winget` and authenticated with user credentials.
- Curated repository topics applied: `vst3`, `juce-framework`, `drum-machine`, `fm-synthesis`, `vibe-coding`, `agentic-coding`, `audio-plugin`, `synthesizer`, `dsp`, `c-plus-plus`.

### 9. Post-v0.3.0 Tagged Release, Knowledge Distillation & Chat Archival
*Detailed Plan: [`docs/post_v030_release_and_archive_plan.md`](post_v030_release_and_archive_plan.md)*  
*Goal: Consolidate institutional memory across all 14+ chat sessions into a permanent Git-versioned Markdown knowledge base, tag and publish the v0.3.0 release, and safely close all active chats with zero lost knowledge.*
- **Automated Transcript Harvester**:
  - Python harvester script scans all `transcript.jsonl` files in `~/.gemini/antigravity/brain/*/` across all project chat sessions.
  - Distills prompts, architectural decisions, solved bugs, and created artifacts into a structured, chronological `docs/DEV_HISTORY.md`.
- **Executive Institutional Memory Index**:
  - Curated cheat-sheet summarizing core DSP invariants, JUCE 9.0.3 hygiene, build heuristics, and data-driven design patterns at the top of `docs/DEV_HISTORY.md`.
- **Git Tagging & GitHub Release**:
  - Create and push Git tag `v0.3.0`.
  - Author comprehensive release notes and publish GitHub Release with bundled artifacts (`.pkg`, `.vst3`, `.exe`).
- **Primary Branch Migration (`master` &rarr; `main`)**:
  - Fast-forward remote `main` with all 100+ commits from `master`.
  - Switch default repository branch to `main` on GitHub (via `gh repo edit --default-branch main`).
  - Update `CMakeLists.txt` comments and cleanly retire/delete obsolete remote `master` branch.
- **Repository Umbrella Rename & Description Polish**:
  - Rename GitHub repository from `TheKlangFarmer` &rarr; `TheKlangSuite` (GitHub automatically redirects all web traffic, Git clones, and release assets).
  - Update repository About description: `"FM drum synthesizers, multi-effects, and sound design tools (VST3/AU). Pair-programmed and vibe-coded with Google Gemini."`
  - Update local remote origin: `git remote set-url origin https://github.com/codygratner/TheKlangSuite.git`.
- **Post-Release Housekeeping & Chat Purge**:
  - Archive all completed v0.3.0 plan files into `docs/completed_plans/`.
  - Update `CHANGELOG.md` with final v0.3.0 diff.
  - Safe signal to close/kill all accumulated chat sessions in the Antigravity UI for a clean, lightning-fast v0.4.0 kickoff.

---


## ?? Milestone: v0.3.1 "Editor Quality & Data Schema"
*Focus: Expanding the Editor's GUI tests, Tree View UX, and upgrading the JSON data schema for rigorous parameter definitions.*

### 1. Editor GUI Test Suite Expansion & Tree View UX — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_editor_tree_ux.md`](completed_plans/2026-10-06_editor_tree_ux.md)*
*Goal: Expand `gui_tests` to fully validate `The Klang Editor` through headless component testing, and improve the Tree View's user experience.*
- **Headless Validation**: Added `juce::UnitTest` module simulating 100% parameter tree node selection with property manager synchronization.
- **Global Tree Controls**: Added mini-toolbar with `Expand All` and `Collapse All` icon buttons.
- **Contextual Tree Controls**: Added right-click context menu to tree items with `Collapse Others`, `Expand All`, and `Collapse All`.

### 2. Extract Hardcoded C++ Parameter Metadata into JSON (Parity Preservation) — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_parameter_metadata_extraction.md`](completed_plans/2026-10-06_parameter_metadata_extraction.md)*
*Goal: Pull all hardcoded parameter descriptions and bipolar flags out of `FarmerEditor.cpp` and populate them into `assets/controls/*.json` to match `PlanterEditor`'s modern `ControlDef` binding pattern, while maintaining 100% exact parity.*
- **Extract Legacy Boilerplate**: Migrated ~120 lines from `getFarmerParamDescription()` into JSON asset schemas.
- **Modernize `bindSlider`**: Refactored `FarmerEditor::bindSlider` to read `def->description`, `def->isBipolar`, `def->doubleClickValue`, and `def->snapPoints` from `ControlDef`.
- **Zero Parity Breakage**: 84/84 tests passing with zero regressions.

### 3. Intensive GUI Test Suite for Plugins & Standalone (Farmer & Planter) — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_intensive_gui_tests.md`](completed_plans/2026-10-06_intensive_gui_tests.md)*
*Goal: Model intensive GUI testing after The Klang Editor's test harness, expanding `gui_tests` to comprehensively validate component trees, page navigation, modal popups, and offscreen rendering for both The Klang Farmer and The Klang Planter.*
- **Headless Component & Parameter Sweep**: Programmatically verified 100% of cards, sliders, and selectors bind correctly to APVTS parameters and display non-empty tooltips (fixed 3 missing tooltips on Planter limiter).
- **Page Navigation & Paint Smoke Test**: Cycled through all page views across 800x600, 1000x750, and 4K dimensions with offscreen paint passes (`paintEntireComponent()`). Zero crashes, zero division-by-zero.
- **Modal Lifecycle Test**: Simulated opening and closing all modals (Settings, About, Quickstart) with zero timer leaks.
- **Verification Metric**: 117 / 117 `gui_tests` passed successfully with 100% assertion pass rate.

### 4. Planter Header Interactions: VU Meter Panic & Limiter CalloutBox — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_planter_header_and_status_bar.md`](completed_plans/2026-10-06_planter_header_and_status_bar.md)*
*Goal: Transform The Klang Planter's header visualizer into an interactive control center with dedicated mouse targets for master panic and instant limiter adjustment.*
- **Limiter Right-Click CalloutBox**:
  - Right-clicking the center `LIMIT` badge launches a floating mini-card `juce::CalloutBox`.
  - Houses an Enable toggle (`planter_limiter_enable`) and 3 mini rotary knobs for Gain (`planter_limiter_gain`), Ceiling/Threshold (`planter_limiter_thresh`), and Release (`planter_limiter_release`).
  - Styled to match Card 6's Doepfer silver & red chassis theme (`0xffe53935`).
- **Peak VU Meter Panic**:
  - Clicking the stereo peak meters flushes all active voice and noise envelope timings, resets the S&H DJ filter, and clears master peak levels.
  - Features a crisp 150ms visual flash on the meter bars upon panic trigger.
- **Parity Safety**:
  - Non-destructive: Binds directly to existing APVTS parameters without altering presets, audio DSP math, or Card 6.

### 5. Interactive Two-Line Status Bar (Values, Mouse Shortcuts & Tooltip Feed) — ✅ COMPLETED
*Detailed Plan: [`docs/completed_plans/2026-10-06_planter_header_and_status_bar.md`](completed_plans/2026-10-06_planter_header_and_status_bar.md)*
*Goal: Implement a Kilohearts/Ableton style 36px bottom status bar across The Klang Farmer and The Klang Planter, providing permanent value readouts, mouse shortcut badges, and a full-width tooltip feed.*
- **Line 1 (Top Bar - Permanent)**:
  - Left: Control Name and formatted Parameter Value in bold (e.g., `Carrier 1: Pitch  +12.0 st [440 Hz]`).
  - Right: Contextual Mouse Shortcuts in subtle pill badges (e.g., `Right-Click: Snap Points | Double-Click: Reset (0.5)`). Always visible even if tooltips are toggled off.
- **Line 2 (Bottom Bar - Tooltip Feed)**:
  - Full-width parameter description and functional explanation.
  - Toggled dynamically by the header `TOOLTIPS` button (when off, Line 2 is quiet or shows engine status).
- **Verification Metric**: 164 / 164 `gui_tests` passed successfully with 100% assertion pass rate across all limiter controls and status bar hover callbacks.

## ?? Milestone: v0.4.0 "The Sound & Chaos Update"
*Focus: Sonic Expansion, Workflow Disruption, and Modulation.*

### 0.5. Interactive Parameter & Curve Overhaul (Breaking Parity)
*Goal: Systematically audit and redesign the interactive tactile feel of all cards, adding custom snap points, logarithmic slider slopes, and ergonomic double-click defaults.*
- Audit all card controls for optimal double-click reset values.
- Tune slider skew factors (slopes) for frequency, time, and resonance controls.
- Add discrete snap points and magnetism to key musical intervals and center points.

### 0.6. Transparent Quota Telemetry & Language Server Probe (`/quota` Skill)
*Goal: Provide instant, transparent visibility into Antigravity model quotas (5-hour rolling bucket, weekly tier allowances) without diving deep into IDE settings menus.*
- **Investigation & Probing**:
  - Probe the local running `language_server.exe` gRPC/HTTP bridge (`localhost:61440/61441`) and Google Cloud Code endpoint to determine if quota/bucket metrics are accessible via a lightweight local socket call.
  - Evaluate creating a custom slash command skill (`/quota`) or status bar widget that displays active tier allowances on demand in <50ms without network roundtrips.
- **Guardrails**:
  - Strictly on-demand execution (never polled automatically during every model evaluation to prevent latency, token bloat, and rate-limiting).

### 0.7. Tier 1 SIMD Voice Summation & Branchless FM Phase Accumulators
*Goal: Optimize real-time FM operator phase modulation and polyphonic voice summation using JUCE 9 SIMD wrappers and branchless bitwise math.*
- **SIMD Voice Summation (`juce::dsp::SIMDRegister<float>`)**:
  - Migrate polyphonic voice summation from sequential loops to hardware-abstracted 64-byte aligned SIMD registers (4-lane SSE / 8-lane AVX2).
  - Adopt a Structure-of-Arrays (SoA) layout for active voice synthesis buffers to eliminate cache-line thrashing.
- **Branchless Power-of-Two FM Phase Wrapping**:
  - Replace conditional phase wrapping with 32-bit fixed-point integer phase accumulators (`uint32_t`) and power-of-two lookup table indexing with bitwise masking (`& 4095`).
  - Completely eliminates CPU branch mispredictions and CRT transcendentals in hot FM feedback and cross-modulation loops.

### 0. Automated Version Bump Guardrail (`/cut-release` Skill)
*Goal: Formalize the "Version Bump = Clean Slate" workflow by building a dedicated AGY slash command to handle version bumps safely.*
- **Action**: Build `C:\Users\codyg\.gemini\config\skills\cut-release\SKILL.md`.
- **Requirements**:
  - Automatically bumps the CMake `project(TheKlangSuite VERSION X.X.X)` string.
  - Builds and tests the new version.
  - Upon success, pops an interactive modal: "SUMMON THE HARVESTER?"
  - If Yes:
    - Runs a `clear_transcript.py --all` python script to scrape ALL active chats.
    - Slices all previous `DEV_HISTORY.md` sessions into a new archive (`docs/archives/DEV_HISTORY_vX.X.X.md`).
    - Globally wipes `transcript.jsonl` for every active chat.
    - Overwrites both `context_clues_build.md` and `context_clues_plan.md` with a clean slate message.
  - Commits the version bump.

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

### 8. Sound Design Safety: Undo / Redo & A/B State Comparison
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](pre_v1_sound_and_workflow_expansion_plan.md)*  
*Goal: Provide full sound design safety and non-destructive experimentation, essential when rolling the d6 Randomizer.*
- **Header Controls & Keyboard Shortcuts**:
  - Subtle `↶` (Undo) and `↷` (Redo) buttons and a tactile `[ A | B ]` toggle button in the header bar.
  - Global hotkeys: `Ctrl+Z` / `Cmd+Z` (Undo) and `Ctrl+Y` / `Cmd+Shift+Z` (Redo).
  - Right-click context menu on `[ A | B ]`: `Copy State A to B` / `Copy State B to A`.
- **APVTS & Randomizer Transactions**:
  - Integrates `juce::UndoManager` into `KlangCoreProcessor` and APVTS slider gestures.
  - Every d6 randomizer roll pushes a named transaction (e.g., "Randomize Pitch Card", "Randomize Synth") so accidental overwrites can be instantly undone.

### 9. Velocity Sensitivity Curves & MIDI CC Learn
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](pre_v1_sound_and_workflow_expansion_plan.md)*  
*Goal: Calibrate dynamic response for external drum pads/keys and enable instant hardware MIDI controller mapping.*
- **Dynamic Velocity Scaling (Voice & Articulation Modal)**:
  - Selectable response curves: `Linear`, `Exponential` (soft touch / wide dynamics), `Logarithmic` (hard touch), and `Fixed (127)` (essential for uniform electronic/techno drum hits).
  - Velocity Depth slider (`0%` = velocity immune &rarr; `100%` = full dynamic range).
- **Right-Click MIDI CC Learn**:
  - Right-click any parameter knob or slider &rarr; `MIDI Learn` (captures next incoming hardware CC) or `Clear MIDI CC`.
  - Mappings stored in user config and persistent across sessions.

### 10. Panic / Kill Audio (Emergency Silence & DSP Flush)
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](pre_v1_sound_and_workflow_expansion_plan.md)*  
*Goal: Instant safety shutoff protecting ears and studio monitors from runaway delay/reverb feedback or stuck MIDI notes.*
- **Header Trigger & MIDI CC Integration**:
  - Double-clicking the Master Peak Meter / CPU indicator instantly cuts all audio.
  - Also triggers on standard incoming MIDI CC 120 (All Sound Off) and CC 123 (All Notes Off).
- **Pop-Free DSP Buffer Flush**:
  - Applies a sub-millisecond (1ms) exponential fade-out to prevent speaker pops.
  - Flushes all internal delay lines, reverb tanks, and comb filter feedback buffers to zero.
  - Resets active MIDI voice tracking and legato gate memory.

### 8. Evaluate Agentic Workflow Strategy (Context Wipes & Strict Chat Roles)
*Goal: After completing v0.4.0, review how well the two-chat workflow held up against prompt drift and task bleeding.*
- Did the `/clear` command with split `context_clues.md` files sufficiently protect against hidden state?
- Did the strict "Planner vs Builder" guardrail successfully prevent task bleeding and keep architecture decisions centralized?
- Document final workflow decisions in `docs/post_v040_workflow_retro.md`.

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

### 6. Dual-Tier 2x / 4x Oversampling Engine (Anti-Aliasing)
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](pre_v1_sound_and_workflow_expansion_plan.md)*  
*Goal: Eliminate FM modulation sideband foldback and non-linear saturation aliasing in both realtime and offline render paths.*
- **Dual-Tier Quality Strategy**:
  - **Realtime / Live**: Selectable `[Off (1x) | 2x | 4x]`. Minimum-phase IIR filters guarantee zero monitoring latency for live finger-drumming and tracking.
  - **Render / Export**: Selectable up to `8x` oversampling for maximum offline fidelity during WAV/SF2 bouncing.
- **DSP Engine Wrapping**:
  - `juce::dsp::Oversampling<float>` wraps the core voice and non-linear effects path in `processBlock()`.
  - Zero allocation audio-thread invariant strictly preserved by pre-allocating oversamplers in `prepareToPlay()`.

### 7. 4 Performance Macro Knobs (TBD-16 Hardware Aligned)
*Detailed Plan: [`docs/pre_v1_sound_and_workflow_expansion_plan.md`](pre_v1_sound_and_workflow_expansion_plan.md)*  
*Goal: Instant front-panel performance tweaking mapped 1:1 to Page 1 of the dadamachines TBD-16 hardware.*
- **Global Front-Panel Access**:
  - 4 persistent macro knobs accessible from the plugin header across all pages.
  - Aligned 1:1 to the 4 physical endless push-encoders on **Page 1 of the dadamachines TBD-16** hardware groovebox.
- **Right-Click Modulation Assignment**:
  - Right-click any parameter knob or slider &rarr; `Assign to Macro 1–4`.
  - Configurable bipolar modulation depth (`-100%` to `+100%`).
  - Macro assignments and positions serialized directly into JSON preset files (`PresetManager.h`).

### 8. Zero-Server GitHub Crash Reporting Engine
*Detailed Plan: [`docs/github_crash_reporting_plan.md`](github_crash_reporting_plan.md)*  
*Goal: Capture real-world crash logs and stack traces from beta testers directly into GitHub Issues with zero server infrastructure, zero hosting costs, and zero secret token leaks.*
- **Pre-Filled GitHub Issue URL Generator**:
  - When an unexpected termination is detected, prompts: *"The Klang Farmer encountered an unexpected shutdown. Submit report to GitHub?"*
  - Clicking launches the user's default browser to a pre-filled GitHub Issue URL with markdown callstack and environment info.
- **Hybrid Crash Detection**:
  - **In-Process Scoped Exception Handling**: Structured exception catching (`__try / __except` / signal traps) around top-level DSP and UI callbacks generates immediate stack traces and mutes audio before host DAW crash.
  - **Heartbeat Session Lockfile**: `%APPDATA%/TheKlangFarmer/sessions/session_active.lock` detects abnormal host DAW crashes on next launch.
- **Full Scrubbed Diagnostics**:
  - Gathers OS, host DAW name/version, buffer size, sample rate, Git commit hash, active preset/FX, and demangled C++ call stack.
  - Automatically scrubs local usernames from paths (e.g. `C:\Users\<redacted>\...` &rarr; `<UserPath>`) to protect privacy.

### 9. One-Time Quick Tour & Gesture Revelation ("Right-Click is the Way")
*Detailed Plan: [`docs/one_time_quick_tour_plan.md`](one_time_quick_tour_plan.md)*  
*Goal: Provide a sleek, non-intrusive first-launch onboarding card that introduces users to the tactile power of right-click quick snaps, randomizer menus, voice articulation callouts, and double-click resets.*
- **Unobtrusive Single-Screen Overlay**:
  - Automatically pops up on first launch only; persistent state stored in `%APPDATA%/TheKlangFarmer/settings.json`.
  - Dismissible with a single click outside the card, hitting `ESC`, or clicking `[ GOT IT, LET'S PLAY ]`.
  - Can be reopened anytime via the header `[ ? ]` button or Settings gear menu.
- **The Core Message**:
  - Visual gesture breakdown emphasizing:
    1. Right-click knobs & sliders for Quick-Snap intervals and MIDI CC Learn.
    2. Right-click selectors for default resets and full dropdown lists.
    3. Right-click card/page headers for the d6 Randomizer depth menu.
    4. Right-click the Voice badge for Gated Bass & Glide, and right-click `[A|B]` to copy states.
    5. Double-click any parameter to reset to factory default.
- **100% JSON-Driven Copy**:
  - All headings, icons, descriptions, and button labels parsed from `assets/controls/global_ui.json` under `"quick_tour"`.

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

### 3. ToadTracker Core Migration & Architectural Port
*Detailed Plan: [`docs/toadtracker_migration_plan.md`](toadtracker_migration_plan.md)*  
*Priority: Super Low (Post-1.0)*
Port the battle-tested, data-driven architecture from *The Klang Farmer* over to the `ToadTracker` codebase to unify DSP and UI workflows:
- **JSON APVTS & UI Builder**: Drop `ParameterManager` and the JSON control schema into ToadTracker's JUCE HAL to instantly generate UI and parameters without hardcoding.
- **Audio Thread Guardrails**: Transplant `TbdAudio::FastMath`, `GEMINI.md` audio invariants, and the `audiothread-guard` skill to guarantee zero-allocation/zero-lock safety.
- **Testing Parity**: Migrate the headless `ReflectionGuardrailSuite.h` and automated GUI smoke testing harness to validate ToadTracker's JUCE layer.

### 4. The Klang R1 (TKR-1) — 7-Voice Rhythm Synthesizer (Electribe ER-1 Tribute)
*Detailed Plan: [`docs/the_klang_r1_plan.md`](the_klang_r1_plan.md)*  
*Goal: Provide a stripped-down, tactile, zero-tab 7-voice drum synthesizer inspired by the iconic Korg Electribe ER-1 with white-key octave-invariant triggering and DAW multi-out routing.*
- **7-Voice Hybrid Architecture**:
  - **Voices 1–4 (Pure Synth)**: Kicks, sub-bass, snares, toms, and resonant FM zaps.
  - **Voices 5–7 (Percussion & Metallic)**: Closed Hat, Open Hat (auto-choked by Voice 5), and Cymbal / Crash.
- **Octave-Invariant White Key Triggering**:
  - White keys in any octave map to Voices 1–7 (`C` = Voice 1 &rarr; `B` = Voice 7).
  - Pitches are fixed to front-panel knobs for authentic drum-machine operation (zero pitch tracking).
  - Black keys (`C#`, `D#`, `F#`, `G#`, `A#`) are unassigned for foolproof live finger drumming anywhere on the keybed.
- **DAW Multi-Out Bus Architecture**:
  - 8 Stereo output pairs: Master Mix + 7 Individual Voice stems (`Voice 1` through `Voice 7`).
  - Auto-mute routing: Routing a voice to an aux track removes it from Master Mix (with parallel toggle).
- **All-in-One Console UI (Zero Tabs)**:
  - 7 vertical mixer-style voice strips with `Pitch`, `Decay`, `Mod Type`, `Mod Speed`, `Mod Depth`, `Pan`, `Level`, and `[DELAY SEND]`.
  - Master section featuring classic ER-1 **Low Boost** sub-punch knob, host-synced **Tempo Delay**, and **Ring Mod** cross-modulation (`Voice 1 × Voice 2`).

---

## 📁 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in `docs/completed_plans/`)*

