# The Klang Farmer — Modular Drum Synthesizer Specification

## 1. Architectural & Implementation Highlights

"The Klang Farmer" is a 22-module dual-FM synthesizer drum voice engineered in modern C++20 / JUCE 8. It replicates the tactile immediacy of a boutique modular hardware drum rack within a single 3-row by 8-column Eurorack-style chassis (1840 × 760 px).

### Built Engine & System Capabilities
- **Dual FM Voice Architecture**: Two fully featured carrier/modulator pairs (`Carrier 1` & `Modulator 1`, `Carrier 2` & `Modulator 2`) with dynamic frequency modulation depth ($\pm 200\%$) and cross-voice ring modulation.
- **Renoise & DAW Offline Bounce Stability**:
  - Zero dynamic heap memory allocations (`malloc`/`new`) on the audio rendering thread.
  - Multi-clap burst buffers, envelope signal arrays, and internal voice sub-buffers are pre-allocated with sample-accurate rendering.
  - Guaranteed stability with arbitrary and varying buffer sizes (from 64 up to 2048+ samples) during offline audio rendering.
- **Advanced Real-Time Visualizations**:
  - **Self-Locked Oscilloscopes**: Modulators and Carriers dynamically phase-lock to their own internal fundamental frequency and rising zero-crossings, preventing visual drift or "wibbly wobbly" phase jitter even during detuning, audio-rate FM, or variable pitch envelopes.
  - **X-Y Frequency vs. Gain Response Plots**: Dedicated real-time magnitude response graphs for both the **Filter** and **Bell EQ** blocks. Plotted on a logarithmic frequency axis ($20\,\text{Hz} - 24\,\text{kHz}$) with grid reference marks ($100\,\text{Hz}$, $1\,\text{kHz}$, $10\,\text{kHz}$), a $0\,\text{dB}$ center line, area fill, glowing response trace, and active frequency cutoff/peak marker dots.
  - **Dynamic Curve Graphs**: Velocity displays its live transfer curve; Slop displays its stepped random distribution.
- **Hardware-Style Interaction & Tactile Feel**:
  - Small rotary knobs with right-aligned text readouts across all continuous parameters.
  - Inline diagrams rendered directly inside text boxes (waveform morphing crossfades, envelope decay curves, velocity response shapes, and filter slope steps).
  - Hovering pop-up numerical entry upon right-clicking any continuous control.
  - Mouse-wheel support across all knobs and diagram readouts.
  - Double-click default return (including Mixer secondary sources snapping immediately to 100%).
  - Exponential tactile scaling on Slop and Velocity controls: the first 50% of knob travel controls the first 10% of modulation depth (and $\pm 25\%$ knob travel covers $\pm 5\%$).
  - Independent stepped random Slop offsets computed per trigger across 24 separate parameters.

---

## 2. Signal Routing Pipeline

```
[Modulator 1] ──(FM Depth)──> [Carrier 1] ──┐
                                             │
[Modulator 2] ──(FM Depth)──> [Carrier 2] ──┼──> [Mixer] ──> [Drive] ──> [Filter] ──> [Wave Folder]
                                             │       │
[Carrier 1 x Carrier 2] ──────(RingMod)─────┼───────┤
                                             │
[Noise Transient] ───────────────────────────┘

      ──> [RingMod FX] ──> [Frequency Shifter] ──> [Grit FX] ──> [Comb Filter]
      ──> [Disperser] ──> [Bell EQ] ──> [Amp] ──> Stereo Audio Output
```

### Modulation Signals
- **Pitch Envelope 1**: Modulates Carrier 1, Modulator 1, or Both.
- **Pitch Envelope 2**: Modulates Carrier 2, Modulator 2, or Both.
- **Filter Envelope**: Modulates Filter Cutoff frequency + Filter Post-Drive.
- **Amp Envelope**: Modulates Master Output Level (with multi-burst clap generator).
- **Velocity**: Modulates volume attenuation, all envelope decays, and all envelope depths via selectable slope curves.
- **Slop**: Injects independent stepped random offsets per trigger into 24 distinct engine parameters.

