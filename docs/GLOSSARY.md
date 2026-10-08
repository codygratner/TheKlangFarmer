# The Klang Suite: Official Architectural Glossary & Taxonomy

> [!NOTE]  
> This glossary is the single source of truth for terminology across UI design, synthesis audio architecture, and Git release lifecycles in The Klang Suite. All documentation, code reviews, and JSON data schemas must adhere to these definitions.

---

## 1. UI Surface & Spatial Hierarchy

| Term | Scope | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Chassis** | Structural | The master application window/frame housing global headers, footers, visualizers, and the main canvas. | The top-level `FarmerEditor` / `PlanterEditor` window (800x600 to 4K resizable). |
| **Page** | View | A full-canvas surface swap. Swapping pages replaces the displayed cards while maintaining global headers and audio playback. | `FarmerEditor` Page 1 ("Main Synth"), Page 2 ("FX Rack 1-4"), Page 3 ("FX Rack 5-8"). |
| **Card** | Visual UI | A rectangular visual UI container with an accent color, title bar, background styling, and slot grid. | Cards 1–8 in Farmer; Cards 1–8 in Planter (defined structurally in `assets/layouts/*.json`). |
| **Module** | Audio DSP | The functional audio-processing unit. A Module generates or alters sound; a Card is the visual UI that exposes it. | S&H Filter, Carrier 1, Master Limiter. (One Card controls one Module; Callouts can also control Modules). |
| **Slot** | Layout | A discrete coordinate inside a Card's grid where a control widget resides. | Row/column cells defined in `tkf_layout.json` (e.g. `row: 0, col: 1`). |
| **CalloutBox / Popover** | Transient UI | A temporary floating mini-panel anchored to a specific trigger badge or control, launched by click or right-click. | `juce::CallOutBox` used for Planter Master Limiter, Snap Point menus, and Color Pickers. |

---

## 2. Interactive Controls & Widgets

| Term | Visual Style | Interaction & Behavior | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Slider** *(Meter Slider)* | Horizontal recessed trough with filled color bar | Dragging horizontally or vertically fills the bar; embedded label on left, value on right. | Primary signature widget across Cards 1–8 (arcade HP meter style). Under the hood: `RotaryKnobSlider` class. |
| **Knob** | Circular rotary dial | Vertical drag rotates pointer/indicator around pivot. | Used in floating Callout mini-cards (Planter Limiter) and hardware-inspired controls. |
| **Fader** | Vertical linear slide potentiometer | Vertical axis drag exclusively reserved for mixing channel levels and summing. | Reserved for mixer channel strips (Card 8 channel levels). |
| **Selector** | Text badge / pill stepper | Clicking opens dropdown or cycles through discrete choice options. | Waveform shapes, MIDI tracking modes, filter types (`DiscreteSelector`). |
| **Toggle** | 2-state latching button / LED | Binary click toggles state between on/off (`0.0` or `1.0`). | Header `TOOLTIPS`, `BYPASS` switches, modal toggles. |

---

## 3. Synthesis & Audio DSP Taxonomy

| Term | Domain | Definition & Industry Standard | The Klang Suite Implementation |
| :--- | :--- | :--- | :--- |
| **Carrier** | FM Audio | An operator whose audio output is **heard directly** in the mix. | Carrier 1 & Carrier 2 (audible sound generators feeding the mixer). |
| **Modulator** | FM Audio | An operator whose audio output is **inaudible directly**, but modulates the phase/frequency of a Carrier. | Modulator 1 & Modulator 2 (generating metallic harmonic sidebands). |
| **Operator** | FM Audio | The complete FM synthesis building block: Oscillator + Dedicated Envelope Generator + Level Scaling. | Chowning / Yamaha DX7 standard architecture. |
| **Engine** | Master Processor | The master audio container managing voice allocation, MIDI event parsing, and audio bus routing. | `TheKlangFarmerAudioProcessor`, `TheKlangPlanterAudioProcessor`. |
| **Voice** | Audio Instance | A single independent sound execution instance that renders one drum hit (tracks active phase, envelope state, filter memory). | Individual polyphonic / monophonic voice renderers in DSP code. |
| **DSP Block** | Algorithm | The C++ class implementing a specific mathematical audio routine. | `juce::dsp::LadderFilter`, `TbdAudio::FastMath`, `SvfFilter`. |

