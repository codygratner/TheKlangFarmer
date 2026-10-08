# 🧭 The Vibe-Coder's Field Guide: Autonomous Pair-Programming, Architecture & Hard Lessons

> **Author:** The Klang Suite Project Journal  
> **Audience:** Developers transitioning into C++ audio development, AI-assisted software engineers, and fellow "vibe coders".  
> **Status:** Living Field Manual (Milestone v0.4.0)

---

## 1. Executive Summary: The "Vibe-Coding" Paradox

In popular tech culture, **"vibe coding"** is often portrayed as casual, unstructured prompting: typing a vague idea into an LLM, watching 500 lines of code appear, running it, and hoping for the best.

When you are building a simple prototype or a script, that works. But when you are building a **hard real-time C++20 desktop application and VST3 audio plugin** with multi-threaded message pumps, strict OS hardware deadlines, and deep visual component trees, that approach completely collapses. 

### The Core Paradox
* **Velocity without guardrails is just high-speed regression.** An AI can generate 1,500 lines of plausible-looking C++ in two minutes—and in the process, inadvertently invert a slider's mathematical curve, introduce a mutex deadlock on the audio thread, leak 200 heap allocations inside a DSP loop, or silently drop UI parameter bindings.
* **The Engineer's True Role Shifts:** Coming from a background in Python, JavaScript, and enterprise VB.NET, the goal here wasn't to memorize every arcane corner of C++ template metaprogramming. The goal was to **architect the system, define the boundaries, enforce the invariants, and build the automated steering wheel and brakes**.

This field guide documents the raw truth of that journey: what failed, what burned us, what we threw into the architectural graveyard, and the production-grade playbook that actually works.

---

## 2. The Hard Invariants: Why Audio Programming Rejects "Lazy Vibes"

In web or backend development, a slow database query or a 50ms garbage collection pause is a minor latency blip. In real-time audio DSP, **it is a catastrophic failure**.

The audio card hardware requests a new buffer of samples every $1.3\,\text{ms}$ (at 64 samples @ $48\,\text{kHz}$). If your code misses that deadline by even one microsecond, the audio stream drops out, resulting in an audible click, pop, or harsh digital glitch.

Therefore, our AI pair-programming rules had to codify non-negotiable **Audio Thread Invariants**:
1. **Zero Heap Allocations:** Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::String` concatenation) on the audio path. Everything must be pre-allocated in `prepareToPlay()`.
2. **Zero Locks:** Never acquire a `std::mutex`, `juce::CriticalSection`, or wait on thread primitives. Audio-to-UI communication must use lock-free atomics (`std::atomic<float>`) or single-reader single-writer FIFOs.
3. **Zero Blocking I/O:** Never call filesystem operations, network sockets, or console output (`std::cout`, `DBG()`, `printf`) on the audio thread.
4. **SIMD & FastMath:** Standard math functions (`std::pow`, `std::sin`, `std::tanh`) take 50–120 CPU cycles. In hot voice loops, you must use rational Padé or polynomial approximations (`TbdAudio::FastMath`).

Teaching an LLM to respect these rules required moving beyond loose prompts and building **automated gatekeepers**.

---

## 3. 🪦 The Strategy Graveyard: What Failed & Why We Abandoned It

Every robust pattern in our repository exists because a simpler, lazier approach burned us first. Here are the hard lessons:

```text
=============================================================================
                       THE STRATEGY GRAVEYARD
=============================================================================
  [Tombstone 1] The Monolithic Chat Trap (50+ Turn Amnesia & Drift)
  [Tombstone 2] Hardcoded C++ Parameter Monoliths
  [Tombstone 3] Interactive Start Modals (The Locked Dropdown Deadlock)
  [Tombstone 4] Manual "Click-and-Listen" Testing
  [Tombstone 5] Blind URL Scraping & Clipboard Sniffing
  [Tombstone 6] Mandatory "@" Mentions (The Micro-Optimization Trap)
