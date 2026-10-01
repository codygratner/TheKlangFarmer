# The Klang Farmer

> **A 22-Module Paged Modular Dual-FM Synthesis Drum Voice**  
> Built in modern C++20 with JUCE 8. Available as a **VST3 Plugin** and **Standalone Application**.

[![Release](https://img.shields.io/github/v/release/codygratner/TheKlangFarmer?color=4a9eff&label=Release)](https://github.com/codygratner/TheKlangFarmer/releases)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Framework](https://img.shields.io/badge/JUCE-8-orange.svg)](https://juce.com/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Format](https://img.shields.io/badge/Format-VST3%20%7C%20Standalone-brightgreen.svg)](#installation)

> [!NOTE]
> **AI / Pair-Programming Note**:  
> **The Klang Farmer was vibe-coded with Google Gemini using Antigravity.** From initial DSP architecture and modular voice routing to real-time oscilloscopes, Bode plots, zero-allocation real-time audio threads, and hardware-style UI ergonomics, the entire project was designed, implemented, and iteratively refined through autonomous pair-programming with Gemini in Antigravity.

---

![The Klang Farmer GUI](screenshots/TheKlangFarmer_GUI.png)

---

## Overview

**The Klang Farmer** is a boutique digital/analog hybrid percussion synthesizer designed to replicate the tactile immediacy, aggressive harmonic palette, and raw sonic punch of modular Eurorack drum synthesizers (such as the *Noise Engineering Basimilus Iteritas Alter* and *Korg Electribe ER-1*).

It arranges 22 discrete sound-shaping modules inside an ergonomic **2-row by 4-column (2x4)** modular rack (1040 &times; 740 px). The interface provides immediate access to dual FM voice pairs, multimode filters, noise and burst transients, 8 multi-instance FX slots, analog drift emulation, and real-time phase-locked visualizers.

---

## Gallery

| Voice 1 (Standard Palette) | Voice 2 (Inverted Complementary Palette) |
| :---: | :---: |
| ![Voice 1](screenshots/page0_voice1.png) | ![Voice 2](screenshots/page1_voice2.png) |

| Amplifier & Master Limiter | Modulations Rack (Mod Envelopes + Dynamics) |
| :---: | :---: |
| ![Amplifier](screenshots/page4_amplifier.png) | ![Modulations](screenshots/page6_modulations.png) |

| Real-Time Modulation Visualization | Right-Click Callout Popup |
| :---: | :---: |
| ![Modulation Visualizer](screenshots/page0_voice1_modulated.png) | ![Callout Popup](screenshots/callout_popup_modulated.png) |

---

## Key Features

### 1. Dual FM Voice Engine & Per-Voice Filtering
- **Two FM Operator Pairs**: Independent `Carrier 1 & Modulator 1` and `Carrier 2 & Modulator 2` engines with 5-point warp decay curves.
- **Carrier Pitch Tracking Modes**: Select between `MIDI` (tracks incoming MIDI with $\pm 24$ st semitone offset, center-detented bipolar meter), `Freq` (fixed continuous frequency from $20\,\text{Hz}$ to $24\,\text{kHz}$, defaulting to $55\,\text{Hz}$), and `Note` (fixed musical note across notes 0–127 with padded readout e.g. `A1   [   55 Hz,  33]`).
- **Pitch Envelope with Inverse Mode**: Dedicated pitch envelopes for each voice feature `Car`, `Mod`, `Both`, and `Opp` (Opposite / Inverse mode, driving carrier and modulator pitch in reciprocal directions).
- **Distinct Voice Palettes**: Voice 2 features an inverted complementary color palette (180° rotated hues on accents and card background tints) for instant visual differentiation.
- **Per-Voice Multimode Filters**: Each voice features its own dedicated multimode filter (LPF, BPF, HPF, BRF with 6/12/18/24/36 dB slopes) with resonance calibrated right to the edge of self-oscillation.
- **Cross-Voice Ring Modulation**: Raw cross-multiplication (`Carrier 1 × Carrier 2`) routeable into the 4-channel mixer.

### 2. Multi-Instance FX Engine (8 Independent Slots)
- **4 Pre-Amp FX Slots** and **4 Post-Amp FX Slots**.
- **No Slot Restrictions**: Any of the 13 effect processors can be instantiated into any slot. You can cascade up to 8 instances of the exact same effect in series (e.g., 8 wavefolders) or mix and match freely.
- **13 Effect Processors**:
  1. **Bell EQ**: Peaking parametric boost/cut and DJ-style dual filter tilt with Bode magnitude visualizer.
  2. **Chorus**: Dual quadrature sine LFOs driving stereo fractional delays with soft-saturation feedback.
  3. **Comb Filter**: Resonant feedback delay line with bipolar wet/dry mix (-100%:0% to +100%:0%, default +50%:50%, double-click reset to dry).
  4. **Phase Smear**: Cascaded all-pass filter network with selectable **2nd Order** or **4th Order** topology for subtle acoustic strike zapping or extreme dispersion.
  5. **Drive**: Soft-clipping saturation with DC bias, post-filter, and limiter (+6 dB default gain, 0 dB double-click reset).
  6. **Filter**: Standalone multi-mode filter (LPF, BPF, HPF, BRF) with selectable slopes (6 dB to 36 dB/oct) and Bode visualizer.
  7. **Flanger**: Sub-millisecond delay line (0.2 ms – 5 ms) with bipolar resonant feedback (-95% to +95%).
  8. **Frequency Shifter**: True quadrature single-sideband frequency shifting with bipolar wet/dry blend (+50%:50% default, double-click reset to dry).
  9. **Grit FX (Bitcrusher)**: Word-length reduction (1.0 to 16.0 bits), sample-rate reduction, and dual-shelf tone shaping.
  10. **Phaser**: 6-stage cascaded allpass ladder with regenerative feedback.
  11. **RingMod**: Morphable oscillator ring modulation with rate, amount, and stereo phase width.
  12. **Tempo Delay**: Pre-allocated stereo delay synced to host musical divisions (1/32 to 1/2) with damping tone filter and ping-pong crossfeed.
  13. **Wave Folder**: High-gain harmonic folder with DC bias and post-filter.
  *(Or **None / Bypass** to bypass the slot and render a brushed-aluminum blank rack plate).*

### 3. Transients & Modulation Matrix
- **Noise Transient Generator**: Dedicated filtered white/pink noise burst with its own filter envelope (`Filter 3 + Filter Env 3`).
- **Multi-Clap Generator**: Multi-burst envelope generator simulating handclap flam bursts.
- **3 Freely Assignable Mod Envelopes (Top Row)**:
  - Dedicated percussive decay envelopes with bipolar depth (-100% to +100%) and flexible slope curvature (Exponential, Linear, Logarithmic).
  - Dropdown target selector with **dynamic loaded-FX naming** (e.g. `Post FX 1 [Ring Mod]: Param 1 [Waveform]` or `Post FX 1 [Empty]: Param 1 [Empty]`) across 108 continuous synthesis and effect parameters.
- **Dynamics & Analog Drift (Bottom Row)**:
  - **Velocity Dynamics**: Dynamic scaling of output volume, envelope decay, and modulation depth.
  - **Key Tracking**: Scales envelope depth, decay, and level relative to incoming MIDI note.
  - **Slop**: Stepped random analog drift per trigger across frequency, FM depth, decay, and stereo panning.
- **Real-Time Modulation Visualization**:
  - Modulated parameters display an animated lower indicator bar under the slider illustrating the live modulation span and instantaneous needle position.
  - Right-click callout popup allows adjusting base slider values while monitoring live modulation sweep.
- **Silver Brushed Eurorack Faceplates**:
  - Mixer module and Limiters (Pre-Limiter & Master Limiter) feature authentic brushed-aluminum faceplates, corner rack screws, white recessed troughs, and signal red accents with dynamic text inversion.

### 4. Real-Time Phase-Locked Visualizations (Slot 5)
- **Self-Locked Oscilloscopes**: Carriers and modulators phase-lock to their own fundamental frequencies, eliminating visual drift even during deep FM sweeps.
- **Bode Magnitude Plots**: Real-time logarithmic X-Y response curves for filters and bell EQs ($20\,\text{Hz} - 24\,\text{kHz}$) with grid reference markers.
- **Auto-Tracking & Lock Toggle**: The visualizer automatically follows whichever module card you click or hover over. A padlock toggle lets you freeze the display on a specific module while tweaking others.

### 5. Rock-Solid Audio Thread Stability
- **Zero Real-Time Allocations**: Strictly **no dynamic heap memory allocations** (`malloc`/`new`) on the audio rendering thread.
- **DAW & Renoise Bounce Stability**: All burst arrays, delay lines, and FFT/scope buffers are pre-allocated, guaranteeing zero audio dropouts or offline bounce crashes across arbitrary host block sizes (64 to 2048+ samples).

---

## Installation

Download the latest pre-compiled binaries from the **[Releases Page](https://github.com/codygratner/TheKlangFarmer/releases)**:

### VST3 Plugin (Windows)
1. Download `TheKlangFarmer-v<version>-VST3.zip`.
2. Extract `The Klang Farmer.vst3` into your standard VST3 folder:
   ```
   C:\Program Files\Common Files\VST3\
   ```
3. Rescan plugins in your DAW (Ableton Live, FL Studio, Reaper, Cubase, Bitwig, Studio One, Renoise).

### Standalone Executable (Windows)
1. Download `TheKlangFarmer-v<version>-Standalone.zip`.
2. Extract and run `The Klang Farmer.exe`.
3. Select your audio device and MIDI input in the audio settings dialog.

---

## Building from Source

### Prerequisites
- **CMake 3.22+**
- **Visual Studio 2022 or 2026** (with the *Desktop development with C++* workload)
- **Git**

### Build Instructions
```bash
# 1. Clone the repository with submodules/JUCE
git clone https://github.com/codygratner/TheKlangFarmer.git
cd TheKlangFarmer

# 2. Configure CMake
cmake -B build

# 3. Build Release binaries in parallel
cmake --build build --config Release --parallel
```

Compiled binaries will be generated at:
- **VST3**: `build/TheKlangFarmer_artefacts/Release/VST3/The Klang Farmer.vst3`
- **Standalone**: `build/TheKlangFarmer_artefacts/Release/Standalone/The Klang Farmer.exe`
- **Test Runner**: `build/Release/dsp_tests.exe`

### Running DSP Verification Tests
```bash
.\build\Release\dsp_tests.exe
```

---

## Architecture & Layout

The 8-slot, 2x4 layout is organized as follows:

| Slot 1 (Top Left) | Slot 2 | Slot 3 | Slot 4 |
| :---: | :---: | :---: | :---: |
| **Navigation Module** *(Fixed)* | Active Page Card | Active Page Card | Active Page Card |
| **Slot 5 (Bottom Left)** | **Slot 6** | **Slot 7** | **Slot 8 (Bottom Right)** |
| **Visualizer Module** *(Fixed)* | Active Page Card | Active Page Card | Active Page Card |

### Navigation Pages
1. **Voice 1**: Carrier 1, Modulator 1, Pitch Env 1, Filter 1, Filter Env 1, Mixer
2. **Voice 2**: Carrier 2, Modulator 2, Pitch Env 2, Filter 2, Filter Env 2, Mixer (Swapped Palette)
3. **Transients**: Clap Burst, Noise Transient, Filter 3, Filter Env 3, Mixer
4. **Pre-Amp FX**: FX Picker + Pre FX 1..4 (Assignable to any of 13 processors) + Pre Limiter
5. **Amplifier**: Main Amplifier + Amp Envelope + Post Limiter + Mixer
6. **Post-Amp FX**: FX Picker + Post FX 1..4 (Assignable to any of 13 processors) + Master Limiter
7. **Modulations**: Mod Envelopes 1–3 (Top Row) + Velocity, Key Tracking, and Slop (Bottom Row)

---

## Changelog
For a detailed chronological record of updates and releases, see [CHANGELOG.md](CHANGELOG.md).

---

## License

This project is open-source software licensed under the [GNU General Public License v3.0 (GPLv3)](LICENSE).

Copyright &copy; 2026 Cody Gratner (R'lyeh Sound).
