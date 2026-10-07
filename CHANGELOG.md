# Changelog

All notable changes to **The Klang Farmer** project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
## [0.3.2] - 2026-10-07 "Agent Infrastructure & Editor Upgrades"

### Added
- **The Klang Editor Unified Filterable Master Tree**:
  - Replaced dual `CONTROLS`/`LAYOUTS` tabs with a single unified `masterTree`.
  - Added 3-button filter toolbar (`[Controls]`, `[Layout]`, `[Theme]`) with Smart Minimum enforcement.
  - Implemented simultaneous property editing: selecting a parameter node in `[Controls]` allows editing DSP numerical bounds and text descriptions side-by-side with smart routing to respective JSON files on save.
- **Dedicated Centralized Text Schema (`assets/text/strings.json`)**:
  - Extracted 100% of parameter descriptions and choice tooltips out of `assets/controls/*.json` into `assets/text/strings.json` under modular namespaces (`"shared"`, `"farmer"`, `"planter"`).
  - Maintained parameter `"name"` in `assets/controls/` as the immutable DAW/Host automation contract.
  - Implemented seamless startup text merge in `ParameterManager` for zero C++ call-site breakage.
- **Developer Logging Subsystem (`TKS_LOG`) & Diagnostics Engine**:
  - Header-only `RlyehSound::DevLogger` (`source/DevLogger.h`) routing simultaneously to system debugger (`OutputDebugString` / `DBG`) and a 5 MB rotating log file (`%LOCALAPPDATA%/TheKlangSuite/dev.log`).
  - Strict audio thread safety guard: runtime assertion (`jassert`) and instant abort before memory allocation or locking.
  - Preprocessor elision: all logging macros compile to empty `do {} while (false)` no-ops in Release builds (`!JUCE_DEBUG`).
  - Unit test suite (`test/DevLoggerTest.h`) validating output formatting, audio thread safety, and log file creation.
- **Repository Architecture & Governance**:
  - Authored official architectural taxonomy and reference dictionary in `docs/GLOSSARY.md`.
  - Codified the "Silent Git Tag" policy for non-breaking architectural refactors in `GEMINI.md` and `GLOSSARY.md`.
  - Codified the "Pro Sanity & Standards Gate" (Flash Drafts, Pro Audits, Flash Builds) in `GEMINI.md`.

### Changed
- **DSP Block File Naming Standardization**:
  - Renamed legacy plural files: `modulator.json`, `filter.json`, `envelope.json`. Purged old plural files from tree.
- **Guardrails & Skills Optimization Audit**:
  - Reduced `GEMINI.md` prompt footprint by 49.3% (~2,500 tokens/turn saved).
  - Synchronized workspace (`.agents/skills/`) and global (`~/.gemini/config/skills/`) stores with 100% exact parity.
  - Updated `audiothread-guard` to flag `TKS_LOG*` macros inside audio loops.
- **Test Suite Expansion**:
  - Expanded `gui_tests` to 294 automated tests across 12 suites (100% assertion pass rate).
  - 100% pass across all 22 modular drum DSP tests.

---

## [0.3.1] - 2026-10-06 "Editor Quality & Data Schema"

### Added
- **The Klang Planter Limiter Right-Click CalloutBox**:
  - Floating mini-card launched by right-clicking the center `LIMIT` badge, exposing Gain, Ceiling, and Release controls.
- **Interactive Two-Line Status Bar**:
  - Line 1: Permanent control name, formatted value readout, and contextual mouse shortcuts.
  - Line 2: Dynamic tooltip feed and parameter descriptions toggled via header button.
- **6-Pillar VST3 Parameter Validation Suite & pluginval Runner**:
  - Dynamic parameter reflection, normalization roundtrip, boundary safety clamping, JSON parity, state serialization roundtrip, and audio smoke passes.
  - Portable headless runner script `tools/run_pluginval.ps1`.
