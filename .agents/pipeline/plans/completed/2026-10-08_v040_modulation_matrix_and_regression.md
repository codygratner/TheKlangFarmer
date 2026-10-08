# Plan: v0.4.0 Modulation Engine & Neo-Slate Industrial Overhaul

## Overview
This milestone executes the complete architectural and visual overhaul of **The Klang Farmer**, integrating:
1. A modular, Vital/Serum-style dynamic modulation matrix with 16 pre-allocated sources.
2. Renoise-style signal-driven Hydra meta-modulators with **audio-rate oscillator inputs (FM textures)**.
3. 4 persistent front-panel Performance Macros (aligned 1:1 to TBD-16 Page 1 hardware encoders).
4. Dynamic Velocity response curves (`Linear`, `Exponential`, `Logarithmic`, `Fixed 127`) and Right-Click **MIDI CC Learn**.
5. Master Audio Panic & Buffer Flush (pop-free silence and delay/reverb flush on double-click meter or CC 120/123).
6. Sound Design Safety: **Undo / Redo (`Ctrl+Z` / `Ctrl+Y`) and A/B State Comparison** in the header.
7. Contextual **d6 Parameter Randomizer** (card-level and synth-level generative jitter with selectable depth).
8. A single-tab Drag-and-Drop FX Rack with immutable Pre-Amp Console and Master Limiter anchors.
9. Gloomy cyberpunk Neo-Slate vector aesthetic (Axel Hartmann Waldorf styling + bundled JetBrains Mono OFL).
10. Automated GUI Text Truncation Audit Suite (`gui_tests`).

In accordance with the **Modulation Engine First** directive, the backend DSP routing matrix, velocity curves, and panic safety are constructed and mathematically proven before visual components are built on top.

---

