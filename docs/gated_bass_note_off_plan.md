# Gated Trigger Mode & Settable Note-Off Release (Staccato Bass Engine)

**Target Milestone**: v0.4.0 "The Sound & Chaos Update"  
**Scope**: Both Plugins (*The Klang Farmer* and *The Klang Planter*)  
**Architecture**: Data-Driven JSON Parameters + Audio Thread Invariants + Header UI Callout  

---

## Executive Summary

While *The Klang Farmer* and *The Klang Planter* were originally conceptualized as one-shot dual-FM drum synthesizers, their versatile carrier/modulator engines, pitch envelopes, and multi-mode filters make them monstrous platforms for synthesizing modern electronic basslines (Reese bass, aggressive acid bass, 808 subs, and resonant FM talk-bass).

However, in one-shot drum mode, releasing a MIDI key is completely ignored: notes ring out for their full decay time. When programming fast 16th-note basslines or staccato grooves, notes bleed together into a sustained drone unless the main decay knob is kept unnaturally short.

This feature introduces an integrated **Gated Trigger Mode** with a **Settable Note-Off Release** (1 ms to 30 ms). When activated:
- Holding a MIDI key sustains/decays the voice normally.
- Releasing the key triggers an immediate, analog-style, pop-free exponential release ramp (default 5 ms) to zero.
- Allows producers to seamlessly toggle between one-shot percussive hits and tight, punchy, articulate basslines.

```
                     ┌───────────────────────────────────┐
                     │          MIDI Input Stream        │
                     └─────────────────┬─────────────────┘
                                       │
                     ┌─────────────────┴─────────────────┐
                     ▼                                   ▼
             [ Note-On Event ]                   [ Note-Off Event ]
                     │                                   │
                     ▼                                   ▼
        engine.trigger(velocity)                Is Mode == GATED?
        activeMidiNote = note;                 and note == activeNote?
        inReleaseRamp = false;                           │
                     │                     ┌─────────────┴─────────────┐
                     ▼                     ▼                           ▼
        Normal Envelope Attack/Decay      NO                          YES
                                      (One-Shot)            inReleaseRamp = true;
                                      Ignore event          Capture current level;
                                                            Exp ramp to 0 in 1–30 ms
```

---

## 1. User Interface & Experience

### 1.1 Header Front-Panel Badge (`ONE-SHOT` / `GATED`)
- Located on the top-right header control strip, beside the `INIT` button and visualizer toggles.
- **Visual Styling**:
  - `ONE-SHOT` (Default): Dim/amber outline badge. Communicates standard drum-machine one-shot behavior.
  - `GATED`: Glowing electric teal/green pill badge. Communicates keyboard-responsive bass/synth behavior.
- **Interactions**:
  - **Left-Click**: Toggles between `One-Shot` and `Gated` modes.
  - **Right-Click**: Launches `GatedReleaseCalloutComponent` modal callout.

### 1.2 Right-Click Callout Popup (`GatedReleaseCalloutComponent`)
- Styled identically to existing `SliderCalloutComponent` and `SelectorCalloutComponent` with rounded corners, dark chassis background, and LED accents.
- **Controls Inside Callout**:
  1. **Mode Selector**: Segmented two-state button (`[ One-Shot ]` `[ Gated ]`).
  2. **Release Time Slider**:
     - Range: `1.0 ms` to `30.0 ms` (default `5.0 ms`).
     - Display: Formatted cleanly as `"X.X ms"`.
     - Skew: Logarithmic warp curve (`0.5`) providing high precision in the sweet spot between `1 ms` and `10 ms`.
     - Quick-Snap points: `1 ms`, `2 ms`, `5 ms`, `10 ms`, `15 ms`, `20 ms`, `30 ms`.

---

## 2. Parameter Architecture (Data-Driven JSON)

All parameters are declared strictly in `assets/controls/global_settings.json` (and `planter_global_settings.json`):

```json
{
  "group": "voice_control",
  "parameters": [
    {
      "id": "trigger_mode",
      "name": "Trigger Mode",
      "type": "choice",
      "choices": ["One-Shot", "Gated"],
      "default": 0,
      "tooltip": "TRIGGER MODE: One-Shot ignores MIDI note-offs (ideal for drum hits). Gated cuts voice on key release (ideal for staccato bass)."
    },
    {
      "id": "note_off_release",
      "name": "Note-Off Release",
      "type": "float",
      "min": 0.001,
      "max": 0.030,
      "default": 0.005,
      "skew": 0.5,
      "unit": "s",
      "tooltip": "NOTE-OFF RELEASE: Fast exponential fade-out time (1ms to 30ms) applied when releasing a key in Gated mode to eliminate DC clicks."
    }
  ]
}
```

---

## 3. Real-Time Audio DSP Implementation

### 3.1 Audio Thread Safety Invariants
- Zero heap allocations (`malloc`, `new`, dynamic vectors) in `noteOff()` or `processStereo()`.
- Zero thread locks or mutexes.
- Fast transcendental approximations using `TbdAudio::FastMath::fastExp`.

### 3.2 Monophonic Active Note Tracking
In monophonic synthesizers, overlapping note playing (legato) must not cause an older note's release to choke a newly played note:
```cpp
// In ModularEngine / PlanterEngine
void noteOn(int noteNumber, float velocity) {
    activeMidiNote.store(noteNumber, std::memory_order_relaxed);
    inReleaseFade = false;
    releaseProgress = 0.0f;
    trigger(velocity);
}

void noteOff(int noteNumber) {
    if (triggerModeParam->getIndex() == 1) { // Gated Mode
        if (noteNumber == activeMidiNote.load(std::memory_order_relaxed)) {
            inReleaseFade = true;
            releaseStartAmp = currentAmpEnv;
            releaseProgress = 0.0f;
        }
    }
}
```

### 3.3 Click-Free Exponential Release Ramp
During the release stage:
$$\text{gainMultiplier} = \text{FastMath::fastExp}\left(-4.0 \cdot \frac{t}{\text{releaseTime}}\right)$$
- At $t = 0$: $\text{gainMultiplier} = 1.0$ (seamless transition with zero DC discontinuity).
- At $t = \text{releaseTime}$: $\text{gainMultiplier} \approx 0.018$ ($-35\,\text{dB}$), dropping to zero at the end of the duration.
- Guarantees $100\%$ click-free audio cutoffs without dulling the transient attack of subsequent strikes.

---

## 4. Verification & Automated Test Plan

### 4.1 DSP Stability Tests (`test/dsp_tests.cpp`)
1. **One-Shot Invariant Test**: Verify that in `One-Shot` mode, emitting a `noteOff()` does not reduce the signal amplitude or alter the decay curve.
2. **Gated Staccato Cutoff Test**: Verify that in `Gated` mode, emitting a `noteOff()` reduces the output amplitude by $> 40\,\text{dB}$ within the specified release time window.
3. **DC Pop / Discontinuity Verification**: Assert that $\max |\Delta s| < 0.05$ across the note-off boundary (proving zero transient clicks).
4. **Legato Overlap Test**: Trigger Note 60, then Note 62, then emit Note-Off for 60; verify Note 62 continues playing without interruption.

### 4.2 GUI Reflection Tests (`test/gui_tests.cpp`)
1. **APVTS Reflection Sweep**: Sweep `trigger_mode` and `note_off_release` across 100% parameter bounds.
2. **Synthetic Event Simulation**: Click the header badge and verify `trigger_mode` toggles between 0 and 1.
3. **Right-Click Callout Verification**: Simulate right-click on the badge, verify `GatedReleaseCalloutComponent` opens, test slider dragging, and verify callout dismisses on outside click.