=============================================================================
```

### Tombstone 1: The Monolithic Chat Trap (Context Bloat & Semantic Drift)
* **What We Tried:** Running planning, architectural debates, code writing, and compiling all inside a single long-running chat session.
* **Why It Failed:** By Turn 35 or 40, the conversation context becomes saturated with thousands of lines of compiler logs and obsolete code snippets. The model begins hallucinating earlier states, forgets recently established data contracts, and suffers from subtle "prompt drift."
* **The Solution:** We cleanly bifurcated the development process into two specialized chat identities:
  - **New Klang City (Planner / Architect):** Read-only for C++ source code. Focuses exclusively on roadmaps, `/grill-me` interviews, data schemas, and documentation.
  - **Klang Industries (Builder / Factory Floor):** Focuses strictly on executing targeted blueprints, running the compiler, and executing tests.
  - Paired with proactive context harvesting (`/refresh-context`), sessions are kept lean and laser-focused.

### Tombstone 2: Hardcoded C++ Parameter Monoliths
* **What We Tried:** Defining parameter names, min/max ranges, default values, visual colors, and tooltip descriptions directly in C++ constructors (e.g. inside `FarmerEditor.cpp` or `Parameters.cpp`).
* **Why It Failed:** Every small UI tweak required recompiling C++. Worse, when adding new features or migrating effects slots, parameter IDs and default values drifted out of parity between the engine and the UI.
* **The Solution:** The **4-Layer Declarative Data Schema**. Parameter contracts, UI hierarchies, color themes, and tooltip text were ruthlessly extracted into separate JSON layers. C++ became a lightweight consumer of immutable data contracts.

### Tombstone 3: Interactive Start Modals (The Locked Dropdown Deadlock)
* **What We Tried:** When an execution task started, the builder agent would trigger an interactive modal (`ask_question`) asking the user: *"Ready to build Phase 1? (Yes/No)"*.
* **Why It Failed:** In modern IDEs (like Google Antigravity), an active interactive modal dialog freezes the entire UI window—including the model selection dropdown in the IDE footer! If the agent advised switching from Pro to Flash, the user was physically locked out from changing the dropdown.
* **The Solution:** Strict non-modal builder gates. The builder outputs the model advisory in regular chat text and simply pauses, keeping the IDE controls completely free.

### Tombstone 4: Manual "Click-and-Listen" Testing
* **What We Tried:** Compiling the VST3 plugin, opening a digital audio workstation (DAW) like Ableton or Reaper, loading the plugin, and manually turning knobs to see if anything broke.
* **Why It Failed:** As the synth grew to 22 modules and over 200 parameters, testing everything manually took 45 minutes per release. Subtle bugs—like a slider failing to update when double-clicked, or an FX slot dropping its tooltip—slipped through constantly.
* **The Solution:** The **Universal Headless GUI Test Harness (`gui_tests`)**. A single native C++ binary that sweeps 100% of registered parameters, simulates synthetic mouse gestures, validates tooltip strings, tests offscreen rendering across 4K resolutions, and stresses lifecycle creation/destruction in under 3 seconds!

### Tombstone 5: Blind URL Scraping & Clipboard Sniffing
* **What We Tried:** Letting the agent automatically peek at the host OS clipboard (`Get-Clipboard`) or fetch external web links during task ingestion.
* **Why It Failed:** The system clipboard frequently contains out-of-band private text, passwords, or unrelated snippets from other open programs. Furthermore, web pages can introduce noisy, unverified third-party code.
* **The Solution:** Strict clipboard and external link guardrails in `GEMINI.md`. Blueprints must originate strictly from version-controlled workspace files (`PLAN.md`, `BACKLOG.md`) or direct, intentional chat prompts.

### Tombstone 6: Mandatory "@" Mentions (The Micro-Optimization Trap)
* **What We Considered:** Forcing the developer to type client-side `@filename` mentions (e.g. `resume @context_clues_build.md` or `/read-plan @plan_to_build.md`) during context refreshes and inter-chat handoffs to eliminate a single tool-call roundtrip (saving ~3–5 seconds of machine time).
* **Why We Rejected It:** It commits the cardinal sin of AI tooling: shifting cognitive load from the machine back onto the human. Instead of quickly tapping "take plan" or "resume" from a phone or mid-thought on desktop, the developer was forced to remember file names, wait for autocomplete popups, and struggle with mobile keyboards. Trading human friction for a tiny sliver of compute is a classic developer micro-optimization trap.
* **The Solution:** The AI handles the plumbing so the human stays in flow. Autonomous skills (`read-plan`, `refresh-context`, `task-finish`) perform targeted background reads (`view_file`) deterministically. If the developer happens to type an `@` mention on desktop, the agent accepts it—but the system never demands or depends on manual syntax chores.

---

## 4. 🛠️ The Active Playbook: The System That Actually Works

Through trial and error, we developed a cohesive operating system for AI pair-programming:

```text
┌────────────────────────────────────────────────────────┐
│               THE PRODUCTION PLAYBOOK                  │
├────────────────────────────────────────────────────────┤
│ 1. Multi-Tier Model Economics (Pro vs. Flash)          │
│ 2. The Granular Blueprint Handshake (PLAN.md)          │
│ 3. Automated 2-Strike Factory Escalation               │
│ 4. Asymmetric Second-Brain Knowledge Bridge (Obsidian) │
│ 5. The Hardened Pre-Flight Sweep (vs git clean -fdx)   │
│ 6. Continuous Backlog Integrity Gates                  │
│ 7. The Pre-Release Regression Gauntlet                 │
└────────────────────────────────────────────────────────┘
```

### 1. Multi-Tier Model Economics (Pro vs. Flash)
LLMs have different strengths and quota costs:
- **Tier 1 (Pro High):** High reasoning capacity, but heavy quota impact. Reserved strictly for deep DSP differential equations, non-linear waveshaping algorithms, SIMD optimization, and high-level architectural sanity gates.
- **Tier 2 (Flash High):** Fast, highly capable, and sustainable daily driver. The universal default for drafting plans, writing JUCE UI components, and executing build phases.
- **The "Pro Sanity Gate":** Draft in Flash High at near-zero quota $\to$ switch to Pro High for a 1-turn architectural "insanity check" $\to$ switch back to Flash High to execute the build.

### 2. The Granular Blueprint Handshake (`PLAN.md`)
Never let an AI write code against vague requirements. When planning a feature:
1. Conduct an interactive `/grill-me` interview to resolve every design fork one by one.
2. The planner authors an exact, drop-in specification in `PLAN.md` with concrete C++ class signatures, JSON keys, and test assertion lines.
3. The builder agent reads the blueprint and executes it mechanically without having to guess intent.

### 3. The 2-Strike Factory Escalation Rule
When the builder encounters a compilation error or test failure:
- **Strike 1 & 2:** The agent has up to two attempts on sustainable Tier 2 (Flash High) to analyze the compiler log, locate the error, and fix it.
- **Strike 3:** If the build is still failing after two attempts, the agent is strictly barred from thrashing in circles. It **hard-pauses**, explains the roadblock, and recommends a Tier 1 (Pro High) escalation for the user to confirm.

### 4. Asymmetric Knowledge Sync (Obsidian + Git)
To enable mobile idea capture without Git headaches:
- A standalone, decoupled Obsidian Vault (`TheKlangVault`) holds personal notes, visual canvases, and mobile inbox captures.
- An automated PowerShell sync engine (`tools/sync_obsidian_vault.ps1`) pulls ideas from the Vault's `Inbox/` into the repo, while mirroring official documentation from `docs/` back to the Vault.
- Moving notes to `Inbox/Archive/` in Obsidian automatically sweeps and prunes the local repo copies.
- An automated normalizer translates mobile `[[wikilinks]]` into standard GitHub-clickable relative Markdown links upon sync.

### 5. The Hardened Pre-Flight Sweep (vs. The Aggressive `git clean` Trap)
Before cutting an official release, the workspace must be pristine. But a common trap in automated build scripts is running a blunt `git clean -fdx`. In a C++ project, that wipes the entire `build/` directory, deleting the CMake cache, precompiled headers, and JUCE modules—turning a 45-second build into a 15-minute recompile, while potentially deleting local uncommitted tools.

Instead, `/cut-release` employs a **4-Tier Surgical Pre-Flight Sweep**:
1. **Recursive Ephemeral Purge:** Scans all subdirectories for disposable scratch scripts, update tools, and backup snapshots (`temp_*.*`, `update_*.py`, `context_snapshot*.md`, `PLAN_BACKUP*.md`).
2. **Pipeline & Inbox State:** Confirms `PLAN.md` is idle, the communiqué handshakes are marked `COMPLETED`, and the Vault inbox is at true Inbox Zero.
3. **Diagnostic Disk Hygiene:** Flushes stale test screenshots from `test_screenshots/` so release assertion failures are fresh and unmistakable.
4. **Static Audio Safety & Debug Leak Audit:** Runs a fast regex scan across modified C++ files to catch rogue `std::cout`, `printf`, audio-loop `DBG(` leaks, or missing `stopTimer()` destructor calls.

### 6. Continuous Backlog Reconciliation & Integrity Gates
In high-velocity pair programming, "Backlog Drift" occurs when the factory floor completes a feature and archives its plan, but the planner jumps straight into brainstorming the next feature without updating `docs/BACKLOG.md`. Over time, the backlog becomes littered with stale `PLAN.md` links and missing completion checkmarks.

To eliminate Backlog Drift, we established a **3-Tier Backlog Integrity System**:
1. **The Ingestion Handshake Rule:** When New Klang City reads a `Status: COMPLETE ✅` report from Klang Industries, its first mandatory action is updating the backlog item to `— ✅ COMPLETED`, recording test metrics, and replacing temporary `PLAN.md` references with the permanent path in `.agents/pipeline/plans/completed/`.
2. **The Passive Sync Linter:** The Obsidian sync bridge (`sync_obsidian_vault.ps1`) runs an automated regex audit across `docs/BACKLOG.md` on every pass, actively warning if any stale `[`PLAN.md`]` links linger in the text.
3. **The Pre-Release Milestone Lockout:** `/cut-release` hard-fails and halts if any item in the target release milestone is uncompleted or links to an unarchived plan, guaranteeing 100% backlog integrity before a release tag is stamped.

---

## 5. Advice for Fellow Developers (From Python/JS to C++)

If you are a developer with experience in dynamic languages (Python, JavaScript, Ruby) or garbage-collected frameworks and want to build high-performance C++ software with an AI assistant:

1. **Don't Let the AI Guess the Architecture:** AI agents are brilliant code-completion engines, but they will default to whatever pattern is easiest in the moment (which is usually monolithic, hardcoded C++). Enforce clean architectural patterns (like data schemas and interface boundaries) from Day 1.
2. **Invest Heavily in Test Harnesses:** Writing a functional test harness (`gui_tests`) feels like a detour when you just want to build your app. In reality, it is the single best investment you will make. It allows you to accept large AI refactors with complete confidence.
3. **Embrace "Living Documentation":** Keep your taxonomy codified in a glossary (`GLOSSARY.md`). If you and the AI agree that a container is a "Card" and a rotary control is a "Knob", you eliminate 90% of naming bugs and mismatched variables.
4. **Treat Failure as Data:** When a bug slips through, don't just fix the code. Ask: *"What guardrail was missing that allowed this to happen?"* Update your `GEMINI.md` or test suite so the exact same mistake can never be made again.

---

*“Code is ephemeral; test harnesses, data contracts, and architectural guardrails are permanent.”*

### 4.7 The "Tick-Tock" Versioning Strategy & Foundation Capstones
*Implemented during the transition from v0.3.x to v0.4.x.*
We recognized a structural risk: diving straight from one massive feature milestone (0.3.0 Data Schema) into another (0.4.0 UI Overhaul) allows technical debt, compiler warnings, and untested DSP edge-cases to silently accumulate beneath the floorboards. 
To prevent this, we codified the **Pre-Flight Capstone Rule**. 
- .0 Minor releases (the "Tick") are strictly for breaking UI/feature changes.
- .Z Patch releases (the "Tock") are strictly for backend CI lockdowns, sanitizer (ASan/TSan) integrations, and DSP safety nets (NaN/Inf failsafes).
Before CMakeLists.txt is ever bumped to an X.Y.0-dev branch, the AI pipeline must execute a final Z hardening patch to mathematically prove the foundation is bulletproof. You cannot build a new house on an unhardened foundation.

### 4.8 CI/CD Remote Matrix Gating (The Final Defense)
*Implemented during v0.3.3 Hardening Gauntlet.*
Because we develop exclusively on Windows, we are blind to how Clang (macOS) and GCC (Linux) compilers handle our C++ changes until we push. We strengthened the /cut-release agent skill by injecting **Phase 4.5: CI/CD Remote Matrix Verification**. The agent now queries the GitHub API (gh run list) post-push and monitors the remote build farm. If a remote OS triggers a strict -Werror failure, the agent halts the deployment and fetches the logs automatically. This guarantees cross-platform stability before a release is ever made public.

### 4.9 Mid-Flight Plan Amendments & Live Factory Telemetry
*Implemented during v0.4.0 UI & Modulation Overhaul.*
In a decoupled multi-agent architecture (Planner in Ivory Tower, Builder on Factory Floor), a subtle blindspot exists: if the Builder is actively working through an implementation phase, and the Planner or user refines requirements or adds polish mid-flight, a silent edit to `PLAN.md` leaves the Builder operating on stale assumptions.
We solved this by establishing a two-way reactive state machine across the Communique Mailbox:
1. **`STATUS: PLAN_AMENDED ⚠️`**: When New Klang City modifies requirements mid-build, it sets this status in `plan_to_build.md` alongside an explicit bulleted changelog. We upgraded the `/step-verify` skill to intercept this: when the Builder completes a phase, it reads the mailbox, detects the amendment, re-syncs `PLAN.md`, resets the mailbox to `IN_PROGRESS`, and adapts dynamically.
2. **`STATUS: BUILDING 🔨 (Phase <N>)`**: Rather than remaining silent until final completion, the `/read-plan` and `/step-verify` skills were upgraded to publish active factory telemetry directly into `build_to_plan.md` the moment a job is ingested and at every phase transition. This gives the entire pipeline live visibility into exactly what code is being forged.

### 4.10 The Universal Documentation Sync Skill & Defense-in-Depth (`/update-docs`)
*Implemented during v0.4.0 UI & Modulation Overhaul.*
As a codebase expands across multiple milestones, documentation drift becomes an acute risk: design specs fall out of alignment with C++ realities, completed tasks linger unchecked in the backlog, and LLM context windows waste valuable tokens parsing sprawling 500-line Markdown documents.
We established a strict three-tier "Defense in Depth" documentation protocol:
1. **The System Map (`docs/SYSTEM_MAP.md`)**: A lightweight "Yellow Pages" root node that immediately orients fresh agent sessions with direct links to active specs, core rulebooks, and communication mailboxes.
2. **Table of Contents (ToC) Indexing**: Fast anchor links injected into the head of major documentation files (`docs/BACKLOG.md`) allowing agents to leap directly to relevant milestone headers without reading hundreds of lines of legacy context.
3. **The `/update-docs` Skill**: An automated synchronization skill executed exclusively by New Klang City. It reconciles completed factory tasks against `BACKLOG.md`, synchronizes technical specifications in `docs/specs/`, captures institutional memory in `DEV_HISTORY.md`, and validates cross-link integrity in a single non-destructive pass.

### 4.11 The Dual-Chat Synergy & The "Grill Lab" (Live Sidecar Visual Sandboxing)
*Implemented during v0.4.0 UI & Theming Expansion.*

#### 1. The Dual-Chat Division of Labor (Ivory Tower vs. Factory Floor)
The greatest workflow revelation in our Antigravity pair-programming setup is the strict bifurcation between two dedicated chat sessions:
- **New Klang City (Planner / Architect / Ivory Tower)**: Operates at the 30,000-foot view. Read-only for C++ source files. Sits in Gemini 3.8 Flash High for rapid brainstorming, with brief 1-turn targeted escalations to Gemini 3.1 Pro High for deep mathematical sanity audits and complex system architecture. Manages `BACKLOG.md`, authors `PLAN.md`, and runs visual interviews.
- **Klang Industries (Builder / Factory Floor)**: Operates with boots on the ground. Strictly read-only for overarching architecture. Executes implementation phases, edits C++ files, runs `audiothread-guard`, builds with CMake, executes `gui_tests` and `dsp_tests`, and deploys binaries via `deploy.ps1`.
- **Why It Solves Context Amnesia**: By completely insulating the planning chat from thousands of lines of compiler logs, and insulating the factory chat from rambling design debates, both context windows remain razor-sharp for 40+ turns without degradation or hallucination.

#### 2. The "Grill Lab" (Visual Grill-Me) Breakthrough
Abstract text interviews (`/grill-me`) are invaluable for backend data logic, but they break down completely when designing visual user interfaces, layout proportions, and color palettes. A developer cannot evaluate whether an "Arcade Meter Fader" feels better than a "Rotary Arc Knob" purely from markdown text.

Furthermore, in native C++ audio development, compiling and linking a full JUCE/VST3 binary takes 30 to 90 seconds. Trying out three visual layouts in C++ means 15 minutes of slow rebuilds.

The **Grill Lab** (`/grilllab`, `/vlab`, `/vrill`) completely revolutionizes this loop:
1. **Instant Interactive Feedback (<200ms)**: The agent creates a living, self-contained HTML+CSS sidecar artifact (`visual_grill_me_preview.html`) displayed side-by-side in Antigravity's artifact panel.
2. **Live-Sync Architecture (Flicker-Free Web Worker)**: To bypass `file:///` CORS restrictions and prevent annoying browser reload flickers, the preview spins up an inline Blob Web Worker (`new Worker(URL.createObjectURL(blob))`) that polls a lightweight companion state file (`visual_grill_me_state.json`). When the agent bumps the version key, the preview hot-reloads DOM fragments instantly without full page reloads.
3. **The Multi-Sensory Sandbox**:
   - **Tactile UI Controls**: Draggable sliders, animated popover menus, and live theme switchers let the user *feel* the interaction before any C++ is written.
   - **Built-in Web Audio & Canvas Scope**: An integrated Web Audio synth engine morphs tones as you drag UI mockups, coupled to a 60fps phosphor oscilloscope, creating a true hardware-testing vibe.
   - **1-Click Code Generation**: Exporters convert approved visual styles into drop-in JUCE `paint()` C++ blocks and APVTS JSON schema definitions.
4. **Modal-Free Sidecar Decision Composer**: To prevent interactive chat modals from locking up the IDE model dropdown or chat pane during design interviews, the Grill Lab sidecar includes an integrated decision radio selector, custom write-in textarea, and a 1-click `[ 📋 Copy Complete Answer to Clipboard ]` button. The user tests the options visually, selects their choice, clicks copy, and pastes directly into the chat (`Ctrl+V`).
5. **Conversational Interleaving & Live Tuning**: The interview state is completely decoupled from the chat loop. The user can freely pause the interview at any time to ask side questions, request font zoom or theme adjustments, or feed extra context without resetting the questionnaire.
6. **Dual Operating Modes (UI Sandbox vs. Architecture & Knowledge)**:
   - *UI Sandbox Mode*: Full interactive controls, tactile sliders, vector dice buttons, popovers, canvas oscilloscope, and live Web Audio synthesis engine.
   - *Architecture & Knowledge Mode*: Automatically hides synth audio/oscilloscope headers and renders in a sleek, eye-pleasing **Soft Dark Blue** slate theme (`antigravity_blue`), delivering comprehensive **3-Lens Deep Evaluations** (🌍 Real-World, 🏛️ Industry Standard, 🏆 Best Practices) for non-UI decisions (data schemas, documentation structures, indexing models).
7. **Zero Pro Quota Burn**: Running visual mockups, drafting CSS, and conducting the interview runs sustainably on Tier 2 (Flash High), reserving Tier 1 (Pro High) strictly for deep DSP math and thread safety audits.

The result is a workflow where design mistakes and architectural ambiguities are caught and resolved in seconds in the web sandbox—ensuring that by the time code reaches the C++ factory floor, it is 100% pre-validated, ergonomically tested, and ready to ship.

---

## 5. Negative Architecture & The Strategy Graveyard (The Hybrid 1+3 Standard)
*Adopted during v0.4.0 Knowledge Architecture BAR-B-Q&A.*

Documenting what a system **does not do** is just as critical as documenting its active features. Without negative architecture, teams and AI agents fall into "idea recycling"—re-proposing rejected patterns or repeating failed experiments weeks later.

The Klang Suite enforces a **Hybrid 1 + 3 Negative Architecture Standard**:

### 5.1 Systemic Tombstones (The Field Guide Graveyard)
Systemic, multi-file anti-patterns are recorded here as numbered "Tombstones" to preserve institutional memory:
- **🪦 Tombstone 1: Monolithic Chat Traps**: Trying to plan, build, and debug in a single 60-turn chat causes context amnesia and token thrashing. Strictly separated into Ivory Tower (New Klang City) vs. Factory Floor (Klang Industries).
- **🪦 Tombstone 2: Hardcoded C++ Parameter Contracts**: Hardcoding min/max, default values, and tooltips in C++ creates fragile divergence. All parameter contracts reside exclusively in `assets/controls/*.json` and `assets/text/strings.json`.
- **🪦 Tombstone 3: Native OS Popup Menus in VST3**: Standard `juce::PopupMenu` windows freeze or glitch inside modern host DAWs on Windows and macOS. Replaced permanently with themed `juce::CallOutBox` popovers.
- **🪦 Tombstone 4: UI Thread Calls from Audio Blocks**: Calling `repaint()` or `setValue()` directly from `processBlock()` causes audio dropouts and crashes. Replaced with lock-free atomics and FIFO queues.

### 5.2 Milestone Non-Goals (`PLAN.md` & `BACKLOG.md`)
Every implementation blueprint in `PLAN.md` and major milestone in `docs/BACKLOG.md` must include an explicit:
```markdown
### 🚫 Non-Goals & Rejected Alternatives
- 🚫 Rejecting Pattern X: [Reasoning and why it is out of scope or unfeasible]
- 🚫 Non-Goal Y: [Clarification on what this milestone does NOT attempt to solve]
```
This primes the LLM builder context immediately at the start of each task, preventing scope creep and unapproved architectural deviations.

### 5.3 Targeted Inline Source Annotations (`// 🚫 REJECTED PATTERN`)
Reserved strictly for **CLEAR PROBLEMS TO AVOID** directly at the C++ code level. Rather than cluttering every file, inline rejections are used selectively for high-risk hazards (audio thread invariants, thread synchronization traps, or compiler-specific crashes):
```cpp
// 🚫 REJECTED PATTERN (v0.3.3):
// Do NOT use std::mutex or critical sections in triggerAudition().
// Audio thread invariant #2 forbids locking; use lock-free atomics only.
void triggerAudition(float velocity, int noteNumber);
```
When an agent or human analyzes that specific function, the warning is impossible to miss.
