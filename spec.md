# The Klang Farmer — Modular Drum Synthesizer Specification

## 1. Architectural & Implementation Highlights

"The Klang Farmer" is a 22-module dual-FM synthesizer drum voice engineered in modern C++20 and JUCE 8. It replicates the tactile immediacy of a boutique modular hardware drum rack within a compact, ergonomically structured 2-row by 4-column Eurorack-style chassis (1040 × 740 px).

### System & Engine Capabilities
- **2x4 Paged Rack Architecture**:
  - The rack presents 8 modular slots in a 2-row by 4-column layout (Slots 1–4 on the top row, Slots 5–8 on the bottom row).
  - **Slot 1 (Top Left)**: Fixed **Navigation Module** providing immediate single-click access across 7 dedicated functional pages:
    1. `Voice 1`
    2. `Voice 2`
    3. `Transients`
    4. `Pre-Amp FX`
    5. `Amplifier`
    6. `Post-Amp FX`
    7. `Modulations`
  - **Slot 5 (Bottom Left)**: Fixed **Visualization Module** dynamically presenting real-time oscilloscopes, logarithmic Bode plots, transfer curves, and auto-tracking with padlock lock toggle.
  - **Slots 2–4 & Slots 6–8**: Dynamically populated with active module cards, multi-instance FX slots, and brushed-aluminum blank plates according to the selected page.
- **Multi-Instance FX Slots (8 Independent FX Blocks)**:
  - Both the **Pre-Amp FX** rack (Slots 1–4) and **Post-Amp FX** rack (Slots 1–4) allow **any effect to be instantiated into any slot without restriction**.
  - A user can load up to 8 instances of the exact same effect (e.g., 8 cascaded Wavefolders) or any mix of the 13 available processors.
  - Each slot maintains its own discrete parameter set (32 APVTS parameters: `pre_fx_1_p1`..`pre_fx_4_p4` and `post_fx_1_p1`..`post_fx_4_p4`), parameter smoothing, DSP instance, and dedicated visualizer buffer.
- **Dual FM Voice Architecture with Per-Voice Filtering**:
  - Two fully featured carrier/modulator pairs (`Carrier 1` & `Modulator 1`, `Carrier 2` & `Modulator 2`).
  - Carrier modulation depths are bipolar controls (**-100% to +100%**, default 0% center) driven directly by their respective modulators.
  - Each voice features its own dedicated multimode filter and filter envelope (`Filter 1` + `Filter Env 1`, `Filter 2` + `Filter Env 2`) prior to entering the mixer.
  - Cross-voice ring modulation (`Carrier 1 × Carrier 2`) is routed into the mixer as an independent, blendable source.
- **Transient Generation**:
  - Dedicated `Noise Transient` generator routed through its own dedicated filter and envelope (`Filter 3` + `Filter Env 3`) before entering the mixer.
- **Dedicated Pre-Amp & Post-Amp Limiters**:
  - Independent brickwall lookahead/saturating limiters placed at the end of the Pre-Amp FX chain and the Post-Amp FX chain, featuring Enable, Input Gain, Threshold, and Release controls.
- **Renoise & DAW Offline Bounce Stability**:
  - Strictly **zero dynamic heap memory allocations** (`malloc`/`new`) on the real-time audio thread.
  - Multi-clap burst buffers, envelope signal arrays, visualizer ring buffers, and internal voice sub-buffers are pre-allocated with sample-accurate rendering.
  - Rock-solid stability guaranteed across arbitrary, varying buffer sizes (from 64 up to 2048+ samples) during offline audio rendering.
- **Advanced Real-Time Visualizations**:
  - **Self-Locked Oscilloscopes**: Modulators and Carriers dynamically phase-lock to their own internal fundamental frequencies and zero-crossings, preventing visual drift or phase jitter even during detuning, audio-rate FM, or variable pitch sweeps.
  - **X-Y Frequency vs. Gain Response Plots**: Dedicated Bode magnitude response plots for all **Filter** and **Bell EQ** blocks. Rendered on a logarithmic frequency axis ($20\,\text{Hz} - 24\,\text{kHz}$) with grid reference marks ($100\,\text{Hz}$, $1\,\text{kHz}$, $10\,\text{kHz}$), $0\,\text{dB}$ center line, translucent area fill, glowing response curve, and interactive cutoff/peak marker handles.
  - **Dynamic Curve Graphs**: Velocity displays its live transfer curve; Slop displays its stepped random distribution.
- **Tactile Feel & Large High-Contrast Typography**:
  - High-contrast, bold typography optimized for legibility across all components (Card titles: 15pt bold; parameter labels: 13pt bold; knob value textboxes: 13.5pt bold; buttons & selectors: 13–13.5pt bold; header title: 17pt bold).
  - Rotary knobs with right-aligned text readouts across all continuous parameters.
  - Inline diagrams rendered directly inside text boxes (waveform morphing crossfades, envelope decay curves, velocity response shapes, and filter slope steps).
  - Hovering pop-up numerical entry upon right-clicking any continuous control.
  - Mouse-wheel support across all knobs and diagram readouts.
  - Double-click default return (Mixer secondary sources snap immediately to 100%).
  - Exponential tactile scaling on Slop and Velocity controls: the first 50% of knob travel controls the first 10% of modulation depth (and $\pm 25\%$ knob travel covers $\pm 5\%$).
  - Independent stepped random Slop offsets computed per trigger across 24 separate parameters.

---

## 2. Signal Routing Pipeline