- **The Klang Editor Tree UX & Property Binding**:
  - Mini-toolbar with `Expand All` and `Collapse All` icon buttons.
  - Headless component test suite testing 100% parameter selection and binding.

### Changed
- **Planter Visualizer Latency Remediation**:
  - Enabled `setOpaque(true)` and downsampled scope resolution to 64 points.
  - Aligned Planter timer rate with Farmer (30 Hz down from 60 Hz), eliminating UI thread latency.
- **Parameter Metadata Extraction**:
  - Extracted hardcoded descriptions from `FarmerEditor.cpp` into data assets.

---
## [0.3.0] - 2026-10-06 "The Architecture Update"

### Added
- **The Klang Suite Umbrella Identity**:
  - Unified project architecture encompassing three synchronized targets: **The Klang Farmer** (flagship 22-module paged modular synthesizer), **The Klang Planter** (compact 8-module single-voice FM percussion synthesizer), and **The Klang Editor** (standalone JSON tooling and visual layout inspector).
  - Common base class architecture (`KlangCoreProcessor` and `KlangCoreEditor`) eliminating boilerplate across plugin and standalone applications.
- **Data-Driven Architecture & Parameter Reflection**:
  - Complete migration of all DSP parameter declarations, min/max ranges, defaults, quick-snaps, string labels, and UI tooltips to modular JSON files (`assets/controls/`).
  - Runtime reflection engine (`RlyehSound::ParameterManager`) embedding JSON assets into `BinaryData` with automated APVTS layout generation and dynamic layout binding.
- **The Klang Editor (TKE Standalone Application)**:
  - Dedicated visual inspection and parameter manipulation tool for sound designers and developers.
  - Live card layout preview, interactive controls inspector, real-time JSON validation, and automated snapshot export/import/restore workflows.
- **Automated Universal Test Harness (`gui_tests` & `dsp_tests`)**:
  - Headless GUI test harness with 82 automated assertions verifying mouse simulation, APVTS synchronization, page switching, modal interactions, offscreen smoke painting, and dynamic reflection guardrails.
  - DSP test suite with 32 real-time audio-thread safety checks verifying zero allocations, zero mutexes, FastMath accuracy, filter slopes, slop random distribution, and brickwall limiting.
- **Automated GitHub Release Version Checker & Settings Modal**:
  - Asynchronous background worker (`VersionChecker`) querying the GitHub Releases API on startup with semantic version comparison (`MAJOR.MINOR.PATCH`).
  - Subtle, glowing emerald header badge (`UpdateBadgeButton`) notifying users when a newer release is published.
  - Comprehensive Settings & About modal (`SettingsModalComponent`) featuring a startup check toggle, manual "Check Now" button, build and system metadata panel, and quick links to the repository and issue tracker.
- **The Klang Planter Dedicated Limiter Card**:
  - Replaced soft-saturation `tanh` approximation with the true lookahead brickwall `LimiterBlock` from The Klang Farmer.
  - Exposed dedicated `cardLimiter` controls in The Klang Planter: Limiter Enable, Input Gain, Threshold, and Release.
- **Zero-Cost macOS FOSS Distribution Pipeline**:
  - Automated `.pkg` installer generated via native `pkgbuild` & `productbuild` in GitHub Actions.
  - Automated root `postinstall` script stripping the Gatekeeper quarantine flag (`xattr -rd com.apple.quarantine`) from all VST3, AU, and Standalone bundles for instant DAW recognition.
  - Stylized drag-and-drop `.dmg` with filesystem symlinks, standalone `Fix_Mac_Permissions.command` script, and illustrated `macOS_Install_Guide.html`.
- **Automated v0.2.0 Parity Audit**:
  - Automated cross-reference script verifying all 204 legacy parameters against v0.3.0 JSON definitions with 0 regressions. Generated slide-deck printable PDF and HTML audit reports.
