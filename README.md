# The Klang Farmer

> **A 22-Module Paged Modular Dual-FM Synthesis Drum Voice**  
> Built in modern C++20 with JUCE 8. Available as a **VST3 Plugin** and **Standalone Application**.

[![Release](https://img.shields.io/github/v/release/codygratner/TheKlangFarmer?color=4a9eff&label=Release)](https://github.com/codygratner/TheKlangFarmer/releases)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Framework](https://img.shields.io/badge/JUCE-8-orange.svg)](https://juce.com/)
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

## Key Features

### 1. Dual FM Voice Engine & Per-Voice Filtering
- **Two FM Operator Pairs**: Independent `Carrier 1 & Modulator 1` and `Carrier 2 & Modulator 2` engines with 5-point warp decay curves.
- **Bipolar Modulation Depth**: FM depths range from **-100% to +100%** (defaulting to 0% center).
- **Per-Voice Multimode Filters**: Each voice features its own dedicated multimode filter (LPF, BPF, HPF, BRF with 6/12/18/24 dB slopes) and filter envelope before hitting the mixer.
- **Cross-Voice Ring Modulation**: Raw cross-multiplication (`Carrier 1 × Carrier 2`) routeable into the main mixer as an independent blendable tone.

### 2. Multi-Instance FX Engine (8 Independent Slots)
- **4 Pre-Amp FX Slots** and **4 Post-Amp FX Slots**.
- **No Slot Restrictions**: Any of the 13 effect processors can be instantiated into any slot. You can cascade up to 8 instances of the exact same effect in series (e.g., 8 wavefolders) or mix and match freely.
- **13 Effect Processors**:
  1. **Wavefolder / Grit**: High-gain harmonic folder with transparent unity bypass.
  2. **Drive / Overdrive**: Soft-clipping saturation with tone tilt.
  3. **Bitcrusher**: Word-length reduction and sample-rate reduction.
  4. **Comb Filter**: Resonant feedback delay line for metallic/karplus-strong tones.
  5. **Disperser**: Cascaded 2nd-order allpass filters for authentic acoustic strike "zapping" and phase smearing.
  6. **Frequency Shifter**: True quadrature single-sideband frequency shifting (+/- 50 Hz or +/- 500 Hz).
  7. **Bell EQ**: Peaking parametric boost/cut and DJ-style dual filter tilt.
  8. **Chorus**: Dual quadrature sine LFOs driving stereo fractional delays.
  9. **Phaser**: 6-stage cascaded allpass ladder with regenerative feedback.
  10. **Flanger**: Sub-millisecond delay line (0.2 ms – 5 ms) with bipolar feedback (-95% to +95%).
  11. **Tempo Delay**: Pre-allocated stereo delay synced to host musical divisions (1/32 to 1/2) with damping tone filter and ping-pong crossfeed.
  12. **Brickwall Limiter**: Lookahead peak-limiting and dynamics control.
  13. **None / Bypass**: Bypasses the slot and renders a brushed-aluminum blank rack plate.

### 3. Transients & Modulation
- **Noise Transient Generator**: Dedicated filtered white/pink noise burst with its own filter envelope (`Filter 3 + Filter Env 3`).
- **Multi-Clap Generator**: Multi-burst envelope generator simulating handclap flam bursts.
- **Slop (Module 17)**: Stepped random analog drift applied per trigger across frequency, FM depth, decay, and stereo panning (defaults strictly to 0% for sample-accurate repeatability).
- **Velocity Dynamics (Module 16)**: Dynamic mapping curves scaling output volume, pitch decay, and bipolar modulation depth with exponential response.

### 4. Real-Time Phase-Locked Visualizations (Slot 8)
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
1. **Voice 1**: Pitch Envelope 1, Modulator 1, Carrier 1, Filter 1, Filter Env 1
2. **Voice 2**: Pitch Envelope 2, Modulator 2, Carrier 2, Filter 2, Filter Env 2
3. **Transients**: Clap Burst, Noise Transient, Filter 3, Filter Env 3
4. **Pre-Amp FX**: Pre FX 1..4 (Assignable to any of the 13 processors) + Pre-Amp Limiter
5. **Amplifier**: Main Mixer (Carrier 1, Carrier 2, RingMod, Noise, Clap, FX Return), Amp Envelope, Master Limiter
6. **Post-Amp FX**: Post FX 1..4 (Assignable to any of the 13 processors) + Post-Amp Limiter
7. **Modulations**: Velocity Modulation, Slop Stepped Randomizer

---

## Changelog
For a detailed chronological record of updates and releases, see [CHANGELOG.md](CHANGELOG.md).

---

## License
Created by Cody Gratner (R'lyeh Sound).