---

## 4. DevOps, Git & Release Lifecycles

| Term | Mechanism | Analogy | The Klang Suite Protocol |
| :--- | :--- | :--- | :--- |
| **Commit** | Local Git record | Checkpoint save on local drive | Atomic, descriptive commit (`feat:`, `fix:`, `docs:`) created via `git commit`. |
| **Push** | Network upload | Syncing local save files to cloud | `git push origin <branch>` to publish local commits to GitHub. |
| **Branch** | Isolated timeline | Parallel universe for experimentation | `main` (stable production), `<version>-dev` (e.g. `0.3.2-dev` active milestone). |
| **Tag** | Immutable pointer | Stamping an official serial number on a build | Annotated Git tag (`git tag -a v0.3.1 -m "Release v0.3.1"`) frozen on one commit forever. |
| **Version (SemVer)** | Numerical standard | Vehicle model year & trim | `MAJOR.MINOR.PATCH` (`v0.3.2`): Breaking changes = Major; Features = Minor; Fixes = Patch. |
| **Milestone** | Project container | The sprint checklist | Roadmap container in `docs/BACKLOG.md` grouping features and fixes targeting a version. |
| **Release** | Public distribution | Placing boxed product on retail shelves | Published GitHub Release bundled with installer artifacts (`.vst3`, `.exe`, `.pkg`) and changelogs. |

> [!TIP]
> **The "Silent Git Tag" Policy**: When a milestone consists strictly of internal architectural cleanup, developer tooling, or schema refactors with zero user-facing sound/UI changes (e.g. `v0.3.2`), we stamp and push an annotated Git Tag (`git tag -a vX.Y.Z`) and merge to `main`, but do not publish a public GitHub Release entry. This preserves clean SemVer and reproducible builds while preventing spurious "Update Available" notifications in end-user DAWs.

---

## 5. Ecosystem Product Acronym Registry & Collision Guardrails

To prevent identity collisions across codebases, logging tags (`TKS_LOG`), asset files, and user documentation, every software plugin, standalone app, and hardware spin-off in The Klang Suite is assigned a strict, immutable 3-letter acronym (`TK<X>`):

| Acronym | Product Name | Category | Scope / Role |
| :--- | :--- | :--- | :--- |
| **`TKS`** | **The Klang Suite** | **Umbrella / Suite** | Master project repository, shared utility libraries, and developer logging (`TKS_LOG`). *RESERVED EXCLUSIVELY FOR THE SUITE.* |
| **`TKF`** | **The Klang Farmer** | Instrument Plugin | Dual-FM percussion and bass synthesizer. |
| **`TKP`** | **The Klang Planter** | Instrument Plugin | Multi-layer percussive sound machine. |
| **`TKM`** | **The Klang Mill** | FX Plugin / Sub-Rack | 1x6 modular multi-effects pedalboard rack. |
| **`TKE`** | **The Klang Editor** | Standalone Desktop App | JSON schema, theme calibration, and curve inspection workstation. |
| **`TKB`** | **The Klang Box** | Embedded Hardware | Standalone hardware synthesizer (Daisy Seed / Teensy 4.1 / dadamachines TBD-16). |
| **`TKR`** | **The Klang R1** | Instrument Plugin | 7-voice Electribe ER-1 tribute rhythm synthesizer. |
| **`TKC`** | **The Klang Cultivator** | Utility Plugin | Standalone MIDI CC / CV / MPE modulation generator rack. |

> [!WARNING]
> **Acronym Collision Guardrail**: No new plugin or spin-off product may adopt an acronym that collides with registered letters (`S`, `F`, `P`, `M`, `E`, `B`, `R`, `C`). `TKS` is strictly forbidden for individual plugins.