```
[Voice 1]
 Modulator 1 ──(FM Depth: -100%..+100%)──> Carrier 1 ──> Filter 1 (Modulated by Pitch Env 1 & Filter Env 1) ──┐
                                                                                                               │
[Voice 2]                                                                                                      │
 Modulator 2 ──(FM Depth: -100%..+100%)──> Carrier 2 ──> Filter 2 (Modulated by Pitch Env 2 & Filter Env 2) ──┼──> [Mixer]
                                                                                                               │       │
[Cross RingMod]                                                                                                │       │
 Carrier 1 x Carrier 2 ────────────────────────────────────────────────────────────────────────────────────────┼───────┤
                                                                                                               │
[Transients]                                                                                                   │
 Noise Transient ──────────────────────────────────────> Filter 3 (Modulated by Filter Env 3) ─────────────────┘
                                                                                                                       │
                                                                                                                       ▼
                                                                                                             [Pre-Amp FX Rack]
                                                                                                              (Slots 1 to 4: Any FX)
                                                                                                                       │
                                                                                                                       ▼
                                                                                                              [Pre-Amp Limiter]
                                                                                                                       │
                                                                                                                       ▼
                                                                                                              [Amplifier Stage]
                                                                                                              (Amp + Amp Envelope)
                                                                                                                       │
                                                                                                                       ▼
                                                                                                             [Post-Amp FX Rack]
                                                                                                              (Slots 1 to 4: Any FX)
                                                                                                                       │
                                                                                                                       ▼
                                                                                                              [Post-Amp Limiter]
                                                                                                                       │
                                                                                                                       ▼
                                                                                                              Stereo Audio Output
```

### Modulation Signals
- **Pitch Envelope 1**: Modulates Carrier 1, Modulator 1, or Both.
- **Pitch Envelope 2**: Modulates Carrier 2, Modulator 2, or Both.
- **Filter Envelope 1**: Modulates Filter 1 Cutoff frequency + Filter 1 Post-Drive.
- **Filter Envelope 2**: Modulates Filter 2 Cutoff frequency + Filter 2 Post-Drive.
- **Filter Envelope 3**: Modulates Filter 3 Cutoff frequency + Filter 3 Post-Drive.
- **Amp Envelope**: Modulates Master Output Level (with multi-burst clap generator).
- **Velocity**: Modulates volume attenuation, all envelope decays, and all envelope depths via selectable slope curves.
- **Slop**: Injects independent stepped random offsets per trigger into 24 distinct engine parameters.

---

## 3. Shared & Standardized Parameter Definitions

To maintain consistency across all modules and effect cards, standard parameter behaviors are shared throughout the synthesizer:

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
*Used in: `Modulator 1 & 2` (Cyclic & Noise modes), `Drive` (Post-Filter), `Wave Folder` (Post-Filter), `Noise Transient` (Filter), `Bell EQ` (DJ Filter)*
- A single center-detented knob providing low-pass and high-pass filtering without dead zones:
  - **-100% to -1% (0.0 to 0.49 normalized)**: Low-Pass Filter ($20\,\text{Hz} - 24\,\text{kHz}$, steep to gentle slope).
  - **0% (0.50 normalized)**: Flat / Completely Bypassed (Default across all DJ filters in the plugin; double-click reset always returns to 0%).
  - **+1% to +100% (0.51 to 1.0 normalized)**: High-Pass Filter ($20\,\text{Hz} - 24\,\text{kHz}$, gentle to steep slope).

### 3.3. Musical Envelope Decay Times
*Used in: `Pitch Env 1`, `Pitch Env 2`, `Filter Env 1`, `Filter Env 2`, `Filter Env 3`, `Amp Env`*
- Five-point piecewise logarithmic/exponential decay curve tailored for percussive punch and long sub sustain:
  - **0%**: $5\,\text{ms}$
  - **25%**: $100\,\text{ms}$
  - **50%**: $1.0\,\text{second}$
  - **75%**: $5.0\,\text{seconds}$
  - **100%**: $60.0\,\text{seconds}$
  - **Default**: $333\,\text{ms}$ ($0.3806$ normalized knob position).
*(Note: Noise Transient uses a tighter percussive decay: $1\,\text{ms}$ at 0%, $50\,\text{ms}$ at 25%, $1\,\text{s}$ at 50%, $60\,\text{s}$ at 100%, default $100\,\text{ms}$).*

### 3.4. Envelope & Velocity Slope Curves
*Used in: `Pitch Env 1`, `Pitch Env 2`, `Filter Env 1–3`, `Amp Env`, `Velocity`*
- Continuously blends between response contours:
  - **0.0 (0%)**: Exponential (Default for snappy drum envelopes).
  - **0.5 (50%)**: Linear.
  - **1.0 (100%)**: Logarithmic.
- Renders a live curvature diagram inside the readout box.

### 3.5. Frequency Ranges
- **Full Audio Spectrum**: $20\,\text{Hz} - 24\,\text{kHz}$ (logarithmic scale, used in Carriers, Grit, Bell EQ, DJ Filters).
- **Wide Modular / LFO Spectrum**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (logarithmic scale, used in Modulators, Filter Cutoff, Comb Filter, Disperser, RingMod).

### 3.6. Bipolar Modulations & Gains
- **Carrier Modulation Depth**: $-100\%$ to $0\%$ to $+100\%$ (bipolar, $0\% = 0.5$ default).
- **EQ & Shelf Gains**: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, $0\,\text{dB} = 0.5$ default).
- **Drive / Saturation**: $-6\,\text{dB}$ to $+24\,\text{dB}$ ($0\,\text{dB} = 0.2$ default).
- **Stereo Width**: $-100\%$ (inverted phase/swap) to $0\%$ (center mono) to $+100\%$ (extra wide).

### 3.7. Tactile Exponential Curve for Slop & Velocity
- To prevent fiddly calibration, modulation depth knobs are warped with an exponential response:
  - **Unipolar**: At 50% controller travel, the modulation value is at 10%.
  - **Bipolar**: At $\pm 25\%$ controller travel, the modulation value is at $\pm 5\%$.