---

## 3. Shared & Standardized Parameter Definitions

To keep module controls clean and consistent across the synth, the following standard parameter paradigms are shared across multiple blocks:

### 3.1. Waveform Morphing
*Used in: `Carrier 1`, `Modulator 1` (Oscillator mode), `Carrier 2`, `Modulator 2` (Oscillator mode), `RingMod FX`*
- Continuously crossfades across five analog-style core shapes:
  - **0%**: Sine (Default)
  - **20%**: Triangle
  - **40%**: Sawtooth
  - **60%**: Square
  - **100%**: Pulse-Width Modulation (PWM 0% / narrow impulse)
- Renders an interactive live waveform diagram inside its value readout box.

### 3.2. DJ-Style Bipolar Filters
*Used in: `Modulator 1 & 2` (Cyclic mode), `Drive` (Post-Filter), `Wave Folder` (Post-Filter), `Noise Transient` (Filter), `Bell EQ` (DJ Filter)*
- A single center-detented knob providing low-pass and high-pass filtering without dead zones:
  - **0% to 49%**: Low-Pass Filter ($20\,\text{Hz} - 24\,\text{kHz}$, steep to gentle slope).
  - **50%**: Flat / Completely Bypassed (Default).
  - **51% to 100%**: High-Pass Filter ($20\,\text{Hz} - 24\,\text{kHz}$, gentle to steep slope).

### 3.3. Musical Envelope Decay Times
*Used in: `Pitch Env 1`, `Pitch Env 2`, `Filter Env`, `Amp Env`*
- Five-point piecewise logarithmic / exponential decay curve tailored for percussive punch and long sub sustain:
  - **0%**: $5\,\text{ms}$
  - **25%**: $100\,\text{ms}$
  - **50%**: $1.0\,\text{second}$
  - **75%**: $5.0\,\text{seconds}$
  - **100%**: $60.0\,\text{seconds}$
  - **Default**: $333\,\text{ms}$ ($0.3806$ normalized knob position).
*(Note: Noise Transient uses a tighter percussive range: $1\,\text{ms}$ at 0%, $50\,\text{ms}$ at 25%, $1\,\text{s}$ at 50%, $60\,\text{s}$ at 100%, default $100\,\text{ms}$).*

### 3.4. Envelope & Velocity Slope Curves
*Used in: `Pitch Env 1`, `Pitch Env 2`, `Filter Env`, `Amp Env`, `Velocity`*
- Continuously blends between response contours:
  - **0.0 (0%)**: Exponential (Default for snappy drum envelopes).
  - **0.5 (50%)**: Linear.
  - **1.0 (100%)**: Logarithmic.
- Renders a live curvature diagram inside the readout box.

### 3.5. Frequency Ranges
- **Full Audio Spectrum**: $20\,\text{Hz} - 24\,\text{kHz}$ (logarithmic scale, used in Carriers, Grit, Bell EQ, DJ Filters).
- **Wide Modular / LFO Spectrum**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (logarithmic scale, used in Modulators, Filter Cutoff, Comb Filter, Disperser, RingMod).

### 3.6. Bipolar Modulations & Gains
- **EQ & Shelf Gains**: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, $0\,\text{dB} = 0.5$ default).
- **Drive / Saturation**: $-6\,\text{dB}$ to $+24\,\text{dB}$ ($0\,\text{dB} = 0.2$ default).
- **Stereo Width**: $-100\%$ (inverted phase/swap) to $0\%$ (center mono) to $+100\%$ (extra wide).

### 3.7. Tactile Exponential Curve for Slop & Velocity
- To prevent fiddly calibration, modulation depth knobs are warped with an exponential response:
  - **Unipolar**: At 50% controller travel, the modulation value is at 10%.
  - **Bipolar**: At $\pm 25\%$ controller travel, the modulation value is at $\pm 5\%$.

