# Changelog

All notable changes to **The Klang Farmer** project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
- **Cascaded 2nd-Order Allpass Disperser**:
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
