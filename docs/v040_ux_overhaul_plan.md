# Master Plan: Milestone v0.4.0 "The Interface & Experience Update"

> **Milestone:** v0.4.0  
> **Theme:** Complete Clean-Slate UX Overhaul & Modern Neo-Slate Vector UI  
> **Ethos:** *"UX IS KING"* (Zero Prototype Debt, Pure Workflow Elegance)  
> **Status:** QUEUED (Scheduled immediately following v0.3.2 release)  

---

## 1. Executive Summary & Design Philosophy

Up through v0.3.2, The Klang Suite maintained strict 1:1 parity with the legacy v0.2.0 monolithic prototype. While this preserved functional continuity while establishing tests and data-driven modularity, it forced the user interface to carry forward prototype design debt: sprawling 204-parameter lists, an awkward 8-card grid where navigation and visualizers consumed 25% of the playing surface, and visual clutter.

With **Milestone v0.4.0**, we execute a **Total Clean-Slate Parity Break**:
1. **Modern Vector Neo-Slate Aesthetic**: Inspired by modern audio software masterpieces (**Kilohearts Phase Plant**, **Vital**, and **Arturia Pigments**). Clean dark-slate surfaces (`0xff121622`), crisp 1px borders, high-contrast typography, and luminous neon accents. **Zero faux-vintage screws, zero fake drop shadows, zero 3D skeuomorphism.**
2. **The 4-Controls-Per-Card Mandate**: Every Card front panel is strictly limited to **exactly 4 high-impact performance controls**, organized as a vertical 1x4 stack of horizontal Meter Sliders.
3. **Hardware-Informed Synergy (dadamachines TBD-16)**: While software-first, the 4-control limit creates immediate 1:1 synergy with 1x4 encoder controllers, matching the physical layout of the TBD-16.
4. **Header-Integrated Navigation & Scope**: Page navigation tabs (`[VOICE 1]`, `[VOICE 2]`, `[TRANSIENTS]`, `[FX 1-4]`, `[FX 5-8]`, `[MOD]`) and the master stereo oscilloscope/meters move to the top Chassis Header, freeing up 100% of the rack canvas for synth modules.
5. **Right-Click Callout Deep Dive**: Advanced and secondary parameters (fine-tuning, tracking modes, snap points, velocity curves) are tucked into sleek popover CalloutBoxes, keeping the primary performance surface distraction-free.

---

## 2. Core Architecture & UI Layout

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│  THE KLANG FARMER   [VOICE 1] [VOICE 2] [NOISE] [FX 1-4] [FX 5-8] [MOD]  [===SCOPE===] │  <-- CHASSIS HEADER
├────────────────────────────────────────────────────────────────────────────────────────┤
│ ┌── CARRIER 1 ─────┐ ┌── MODULATOR 1 ──┐ ┌── PITCH ENV 1 ──┐ ┌── FILTER 1 ─────┐       │
│ │ [Shape    ~~~ 0] │ │ [Speed  1.00x]  │ │ [Slope   \ 0.5] │ │ [Type       LPF]│       │
│ │ [Pitch  +12.0st] │ │ [Depth   45%]   │ │ [Depth    +24st]│ │ [Slope    24 dB]│       │
│ │ [Detune +0.10st] │ │ [Track   NOTE]  │ │ [Decay   140ms] │ │ [Cutoff 2.4 kHz]│       │
│ │ [Level   0.0 dB] │ │ [Shape   /\/\]  │ │ [Curve     EXP] │ │ [Resonance  35%]│       │
│ └──────────────────┘ └─────────────────┘ └─────────────────┘ └─────────────────┘       │
│ ┌── AMP ENV 1 ─────┐ ┌── VELOCITY 1 ───┐ ┌── MIXER ────────┐ ┌── OUTPUT/PANIC ─┐       │
│ │ [Attack     2ms] │ │ [Vol Sens 80%]  │ │ [Voice 1  0 dB] │ │ [Master  0.0 dB]│       │
│ │ [Decay    280ms] │ │ [Pitch Sens 0%] │ │ [Voice 2 -6 dB] │ │ [Pan     Center]│       │
│ │ [Sustain    0%]  │ │ [Filter Sens]   │ │ [Noise  -12 dB] │ │ [Limiter   +3dB]│       │
│ │ [Drive    +4 dB] │ │ [Floor    10%]  │ │ [RingMod    0%] │ │ [PANIC!   FLASH]│       │
│ └──────────────────┘ └─────────────────┘ └─────────────────┘ └─────────────────┘       │
├────────────────────────────────────────────────────────────────────────────────────────┤
│ Carrier 1: Shape  Sine -> Saw [0.42]  |  Right-Click: Snap Points  |  Double-Click: 0.0│  <-- STATUS BAR
└────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Phased Implementation Roadmap

### Phase 1: Curated 4-Control Data Schema & Parameter Pruning
- Author new curated parameter definitions in `assets/controls/*.json`.
- Group each module into exactly 4 primary parameters and define secondary callout parameters.
- Update `assets/layouts/tkf_layout.json` and `tkp_layout.json` to reflect the clean 4-control cards.
- Update `assets/text/strings.json` with polished, descriptive copy for the curated set.

### Phase 2: Neo-Slate Chassis Header & Integrated Oscilloscope
- Evict the legacy Navigation Card and Visualizer Card from `FarmerEditor`.
- Build the modern horizontal Header Tab Bar with active page indicators.
- Port and adapt `PlanterHeaderVisualizer` into a universal `KlangHeaderVisualizer` featuring real-time stereo waveform scope, dual peak VU meters, and Limiter trigger badge.
- Expand the main canvas layout from 6 modular slots to 8 full-width sound-sculpting card slots.

### Phase 3: Vertical 1x4 Meter Slider Stacks & Real-Time Diagrams
- Refactor `ModuleCardComponent` to render 4 horizontal Meter Sliders in a clean vertical stack.
- Integrate real-time vector curve drawing (`DiagramType::Waveform`, `DiagramType::EnvelopeSlope`, `DiagramType::FilterSlope`) inside slider troughs.
- Style with neo-slate dark themes (`0xff121622` chassis, `0xff181c26` card trough, vibrant cyan/coral/amber accent fills).

### Phase 4: Right-Click Callout Deep-Dive Engine
- Standardize right-click listeners across all module cards and sliders.
- Implement floating `juce::CallOutBox` dialogs exposing secondary parameters:
  - Pitch & Harmonic Snap Point detent grids.
  - S&H clock jitter and frequency tracking modes.
  - Velocity mapping curves and bipolar modulation offsets.

### Phase 5: The Klang Editor Interactive Curve & Snap Calibration Workspace
- Update The Klang Editor's property panel and preview canvas for the new 4-control card architecture.
- Integrate live interactive slider skew tuning and snap point testing.

### Phase 6: Automated Test Suite Overhaul & Validation
- Update `test/PluginIntensiveTestSuite.h`, `test/ParameterSchemaAuditTest.h`, and `test/EditorTestSuite.h` to validate the new curated parameter reflection, 4-control slot limits, and header navigation.
- 100% assertion pass across `dsp_tests.exe` and `gui_tests.exe`.
- Compile Release binaries and deploy via `deploy.ps1`.