### 3.8. Tactile Hardware Button Selectors & Toggles
- All discrete module state selectors and bypass toggles use consistent, large, tactile hardware-style single-row buttons with glowing status LED indicators:
  - **Bypass / Toggle Buttons**: Standardized 2-column single-row layout (`Off`, `On`) with large 26px tactile touch zones (used in `Drive`, `Wave Folder`, `Comb Filter`, `Disperser`, `Amplifier Limiter`, `Pre-Limiter`, and `Post-Limiter`).
  - **Carrier Pitch Tracking**: Single-row 3-button selector (`MIDI`, `Freq`, `Note`).
    - **MIDI**: Tracks incoming MIDI pitch with a semitone offset slider ($-24$ to $+24$ st, default $0\text{ st}$, center-split bipolar meter).
    - **Freq**: Fixed continuous frequency slider ($20\,\text{Hz} - 24\,\text{kHz}$, logarithmic, default $55\,\text{Hz}$).
    - **Note**: Fixed musical note across MIDI notes 0–127 (`C-1` to `G9`, default `A1 = 55 Hz = note 33`) with fixed-width padded readout: `note name [frequency, midi note number]` (e.g. `A1   [   55 Hz,  33]`).
  - **Modulator Tracking & Mode**: Two stacked single-row 3-button selectors: Pitch Tracking (`Fixed`, `Follow`, `FM`) and Type (`Osc`, `Cyclic`, `Noise`).
  - **Pitch Envelope Target**: Single-row 4-button selector (`Car`, `Mod`, `Both`, `Opp`). `Opp` drives carrier and modulator pitch in inverse directions.
  - **Filter Mode & Slope**: Two stacked single-row selectors: Type (`LPF`, `BPF`, `HPF`, `BRF` — no bypass/off mode) and Slope (`6`, `12`, `18`, `24`, `36` dB/oct).

---

## 4. Hardware Rack Module Specifications (2 × 4 Paged Rack Layout)

The UI is organized as 8 modular slots across 2 rows (Slots 1–4 on top, Slots 5–8 on bottom). Both left-hand slots (Slot 1 Navigation and Slot 5 Visualizer) remain permanently anchored across all pages.

```
┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│     SLOT 1      │     SLOT 2      │     SLOT 3      │     SLOT 4      │
│ Navigation Card │  Page Module 1  │  Page Module 2  │  Page Module 3  │
├─────────────────┼─────────────────┼─────────────────┼─────────────────┤
│     SLOT 5      │     SLOT 6      │     SLOT 7      │     SLOT 8      │
│  Visualizations │  Page Module 4  │  Page Module 5  │  Page Module 6  │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

### 4.1. Navigation Module (Slot 1 — Fixed Across All Pages)
- **Grid of 7 Page Buttons**:
  - `Voice 1`
  - `Voice 2`
  - `Transients`
  - `Pre-Amp FX`
  - `Amplifier`
  - `Post-Amp FX`
  - `Modulations`
- Highlighted active LED indicator denoting current page.

### 4.2. Visualization Module (Slot 5 — Fixed Across All Pages)
- **Automatic Context Tracking**: Buttons have been removed to maximize screen real estate for the high-resolution scope display. The visualizer automatically changes based on whatever module or parameter is being edited or clicked on (including clicking anywhere on the panel/background of a module card).
- **Active Block Readout**: Top-left corner displays `VISUALIZER: <BLOCK NAME>` (e.g., `VISUALIZER: CARRIER 1`, `VISUALIZER: FILTER 1`, `VISUALIZER: PRE 1: WAVEFOLDER`).
- **Interactive Header Controls (Top Right)**:
  - **OFF Button**: Situated directly to the left of the padlock icon.
    - *Running (Default)*: Dim subtle grey (`#687488`) with transparent background.
    - *OFF*: Glowing signal red (`#ff3b5c`) with crimson translucent background and border. Completely halts all real-time waveform, filter, and Bode analysis, rendering a calm, motionless flat baseline to eliminate CPU consumption and visual distraction.
  - **Grey Unlocked Icon**: Auto-tracking mode. As the user clicks or edits any module or turns any knob, the visualizer automatically follows the active block.
  - **Yellow Locked Icon**: Locked mode. When clicked, the lock turns bright yellow and locks the visualizer to the currently displayed block. The visualizer will not auto-change, even when navigating between pages or tweaking controls on other modules.
  - *Pro Workflow*: Lock the visualizer to a downstream effect (such as the Pre-Amp Wavefolder or Master Limiter), navigate back to Voice 1, and tweak Carrier/Modulator parameters while observing the folding waveform in real time.
- **Display Modes**:
  - **Oscilloscope Mode**: Real-time waveform rendering with self-locking on fundamental zero-crossings for modulators, carriers, and FX audio probes.
  - **X-Y Bode Magnitude Plot Mode**: Logarithmic frequency ($20\,\text{Hz} - 24\,\text{kHz}$) vs. gain ($\text{dB}$) magnitude plot for any Filter block or Bell EQ block, showing exact filter shapes, slopes, resonance peaks, and interactive cutoff/gain markers.
  - **Velocity Transfer Plot Mode**: Live graph of velocity response transfer curves.
  - **Slop Distribution Plot Mode**: Live visualization of stepped random parameter offsets.

