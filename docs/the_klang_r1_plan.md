# Architectural Plan: The Klang R1 (TKR-1)
**Product Category**: Sibling Spin-Off Drum Synthesizer  
**Inspiration**: Korg Electribe R (ER-1) Rhythm Synthesizer  
**Status**: Planned (Post-1.0 Sibling Target)

---

## 1. Concept & Vision
*The Klang R1* is a stripped-down, lightning-fast 7-voice drum synthesizer engineered to deliver the raw, immediate, tactile groovebox workflow of the legendary **Korg Electribe ER-1** directly inside modern DAWs. By stripping away chromatic MIDI note tracking and focusing on dedicated percussive voices mapped across the white keys in any octave, it turns any MIDI keyboard into a mistake-proof, zero-latency finger drumming console with dedicated multi-out routing.

---

## 2. Voice Architecture & Sound Engine
The synthesizer features 7 concurrent monophonic rhythm voices divided into two specialized sonic tiers:

### Tier 1: Voices 1–4 (Pure Synthesizer Parts)
- **Role**: Kicks, Sub-Basses, Snares, Tuned Toms, Laser Zaps, and Resonant Sci-Fi Percussion.
- **Controls per Voice**:
  - `Pitch`: Base pitch knob ($20\,\text{Hz} - 2\,\text{kHz}$) with exponential tuning curve.
  - `Decay`: Exponential amplitude decay envelope ($5\,\text{ms} - 3.5\,\text{s}$).
  - `Mod Type`: Selectable modulation waveform:
    1. `Saw Drop`: Fast exponential pitch drop (classic 90s FM techno kick & zap transient).
    2. `Sine LFO`: Smooth vibrato / FM wobble.
    3. `Tri LFO`: Linear frequency sweep.
    4. `Noise`: Sample & hold noise modulation for industrial snares and crunchy percs.
  - `Mod Speed`: Modulation frequency ($0.1\,\text{Hz} - 2.5\,\text{kHz}$).
  - `Mod Depth`: Bipolar modulation depth ($-100\%$ to $+100\%$).
  - `Pan`: Stereo placement ($L100$ to $R100$).
  - `Level`: Output gain ($-\infty$ to $+6\,\text{dB}$).
  - `Delay Send`: Toggle button routing voice into the Master Tempo Delay.

### Tier 2: Voices 5–7 (Percussion & Metallic Parts)
- **Voice 5 (Closed Hi-Hat)**: Filtered metallic noise + bandpass ring generator with tight decay ($10\,\text{ms} - 400\,\text{ms}$).
- **Voice 6 (Open Hi-Hat)**: Sustained metallic sizzle with longer decay envelope ($50\,\text{ms} - 2.5\,\text{s}$).
  - **Automatic Choke Group**: Triggering Voice 5 immediately cuts Voice 6 with a click-free $1\,\text{ms}$ release ramp, perfectly replicating acoustic hi-hat foot pedals and classic drum machines.
- **Voice 7 (Cymbal / Crash / Metal Perc)**: Rich, dense multi-oscillator metallic cluster with tone filter and extended decay.

### Master Section & ER-1 Heritage Effects
- **Low Boost**: Classic Electribe resonant sub-bass boost ($40\,\text{Hz} - 80\,\text{Hz}$ peaking saturation) designed to give kicks seismic weight.
- **Ring Mod**: Cross-multiplication toggle (`Voice 1 × Voice 2`) producing aggressive clangorous metallic timbres and metallic snares.
- **Master Tempo Delay**:
  - Synced to host tempo ($1/32$ through $1/2$).
  - Feedback/Depth control with soft-clipping tape feedback.
  - Independent per-voice `Delay Send` buttons.
- **Master Limiter**: Transparent brickwall safety ceiling.

---

## 3. Performance & Keyboard Mapping
- **Octave-Invariant White Key Triggering**:
  - Incoming MIDI note pitch tracking is completely decoupled from voice frequency.
  - The voice triggered is determined by musical pitch class:
    - `C` (Note % 12 == 0) &rarr; **Voice 1 (Synth 1 / Kick)**
    - `D` (Note % 12 == 2) &rarr; **Voice 2 (Synth 2 / Snare/Zap)**
    - `E` (Note % 12 == 4) &rarr; **Voice 3 (Synth 3 / Low Tom)**
    - `F` (Note % 12 == 5) &rarr; **Voice 4 (Synth 4 / High Tom)**
    - `G` (Note % 12 == 7) &rarr; **Voice 5 (Perc 1 / Closed Hat)**
    - `A` (Note % 12 == 9) &rarr; **Voice 6 (Perc 2 / Open Hat)**
    - `B` (Note % 12 == 11) &rarr; **Voice 7 (Perc 3 / Cymbal)**
- **Safe Mode Black Keys**:
  - Black keys (`C#`, `D#`, `F#`, `G#`, `A#`) are completely inactive.
  - Live finger drumming can be played anywhere on a keyboard with zero risk of triggering false notes.

---

## 4. DAW Multi-Out Bus Architecture
- **Bus Configuration (`juce::AudioProcessor::BusesProperties`)**:
  - `Bus 0`: Master Stereo Mix (Default L/R)
  - `Bus 1`: Voice 1 Stereo Out
  - `Bus 2`: Voice 2 Stereo Out
  - `Bus 3`: Voice 3 Stereo Out
  - `Bus 4`: Voice 4 Stereo Out
  - `Bus 5`: Voice 5 Stereo Out
  - `Bus 6`: Voice 6 Stereo Out
  - `Bus 7`: Voice 7 Stereo Out
- **Intelligent Auto-Muting**:
  - When the DAW activates an aux bus, that voice's dry signal is automatically removed from the Master Mix.
  - Wet Delay and Low Boost returns can remain on the Master Bus or follow user routing preferences.

---

## 5. UI Layout: All-in-One Console (Zero Tabs)
- **Single-Screen Form Factor**:
  - 7 vertical mixer-style channel strips arranged horizontally side-by-side.
  - Each strip contains:
    - Voice label & activity LED (lights up on trigger, click to audition).
    - Large tactile `Pitch` and `Decay` knobs.
    - `Mod Type` selector button + `Speed` / `Depth` knobs.
    - `Pan` and `Level` faders/knobs.
    - Glowing red `[DELAY SEND]` button.
  - Far right section: Master Console housing **Low Boost**, **Ring Mod 1x2**, **Tempo Delay**, and Master Output Meter.

---

## 6. Codebase Architecture & Reusability
- Built as a sibling build target in `CMakeLists.txt`:
  ```cmake
  juce_add_plugin(TheKlangR1
      COMPANY_NAME "R'lyeh Sound"
      IS_SYNTH TRUE
      NEEDS_MIDI_INPUT TRUE
      FORMATS ${TKF_FORMATS}
      PRODUCT_NAME "The Klang R1"
  )
  ```
- Inherits from `KlangCoreProcessor` and `KlangCoreEditor`.
- Reuses:
  - `TbdAudio::FastMath` for all DSP calculations.
  - `assets/controls/r1_voices.json` for APVTS definitions.
  - Universal theme system and vector LookAndFeel.
  - Automated `gui_tests` and `dsp_tests` reflection suites.