- **Toolchain & Framework Upgrade**:
  - Upgraded toolchain to modern C++20 (`CMAKE_CXX_STANDARD 20`) and JUCE 9.0.3.
  - Enforced strict timer hygiene (`stopTimer()` on destructors) across all `juce::Timer` subclasses to prevent JUCE 9 unload crashes.

### Changed
- **Comprehensive Cruft Purge & 8-Slot Multi-Instance Architecture**:
  - Purged all obsolete legacy fixed-slot FX blocks and orphaned APVTS parameters.
  - Fully standardized on 8 assignable multi-instance FX slots (4 Pre-Amp, 4 Post-Amp) capable of hosting any of the 13 DSP algorithms in any sequence or combination.
- **Quickstart Guide Copywriting**:
  - Refined Quickstart Guide modal in The Klang Farmer to document the Settings & About modal, the Trigger audition button, and updated signal flow.
- **Repository URLs**:
  - Updated all internal links and release checker endpoints to `https://github.com/codygratner/TheKlangSuite`.

---

## [0.2.0] - 2026-10-04

### Added
- Updated documentation and bumped version to 0.2.0.


## [0.1.6] - 2026-10-01

### Added
- **The Klang Planter**: A compact, dedicated 8-module single-voice FM drum synthesizer companion plugin and standalone application built alongside The Klang Farmer in a unified dual-target CMake build:
  - **Top Row Modules**: Carrier (Red/Cyan), Modulator (Cyan/Red), Pitch Envelope (Silver/Dark Grey), Noise Transient (Dark Grey/Silver).
  - **Bottom Row Modules**: Filter (Blue/Amber), Filter Envelope (Amber/Blue), Amplifier (Green/Magenta), Amp Envelope (Magenta/Green).
  - **FM / NOISE Pre-Filter Crossfader**: Dedicated bipolar crossfader on Noise Transient Knob 4 (-100% Noise only, 0% Both signals at full volume, +100% FM pair only) feeding directly into the filter stage, with inverted visual styling matching Pitch Envelope sliders.
  - **Amplifier Velocity Controls**: Knob 3 controls Velocity Slope (Linear default, Exponential double-click); Knob 4 controls Velocity Floor (1% to 100%, 50% default).
  - **Permanent Master Limiter & Visualizer LIMIT Badge**: Permanent transparent soft-saturation brickwall limiting on the master output with an illuminated real-time warning badge on the header mini-oscilloscope.
  - **Dynamic Modulator Controls**: S&H Rate and DJ Filter dynamically mapped for Cyclic and Noise modes.
- **Pre-Filter Drive across Both Plugins**: Converted filter drive in both The Klang Farmer and The Klang Planter to pre-filter saturation (`tanh`), driving harmonics into the SVF stages rather than post-filter clipping. Labeled `"Pre-Filter Drive"` on the Filter Envelope card and `"Pre-Limiter Drive"` on the Amplifier card.
- **True FM Ratio Calibration**: Re-calibrated FM ratios across both plugins to standard operator range: `1:32.0` to `1:1` to `32.0:1` with exact `1:1` center detent.
- **Widened Band Reject Filter Base Q**: Calibrated 0% resonance notch base Q from 0.707 down to 0.25 for a noticeably deeper, wider band reject notch scoop across both plugins.
- **Admin Deployment Batch Script**: Added `deploy_vst3.bat` with automatic UAC self-elevation to reliably install builds to `C:\Program Files\Common Files\VST3\`.

---

## [0.1.5] - 2026-10-01

### Added
- **Visualizer OFF Switch**: Added an `OFF` toggle badge to the right of the Visualizer header title (beside the Lock icon). Lit dim grey (`#687488`) when running (default); glowing signal red (`#ff3b5c`) with crimson background and border when turned off. When off, live oscilloscope, Bode, and filter analysis calculations are completely suspended, displaying a motionless flat reference line to reduce CPU usage and visual distraction.
- **Carrier Frequency Modulation Range (No Needle)**: Active carrier `MOD DEPTH` now dynamically highlights the affected modulation span directly on the Carrier pitch / offset slider as a modulation bar, without rendering the real-time needle indicator (preventing visual clutter and distraction from audio-rate FM oscillations).
- **INIT 3-Option Confirmation Dialog**: Clicking `INIT` launches an interactive confirmation modal with three distinct choices:
  - `Default`: Restores factory patch state (default synth parameters and default loaded FX rack).
  - `Clean`: Restores factory synth parameters while stripping all 8 Pre-FX and Post-FX slots to "None" (empty racks with blank plates).
  - `Cancel`: Dismisses the dialog leaving the current state untouched.