### 4.2.1. Quickstart Guide Modal Dialog
- **Header Button**: A dedicated `GUIDE` button is situated on the header bar directly to the left of `INIT`.
- **Non-Scrollable One-Page Layout**: Clicking `GUIDE` opens an instantaneous, non-scrollable modal overlay covering the plugin window, presenting a structured 4-panel quick reference:
  1. *Architecture & Signal Flow*: Dual FM voices, transient noise, 4-channel mixer, serial pre/post amp FX racks, and dual limiters.
  2. *Navigation & Smart Auto-Visualizer*: Page tabs, automatic context-switching, Bode/oscilloscope modes, and padlock locking.
  3. *Multi-Instance Effects Catalog*: All 13 DSP effects with 4-knob standardized tactile controls and multi-instance chaining.
  4. *Sound Design Recipes & Pro Tips*: Quick kick, snare, velocity shaping, analog slop drift, and audition button.
- **Dismissal**: Closes via the top-right `✕` button, clicking outside the modal dialog card, or pressing the `Escape` key.

### 4.2.2. INIT 3-Option Confirmation Dialog
- **Header Button**: `INIT` button situated between `GUIDE` and `AUDITION HIT` in the top navigation bar.
- **Confirmation Modal**: Clicking `INIT` launches an interactive confirmation dialog with three distinct actions:
  1. **Default**: Reverts the synthesizer to the factory initialized state (loaded default sound and standard loaded FX rack: Drive, Wave Folder, RingMod, Freq Shift, Grit FX, Comb Filter, Phase Smear, Bell EQ).
  2. **Clean**: Reverts all synthesis and modulation parameters to factory defaults, but strips all 8 FX slots (Pre-FX 1–4 and Post-FX 1–4) to `None`, providing a clean slate with empty rack faceplates.
  3. **Cancel**: Closes the dialog leaving the active patch completely untouched.

---

### 4.3. Page 1: Voice 1
- **Slot 1**: Navigation Module.
- **Slot 2: Carrier 1**
  - **Tracking (Selector)**: `MIDI` (0, default), `Freq` (1), `Note` (2).
    - `MIDI`: Tracks incoming MIDI pitch with a semitone offset slider: $-24$ to $+24$ semitones (default $0\text{ st}$, center-split bipolar meter).
    - `Freq`: Fixed continuous frequency $20\,\text{Hz} - 24\,\text{kHz}$ (logarithmic, default $55\,\text{Hz}$).
    - `Note`: Fixed musical note across MIDI notes 0–127 (`C-1` to `G9`, default `A1 = 55 Hz = note 33`) with padded readout: `note name [frequency, midi note number]` (e.g. `A1   [   55 Hz,  33]`).
  - **Shape (Continuous)**: Waveform Morph (Sine $\to$ Tri $\to$ Saw $\to$ Square $\to$ PWM).
  - **Mod Depth (Continuous)**: Modulation depth from Modulator 1: $-100\%$ to $+100\%$ (default $0\% = 0.5$). When non-zero, the Carrier Pitch / Offset slider dynamically highlights the active FM modulation span (without a real-time needle indicator to avoid high-frequency visual flicker).
  - **Visualizer**: Self-locked oscilloscope phase-locked to Carrier 1 fundamental.
- **Slot 3: Modulator 1**
  - **Pitch Tracking (Selector)**: `Fixed` (0, default), `Follow` (1), `FM` (2).
  - **Type (Selector)**: `Osc` (0, default), `Cyclic` (1, Sine $\times$ Noise), `Noise` (2, S&H Noise).
  - **Knob 0 (Continuous)**:
    - *Osc mode*: Waveform Morph (Sine $\to$ Tri $\to$ Saw $\to$ Square $\to$ PWM).
    - *Cyclic & Noise modes*: DJ Filter (bipolar $-100\%$ to $+100\%$, default $0\%$, double-click reset to $0\%$).
  - **Knob 1 (Speed, Continuous)**:
    - *Osc & Cyclic modes*: Frequency ($0.1\,\text{Hz} - 24\,\text{kHz}$) / Offset ($-64$ to $+64$ st) / FM Ratio (1:32 to 32:1).
    - *Noise mode*: S&H Clock Rate (labeled "Speed", $0.1\,\text{Hz} - 24\,\text{kHz}$, default $24\,\text{kHz}$, double-click reset to $24\,\text{kHz}$), dynamically swept by Pitch Envelope 1.
  - **Visualizer**: Self-locked oscilloscope phase-locked to Modulator 1's own internal frequency and zero crossings.
- **Slot 4: Pitch Envelope 1**
  - **Target (Selector)**: `Car` (0, default), `Mod` (1), `Both` (2), `Opp` (3).
  - **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
  - **Depth (Continuous)**: $-5$ to $+5$ octaves (default $0$).
  - **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Slot 5**: Visualization Module.
- **Slot 6: Filter 1**
  - **Type (Selector)**: `LPF` (0, default), `BPF` (1), `HPF` (2), `BRF` (3). *(Off option removed; Filter is always active).*
  - **Slope (Selector)**: `6` (0, 6 dB/oct), `12` (1, default, 12 dB/oct), `18` (2, 18 dB/oct), `24` (3, 24 dB/oct), `36` (4, 36 dB/oct).
  - **Cutoff (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
  - **Resonance (Continuous)**: $0\% - 100\%$ ($Q \approx 0.707 - 18.7$).
  - **Visualizer**: X-Y Frequency vs. Gain Bode plot with cutoff marker.
- **Slot 7: Filter Envelope 1**
  - **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
  - **Depth (Continuous)**: $-10$ to $+10$ octaves (default $0$ oct).
  - **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
  - **Post-Drive (Continuous)**: Filter post-saturation gain: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
- **Slot 8: Mixer**
  - **Silver Faceplate Aesthetic**: Styled as an authentic brushed-aluminum Eurorack utility module with countersunk corner rack screws, solid black screenprinted DIN typography, recessed white/light-satin slider troughs, and signal red level fills with dynamic text inversion (white over red fill, black over white trough).
  - **Carrier 1 Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $100\%$).
  - **Carrier 2 Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).
  - **RingMod Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).
  - **Noise Level (Continuous)**: $0\%$ to $100\%$ ($0.5$) to $400\%$ (default $0\%$, double-click snaps to $100\%$).