---

## 4. Hardware Rack Module Specifications (3 × 8 Layout)

### Row 1: Voice Generation & Primary Mixing

#### Module 1: Carrier 1
- **Tracking (Selector)**: `Fixed Frequency` (0), `Fixed Pitch` (1), `MIDI Pitch` (2, default).
- **Pitch / Freq (Continuous)**:
  - Fixed Frequency: $20\,\text{Hz} - 24\,\text{kHz}$ (default $55\,\text{Hz}$).
  - Fixed Pitch: MIDI note 0 to 127 (default `A1 [33]`, shows note name and number).
  - MIDI Pitch: Note offset $-60$ to $+60$ semitones (default $0$).
- **Shape (Continuous)**: Waveform Morph (Sine $\to$ Tri $\to$ Saw $\to$ Square $\to$ PWM).
- **Depth (Continuous)**: FM Depth from Modulator 1: $-200\%$ to $+200\%$ (default $0\% = 0.5$).
- **Display**: Self-locked oscilloscope triggered to Carrier 1 fundamental.

#### Module 2: Modulator 1
- **Pitch Tracking (Selector)**: `Fixed` (0), `Following` (1), `FM Operator` (2).
- **Type (Selector)**: `Oscillator` (0), `Cyclic` (1, Sine $\times$ Noise), `Noise` (2, S&H).
- **Shape (Continuous)**: Dynamic context-dependent control:
  - Oscillator: Waveform Morph (Sine $\to$ PWM).
  - Cyclic: White noise DJ-style filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
  - Noise: Sample & Hold noise clock rate ($0.1\,\text{Hz} - 24\,\text{kHz}$, default $24\,\text{kHz}$).
- **Speed (Continuous)**: Context-dependent frequency:
  - Fixed: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $55\,\text{Hz}$).
  - Following: Pitch offset $-64$ to $+64$ semitones (default $0$).
  - FM Operator: Ratio 1:32.0 to 1.0:1.0 to 32.0:1 (default 1.0:1.0).
  - Cyclic Sine: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Display**: Self-locked oscilloscope phase-locked to Modulator 1's own internal frequency and zero crossings.

#### Module 3: Pitch Envelope 1
- **Target (Selector)**: `Off` (0, default), `Carrier` (1), `Modulator` (2), `Both` (3).
- **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
- **Depth (Continuous)**: $-5$ to $+5$ octaves (default $0$).
- **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Display**: Real-time decaying envelope oscilloscope trace.

#### Module 4: Carrier 2
- **Tracking (Selector)**: `Fixed Frequency` (0), `Fixed Pitch` (1), `MIDI Pitch` (2, default).
- **Pitch / Freq (Continuous)**: Same range as Carrier 1.
- **Shape (Continuous)**: Waveform Morph (Sine $\to$ PWM).
- **Depth (Continuous)**: FM Depth from Modulator 2: $-200\%$ to $+200\%$ (default $0\% = 0.5$).
- **Display**: Self-locked oscilloscope triggered to Carrier 2 fundamental.

#### Module 5: Modulator 2
- **Pitch Tracking (Selector)**: `Fixed` (0), `Following` (1), `FM Operator` (2).
- **Type (Selector)**: `Oscillator` (0), `Cyclic` (1), `Noise` (2).
- **Shape (Continuous)**: Waveform Morph / DJ Filter / S&H Clock Rate.
- **Speed (Continuous)**: Frequency / Offset / FM Ratio.
- **Display**: Self-locked oscilloscope phase-locked to Modulator 2's own internal frequency and zero crossings.

#### Module 6: Pitch Envelope 2
- **Target (Selector)**: `Off` (0, default), `Carrier` (1), `Modulator` (2), `Both` (3).
- **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
- **Depth (Continuous)**: $-5$ to $+5$ octaves (default $0$).
- **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Display**: Real-time decaying envelope oscilloscope trace.

