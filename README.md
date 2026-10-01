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
- **Carrier Pitch Tracking Modes**: Select between `MIDI` (tracks incoming MIDI with $\pm 24$ st semitone offset, center-detented bipolar meter), `Freq` (fixed continuous frequency from $20\,\text{Hz}$ to $24\,\text{kHz}$, defaulting to $55\,\text{Hz}$), and `Note` (fixed musical note across notes 0–127 with padded readout e.g. `A1   [   55 Hz,  33]`).
- **Bipolar Modulation Depth**: FM depths range from **-100% to +100%** (defaulting to 0% center).
- **Per-Voice Multimode Filters**: Each voice features its own dedicated multimode filter (LPF, BPF, HPF, BRF with 6/12/18/24 dB slopes) and filter envelope before hitting the mixer.
- **Cross-Voice Ring Modulation**: Raw cross-multiplication (`Carrier 1 × Carrier 2`) routeable into the main mixer as an independent blendable tone.

### 2. Multi-Instance FX Engine (8 Independent Slots)
- **4 Pre-Amp FX Slots** and **4 Post-Amp FX Slots**.
- **No Slot Restrictions**: Any of the 13 effect processors can be instantiated into any slot. You can cascade up to 8 instances of the exact same effect in series (e.g., 8 wavefolders) or mix and match freely.
- **13 Effect Processors**:
  1. **Bell EQ**: Peaking parametric boost/cut and DJ-style dual filter tilt with Bode magnitude visualizer.
  2. **Chorus**: Dual quadrature sine LFOs driving stereo fractional delays with soft-saturation feedback.
  3. **Comb Filter**: Resonant feedback delay line for metallic/karplus-strong tones.
  4. **Disperser**: Cascaded 2nd-order allpass filters for authentic acoustic strike "zapping" and phase smearing.
  5. **Drive**: Soft-clipping saturation with DC bias, post-filter, and hard-clipper limiter.
  6. **Filter**: Standalone multi-mode filter (LPF, BPF, HPF, BRF) with selectable slopes (6 dB to 36 dB/oct) and Bode visualizer.
  7. **Flanger**: Sub-millisecond delay line (0.2 ms – 5 ms) with bipolar resonant feedback (-95% to +95%).
  8. **Frequency Shifter**: True quadrature single-sideband frequency shifting (+/- 50 Hz or +/- 500 Hz).
  9. **Grit FX (Bitcrusher)**: Word-length reduction (1.0 to 16.0 bits), sample-rate reduction, and dual-shelf tone shaping.
  10. **Phaser**: 6-stage cascaded allpass ladder with regenerative feedback.
  11. **RingMod**: Morphable oscillator ring modulation with rate, amount, and stereo phase width.
  12. **Tempo Delay**: Pre-allocated stereo delay synced to host musical divisions (1/32 to 1/2) with damping tone filter and ping-pong crossfeed.
  13. **Wave Folder**: High-gain harmonic folder with DC bias and post-filter.
  *(Or **None / Bypass** to bypass the slot and render a brushed-aluminum blank rack plate).*

### 3. Transients & Modulation Rack
- **Noise Transient Generator**: Dedicated filtered white/pink noise burst with its own filter envelope (`Filter 3 + Filter Env 3`).
- **Multi-Clap Generator**: Multi-burst envelope generator simulating handclap flam bursts.
- **Top-Row Dynamics**:
  - **Velocity Dynamics**: Dynamic mapping scaling output volume, pitch decay, and modulation depth with exponential response.
  - **Key Tracking**: Scales envelope depth, decay, and level relative to incoming MIDI note (0–127, center at note 64).
  - **Slop**: Stepped random analog drift applied per trigger across frequency, FM depth, decay, and stereo panning (defaults strictly to 0% for sample-accurate repeatability).
- **Freely Assignable Mod Envelopes (Mod Env 1, 2, 3)**:
  - 3 dedicated modulation envelopes filling the bottom row of the Modulations page.
  - Each envelope features Slope, bipolar Depth (-100% to +100%), Decay (1 ms – 2000 ms), and a dropdown ComboBox that can modulate **any continuous parameter on the entire synthesizer** (138 destinations across dual voices, filters, mixer, amplifier, and all 8 FX slots).
- **High-Precision Slope Curve Response**:
  - Slope controls feature a linear center at **75%** of slider travel, matching logarithmic response up to 100%, and an extended exponential curve descending down to **4× steeper** curvature at 0%. Default double-click reset set to `0.5886` matches the original classic exponential response.
- **Doepfer-Style Dark Silver Panels**:
  - Mixer module and Limiters (Pre-Limiter & Master Limiter) feature dark silver anodized finishes (`#606060`), red accents, and mounting screws, with satin light troughs on the Mixer and dark troughs on the Limiters for immediate visual contrast in the rack.

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