---

### 4.4. Page 2: Voice 2
- **Slot 1**: Navigation Module.
- **Slot 2: Carrier 2**
  - Identical parameters and behavior to Carrier 1.
- **Slot 3: Modulator 2**
  - Identical parameters and behavior to Modulator 1.
- **Slot 4: Pitch Envelope 2**
  - Identical parameters and behavior to Pitch Envelope 1.
- **Slot 5**: Visualization Module.
- **Slot 6: Filter 2**
  - Identical parameters and behavior to Filter 1 (dedicated to Voice 2).
- **Slot 7: Filter Envelope 2**
  - Identical parameters and behavior to Filter Envelope 1 (dedicated to Filter 2).
- **Slot 8: Mixer** (Carrier 1, Carrier 2, RingMod, Noise levels).

---

### 4.5. Page 3: Transients
- **Slot 1**: Navigation Module.
- **Slot 2: Noise Transient**
  - **S&H Rate (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
  - **Filter (Continuous)**: DJ-style filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
  - **Drive (Continuous)**: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
  - **Decay (Continuous)**: Percussive decay ($1\,\text{ms} - 60\,\text{s}$, default $100\,\text{ms}$).
- **Slot 3 & 4**: Blank Rack Plates.
- **Slot 5**: Visualization Module.
- **Slot 6: Filter 3**
  - Identical parameters and behavior to Filter 1 (dedicated to Noise Transient).
- **Slot 7: Filter Envelope 3**
  - Identical parameters and behavior to Filter Envelope 1 (dedicated to Filter 3).
- **Slot 8: Mixer** (Carrier 1, Carrier 2, RingMod, Noise levels).

---

### 4.6. Page 4: Pre-Amp FX
- **Slot 1**: Navigation Module.
- **Slot 2: FX Picker**
  - **4 Dropdown Selectors**: Independently assign an effect into Pre-Amp FX Slots 1, 2, 3, and 4.
  - Multi-instance allowed: any effect can be chosen in any number of slots.
- **Slot 3: Pre-Amp FX Slot 1**
- **Slot 4: Pre-Amp FX Slot 2**
- **Slot 5**: Visualization Module.
- **Slot 6: Pre-Amp FX Slot 3**
- **Slot 7: Pre-Amp FX Slot 4**
- **Slot 8: Pre-Amp Limiter**
  - Styled with cohesive signal red accent (`#e53935`) on regular dark chassis background.
  - **Enable (Selector)**: `Off` (0), `On` (1, default).
  - **Input Gain (Continuous)**: $-12\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
  - **Threshold (Continuous)**: $-24\,\text{dB}$ to $0\,\text{dB}$ (default $0\,\text{dB}$).
  - **Release (Continuous)**: $1\,\text{ms}$ to $500\,\text{ms}$ (default $50\,\text{ms}$).

---

### 4.7. Page 5: Amplifier
- **Slot 1**: Navigation Module.
- **Slot 2: Amp**
  - **Level (Continuous)**: Master level $0\% - 100\%$ (default $100\%$).
  - **Pan (Continuous)**: Stereo pan 100% Left $\leftrightarrow$ Center $\leftrightarrow$ 100% Right (default Center).
  - **Drive (Continuous)**: Output saturation $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
  - **Limiter (Selector)**: `Off` (0), `On` (1, default, post-drive safety limiter).
- **Slot 3: Amp Envelope**
  - **Claps (Continuous)**: Multi-burst clap triggers: 0 to 32 claps (default 0).
  - **Clap Speed (Continuous)**: Decay time per clap: $1\,\text{ms} - 15\,\text{ms}$ (default $3\,\text{ms}$).
  - **Slope (Continuous)**: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
  - **Decay (Continuous)**: Standard musical decay ($5\,\text{ms} - 60\,\text{s}$, default $333\,\text{ms}$).
- **Slot 4**: Blank Rack Plate.
- **Slot 5**: Visualization Module.
- **Slot 6**: Blank Rack Plate.
- **Slot 7: Post-Amp Limiter** (Master Limiter controls).
- **Slot 8: Mixer** (Carrier 1, Carrier 2, RingMod, Noise levels).

---

### 4.8. Page 6: Post-Amp FX
- **Slot 1**: Navigation Module.
- **Slot 2: FX Picker**
  - **4 Dropdown Selectors**: Independently assign an effect into Post-Amp FX Slots 1, 2, 3, and 4.
  - Multi-instance allowed: any effect can be chosen in any number of slots.
- **Slot 3: Post-Amp FX Slot 1**
- **Slot 4: Post-Amp FX Slot 2**
- **Slot 5**: Visualization Module.
- **Slot 6: Post-Amp FX Slot 3**
- **Slot 7: Post-Amp FX Slot 4**
- **Slot 8: Post-Amp Limiter**
  - Styled with cohesive signal red accent (`#e53935`) on regular dark chassis background.
  - **Enable (Selector)**: `Off` (0), `On` (1, default).
  - **Input Gain (Continuous)**: $-12\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$).
  - **Threshold (Continuous)**: $-24\,\text{dB}$ to $0\,\text{dB}$ (default $0\,\text{dB}$).
  - **Release (Continuous)**: $1\,\text{ms}$ to $500\,\text{ms}$ (default $50\,\text{ms}$).

---