## Architectural Decisions (from `/grill-me` Consensus)
1. **Modulation Sequencing**: Backend DSP Modulation Engine first &rarr; Drag-and-Drop FX &rarr; Neo-Slate UI & Typography &rarr; UI Modulation Rings, Macros, Undo/Redo & Randomizer &rarr; SQA Text Audit.
2. **Core Routing Model**: Dynamic Matrix (Vital/Phase Plant style). Every modulator is an assignable source that can target ANY parameter (including other modulators' rates/depths).
3. **Performance Macros vs. Signal-Driven Hydras (Clear Distinction)**:
   - **4 Performance Macros (`Macro 1..4`)**: Manual/hardware performance knobs mapped 1:1 to TBD-16 Page 1 hardware encoders, permanently accessible in the top header mini-dock across all pages and on the Modulations page.
   - **4 Hydra Meta-Hubs (`Hydra 1..4`)**: True Renoise-style signal-driven meta-routing hubs with selectable input sources (LFO, Env, Random, Slop, Velocity, KeyTrack, Macros, or **Oscillators for Audio-Rate FM!**) that fan out to multiple destinations with individual Min/Max ranges and curves.
4. **Hybrid Rate Audio Architecture**:
   - Hydras evaluate at full audio rate (per-sample) when an Oscillator is selected as input for gritty metallic FM drum sidebands, and drop down to block rate for LFO/Env inputs to keep CPU near-zero.
5. **Velocity Curves & MIDI CC Learn**:
   - Velocity tracker provides selectable response curves (`Linear`, `Exponential` for wide dynamics, `Logarithmic`, `Fixed 127` for uniform techno hits).
   - Right-click any knob, slider, or Performance Macro &rarr; `MIDI Learn` (captures incoming hardware CC).
6. **Master Audio Panic**:
   - Double-clicking the master peak meter, pressing `Esc`, or receiving MIDI CC 120/123 flushes all audio buffers (delay lines, reverb tanks, comb filters) and resets modulation states with a 1ms pop-free exponential fade-out.
7. **Header Sound Design Safety (Undo/Redo + A/B)**:
   - Top header features subtle `↶` (Undo), `↷` (Redo), and `[ A | B ]` comparison buttons.
   - Supports `Ctrl+Z` / `Cmd+Z` and `Ctrl+Y` / `Cmd+Shift+Z`.
   - Every d6 randomizer roll pushes an undo transaction so chaos rolls can be reverted instantly.
8. **Contextual d6 Randomizer**:
   - Small `[d6]` icon on each Card header and global header.
   - Jitters continuous parameters by selectable depth (5%, 15%, 25%, 50%, 75%, 100%) and flips discrete selectors with probability.
9. **Secondary "Via" Modulation**: Matrix routes feature a `Via` source column (e.g. Velocity modulates the depth of LFO 1 &rarr; Filter Cutoff).
10. **Tactile Modulation & Permanent Lower Strip**:
    - The UI is divided into 3 vertical zones: Top Header, Tabbed Center Workspace, and a **Persistent Lower Modulator Strip**.
    - The lower strip contains animated "drag-from" tiles for all LFOs, Envelopes, and Hydras. Users drag a tile directly onto any knob in the center workspace to assign modulation.
11. **4-Point Standard Interaction Scheme**:
    - **Left-Click & Drag**: Standard value adjustment.
    - **Double-Click**: Opens **Smart Text Entry** (type `C2+37c`, `1/4d`, `-6dB`, or `55 Hz` and the `SmartValueParser` calculates the float).
    - **Alt+Click**: Instant Reset to Default.
    - **Right-Click**: Opens Context Menu (MIDI CC Learn, Modulation assignments, Quick-edit popovers for modulators).
12. **Information Scent (Permanent Status Bar)**:
    - The bottom status bar is permanent (cannot be toggled off) and provides deeply descriptive tooltips for whatever control the mouse is hovering over.
13. **Drag-and-Drop FX Rack**: Single unified `EFFECTS` page with two horizontal lanes (Pre-Amp top lane, Post-Amp bottom lane). Supports tactile card dragging with full intra-lane and cross-lane swapping.
14. **Pre-Amp Stage Anchor**: Dedicated immutable 5th slot in the Pre-Amp top lane (`[PRE-AMP DRIVE & COLOR]`) matching the immutable Master Limiter anchor at the end of the Post-Amp bottom lane.
15. **Aesthetic & Typography**: Bundled open-source `JetBrains Mono` font. Gloomy cyberpunk / Axel Hartmann Waldorf industrial design (90% neutral dark titanium/anthracite metal, functional rain-slicked cyan/amber/emerald accents). Full preservation of the `Cykranosh` alternate theme.
16. **SQA Text Truncation Suite**: Dynamic reflection sweep in `gui_tests` auditing 100% of labels, values, and buttons across 100% to 200% display scalings to guarantee zero clipped text strings.

---

## Phases

### Phase 1: Modulation Engine Core, Velocity Curves & Panic Safety (DSP First)
- [x] In `source/ModulationEngine.h` (or within DSP core), implement a pre-allocated, zero-allocation modulation matrix:
  - Source array: `LFO 1..4`, `Env 1..4`, `Random 1..2`, `Velocity`, `KeyTrack`, `Slop`, `Macro 1..4`, `Hydra 1..4`.
  - Fast, bounded routing table (up to 64 active routes per processor instance).
  - Route struct: `sourceId`, `targetParamId`, `depth`, `bipolar`, `viaSourceId`, `viaDepth`, `curve`.
- [x] Implement lock-free, SIMD-accelerated sample/block modulation evaluation in `processBlock()`:
  - Pre-calculate source buffer blocks per audio cycle.
  - Apply primary modulation: `dest += source * depth`.
  - Apply secondary "Via" modulation: `effectiveDepth = depth * (viaSource * viaDepth)`.
  - Audio thread invariants strictly verified (zero heap allocations, zero locks).
- [x] Implement the **Hydra Meta-Modulator DSP Engine**:
  - Connect dynamic input sources (LFO, Env, Random, Velocity, Macro, or Carrier/Modulator oscillators).
  - Hybrid-rate evaluation: evaluate per-sample when an oscillator is selected (for authentic FM texture), and block-rate otherwise.
  - Fan out to up to 8 destinations per Hydra with individual Min/Max bounds and curve mapping.
- [x] Implement Dynamic Velocity Response Curves in `VelocityTracker`:
  - Support `Linear`, `Exponential`, `Logarithmic`, and `Fixed 127` curves.
- [x] Implement **Master Audio Panic DSP Flush**:
  - `triggerPanic()` method on processor that applies a 1ms fast exponential fade-out, zeros all delay/reverb/comb buffers, and resets modulation accumulators.
- [x] Author test suite in `test/dsp_tests.cpp` validating:
  - Sources generate accurate normalized waveforms.
  - Route depth scaling, bipolar/unipolar inversion, and "Via" cross-modulation.
  - Velocity curve math and Panic flush silence.
  - Audio-rate oscillator routing into Hydra destinations without denormals, NaNs, or audio glitches.
  - 100% audio thread safety via `audiothread-guard`.

### Phase 2: Drag-and-Drop FX Rack & Pre-Amp Console Anchor
- [x] In `FarmerEditor` and layout schemas, consolidate `Pre-Amp FX` and `Post-Amp FX` pages into a single unified `EFFECTS` page.
- [x] Build two 5-slot horizontal card lanes:
  - **Top Lane (Pre-Amp)**: Draggable Slots 1–4 &rarr; Anchor 5: `[PRE-AMP DRIVE & COLOR]` (Input Gain, Drive Curve, Tone, Output Level).
  - **Bottom Lane (Post-Amp)**: Draggable Slots 5–8 &rarr; Anchor 5: `[MASTER LIMITER / OUT]`.
- [x] Implement JUCE `DragAndDropContainer` & `DragAndDropTarget` on FX cards:
  - Visual drag feedback (card ghost / target highlight border).
  - Tactile swap execution: dragging Slot A onto Slot B cleanly swaps their algorithms and parameter settings.
  - Allow seamless dragging across both Pre-Amp and Post-Amp lanes.
- [x] Ensure immutable Anchor 5 cards cannot be dragged or replaced.
- [x] Verify APVTS synchronization, host automation compatibility, and test coverage in `test/gui_tests.cpp`.

### Phase 3: Neo-Slate Industrial Chassis & JetBrains Mono Typography
- [x] Bundle `JetBrains Mono` (Open Source OFL) into `assets/fonts/` and register in `TkfAssets` (with `TkfTypography::getFont` fallback hierarchy).
- [x] Update `assets/themes/theme.json` with the new default **Neo-Slate / Gloomy Cyberpunk** palette:
  - Anthracite matte chassis (`0xff0d1117`), dark titanium card bodies (`0xff161b22`), subtle 1px hairline borders (`0xff283141`).
  - Functional neon accents: electric cyan (`0xff38bdf8`) for modulation, solar amber (`0xfff59e0b`) for drive, acid emerald (`0xff10b981`) for power/status.
  - Crisp cool-silver text hierarchy (10pt, 12pt, 14pt JetBrains Mono).
- [x] Implement the **3-Tier Vertical UI Layout**:
  - **Top Header**: Horizontal Page Navigation tabs, Stereo Oscilloscope/Meter (double-click Panic), 4 Performance Macros, Sound Design Safety (`↶`, `↷`, `[ A | B ]`), and Global `[d6]`.
  - **Center Workspace**: Tabbed pages (`[VOICE 1]`, `[VOICE 2]`, `[TRANSIENTS]`, `[EFFECTS]`, `[MOD]`).
  - **Lower Modulator Strip & Status Bar**: A permanently visible horizontal lane showing animated tiles for all modulators, and a dedicated Status Bar at the very bottom displaying deeply informative hover tooltips.
- [x] Add contextual `[d6]` dice button to every Card header.
- [x] Verify full theme engine parity: ensure switching to `Cykranosh`, `Cyberpunk Neon`, `Dracula`, or `Monokai Pro` correctly restyles the new layout without artifacts.
- [x] Update custom `LookAndFeel` with sharp, flat vector styling (zero skeuomorphic 3D lighting, clean modern borders, subtle 2px card drop shadows).

### Phase 4: Modulation Matrix UI, Performance Macros, Undo/Redo & MIDI Learn
- [x] Implement `juce::UndoManager` integration in `KlangCoreProcessor`:
  - Push named undo transactions for slider drags and d6 randomizer rolls.
  - Wire `↶` and `↷` header buttons and `Ctrl+Z` / `Ctrl+Y` shortcuts.
  - Implement A/B State comparison memory buffers with right-click `Copy A to B` / `Copy B to A`.
- [x] Implement contextual **d6 Parameter Randomizer Engine**:
  - Clicking card `[d6]` randomizes that card; clicking global `[d6]` randomizes the whole synth.
  - Right-click die &rarr; Depth selection modal (5%, 15%, 25%, 50%, 75%, 100%).
  - Logic: Incremental jitter on continuous sliders, probability flip on discrete selectors.
- [x] Implement the **4-Point Standard Interaction Scheme** across all controls:
  - Left-Click & Drag to adjust.
  - Double-Click to open text entry.
  - Alt+Click to reset to Default/Init value.
  - Right-Click to open Context Menu (MIDI CC Learn, Modulation assign, etc.).
- [x] Implement **SmartValueParser** for text entry:
  - Parses advanced strings (e.g. `C2+37c` &rarr; `66.82 Hz`, `1/4d` &rarr; dotted quarter ms, `-6dB` &rarr; gain float).
- [x] Top Header Performance Macro Mini-Dock:
  - Embed 4 compact rotary knobs in the chassis header (`M1`, `M2`, `M3`, `M4`) permanently visible.
- [x] On rotary knobs and arcade meter bars, implement animated **Modulation Rings**:
  - Draw a high-contrast colored arc showing total modulated range, and a real-time animated dot for instantaneous value.
- [x] Build the **Persistent Lower Modulator Strip**:
  - Horizontal lane of compact tiles representing LFOs, Envelopes, and Hydras.
  - Tiles feature real-time animated playheads/waveforms.
  - Drag-and-drop: click and drag a tile upwards onto any center workspace knob to instantly map modulation.
  - Right-click a tile to open the **Spacious Inspector Popover** (380x280 px) to fine-tune destination Min/Max ranges and curves.
- [x] Implement **Visual Modulation Tracer Cables** (Bézier Glow Overlay):
  - Pass-through overlay component (`ModulationTracerOverlay`) with zero click interception.
  - **Single Parameter Focus**: When hovering or inspecting a single knob, draws a sleek glowing Bézier spline cable directly from its driving source(s) in the Lower Strip up into the knob.
  - **Card Inspector Full Mode (4 Colors & Edge Socket Docking)**: When the module Inspector Popover is open, maps the card's 4 knobs to high-contrast neon accents (Cyan `#38bdf8`, Amber `#f59e0b`, Emerald `#10b981`, Amethyst `#c084fc`). Cables terminate cleanly into 4 colored patch jack sockets along the bottom perimeter bezel of the popover (Eurorack style) with matching LED indicator dots inside the window, preventing text/knob visual occlusion.
  - Renders smooth anti-aliased curves with soft outer drop-glows; completely idles when no modulation is being inspected.
- [x] Build the dedicated **MODULATIONS** page:
  - Scrollable Matrix Table listing all active connections (Source, Target, Depth, Via Source, Via Depth).

### Phase 5: Automated GUI Text Truncation Suite, Full Pre-Release Gauntlet & Remote CI
- [x] In `test/GuiTestHelpers.h`, implement a font bounds measurement helper:
  - Compares rendered glyph bounding boxes against component client bounds.
- [x] In `test/HardeningSuites.h`, add `TextTruncationAuditSuite`:
  - Iterates 100% of labels, buttons, card headers, and dynamic formatted parameter readouts across all pages.
  - Tests across 800x600, 1040x740, and 1400x900 window sizes.
  - Tests across 100%, 125%, 150%, and 200% display scalings.
  - Asserts zero text clipping or ellipsis truncation.
- [x] Execute Hardened Pre-Flight Cruft Sweep & Static Audits (`cut-release` Phase 1):
  - Purge disposable scratch and temporary files (`temp_*`, `*.dump`, `test_screenshots/`).
  - Static audio-safety & debug leak audit: scan modified C++ files for `std::cout`, `printf`, unvectorized `DBG(`, and verify JUCE 9.0.3 timer hygiene (`stopTimer()` in all `juce::Timer` destructors).
- [x] Execute Dual-Configuration Regression Gauntlet (`cut-release` Phase 2):
  - **Debug Build & Run**: `cmake -B build -DCMAKE_BUILD_TYPE=Debug` & run `dsp_tests.exe` and `gui_tests.exe` to catch MSVC debug runtime assertions.
  - **Release Build & Run**: `cmake --build build --config Release` & run `dsp_tests.exe` (100% audio invariants, SIMD, FastMath) and `gui_tests.exe` (100% pass across **ALL** suites including **PluginIntensiveTestSuite** 6-pillar sweeps and `--chaos` Chaos Monkey).
  - **Headless Host Compliance**: `powershell -ExecutionPolicy Bypass -File tools/run_pluginval.ps1 -Strictness 5`.
  - **Deployment**: Execute `deploy.ps1` to sync artifacts to system VST3 directories and `current_build/`.
- [x] Multi-Platform Remote GitHub CI Matrix Verification:
  - Commit all verified code changes to `0.4.0-dev`.
  - Push branch `0.4.0-dev` to GitHub remote to trigger multi-platform CI matrix (`.github/workflows/ci.yml`).
  - Verify 100% green pass across macOS (AU/VST3 + auval + pluginval), Linux (VST3 + Xvfb + pluginval), and Windows x64.
  - Monitor run via GitHub API / `gh run list` following the two-stage watchdog protocol.
  - *(Optional)* Cleanup / delete CI run logs via `gh run delete` after green validation if clean run history is desired.
- [x] **STRICT RELEASE SAFETY GATE**: Do NOT bump CMakeLists.txt version, do NOT create git tags, and do NOT cut a GitHub release. The Ivory Tower remains the sole release authority!

---

## Acceptance Criteria
- [x] 4 Performance Macros and 4 Hydra Meta-Hubs evaluate cleanly in real time without audio glitches, locks, or heap allocations.
- [x] Hydras accept audio-rate oscillators as inputs and modulate targets at per-sample speed.
- [x] Double-clicking peak meters or receiving CC 120/123 executes pop-free Panic buffer flush.
- [x] Undo (`Ctrl+Z`), Redo (`Ctrl+Y`), and A/B state comparison function seamlessly in the header.
- [x] Dragging an animated tile from the Lower Modulator Strip onto a center workspace knob maps modulation instantly.
- [x] Double-clicking any control opens the Smart Text Entry box parsing strings (`Hz`, `ms`, `dB`, `C2`) perfectly.
- [x] Alt-clicking any control instantly resets it to its default Init value.
- [x] Right-clicking any control opens the context menu (MIDI CC Learn, Unmap).
- [x] Rolling the d6 randomizer jitters parameters by selected depth and pushes a reversible undo action.
- [x] FX cards drag and drop to swap slots freely across Pre-Amp and Post-Amp lanes.
- [x] Neo-Slate dark industrial aesthetic renders crisply with JetBrains Mono.
- [x] `Cykranosh` and alternate themes load cleanly via the dynamic theme engine.
- [x] 100% of UI text strings pass the Text Truncation Audit across all display scales.
- [x] 100% test pass in `dsp_tests` and `gui_tests` (including Plugin Intensive Suite & Chaos Monkey) on MSVC, Clang, and GCC.
- [x] Strictness 5 pass on `pluginval`.
- [x] Multi-Platform GitHub CI (`ci.yml`) passes 100% green across macOS, Linux, and Windows.