- **Multi-Platform Automated Cloud CI/CD (GitHub Actions)**: Added automated workflows building macOS Universal Binaries (AU, VST3, Standalone for Apple Silicon & Intel), Linux x86_64, and Windows x64 with automated testing and GitHub release asset publishing.
- **GPLv3 Licensing**: Standard GNU General Public License v3.0 license text (`LICENSE`) and repository compliance for free open-source JUCE tier.

### Changed
- **Global DJ Filter Defaults & Double-Click Resets**: Standardized all DJ Filter controls across the plugin (`Cyclic Noise`, `S&H Noise`, `Bell EQ`, `Drive` post-filter, `Wave Folder` post-filter, and `Noise Transient` filter) to default to `0%` (center `0.5` normalized) with double-click reset to `0%`.
- **S&H Noise Modulator Architecture**:
  - Parameter 3 (Knob 0 on the card) is now a bipolar DJ Filter, matching Cyclic Noise behavior.
  - Parameter 4 (Knob 1 on the card) is now the S&H Clock Rate, labeled `"Speed"`, frequency-formatted from $0.1\,\text{Hz}$ to $24\,\text{kHz}$, defaulting to $24.00\,\text{kHz}$ with double-click reset to $24\,\text{kHz}$.
  - The Pitch Envelope accurately sweeps this S&H clock rate up to $\pm 5$ octaves when targeted at the modulator.
  - Mode switching in the UI automatically initializes sensible defaults (DJ filter to 0%, S&H speed to 24 kHz).
- **Percussive Decay Envelope Clarification**: Formally clarified across documentation that all envelopes are pure percussive decay envelopes with variable exponential/linear/logarithmic slope curves (no sustain or DAHDSR stages).
- **Project Version**: Bumped project version to `0.1.5` in `CMakeLists.txt`, `source/PluginEditor.cpp`, and top header version badge.

---

## [0.1.4] - 2026-10-01

### Added
- **Dynamic FX Slot Parameter Naming in Mod Targets**: Mod envelope target dropdown dynamically inspects loaded FX types in Pre-FX and Post-FX slots and labels parameters accordingly (e.g. `Post FX 1 [Ring Mod]: Param 1 [Waveform]` or `Post FX 1 [Empty]: Param 1 [Empty]`). Names update dynamically on effect changes.
- **Voice 2 Inverted Color Palette**: Voice 2 modules (Carrier 2, Modulator 2, Pitch Env 2, Filter 2, Filter Env 2) now invert their accent and complementary colors (180° hue rotation) compared to Voice 1, giving Voice 2 an instantly distinct visual identity.

### Changed
- **Page 6 Row Swap**: Moved Mod Envelopes 1–3 to the top row and Velocity, Key Tracking, and Slop to the bottom row for a more ergonomic modulation workflow.
- **Mod Destinations Cleanup**: Removed 34 phantom standalone effect parameters from the modulation routing matrix, leaving 108 live, continuously modulated parameters.
- **Removed Model Code Text**: Removed "A-138" lettering from Mixer and Limiter silver faceplates.
- **Project Version**: Bumped project version to `0.1.4` in `CMakeLists.txt`, `source/PluginEditor.cpp`, and top header version badge.

