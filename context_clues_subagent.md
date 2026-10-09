# Subagent Quick-Reference Index & Context Clues

> **Step 0 Bootstrapping**: Subagents waking up from `invoke_subagent` must index this document on Turn 1 to orient on project taxonomy, ecosystem pillars, and core invariants without prompt bloat.

---

## 🔬 Subagent Roles & Operating Modes
- **`research` (`Model="pro"` / Tier 1 Sprint)**: Deep DSP differential equations, non-linear saturation proofs, lock-free ring-buffer concurrency, and complex multi-file architectural synthesis.
- **`explore` (`Model="flash"` / Tier 2 Marathon)**: Broad UI/UX competitive benchmarking, community sentiment analysis (Reddit/KVR/Gearspace), preset browser taxonomy, and responsive layout sweeps.
- **`contrarian` (`Model="flash"` / Tier 2 Advocate)**: The Devil's Advocate; stress-tests proposals, identifies hidden architectural traps, dependency bloat, and derives the pragmatic 80/20 middle ground.

---

## 🏛️ The Four Ecosystem Pillars (Links)
1. **The Klang Suite (Desktop Audio Plugins - VST3/AU)**: [`c:\Dev\TheKlangSuite`](file:///c:/Dev/TheKlangSuite)
2. **ToadTracker (16-Step Embedded Hardware Tracker)**: [`c:\Dev\ToadTracker`](file:///c:/Dev/ToadTracker)
3. **N'kai (Asymmetric Sidecar Framework & Triage Lab)**: [`c:\Dev\nkai`](file:///c:/Dev/nkai)
4. **The Klang Research & Obsidian Vault**: [`C:\Dev\Research`](file:///C:/Dev/Research) & [`C:\Dev\TheKlangVault`](file:///C:/Dev/TheKlangVault)

---

## 📖 Canonical Taxonomy & Architecture References (Links)
- **Official Architectural Glossary**: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md)
  - *Taxonomy*: *Cards* (UI containers), *Modules* (DSP units), *Sliders* (meter bars), *Knobs* (rotary controls).
  - *Acronyms*: `TKS` (Suite), `TKF` (Farmer), `TKP` (Planter), `TKM` (Mill), `TKR` (R1), `TKB` (Hardware Box), `TKG` (Klang-Brain).
- **Master Documentation Catalog**: [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) (100% indexed spec inventory).
- **System Architecture Map**: [`docs/SYSTEM_MAP.md`](file:///c:/Dev/TheKlangSuite/docs/SYSTEM_MAP.md)
- **Product Intent Matrix**: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md) ("Stay in Your Lane" drift guardrail).

---

## ⚡ Core Invariants & Access Policies
- **Real-Time Audio Thread (CRITICAL)**: Zero allocations (`new`/`malloc`/`vector::push_back`), zero locks (`std::mutex`), zero I/O (`DBG`/`printf`) in `processBlock`. SIMD `FastMath` over CRT.
- **Data-Driven Separation**: DSP parameters in `assets/controls/*.json`, structural layout in `assets/layouts/*.json`, styling in `assets/themes/*.json`, copy in `assets/text/*.json`.
- **Web Access Invariant**: Subagents are pre-authorized for `search_web` and `read_url_content` by default. If charter specifies `--no-web` or "offline", research is strictly restricted to local files and documentation.

---

## 🛠️ Subagent Implementation Delegation Protocol (The 20-Line / 1-File Rule)
- **Role**: `Factory Implementation Subagent` (`Model="flash"` default / Tier 2)
- **Scope**: When the user requests implementation during an active research or triage session, tasks are delegated to subagents to shield the main chat context from file diffs and compiler noise.
- **Triage Threshold**:
  - *< 20 lines, 1 file*: Execute directly in main chat (instant, zero startup overhead).
  - *> 20 lines, multi-file, or running builds/tests*: Delegate to Flash subagent (`invoke_subagent(Model="flash")`).
  - *Deep DSP / Lock-Free Math*: Delegate to Pro subagent (`invoke_subagent(Model="pro")`).
- **2-Strike Escalation**: If a Flash implementation subagent fails 2 compilation/test attempts or gets stuck, it hard-pauses and escalates to a Pro subagent.
- **Completion Receipt**: Subagents return only a concise diff summary, verification status, and git state to the parent agent.