### 4.9. Page 7: Modulations
- **Slot 1**: Navigation Module.
- **Slot 2: Mod Envelope 1**
  - **Slope (Continuous)**: Percussive decay curve shape (Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic, default Exponential).
  - **Depth (Continuous)**: Bipolar modulation depth ($-100\%$ to $+100\%$, default $0\%$).
  - **Decay (Continuous)**: Envelope decay time ($1\,\text{ms} - 2000\,\text{ms}$, default $333\,\text{ms}$).
  - **Destination (Selector ComboBox)**: Selects one of 108 continuous synthesis and effect parameter targets, dynamically labeled with loaded FX names (e.g. `Post FX 1 [Ring Mod]: Param 1 [Waveform]`).
- **Slot 3: Mod Envelope 2**
  - Identical controls and routing to Mod Envelope 1.
- **Slot 4: Mod Envelope 3**
  - Identical controls and routing to Mod Envelope 1.
- **Slot 5**: Visualization Module (defaults to Mod Envelope 1 oscilloscope).
- **Slot 6: Velocity**
  - **Slope (Continuous)**: Velocity response curve: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
  - **Depth (Continuous)**: Velocity-to-envelope depth scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
  - **Decay (Continuous)**: Velocity-to-decay scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
  - **Volume (Continuous)**: Velocity-to-output volume attenuation: $0\%$ (full volume) to $-100\%$ (min velocity is silent) (default $0\%$).
- **Slot 7: Key Tracking**
  - **Slope (Continuous)**: Key tracking curve shape: Exponential $\leftrightarrow$ Linear $\leftrightarrow$ Logarithmic (default Exponential).
  - **Depth (Continuous)**: Key tracking depth scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
  - **Decay (Continuous)**: Key tracking decay scaling: $-100\%$ to $+100\%$ (bipolar, default $0\%$).
  - **Volume (Continuous)**: Key tracking volume scaling: $0\%$ to $-100\%$ (default $0\%$).
- **Slot 8: Slop**
  - Injects independent, stepped random values per trigger hit across 24 engine parameters.
  - **Frequency (Continuous)**: $0\%$ to $\pm 100\%$ (controls pitch and filter frequencies).
  - **Envelope Depths (Continuous)**: $0\%$ to $\pm 100\%$ (controls all envelope depths).
  - **Envelope Decays (Continuous)**: $0\%$ to $\pm 100\%$ (controls all envelope decay times).
  - **Pan (Continuous)**: $0\%$ to $\pm 100\%$ (controls output stereo panning).

---

## 5. Effects Catalog (Selectable into any Pre-Amp or Post-Amp FX Slot)

Any of the following 13 processors (or `None / Bypass`) can be assigned to any of the 8 FX slots simultaneously. To support standard 4-knob modular hardware surfaces (e.g., TBD-16), **every single effect is standardized with exactly 4 controls**:

### 1. Bell EQ
- **Frequency (Continuous)**: $20\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Width (Continuous)**: $0.1 - 10$ octaves (default $0.1$ octaves).
- **Gain (Continuous)**: $-24\,\text{dB}$ to $+24\,\text{dB}$ (default $0\,\text{dB}$, transparent passthrough).
- **DJ Filter (Continuous)**: DJ-style tilt filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- *Visualizer*: Real-time X-Y Frequency vs. Gain Bode magnitude plot with peaking curve and DJ tilt.

### 2. Chorus
- **Rate (Continuous)**: Modulation LFO rate $0.1\,\text{Hz} - 10.0\,\text{Hz}$ (default $1.2\,\text{Hz}$).
- **Depth (Continuous)**: Excursion depth $0\% - 100\%$ ($0$ to $8\,\text{ms}$, default $60\%$).
- **Feedback (Continuous)**: Stereo delay feedback $-100\%$ to $+100\%$ (default $+20\%$).
- **Mix (Continuous)**: Dry/Wet balance $0\% - 100\%$ (default $50\%$).
- *Architecture*: Quadrature ($90^\circ$ phase-offset) dual sine LFO driving stereo fractional delay lines with soft-saturation feedback limiting.

### 3. Comb Filter
- **Dampening (Continuous)**: Internal feedback damping $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Cutoff (Continuous)**: Comb fundamental frequency $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Resonance (Continuous)**: Feedback $-100\%$ to $+100\%$ (default $0\%$).
- **Mix (Continuous)**: Bipolar Wet/Dry balance: $-100\%:0\%$ to $0\%:100\%$ (dry) to $+100\%:0\%$ (default $+50\%:50\%$, double-click snaps to $0\%:100\%$ dry).

### 4. Phase Smear
- **Order (Selector)**: `2nd` (0, default, 2nd-order APF cascade), `4th` (1, 4th-order APF cascade for extreme phase dispersion).
- **Amount (Continuous)**: Cascaded all-pass filter stages: 0 to 32 stages (default 4).
- **Cutoff (Continuous)**: APF center frequency $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $220\,\text{Hz}$).
- **Resonance (Continuous)**: APF Q factor $-100\%$ to $+100\%$ (default $0\%$).

### 5. Drive / Saturation
- **Drive (Continuous)**: $-6\,\text{dB}$ to $+24\,\text{dB}$ (default $+6\,\text{dB}$, double-click snaps to $0\,\text{dB}$).
- **Bias (Continuous)**: DC offset $-1.0$ to $+1.0$ (bipolar, default $0.0$).
- **Post-Filter (Continuous)**: DJ-style bipolar filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).
- **Limiter (Selector)**: `Off` (0), `On` (1, default, post-saturation hard clipper).