---

## [0.1.3] - 2026-10-01

### Added
- **3 Freely Assignable Modulation Envelopes**: Fast percussive decay envelopes with bipolar modulation depth (-100% to +100%) and flexible slope curve shaping (Exponential, Linear, Logarithmic).
- **Real-Time Modulation Visualization**: Animated lower indicator bar on modulated Arcade HP sliders displaying live modulation range and instantaneous position; right-click callout popup to edit base parameter value while monitoring modulation span.
- **Sub-Block Modulation Smoothing**: Real-time sample-accurate sub-block modulation matrix routing across all active synthesis and effect parameters.
- **Key Tracking Module**: Pitch tracking module with configurable root key, tracking rate, and key center.
- **Pitch Envelope OPP Mode**: Added `OPP` (Opposite / Inverse) mode to pitch envelopes alongside `CAR`, `MOD`, and `BOTH`, driving carrier and modulator pitch in reciprocal directions.
- **Phase Smear 2nd / 4th Order Selector**: Replaced PhaseSmear on/off switch with a 2nd Order / 4th Order all-pass filter cascade selector for standard or extreme phase dispersion.

### Changed
- **Complementary Color Panel Tints**: Added 10–15% complementary color tinting to module cards, enhancing visual separation across the modular rack.
- **Filter Resonance Recalibration**: Calibrated 100% resonance to sit right at the threshold of self-oscillation across -12dB and -24dB filter topologies.
- **Comb Filter & Frequency Shifter Bipolar Mix**: Replaced Comb Filter on/off toggle with bipolar wet/dry mix (-100%:0% to +100%:0%), defaulting to +50%:50% with double-click reset to 0%:100% (dry). Updated Frequency Shifter default and reset behavior to match.
- **Drive Effect Defaults**: Changed default drive gain to +6dB with double-click reset to 0dB.
- **Doepfer A-138 Silver Mixer Faceplate**: Redesigned the Mixer module (Slot 8) with an authentic brushed-aluminum Eurorack faceplate aesthetic featuring countersunk dark corner rack screws, solid black screenprinted DIN typography, red model code badge (`A-138`), and a dark industrial bezel frame.
- **Mixer White Recessed Troughs & Dynamic Text Inversion**: Upgraded the Mixer sliders to recessed white/light-satin troughs with signal red (`#e53935`) level fills and real-time dual-pass text clipping (labels and values dynamically invert from sharp black over the white trough to pure white over the red fill bar).
- **Limiter Red Accent Alignment**: Styled `Pre Limiter`, `Post Limiter`, and the `Amp Limiter` toggle switch with the same signal red accent (`#e53935`) while keeping the standard dark slate chassis, creating visual unity between gain staging and dynamic control modules.

---

## [0.1.2] - 2026-09-30

### Added
- **Release Packaging**: Automated packaging of distribution builds into separate zip archives:
  - `TheKlangFarmer-v0.1.2-VST3.zip` (DAW plugin bundle)
  - `TheKlangFarmer-v0.1.2-Standalone.zip` (standalone desktop executable)

### Changed
- **Carrier Pitch Tracking Overhaul (`MIDI`, `Freq`, `Note`)**: Revamped Carrier 1 & 2 pitch tracking from the legacy `MIDI / Fixed / Offset` design:
  - **`MIDI` Mode**: Tracks incoming MIDI note with semitone offset constrained to $\pm 24\text{ st}$ (default $0\text{ st}$), displaying a center-split bipolar meter with center detent mark.
  - **`Freq` Mode**: Fixed continuous frequency slider from $20\,\text{Hz}$ to $24\,\text{kHz}$ (default $55\,\text{Hz}$), unipolar fill.
  - **`Note` Mode**: Fixed musical note across MIDI notes 0–127 (`C-1` to `G9`, default `A1 = 55 Hz = note 33`) with padded fixed-width readout `note name [frequency, midi note number]` (e.g. `A1   [   55 Hz,  33]`), preventing layout shift or text jumping while dragging.