#### Module 7: Noise Transient
- **S&H Rate (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Filter (Continuous)**: DJ-style filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- **Drive (Continuous)**: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
- **Decay (Continuous)**: Percussive decay ($1\,\text{ms} - 60\,\text{s}$, default $100\,\text{ms}$).
- **Display**: Noise burst oscilloscope trace.

#### Module 8: Mixer
- **Carrier 1 Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $100\%$).
- **Carrier 2 Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).
- **RingMod Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).
- **Noise Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).
- **Display**: Mixed voice summing oscilloscope trace.

---

### Row 2: Tone Shaping & Color FX

#### Module 9: Drive
- **Drive (Continuous)**: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
- **Bias (Continuous)**: DC offset $-1.0$ to $+1.0$ (default $0.0$).
- **Post-Filter (Continuous)**: DJ-style filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- **Limiter (Selector)**: `Off` (0), `On` (1, default, safety hard clipper after saturation).
- **Display**: Saturated output oscilloscope trace.

#### Module 10: Filter
- **Type (Selector)**: `Off` (0, default), `LPF` (1), `BPF` (2), `HPF` (3), `BRF / Notch` (4).
- **Slope (Selector)**: `-6dB/oct` (0), `-12dB/oct` (1, default), `-18dB/oct` (2), `-24dB/oct` (3), `-36dB/oct` (4).
- **Cutoff (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Resonance (Continuous)**: $0\% - 100\%$ ($Q \approx 0.707 - 18.7$).
- **Display**: **Real-Time X-Y Magnitude Response Plot**:
  - Logarithmic frequency axis ($20\,\text{Hz} - 24\,\text{kHz}$) vs Gain (dB).
  - Accurate multi-pole curves for all 5 filter types and 5 slope orders.
  - Interactive Cutoff frequency marker line.

#### Module 11: Filter Envelope
- **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
- **Depth (Continuous)**: $-10$ to $+10$ octaves (default $0$ oct).
- **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Post-Drive (Continuous)**: Filter post-saturation gain: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
- **Display**: Real-time decaying filter envelope trace.

#### Module 12: Wave Folder
- **Type (Selector)**: `Off` (0, default), `On` (1).
- **Fold (Continuous)**: 0 to 8 wavefolds (default 0).
- **Bias (Continuous)**: DC offset $-1.0$ to $+1.0$ (default $0.0$).
- **Post-Filter (Continuous)**: DJ-style filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- **Display**: Wavefolded output oscilloscope trace.

#### Module 13: RingMod FX
- **Waveform (Continuous)**: Waveform Morph (Sine $\to$ PWM).
- **Rate (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $55\,\text{Hz}$).
- **Amount (Continuous)**: Dry/Wet $0\% - 100\%$ (default $0\%$).
- **Width (Continuous)**: Stereo phase width $-100\% - +100\%$ (default $0\%$).
- **Display**: Ring-modulated audio oscilloscope trace.

#### Module 14: Frequency Shifter
- **Shift (Continuous)**: Bipolar shift $-X\,\text{Hz}$ to $0\,\text{Hz}$ to $+X\,\text{Hz}$ (default $0\,\text{Hz}$).
- **Range (Continuous)**: Maximum shift range $0\,\text{Hz} - 5\,\text{kHz}$ (default $3\,\text{Hz}$).
- **Blend (Continuous)**: $-100\%$ (wet negative sideband) $\to 0\%$ (dry) $\to +100\%$ (wet positive sideband).
- **Width (Continuous)**: Stereo quadrature phase width $-100\% - +100\%$ (default $0\%$).
- **Display**: Frequency-shifted audio oscilloscope trace.

#### Module 15: Grit FX
- **Bit Rate (Continuous)**: $1.0\,\text{bit} - 16.0\,\text{bit}$ (default $16.0\,\text{bit}$).
- **Sample Rate (Continuous)**: $20\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Low (Continuous)**: Low shelf filter: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, default $0\,\text{dB}$).
- **High (Continuous)**: High shelf filter: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, default $0\,\text{dB}$).
- **Display**: Decimated lo-fi audio oscilloscope trace.

