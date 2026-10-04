# Backlog & Roadmap for The Klang Farmer & The Klang Planter

> [!IMPORTANT]
> **NEXT SESSION KICKOFF REMINDER**:  
> When opening the next session, review the prioritized items below:
> 1. **Priority #1 (Core DSP)**: `FastMath.h` & SIMD-Accelerated DSP Core ([THE-8](https://linear.app/the-klang-farmer/issue/THE-8/fastmathh-and-simd-accelerated-dsp-core-desktop-and-embedded)) — 2x–4x CPU speedup.
> 2. **Priority #2 (Compatibility & CI)**: Mac Compatibility Investigation, Docker Multi-Platform Build Testing, & GitHub Actions CI `pluginval` Validation ([THE-5](https://linear.app/the-klang-farmer/issue/THE-5/mac-and-linux-compatibility-dynamic-host-loading-ci-and-dockerization)).
> 3. **Priority #3 (UX Standardization)**: Amp Drive Default Standardization (0 dB reset).
> 4. **Priority #4 (DSP / FX Architecture)**: Universal Dual-Mode FX Mix Knob Architecture & Parameter Uniformity Audit.

---

## 📌 Top Priorities for Upcoming Sessions

### 1. [PRIORITY #1] `FastMath.h` & SIMD-Accelerated DSP Core (Desktop & Embedded)
Tracked in Linear: **[THE-8](https://linear.app/the-klang-farmer/issue/THE-8/fastmathh-and-simd-accelerated-dsp-core-desktop-and-embedded)**  
Implement the pure C++ `FastMath.h` acceleration core:
- **Core Math Approximations**:
  - `fast_pow2(float x)`: 2–3 cycle IEEE 754 exponent bit-manipulation with 2nd-order Chebyshev correction polynomial ($< 0.05\%$ max error). Replaces heavy `std::pow(2.0f, x)` across Carrier, Modulator, and Filter sweeps.
  - `fast_sin(float phase)`: 4th-order minimax parabolic polynomial or 1024-point LUT. Replaces `std::sin(phase * TWO_PI)`.
  - `fast_tanh(float x)`: Padé rational saturation approximation $\frac{x \cdot (27 + x^2)}{27 + 9x^2}$ for pre-filter drive, carrier shaping, and limiter stages. Replaces `std::tanh(x)`.
  - `fast_exp(float x)`: Fast percussive decay curve generator.
  - `uint32_t` integer phase accumulators: Branchless natural 32-bit phase wrapping, eliminating `float` floor and conditional branches in inner oscillator loops.
- **Desktop Benefits**:
  - Unlocks MSVC/Clang auto-vectorization (AVX2/SSE) across sample loops.
  - Provides a **2x to 4x reduction in raw DSP CPU cycles** on desktop, lowering CPU load for dense polyphonic DAW projects and low-latency buffer settings (32–64 samples).
  - **Hybrid Architecture**: Branchless fixed-point integers (`uint32_t`) for phase accumulators + single-cycle 32-bit hardware float for audio/FPU processing.
- **Embedded Portability**:
  - Zero dependencies on standard runtime transcendental routines, enabling the exact same DSP code to run seamlessly on low-power microcontrollers.
- **Verification**:
  - Build `test/benchmark_dsp.cpp` (`benchmark_dsp.exe`) to measure cycle counts, verify $< 0.05\%$ accuracy against `std::math`, and run all 33 unit tests in `dsp_tests.exe`.

---

### 2. [PRIORITY #2] Mac Compatibility Diagnostics, GitHub Actions CI Dynamic Host Loading Tests (`pluginval` & `auval`), & Docker Linux Container
Tracked in Linear: **[THE-5](https://linear.app/the-klang-farmer/issue/THE-5/mac-and-linux-compatibility-dynamic-host-loading-ci-and-dockerization)**  
- **Diagnose & Fix Mac Loading Failures**:
  - **Missing `CMAKE_OSX_DEPLOYMENT_TARGET`**: Declare `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` **before** `project(TheKlangFarmer ...)` in `CMakeLists.txt` so Apple Clang initializes the macOS 11.0 SDK target triple prior to compiler detection.
  - **Gatekeeper Quarantine**: Apply ad-hoc codesigning (`codesign --force --deep -s -`) to all macOS bundles. Ship both `README_MAC_INSTALL.txt` and a double-clickable `Unlock_and_Install_Mac.command` one-click script in the release `.zip` (`zip -r -y`) to strip `com.apple.quarantine` (`xattr -cr`), install to `~/Library/Audio/Plug-Ins/`, and refresh CoreAudio.
- **JUCE 9.0.3 Linux Build & Portability Hygiene**:
  - Statically link `libstdc++` and `libgcc` on Linux (`-static-libstdc++ -static-libgcc`) so compiled `.vst3` bundles load without `GLIBCXX` version mismatches across distros.
  - Set `VST3_AUTO_MANIFEST FALSE` on both plugin targets to avoid `juce_vst3_helper` hanging/crashing in headless CI.
  - Define `JUCE_VST3_CAN_REPLACE_VST2=0`, `JUCE_WEB_BROWSER=0`, and `JUCE_USE_CURL=0` globally.
  - Guard against JUCE 9.0.3 issue `#1696`: audit `stopTimer()` as first line in editor destructors.
- **Dynamic Plugin Host Loading Verification in CI (`.github/workflows/ci.yml`)**:
  - **Tracktion `pluginval` v1.0.4**: Run `--strictness-level 5 --validate-in-process` against `.vst3` across macOS, Linux (under `xvfb-run -a`), and Windows to test audio bus negotiation, thread safety, automation sweeps, and memory leaks.
  - **macOS Native `auval`**: Run `auval -v aumu Tkf1 Rlyh` and `auval -v aumu Tkp1 Rlyh` in CI to guarantee Logic Pro / GarageBand host acceptance.
  - **Headless GUI Instantiation**: Run `capture_screenshot` and `capture_planter_screenshot` under `Xvfb` on Linux/Docker.
  - **PR Test Artifacts**: Upload test binaries automatically on every PR.
- **Docker Multi-Platform Container (`docker/Dockerfile.linux` & `docker-compose.yml`)**:
  - Reproducible Ubuntu 22.04 container with `ninja-build`, `xvfb`, and `pluginval` v1.0.4 pre-installed.
  - Run via `scripts/test_docker_linux.bat` for single-command verification on Windows.

---

### 3. [PRIORITY #3] Amp Drive Default Standardization
- Change `amp_drive` default from +6 dB to **0 dB** (with double-click reset to 0 dB) in `createParameterLayout()` and UI initialization.

---

### 4. [PRIORITY #4] Universal Dual-Mode FX Mix Knob Architecture & Parameter Uniformity Audit
- **Universal Dual-Mode Mix Behavior**: Standardize the Mix knob across **ALL** FX processors to a single bipolar range (`-100%` ↔ `0%` ↔ `+100%`):
  - **Negative Range (`-100%` to `0%` — Wet Crossfade)**: Classic crossfade where dry fades out as wet increases. At `-100%`, signal is 100% wet (0% dry). At `0%`, signal is 100% dry (0% wet).
  - **Center (`0%` — Pure Dry)**: 100% Dry signal, 0% Wet. Default and double-click reset value.
  - **Positive Range (`0%` to `+100%` — Parallel Blend)**: Dry signal remains pinned at 100% while wet signal is added on top up to 100% (true parallel processing without losing dry transient punch).
- **Drive Effect**: Replace the Filter knob on the Drive effect with this standardized Mix control.
- **Uniformity Audit**: Cross-reference all 13 (and new) FX processors to ensure the `Mix` parameter is uniformly located in the exact same parameter slot (e.g. Knob 4) across every effect for muscle memory and hardware surface mapping.

---

## 🚀 Engine & Synthesis Features (Backlog)

### 5. APF (All-Pass Filter) Mode for Voice 1, Voice 2, and Noise Filters
- **New Filter Type**: Add `APF` to the filter type selectors alongside `LPF`, `BPF`, `HPF`, and `BRF`.
- **Order Mapping**: Slopes `6`, `12`, `18`, `24`, `36` map to $N^\text{th}$-order APF stages:
  - `6` → 1st Order APF
  - `12` → 2nd Order APF
  - `18` → 3rd Order APF
  - `24` → 4th Order APF
  - `36` → 6th Order APF
- **Phase Response Visualizer**: When a filter is set to `APF`, the visualizer renders a dedicated **Phase Plot** (phase shift angle vs frequency, $20\,\text{Hz} - 24\,\text{kHz}$) instead of the magnitude response.
- **Sound Design Target**: Exceptional for bass sound design, sub-frequency smearing, and phase dispersion before wavefolders and saturators.

---

### 6. Dual Sample Players for Noise Transient Page (Plugin Only)
- Add two dedicated sample player modules to the Transients page (desktop plugin specific; not constrained to TBD-16 4-control limits).
- **Controls per Player**:
  1. **File Picker**: File browser / drag-and-drop audio file loader.
  2. **Play Speed**: Bipolar playback speed with reverse: `-400%` ↔ `0%` ↔ `+400%` (defaults and double-clicks to `+100%`).
  3. **Decay Time**: Percussive sample amplitude decay envelope.
  4. **Level**: Output gain level.
- **Choke / Split Modal**: Modal dialog to configure split/choke groups (e.g. allowing one player to be an open hi-hat and the other a closed hi-hat that chokes the open sound).

---

## 🎛️ New Effects Processors (Backlog)

### 7. Transient Shaper Effect
- 4-knob envelope dynamic processor (Kilohearts style):
  1. **Attack**: Boost or attenuate initial transient impact.
  2. **Pump**: Sustained envelope swell and recovery.
  3. **Sustain**: Tail length and body amplification.
  4. **Speed**: Detection envelope attack/release tracking speed.

### 8. Custom Waveshaper Effect
- Uses the current crossfaded waveform morph (sine / tri / saw / square / PWM) as the non-linear transfer function / transform for waveshaping.
- **Controls**:
  1. **Waveshape**: Selects/morphs the shaping curve.
  2. **Drive**: Input pre-gain drive.
  3. **DJ Filter**: Bipolar tilt filter (0% flat center).
  4. **Mix**: Universal dual-mode mix.

### 9. Channel Mixer Effect
- 4-knob cross-channel matrix mixer (Kilohearts style):
  1. **L → L**: Incoming Left to Outgoing Left ($-100\%$ to $+100\%$).
  2. **R → L**: Incoming Right to Outgoing Left ($-100\%$ to $+100\%$).
  3. **L → R**: Incoming Right to Outgoing Right ($-100\%$ to $+100\%$).
  4. **R → R**: Incoming Right to Outgoing Right ($-100\%$ to $+100\%$).

### 10. Stereo Enhancer Effect
- Advanced stereo field shaper:
  1. **Mid**: Mid-channel gain ($0\%$ to $100\%$).
  2. **Width**: Side-channel expansion ($0\%$ to $100\%$ to $600\%$ super-wide).
  3. **Pan**: Stereo balance (Left ↔ Center ↔ Right).
  4. **Slop**: Random analog drift depth ($0\%$ to $100\%$) applied across Mid, Width, and Pan per trigger hit.

### 11. Haas Delay Effect
- Psychoacoustic spatial widener via the Haas effect:
  - **Haas Delay Knob**: Single bipolar delay slider:
    - `-100 ms`: Left channel delayed up to 100 ms (Right arrives first → sound localized Right).
    - `0 ms`: No delay on either channel (Center).
    - `+100 ms`: Right channel delayed up to 100 ms (Left arrives first → sound localized Left).

### 12. Lo-Fi Early Reflections Gated Reverb Effect
- Characterized by dense, vintage, "kinda crap sounding" retro early reflections specifically tailored for 80s-style gated snare and punchy drum sounds.
- **Controls**:
  1. **Time**: Reverb decay time / room size.
  2. **DJ Filter**: Bipolar tilt filter (0% flat center) for reflection high/low spectral shaping.
  3. **Mix**: Universal dual-mode mix (`-100%` pure wet crossfade ↔ `0%` dry ↔ `+100%` parallel additive).
  4. **Gate Time**: BPM-synced gate cutoff time in 64th note increments (1/64, 2/64, 3/64, etc.) to abruptly terminate the reverb tail in sync with the song tempo.

---

## 🖱️ UI / UX & Modal Enhancements (Backlog)

### 13. Right-Click Parameter Edit Modal: Quick-Snap Preset Buttons (Approved Spec)
- **Waveshape Edit Modal**:
  - Add quick preset buttons directly into the right-click edit modal: **Sine** (`0.0`), **Triangle** (`0.25`), **Saw** (`0.5`), and **Square** (`0.75` / `1.0`).
- **Contextual Quick-Snap Buttons Across the Synth**:
  1. **Pitch / Semitones / Coarse Tune**: `-24`, `-12`, `-7`, `0`, `+7`, `+12`, `+24`
  2. **Dual-Mode FX Mix Knobs**: `-100% (Wet)`, `0% (Dry)`, `+100% (Parallel)`
  3. **Filter Cutoff**: `60 Hz (Sub)`, `250 Hz (Warmth)`, `1 kHz (Body)`, `3.5 kHz (Edge)`, `10 kHz (Air)`
  4. **Filter Resonance / Q**: `0.5 (Gentle)`, `0.707 (Flat/Butterworth)`, `1.414 (Musical Peak)`, `4.0 (Ring)`, `10.0 (Self-Osc)`
  5. **Envelopes (Decay / Release)**: `10 ms (Click)`, `60 ms (Tight Snare)`, `150 ms (Punchy Kick)`, `600 ms (808 Boom)`, `2.0 s (Tail)`
  6. **Tempo Delay & Reverb Gate Times**: `1/16`, `1/8`, `1/8D (Dotted)`, `1/8T (Triplet)`, `1/4`
  7. **Stereo Enhancer / Width**: `0% (Mono)`, `100% (Normal)`, `200% (Wide)`, `400% (Hyper-Wide)`
  8. **Sample Playback Speed (Plugin Transient Players)**: `-100% (Reverse)`, `+50% (Half-Speed)`, `+100% (Normal)`, `+200% (Double-Speed)`

---

## 📐 Layout & System Architecture (Backlog)

### 14. 2x5 Eurorack Modular Layout Exploration
- Investigate moving from the current 2x4 (8-card) chassis to an expanded **2-row by 5-column (2x5, 10-card)** layout.

### 15. Hardware Standalone Synthesizer Port (BACK BURNER)
- Put on the back burner per user instruction, but fully architected for execution:
  - **Teensy 4.1 Hardware Drum Machine ("The Klang Planter 8-Voice Hardware")**:
    - NXP i.MX RT1062 ARM Cortex-M7 running at **600 MHz**.
    - 8 Mono Voices takes ~2,000 cycles/sample ($\approx 14.7\%$ CPU load).
    - Multi-channel audio output via Cirrus Logic CS42448 (8-channel 24-bit 192 kHz codec).
    - Switched jack normaling for individual voice outs vs. Master Stereo bus.
    - 8 independent MIDI channels with identical CC mappings.
  - **Daisy Seed (Electro-Smith)**: Alternative open hardware platform ($29, STM32H750 Cortex-M7 @ 480 MHz).
  - **Embedded Core Engine (`embedded/KlangPlanterEmbedded.h`)**: Single-file, zero-allocation DSP engine.

### 16. WAV Render & Multi-Sample Export Dialog
- Top navigation `RENDER` button with "Last Note Played", multi-sample velocity/note ranges, and round-robin export for Slop variations.

### 17. Instant DAW Drag 'n' Drop (Tekno-Style)
- UI icon to drag the most recently rendered drum hit directly onto DAW arrangement tracks.

---

## 📦 Completed & Archived Milestones
All completed tasks, architectural decisions, and release summaries are archived in:
👉 **[`docs/BACKLOG_ARCHIVE.md`](BACKLOG_ARCHIVE.md)**  
*(Individual phase execution plans are preserved in [`docs/completed_plans/`](completed_plans/))*