- **Arcade HP Meter-Style Sliders**: Upgraded parameter controls from rotary knobs and separate value boxes into fighting-game "HP meter" horizontal bar meters featuring embedded left-aligned labels, static right-aligned value readouts, bidirectional center-split fills for bipolar parameters, and 2D mouse dragging.
- **Card Geometry & Typography Scaling**: Scaled up control heights, fonts, and spacing within the existing 1040 × 740 px chassis for improved readability and ergonomic tweaking.
- **Visualizer Layout Reorganization**: Relocated the Real-Time Phase-Locked Visualizer from Slot 8 (bottom-right) to Slot 5 (bottom-left), anchoring both left-hand corner slots (Slot 1 Navigation and Slot 5 Visualizer) as permanent anchors across all 7 pages.
- **Alphabetized Effects Catalog**: Reordered the 13 multi-instance effect processors alphabetically (`Bell EQ` through `Wave Folder`) across all FX picker dropdowns, DSP block dispatch, Quickstart modal, and documentation (`spec.md`, `README.md`).
- **Project Version**: Bumped project version to `0.1.2` in `CMakeLists.txt` and VST3 `moduleinfo.json`.

---

## [0.1.1] - 2026-09-30

### Added
- **UI Header Version Badge**: Dynamic version badge (`v0.1.1`) drawn directly in the top navigation header bar next to the plugin title, linked automatically to `JucePlugin_VersionString`.
- **Release Packaging**: Automated packaging of distribution builds into separate zip archives:
  - `TheKlangFarmer-v0.1.1-VST3.zip` (DAW plugin bundle)
  - `TheKlangFarmer-v0.1.1-Standalone.zip` (standalone desktop executable)
- **Specification Section 5**: Formalized Semantic Versioning, release workflows, and packaging standards in `spec.md`.
- **Changelog**: Comprehensive project changelog tracking all development milestones.

### Changed
- **Vendor Name**: Updated plugin vendor from "CustomDSP" to "R'lyeh Sound" (manufacturer code `Rlyh`, bundle ID `com.rlyehsound.theklangfarmer`).
- **Project Version**: Bumped project version to `0.1.1` in `CMakeLists.txt` and VST3 `moduleinfo.json`.
- **Header Layout**: Adjusted title bar bounds to prevent subtitle clipping or button collisions at smaller window sizes.

---

## [0.1.0] - 2026-09-30

### Added
- **2x4 Paged Eurorack-Style Chassis**:
  - Re-architected interface into an ergonomic 8-slot, 2-row by 4-column layout (1040 × 740 px).
  - **Slot 1 (Top Left)**: Fixed **Navigation Module** providing instant 1-click access across 7 dedicated functional pages:
    1. `Voice 1`
    2. `Voice 2`
    3. `Transients`
    4. `Pre-Amp FX`
    5. `Amplifier`
    6. `Post-Amp FX`
    7. `Modulations`
  - **Slot 8 (Bottom Right)**: Fixed **Visualization Module** presenting real-time self-locked oscilloscopes, logarithmic Bode plots, and transfer curves.
- **Multi-Instance FX Engine (8 Independent FX Slots)**:
  - Enabled free assignment of any of the 13 effect processors into any of the 4 Pre-Amp FX and 4 Post-Amp FX slots.
  - Users can instantiate up to 8 independent copies of the same effect in series (e.g., 8 cascaded wavefolders) or any combination thereof.
  - Dedicated parameter sets, smoothing, DSP instances, and scope buffers for each slot.