#### Module 16: Comb Filter
- **Type (Selector)**: `Off` (0, default), `On` (1).
- **Dampening (Continuous)**: Internal feedback damping $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Cutoff (Continuous)**: Comb fundamental frequency $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Resonance (Continuous)**: Feedback $-100\%$ to $+100\%$ (default $0\%$).
- **Display**: Comb-filtered resonant oscilloscope trace.

---

### Row 3: Spatial, Output & Master Modulation

#### Module 17: Disperser
- **Type (Selector)**: `Off` (0, default), `On` (1).
- **Amount (Continuous)**: Cascaded 2nd-order all-pass filter stages: 0 to 32 stages (default 4).
- **Cutoff (Continuous)**: APF center frequency $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $220\,\text{Hz}$).
- **Resonance (Continuous)**: APF Q factor $-100\%$ to $+100\%$ (default $0\%$).
- **Display**: Phase-smeared / zapped audio oscilloscope trace.

#### Module 18: Bell EQ
- **Frequency (Continuous)**: $20\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Width (Continuous)**: $0.1 - 10$ octaves (default $0.1$ octaves).
- **Gain (Continuous)**: $-24\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$, transparent passthrough).
- **DJ Filter (Continuous)**: DJ-style tilt filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- **Display**: **Real-Time X-Y Magnitude Response Plot**:
  - Logarithmic frequency axis ($20\,\text{Hz} - 24\,\text{kHz}$) vs Gain (dB).
  - RBJ peaking/dipping bell curve combined with DJ filter tilt.
  - Interactive Center Frequency / Gain handle marker dot.

#### Module 19: Amp
- **Level (Continuous)**: Master level $0\% - 100\%$ (default $100\%$).
- **Pan (Continuous)**: Stereo pan 100% Left $\leftrightarrow$ Center $\leftrightarrow$ 100% Right (default Center).
- **Drive (Continuous)**: Output saturation $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
- **Limiter (Selector)**: `Off` (0), `On` (1, default, post-drive safety limiter).
- **Display**: Master output stereo waveform oscilloscope trace.

#### Module 20: Amp Envelope
- **Claps (Continuous)**: Multi-burst clap triggers: 0 to 32 claps (default 0).
- **Clap Speed (Continuous)**: Decay time per clap: $1\,\text{ms} - 15\,\text{ms}$ (default $3\,\text{ms}$).
- **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
- **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Display**: Multi-burst clap & main decay amplitude envelope trace.

#### Module 21: Velocity
- **Slope (Continuous)**: Velocity response curve: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
- **Decay (Continuous)**: Velocity-to-decay scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
- **Depth (Continuous)**: Velocity-to-envelope depth scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
- **Volume (Continuous)**: Velocity-to-output volume attenuation: $0\%$ (full volume) to $-100\%$ (min velocity is silent) (default $0\%$).
- **Display**: Dynamic velocity input-to-output transfer curve visualization.

#### Module 22: Slop
- Injects independent, stepped random values per trigger hit across 24 engine parameters.
- **Frequency (Continuous)**: $0\%$ to $\pm 100\%$ (controls pitch and filter frequencies).
- **Envelope Depths (Continuous)**: $0\%$ to $\pm 100\%$ (controls all envelope depths).
- **Envelope Decays (Continuous)**: $0\%$ to $\pm 100\%$ (controls all envelope decay times).
- **Pan (Continuous)**: $0\%$ to $\pm 100\%$ (controls output stereo panning).
- **Display**: Stepped random distribution visualization.

#### Slots 23 & 24: Blank Rack Plates
- Brushed anodized dark faceplates with hardware rack corner screws, preserving modular rack aesthetics.