### 6. Filter (Standalone Effect)
- **Type (Selector)**: `LPF` (0, default), `BPF` (1), `HPF` (2), `BRF` (3). *(Off option removed; Filter is always active in one of 4 modes; use FX slot None to bypass).*
- **Slope (Selector)**: `6` (0, 6 dB/oct), `12` (1, default, 12 dB/oct), `18` (2, 18 dB/oct), `24` (3, 24 dB/oct), `36` (4, 36 dB/oct).
- **Cutoff (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Resonance (Continuous)**: $0\% - 100\%$ ($Q \approx 0.707 - 18.7$).
- *Visualizer*: Real-time X-Y Frequency vs. Gain Bode magnitude plot.

### 7. Flanger
- **Rate (Continuous)**: LFO rate $0.05\,\text{Hz} - 5.0\,\text{Hz}$ (default $0.25\,\text{Hz}$).
- **Depth (Continuous)**: Delay sweep excursion $0\% - 100\%$ ($0$ to $4\,\text{ms}$, default $70\%$).
- **Feedback (Continuous)**: Resonant comb feedback $-95\%$ to $+95\%$ (bipolar, default $+70\%$; negative values create hollow subtractive flanging, positive values create full resonant jet swoosh).
- **Mix (Continuous)**: Dry/Wet balance $0\% - 100\%$ (default $50\%$).
- *Architecture*: Sub-millisecond fractional delay line ($0.2\,\text{ms} - 5.0\,\text{ms}$) with saturating feedback loop.

### 8. Frequency Shifter
- **Shift (Continuous)**: Bipolar shift $-X\,\text{Hz}$ to $0\,\text{Hz}$ to $+X\,\text{Hz}$ (default $0\,\text{Hz}$).
- **Range (Continuous)**: Maximum shift range $0\,\text{Hz} - 5\,\text{kHz}$ (default $3\,\text{Hz}$).
- **Blend (Continuous)**: Bipolar Wet/Dry balance: $-100\%:0\%$ to $0\%:100\%$ (dry) to $+100\%:0\%$ (default $+50\%:50\%$, double-click snaps to $0\%:100\%$ dry).
- **Width (Continuous)**: Stereo quadrature phase width $-100\% - +100\%$ (default $0\%$).

### 9. Grit FX (BitCrusher)
- **Bit Rate (Continuous)**: $1.0\,\text{bit} - 16.0\,\text{bit}$ (default $16.0\,\text{bit}$).
- **Sample Rate (Continuous)**: $20\,\text{Hz} - 24\,\text{kHz}$ (default $24\,\text{kHz}$).
- **Low (Continuous)**: Low shelf filter: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, default $0\,\text{dB}$).
- **High (Continuous)**: High shelf filter: $-24\,\text{dB}$ to $+24\,\text{dB}$ (bipolar, default $0\,\text{dB}$).

### 10. Phaser
- **Rate (Continuous)**: LFO sweep speed $0.05\,\text{Hz} - 8.0\,\text{Hz}$ (default $0.5\,\text{Hz}$).
- **Depth (Continuous)**: Center frequency sweep excursion $0\% - 100\%$ (default $70\%$).
- **Feedback (Continuous)**: Notch resonance regeneration $-95\%$ to $+95\%$ (default $+50\%$).
- **Mix (Continuous)**: Dry/Wet blend $0\% - 100\%$ (default $50\%$ for maximum notch cancellation).
- *Architecture*: 6-stage cascaded all-pass filter ladder per channel with quadrature stereo LFO modulation and feedback soft clipping.

### 11. Ring Modulator
- **Waveform (Continuous)**: Waveform Morph (Sine $\to$ Tri $\to$ Saw $\to$ Square $\to$ PWM).
- **Rate (Continuous)**: $0.1\,\text{Hz} - 24\,\text{kHz}$ (default $55\,\text{Hz}$).
- **Amount (Continuous)**: Dry/Wet $0\% - 100\%$ (default $0\%$).
- **Width (Continuous)**: Stereo phase width $-100\% - +100\%$ (default $0\%$).

### 12. Tempo Delay
- **Division (Continuous / Stepped)**: Musical beat divisions synced to host tempo:
  - `1/32` (0.125 beats)
  - `1/16T` (0.167 beats)
  - `1/16` (0.25 beats)
  - `1/16D` (0.375 beats)
  - `1/8T` (0.333 beats)
  - `1/8` (0.5 beats, default)
  - `1/8D` (0.75 beats)
  - `1/4` (1.0 beats)
  - `1/4D` (1.5 beats)
  - `1/2` (2.0 beats)
- **Feedback (Continuous)**: Echo feedback regeneration $0\% - 100\%$ (default $40\%$).
- **Tone (Continuous)**: Low-pass damping filter cutoff $500\,\text{Hz} - 20\,\text{kHz}$ (default $8.0\,\text{kHz}$), providing analog-style warmth as repeats decay.
- **Mix (Continuous)**: Dry/Wet blend $0\% - 100\%$ (default $35\%$).
- *Architecture*: Pre-allocated stereo delay lines with automatic DAW BPM synchronization (120 BPM fallback in standalone), ping-pong stereo crossfeed, and feedback saturation.

### 13. Wave Folder
- **Type (Selector)**: `Off` (0, default), `On` (1).
- **Fold (Continuous)**: 0 to 8 wavefolds (default 0).
- **Bias (Continuous)**: DC offset $-1.0$ to $+1.0$ (bipolar, default $0.0$).
- **Post-Filter (Continuous)**: DJ-style bipolar filter ($20\,\text{Hz} - 24\,\text{kHz}$, default 50% flat).

### None / Bypass
- Fully bypasses processing for that slot and renders a brushed-aluminum blank rack plate.

---

## 5. Versioning & Release Management

- **Semantic Versioning Standard (`MAJOR.MINOR.PATCH`)**:
  - The plugins and standalone binaries adhere strictly to 3-part SemVer (e.g. `0.1.6`), with both **The Klang Farmer** and **The Klang Planter** sharing unified version numbering.
  - Major and minor numbers are bumped for architectural milestones and major features.
  - The patch number increments with each tagged release and distribution build.