- **New 4-Knob FX Processors**:
  - **Chorus**: Dual quadrature sine LFOs, stereo fractional delay lines, soft-saturating feedback.
  - **Phaser**: 6-stage cascaded allpass ladder per channel with feedback soft-clipping.
  - **Flanger**: Sub-millisecond delay line (0.2 ms – 5 ms) with bipolar feedback (-95% to +95%).
  - **Tempo Delay**: Pre-allocated stereo delay synced to host musical divisions (1/32 through 1/2) with damping tone filter and ping-pong crossfeed.
- **Dynamic Auto-Tracking & Lockable Visualizer**:
  - Automatically switches the visualizer to inspect whichever module card the user interacts with or hovers over.
  - **Lock / Padlock Toggle**: Allows the user to freeze the visualizer on a specific module while tweaking others.
  - **Top-Left Block Readout**: Always displays the currently monitored module name.
- **Non-Scrollable Quickstart Guide Modal**:
  - On-screen modal overlay summarizing signal flow, page shortcuts, and controls on a single, easy-to-read screen.
- **Tactile LED Button Selectors**:
  - Converted comboboxes and dropdowns into vintage hardware-style tactile illuminated push buttons.

### Changed
- **Renamed to The Klang Farmer**: Complete rebranding across CMake targets, processor, editor, and workspace definitions.
- **Filter Topology**: Replaced "Off" filter bypass with pure 6 dB/oct gentle slope; guaranteed active filtering throughout the signal chain.
- **Carrier Modulation Depths**: Converted Carrier 1 & Carrier 2 FM depth controls to true bipolar ranges (-100% to +100%).

---

## [0.0.3] - 2026-09-29

### Added
- **Module 17: Slop Stepped Randomization**:
  - Analog drift simulation with independent stepped random modulation for Frequency, Depth, Decay, and Stereo Pan.
  - Defaults to zero for sample-accurate repeatability until introduced.
- **Module 16: Velocity Sensitivity**:
  - Dynamic response curves mapping MIDI velocity to volume, pitch decay, and modulation depth.
  - Bipolar modulation depth sensitivity (higher velocities increase positive values and decrease negative values).
  - Exponential response curves for musical dynamic control.
- **Renoise & DAW Offline Bounce Stability**:
  - Enforced zero dynamic heap allocations (`malloc`/`new`) on the audio thread.
  - Fixed variable buffer rendering crashes during offline DAW exports.

### Changed
- Extended audio frequency ranges across all filters and oscillators up to Nyquist / 24 kHz.
- Upgraded Grit effect to allow 100% transparent bypass at unity drive.

---

## [0.0.2] - 2026-09-29

### Added
- **Cascaded 2nd-Order Allpass PhaseSmear**:
  - Multi-stage phase smearing module for authentic acoustic transient "zapping" and physical strike dispersion.
  - True energy-conserving allpass implementation.
- **Dedicated Comb Filter Module**:
  - Feedback comb resonator separated into an independent module preceding the main amplifier.
- **Init to Defaults Button**:
  - Quick-reset button with confirmation safeguard restoring all parameters to factory defaults.
  - Double-click reset on all rotary knob sliders.

### Changed
- Optimized typography and layout spacing across modular card panels.
- Fixed frequency shifter negative shift direction and non-linear range warping.

---

## [0.0.1] - 2026-09-29

### Added
- **Initial Modular Drum Voice Engine**:
  - Dual FM synthesis core inspired by boutique analog percussion synthesizers (Basimilus Iteritas Alter & Electribe ER-1).
  - Two Carrier/Modulator FM operator pairs with 5-point warp decay curves.
  - Filter envelopes, transient noise generator, and multi-clap burst generator.
  - Cross-voice ring modulation (`Carrier 1 × Carrier 2`).
  - Self-locked real-time oscilloscopes for carriers and modulators.
- **DSP Test Suite**:
  - Unit test executable (`dsp_tests.exe`) validating audio generation, parameter stability sweeps, filter topologies, and energy conservation.
- **CMake & JUCE 8 Architecture**:
  - C++20 build pipeline compiling both VST3 plugin and Standalone executables.
