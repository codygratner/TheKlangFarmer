# Context Clues: New Klang City (Planning Headquarters)

## 📌 Active SemVer & Branch State
- **Current Git Branch:** `0.4.0-dev` (ahead of origin)
- **Ecosystem SemVer Matrix:**
  - `The Klang Suite (TKS)`: `v0.4.0-dev`
  - `ToadTracker (TT)`: `v0.1.0`
  - `N'kai (NK)`: `v0.1.0`
- **Active Working Milestone:** `v0.4.0 "The Interface & Experience Update"` (🔨 READY FOR RELEASE CUT)
- **Next Target Milestone:** `v0.4.1 "The Complete Control Surface & UI Shells"` (Staged Draft Plan Ready in `.agents/pipeline/plans/drafts/`)
- **Follow-up Milestone:** `v0.4.5 "The Klang Planter Refresh"` (Rebuilding Planter on TKF's Neo-Slate & v0.4.1 Shells)
- **Upcoming Sonic Milestone:** `v0.5.0 "The Sound & Chaos Update"` (Sonic Expansion & DSP Hardening)
- **Completed & Locked Milestones (NEVER GO BACK):** 
  - `v0.3.0` Architecture & Parity Audit — ✅ COMPLETED
  - `v0.3.1` Editor Quality & Data Schema — ✅ COMPLETED
  - `v0.3.2` Agent Infrastructure & Logging — ✅ COMPLETED
  - `v0.3.3` The Hardening Gauntlet — ✅ COMPLETED

## 🎯 Immediate Return Agenda
- **🔔 REMINDER: Finish Post-Mortem & Triage Lab**:
  - The Layouts & Ergonomics Post-Mortem is paused and packaged in:
    [`c:\Dev\TheKlangSuite\.agents\sidecar\packages\2026-10-08_layouts_and_ergonomics_triage_package.json`](file:///c:/Dev/TheKlangSuite/.agents/sidecar/packages/2026-10-08_layouts_and_ergonomics_triage_package.json)
  - Active Sidecar Canvas: [`ui_ux_research_board.html`](file:///C:/Users/codyg/.gemini/antigravity/brain/9241988e-1227-466e-a0a5-446d61a69d77/ui_ux_research_board.html)
  - First Pending Item to Tackle: **Item 1.1: Monolithic Sculpted Titanium Faceplate vs. Modular Cards** (followed by 6 remaining items).
  - Trigger command: `/resume-post-mortem`.


## 🧠 Architectural Memory & "Unforgettables"
1. **Agent Model Protocol:**
   - Main Chat / Orchestrator runs strictly on **Gemini 3.8 Flash (Thinking: High)** for near-zero quota burn and fast iterations.
   - When deep DSP math, circuit analysis, or complex architecture is required, spawn an on-demand **Pro Subagent** (`Model="pro"`).
2. **DSP & Audio Thread Invariants:**
   - Zero runtime allocations (`new`/`malloc`) or locks (`std::mutex`) in `processBlock`.
   - Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`).
   - Hardware Target: dadamachines TBD-16 (ESP32-P4 @ 400MHz, PSRAM memory budget).
3. **Synthesis Engine Realities:**
   - **Envelopes:** Continuous `_slope` controls ALREADY exist on all envelopes (`ampenv_slope`, `filterenv_slope`, etc.). Do not propose duplicate parameters; optimizations focus on replacing CRT `std::pow` with `FastMath`.
   - **Approved v0.5.0 Modules:** Casio CZ Phase Distortion, Alpha Juno "Hoover" Saw-PWM, Alpha Juno IR3R05 Dual-Stage Resonant VCF (Zero Bass Drop), Buchla Wavefolder + Pre-APF Phase Dispersion, Elektron Octatrack Tuned Comb Filter / Karplus-Strong Resonator.
   - **Approved v0.6.0 Workflow:** MTS-ESP / Scala Microtonal Support (Desktop Only), Offline Pitch-Tracked Oversampling, Fullscreen Splice-Style Browser with Embedded Macros.
4. **Data-Driven Hierarchy:**
   - All parameters live in `assets/controls/*.json`.
   - Structural UI lives in `assets/layouts/*.json`.
   - Themes live in `assets/themes/*.json`.
   - Strings live in `assets/text/*.json`.

## 📖 Essential Taxonomy, Product Lineup & Specs (Links)
- **Official Architectural Glossary & Taxonomy:** [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - UI vs DSP Terminology (*Chassis*, *Pages*, *Cards*, *Modules*, *Sliders* vs *Knobs*).
  - Acronym Collision Registry: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKE` (Editor), `TKB` (Box / Hardware), `TKR` (R1), `TKC` (Cultivator).
  - Generative Engine: `The Klang-Brain` (internal code `TKG`).
- **Product Lineup & Core Intent Matrix:** [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md)
  - "Stay in Your Lane" drift guardrail (TKF = deep sound design, TKP = lean & mean BIA machine, TKM = pure multi-FX rack, TKR = 7-voice zero-tab rhythm console, TKB = bare-metal zero-allocation hardware).
- **Core Spin-Off & Hardware Specs:**
  - [`docs/specs/spinoffs/the_klang_r1.md`](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_r1.md) (TKR-1 ER-1 Tribute & Klang-Brain Engine)
  - [`docs/specs/spinoffs/the_klang_mill.md`](file:///c:/Dev/TheKlangSuite/docs/specs/spinoffs/the_klang_mill.md) (TKM 1x6 Pedalboard Rack)
  - [`docs/specs/tbd16_klang_seed_effects.md`](file:///c:/Dev/TheKlangSuite/docs/specs/tbd16_klang_seed_effects.md) (TKB-TBD Embedded Hardware Integration)