- **Header Display**:
  - The active version is dynamically displayed in the top header bar next to plugin titles as an accent badge (e.g. `v0.1.6`), linked directly to JUCE's `JucePlugin_VersionString`.
- **Release Packaging**:
  - Distributed via GitHub Releases with multi-platform zip archives:
    - `TheKlangFarmer-v<version>-Windows.zip`: contains `The Klang Farmer.vst3`, `The Klang Planter.vst3`, and standalone executables.
    - `TheKlangFarmer-v<version>-macOS.zip`: Universal binaries for Apple Silicon & Intel (AU, VST3, Standalone).
    - `TheKlangFarmer-v<version>-Linux.zip`: x86_64 VST3 and Standalone binaries.

---

## 6. The Klang Planter — Compact FM Percussion Synthesizer

"The Klang Planter" is the streamlined, single-voice companion drum synthesizer built alongside The Klang Farmer. It distills the core FM and noise drum synthesis capabilities into an immediate, non-paged 2x4 rack layout.

### 2x4 Module Architecture
```
+---------------------------------------------------------------------------------------------------+
| THE KLANG PLANTER  v0.1.6    [Live Mini-Oscilloscope] [LIMIT] [Peak Meters]    [INIT]   [TRIGGER] |
+---------------------------+---------------------------+-----------------------+-------------------+
| [1] CARRIER               | [2] MODULATOR             | [3] PITCH ENV         | [4] NOISE TRANS   |
| Accent: Red               | Accent: Cyan              | Accent: Silver        | Accent: Dark Grey |
| Panel:  Cyan Tint         | Panel:  Red Tint          | Panel:  Dark Grey Tint| Panel:  Silver    |
| [MIDI] [Freq] [Note]      | [Fixed][Follow][FM]       |                       |                   |
|                           | [Osc] [Cyclic][Noise]     |                       |                   |
| 1. Offset                 | 1. Shape / DJ Filter      | 1. Slope              | 1. S&H Rate       |
| 2. Shape                  | 2. Speed / S&H Rate       | 2. Depth              | 2. DJ Filter      |
| 3. Mod Depth              |                           | 3. Decay              | 3. Decay          |
|                           |                           | Target: Car/Mod/../Opp| 4. FM/NOISE (Inv) |
+---------------------------+---------------------------+-----------------------+-------------------+
| [5] FILTER                | [6] FILTER ENV            | [7] AMPLIFIER         | [8] AMP ENVELOPE  |
| Accent: Blue              | Accent: Amber             | Accent: Green         | Accent: Magenta   |
| Panel:  Amber Tint        | Panel:  Blue Tint         | Panel:  Magenta Tint  | Panel:  Green Tint|
| [LPF][BPF][HPF][BRF]      |                           |                       |                   |
| [ 6 ][ 12][ 18][ 24][ 36] |                           |                       |                   |
| 1. Cutoff                 | 1. Slope                  | 1. Pre-Limiter Drive  | 1. Claps          |
| 2. Resonance              | 2. Depth                  | 2. Pan                | 2. Clap Speed     |
|                           | 3. Decay                  | 3. Vel Slope (LIN/EXP)| 3. Slope          |
|                           | 4. Pre-Filter Drive       | 4. Velocity (1%..100%)| 4. Decay          |
+---------------------------+---------------------------+-----------------------+-------------------+
```

### Signal Flow & Key Features
1. **FM Synthesis Pair**:
   - Carrier tracks incoming MIDI pitch ($\pm 24$ st), continuous frequency ($20\,\text{Hz} - 24\,\text{kHz}$), or fixed note.
   - Modulator operates in `Fixed`, `Follow`, or calibrated `FM Operator` ratio mode (`1:32.0` to `1:1` to `32.0:1`).
   - Waveform morphing from Sine $\to$ Triangle $\to$ Saw $\to$ Square with anti-aliased tanh shaping.
2. **Noise Transient Generator**:
   - Dedicated S&H clock rate ($0.1\,\text{Hz} - 24\,\text{kHz}$), bipolar DJ filter, and independent 5-point warp decay envelope ($1\,\text{ms} - 60\,\text{s}$).
3. **Pre-Filter Crossfader (Knob 4 on Noise Transient)**:
   - Sets the relative mix between the FM synthesis pair and the Noise Transient before entering the filter.
   - Bipolar curve: $-100\%$ (Noise only) $\leftrightarrow$ $0\%$ (Both at full volume) $\leftrightarrow$ $+100\%$ (FM pair only).
   - Inverted visual styling: dark recessed trough with bright silver fill and crisp light text, matching the Pitch Envelope sliders.
4. **Filter Stage with Pre-Filter Drive**:
   - The mixed audio passes through a pre-filter saturation stage (`tanh` drive from $-6\,\text{dB}$ to $+24\,\text{dB}$, controlled on Filter Env Knob 4).
   - Multimode cascaded SVF: LPF, BPF, HPF, and wide BRF (0.25 base Q) with 6, 12, 18, 24, 36 dB/oct slopes.
5. **Amplifier & Velocity Dynamics**:
   - Knob 1: **Pre-Limiter Drive** ($-\infty\,\text{dB}$ to $+24\,\text{dB}$, with $0\,\text{dB}$ at center 50%).
   - Knob 2: **Stereo Pan** ($100\%\,\text{L} \dots \text{Center} \dots 100\%\,\text{R}$).
   - Knob 3: **Vel Slope**: Controls the curvature of velocity sensitivity (default Linear, double-click Exponential).
   - Knob 4: **Velocity Floor**: Sets the volume floor at velocity 1 from $1\%$ to $100\%$ (default 50%).
   - Permanent transparent master brickwall limiter with live `LIMIT` reduction warning indicator on the header oscilloscope.
