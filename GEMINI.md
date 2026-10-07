# Agent Guidelines & Guardrails

## Audio Thread Invariants (CRITICAL)
- **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`, `std::string` concatenation) inside `processBlock()`, `processStereo()`, or per-sample render loops. Pre-allocate in `prepareToPlay()`.
- **Zero Locks**: Never acquire a `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio path. Use lock-free atomics (`std::atomic`) or bounded FIFO queues for audio-to-UI messaging.
- **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
- **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.

## Strict Model Advisory Protocol & Anti-Spam Gate
- **Display Only on Kickoff / First Presentation**: The Model Advisory block MUST ONLY be shown when an agent initially presents a new architectural plan or prepares to execute Phase 1 of a task before the user says "proceed". Do NOT repeat or spam the banner on routine conversational turns or within the same tier.
  > 🧠 **MODEL ADVISORY: Tier [1 | 2 | 3]**
  > - **Recommended Setting**: [Gemini 3.1 Pro (Thinking: High) | Gemini 3.8 Flash (Thinking: High) | Gemini 3.8 Flash (Low/Medium)]
  > - **Quota Impact**: [⚠️ HIGH IMPACT | 🟢 SUSTAINABLE | ⚡ MINIMAL]
  > - **Active Model Check**: Please verify your model dropdown in the IDE footer matches this tier before proceeding!

- **Complexity Tiers**:
  | Tier | Scope | Recommended Model | Quota Impact |
  | :--- | :--- | :--- | :--- |
  | **Tier 1 (High Reasoning / Critical DSP)** | Differential equations, non-linear saturation, polyphonic voice allocation, SIMD FastMath, lock-free concurrency, deep architecture. | Gemini 3.1 Pro (Thinking: High) | ⚠️ HIGH IMPACT (Reserve strictly for deep math/DSP/architecture) |
  | **Tier 2 (Balanced Engineering & UI)** *(Universal Default)* | Standard JUCE UI components, modal dialogs, APVTS bindings, expanding `gui_tests` or `dsp_tests`, standard phase execution. | Gemini 3.8 Flash (Thinking: High) | 🟢 SUSTAINABLE (Fast, highly capable daily driver) |
  | **Tier 3 (Rapid Iteration & Data)** | JSON schemas in `assets/`, documentation/backlog updates, git operations, mechanical find-and-replace, CMake tweaks. | Gemini 3.8 Flash (Thinking: Low/Medium) | ⚡ MINIMAL (Near-zero burn rate) |

- **Tier Transition & Verification Rules**:
  - **Flash 3.8 High is the Universal Default**: All planning in New Klang City and building in Klang Industries defaults to Tier 2. Tier 1 Pro High is strictly an on-demand escalation for heavy DSP math or concurrency.
  - **Granular Blueprinting (Pro Plans, Flash Builds)**: When Tier 1 Pro High architects a plan, it MUST decompose `PLAN.md` into exact drop-in method signatures, JSON keys, and test assertion lines so Klang Industries executes immediately on Tier 2 (Flash High).
  - **2-Strike Factory Escalation**: Klang Industries has up to 2 attempts to resolve a compilation or test failure on Tier 2. If still failing, it hard-pauses, recommends a Tier 1 upgrade, and waits for user confirmation.
  - **Tier Upgrade**: Display banner and **hard-pause** for explicit user "proceed". Never auto-proceed into a higher tier.
  - **1-Level Downgrade (Tier 1 ➔ 2, Tier 2 ➔ 3)**: Display banner, schedule a 5-minute grace timer (`schedule(DurationSeconds=300)`), and pause in chat. If timer expires without reply, auto-proceed on existing tier.
  - **2-Level Downgrade (Tier 1 ➔ 3)**: Display banner and **hard-pause indefinitely** without timer to protect critical Pro quota.
  - **Matched Tier Auto-Proceed**: If active model matches or exceeds the recommended tier, output subtle badge `✓ Model Verified: <Model> (<Thinking>) matches Tier <N>` and proceed immediately.
  - **Builder Pause Gate**: Klang Industries MUST NEVER use `ask_question` modals at execution start (modals freeze IDE model dropdowns). Output the Model Advisory banner in chat text and pause.
  - **Subagent Policy**: Always use `Model="flash"` for read-only research subagents (`invoke_subagent`), and `Model="pro"` only for heavy multi-file reasoning.

## Target Toolchain & Standards
- **C++ Standard**: C++20 (`CMAKE_CXX_STANDARD 20`).
- **Framework**: JUCE 9.0.3.
- **Timer Hygiene**: Always call `stopTimer()` as the first line of destructors in all `juce::Timer` subclasses to prevent `#1696` unload crashes.
- **macOS Deployment Target**: Set `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` before `project()` in `CMakeLists.txt`.
- **Linux Portability**: Use `-static-libstdc++ -static-libgcc` and set `VST3_AUTO_MANIFEST FALSE`.

## Strict TODO & Backlog Handling
- **Record Only, Do Not Implement**: When the user mentions items "for the todo", "on the backlog", "for later", or lists future features/fixes, you must **ONLY** document them into `docs/BACKLOG.md`.
- **No Premature Implementation**: Under no circumstances should you edit source code, run compilers, trigger builds, or execute tests for backlog items until the user explicitly directs implementation.
- **Single Source of Truth**: The *only* valid backlog file is `docs/BACKLOG.md`. Never create rogue backlog files (`TODO.md`, `future_backlog.md`, etc.).

## Strict VST3 Deployment Directory
- **Program Files Only**: When deploying or copying built VST3 plugins on Windows, you must **ONLY** install to `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3` and `C:\Program Files\Common Files\VST3\The Klang Planter.vst3`.
- **Never Use AppData**: Under no circumstances should you copy, deploy, or fallback to `%LOCALAPPDATA%\Programs\Common\VST3` or any local user directories.

## Strict Data-Driven Architecture (CRITICAL)
- **JSON First Priority**: Always default to data-driven solutions for new features, UI layouts, colors, and DSP parameters.
- **No Hardcoded Values (Unless Mandatory)**: Under no circumstances should APVTS parameter IDs, default values, min/max ranges, string labels, snap points, or tooltips be hardcoded in C++ source files unless technically mandatory.
- **Strict Schema Separation of Concerns**: All JSON files must rigidly adhere to their designated layer:
  1. `assets/controls/*.json`: Strictly DSP & APVTS parameter contracts (type, min/max ranges, default, format, snap_points, choices). ZERO visual styling, ZERO colors, and ZERO text descriptions.
  2. `assets/layouts/*.json`: Structural UI surface hierarchy (Pages -> Cards -> array of bound parameter IDs).
  3. `assets/themes/*.json`: Visual styling, color palettes, typography (`theme.json`), and floating overlay/callout geometry, background/border colors, and bound parameter arrays (`callouts.json`).
  4. `assets/text/*.json`: Parameter descriptions, choice tooltips, and localized UI copy (`strings.json`).
- **Architectural Taxonomy**: Adhere strictly to `docs/GLOSSARY.md` (*Cards* = UI containers, *Modules* = DSP units, *Sliders* = meter bars, *Knobs* = rotary controls). Consult the glossary on-demand whenever planning new features or resolving naming ambiguities.

## Strict Planning Guardrails
- **No Spontaneous Implementation**: When planning commands (`/plan`, `/read-plan`, `/execute-task`) are invoked, NEVER automatically start writing C++ code, compiling, or modifying source files.
- **Mandatory Confirmation**: Always stop, summarize the loaded plan, and explicitly ask the user for permission to begin implementation.
- **The "Pro Sanity & Standards Gate" (Flash Drafts, Pro Audits, Flash Builds)**:
  - **Scope**: For major architectural features, new schema layers, multi-module UI overhauls, or milestone kickoffs, New Klang City may recommend a targeted 1-turn Pro High sanity audit before dispatching to the factory floor.
  - **Workflow**:
    1. *Drafting (Tier 2 Flash High)*: Conducts `/grill-me`, codebase research, and drafts the initial `PLAN.md` at near-zero quota cost.
    2. *Sanity Gate (Tier 1 Pro High)*: User temporarily swaps to Pro High to stress-test the draft for DAW automation compatibility, edge cases, industry standards, and daily developer ergonomics (the "insanity check").
    3. *Dispatch & Build (Tier 2 Flash High)*: User swaps back to Flash High so Klang Industries executes the refined plan on sustainable quota.
  - **Routine Exemption**: Skip the Pro Sanity Gate for routine bug fixes, test suite expansions, CMake tweaks, and mechanical chores.

## Strict Background Task Etiquette & Watchdog Timer Policy
- **No Polling or Pinging**: When a command goes to the background, NEVER use `manage_task` to poll its status in a loop or spam the chat.
- **No Task Log Peeking**: NEVER inspect running task logs via `cat`, `Get-Content`, `type`, `head`, `tail`, or `view_file` on `.system_generated/tasks/task-*.log` while actively running.
- **Two-Stage Watchdog Timer Protocol**:
  | Task Type | T = 0 (Launch & Yield) | T = Checkpoint 1 | T = Hard Timeout |
  | :--- | :--- | :--- | :--- |
  | **Quick Tasks / Single Targets** (`dsp_tests`, incremental builds, git) | Schedule 5m watchdog (`TimerCondition="<task-id>"`) | T=5m: check status once silently via `manage_task`; schedule 10m timeout | T=15m: terminate task, inspect logs, alert user |
  | **Full Rebuilds / Multi-Target Suites** (full `/build-validate`, `--clean-first`) | Schedule 10m watchdog (`TimerCondition="<task-id>"`) | T=10m: check status once silently; schedule 10m timeout | T=20m: terminate task, inspect logs, alert user |
  | **Remote GitHub CI Workflows** (Linux, macOS, Windows matrix, auval) | Schedule 10m watchdog | T=10m, 20m, 30m: query GitHub API once, output markdown status table | T=50m: auto-cancel remote run via API, fetch failure logs, alert user |

## Strict C++ Formatting, Style & Memory Safety
- **Indentation & Naming**: Exactly 4 spaces. No tabs. `camelCase` for variables and methods. `PascalCase` for classes and structs.
- **Braces**: Allman style (opening brace on new line) for functions and classes; K&R style (opening brace on same line) for control flow (`if`, `for`, `while`). Omit braces for single-line `if`.
- **Pointers & References**: Attach asterisk/ampersand to type (`float* myPtr`, `const juce::String& text`). Modern C++: generous `const`, obvious `auto`.
- **Memory Safety**: No raw ownership (`new`/`delete`). Dynamic object ownership must use `std::unique_ptr` and `std::make_unique`. Raw pointers (`T*`) and references (`T&`) are for non-owning observation only.
- **Thread Separation**: Never call UI methods (`repaint()`, `setValue()`) from `processBlock()` or `processStereo()`. Audio-to-UI communication must use lock-free atomics/FIFOs or `juce::AsyncUpdater`.

## Strict Git Commit & Branch Safety Gate
- **Conventional Commits**: Format strictly as `<type>(<optional scope>): <description>` (`feat:`, `fix:`, `refactor:`, `chore:`, `docs:`).
- **NEVER Work Directly on Master/Main**: Check active branch (`git branch --show-current`). If on `master` or `main`, immediately create and switch to a descriptive branch (`git checkout -b feat/<slug>`). Direct commits to `master`/`main` are allowed ONLY by explicit user order.
- **Ruthless Deletion (Git is the Archive)**: Ruthlessly delete obsolete source code and files. Never comment out dead code blocks or create `old_Component.cpp`/`legacy/` folders.
- **Silent Git Tags for Architectural Refactors**: When a milestone consists strictly of internal architectural cleanup, developer tooling, or data schema refactoring with zero user-facing sound/UI changes, create and push an annotated Git Tag (`git tag -a vX.Y.Z -m "..."`) and merge to `main`, but do NOT publish a public GitHub Release entry. This preserves clean SemVer and reproducible build history while preventing spurious 'Update Available' prompts in end-user DAWs.

## Strict Debugging Heuristics
- **Initialization Order First**: When encountering garbage data, parse failures, or unexplained crashes during asset loading, ALWAYS verify C++ object lifecycles and static initialization order *before* investigating encoding or unicode issues.
- **JSON Merge Priority (Overwrite Trap)**: Base/Master JSON files MUST be loaded *first*, followed by specific modules/effects. Loading out of order will silently overwrite module data.

## Interactive Decision Modals & Wrap-Up
- **Interactive Modal Required for Design Menus**: When presenting design options (at the end of `/grill-me`, `/plan`, or architectural branching forks), NEVER output raw numbered text lists forcing the user to type "3". ALWAYS invoke `ask_question`.
- **CRITICAL EXCEPTION**: Under NO circumstances use `ask_question` at the builder execution start gate (`/read-plan`, `/execute-task`). Modals freeze the IDE model dropdown. Pause in chat text.
- **Wrap-Up Housekeeping**: Document interview conclusions in Backlog or Plan.

## Strict Scratch Script Hygiene
- **Instant Cleanup**: When temporary scripts (`update_*.py`, `temp_*.py`) are created for refactors or tests, ALWAYS delete them immediately in the terminal command chain (e.g., `python script.py ; rm script.py`). Never leave temporary scripts in the tree.

## Strict Test-Driven Guardrail & Build Validation
- **Zero Orphaned Features**: No new DSP algorithm, parameter, UI card, page, modal, or editor tool may be merged without test coverage in `test/dsp_tests.cpp` and `test/gui_tests.cpp`.
- **Dynamic Reflection Compliance**: `gui_tests` dynamically sweeps 100% of registered APVTS parameters and JSON assets. Missing UI bindings or definitions will hard-fail the build.
- **Mandatory Planning Test Phase**: All plans (`/plan`) MUST include an explicit Test Suite Update phase.
- **Always Deploy**: After building successfully, run `deploy.ps1` to sync `.exe` and `.vst3` artifacts into `current_build/` and system directories.

## Strict Context Management & Soul Harvest Protocol
- **Proactive Context Refreshing**: When a conversation crosses ~40 turns or approaches high token density, suggest running `/refresh-context` (or clicking "Replace with New" in the sidebar).
- **Threshold-Aware Factory Clean Slate**:
  - Fresh builder sessions (< 12 turns): Instant engage on `proceed`.
  - Mature builder sessions (> 15 turns): Output subtle 1-line notice: `💡 Factory Context Notice: ~N turns accumulated. Reply 'proceed' to build, or 'harvest & proceed' for a clean slate.`
  - Power-User Flag (`harvest & proceed`): Harvests session into `docs/DEV_HISTORY.md`, updates `context_clues_build.md`, and prompts refresh.

## Strict Chat Role Enforcement (Planner vs. Builder)
- **Role Identification**: Determine role by checking the user's initial prompt or reading `context_clues_plan.md` vs `context_clues_build.md`.
- **New Klang City (Planner / Ivory Tower)**: Strictly Read-Only for C++ source files. May ONLY draft `PLAN.md`, update `docs/BACKLOG.md`, write documentation, and manage agent skills. Live factory radar (`git status --short`, `git diff --stat`) encouraged.
- **Klang Industries (Builder / Factory Floor)**: Strictly Read-Only for overarching architecture. Strictly forbidden from modifying `docs/BACKLOG.md` or drafting core `PLAN.md` features. Refuses architectural planning requests.
- **Strict Release Authority Gate (Ivory Tower Only)**:
  - Klang Industries is strictly forbidden from executing `/cut-release`, modifying release version strings, creating/moving `git tag`s, or pushing tags. Its job ends after tests pass, `deploy.ps1` runs, changes are committed, and the "JOB'S DONE" chime sounds in `build_to_plan.md`.
  - New Klang City is the sole release authority (reviews diffs, verifies desktop testing, updates changelogs, tags Git releases, and pushes to GitHub).
- **Strict Guardrail & Skills Governance Gate (Ivory Tower Only)**:
  - Klang Industries is strictly forbidden from modifying `GEMINI.md`, system rules, or authoring/editing agent Skills (`.agents/skills/`, `.gemini/config/skills/`). It must refuse rule changes and direct the user to New Klang City.
  - **The "Just Do It" Emergency Override**: Klang Industries may ONLY bypass this restriction if the user explicitly includes the exact phrase `"just do it"` in their prompt.
  - New Klang City is the sole authority for system architecture, `GEMINI.md` guardrails, agent workflows, skill definitions, and project backlogs.

## Mobile-Aware Autopilot Factory Pipeline
- When the user indicates mobile (*"I'm on mobile"*, *"run full task"*), Klang Industries executes all plan phases continuously end-to-end (`Code Edits ➔ audiothread-guard ➔ cmake build ➔ gui_tests ➔ deploy.ps1 ➔ archive PLAN.md ➔ update build_to_plan.md`) without halting for intermediate chat approvals.

## Strict Inter-Chat Communique Protocol & "Job's Done" Handshake
- **Mailbox Protocol (`docs/communique/`)**:
  - `plan_to_build.md`: Authored strictly by New Klang City. Declares state machine: `STATUS: DRAFTING` ➔ `STATUS: READY_FOR_EXECUTION` ➔ `STATUS: IN_PROGRESS` ➔ `STATUS: COMPLETED`.
  - `build_to_plan.md`: Authored strictly by Klang Industries. Contains live execution status, test results, and completion reports.
- **Zero-Polling Discipline**: No terminal status loops waiting for the other chat. Handoffs are event-driven.
- **The "Job's Done" Chime & Dual Mailbox Closure**: When Klang Industries finishes compilation, test passes, and artifact deployment, it MUST update BOTH `build_to_plan.md` (to `Status: COMPLETE ✅`) and `plan_to_build.md` (to `Status: COMPLETED ✅`), archive `PLAN.md` to `docs/completed_plans/`, and conclude its turn with this prominent handoff chime:
  > 🔔 **JOB'S DONE!** `<Task Name>` is fully built, tested, and deployed. Switch to New Klang City to review and advance!
- **Single-Read Ingest**: Returning to New Klang City triggers **exactly ONE read** of `build_to_plan.md` to confirm completion.
- **Auto-Archive & Reset on Task Finish**: Klang Industries must immediately archive `PLAN.md` to `docs/completed_plans/<YYYY-MM-DD>_<task_slug>.md` and reset `PLAN.md` to an idle state (`# No Active Plan`).

## Pre-Release "Clean Slate & Regression Gauntlet" Guardrail
- Mandatory pre-flight checklist before any version bump, git release tagging, or `/cut-release`:
  1. **Cruft & Scratch Sweep**: Zero untracked scratch scripts (`*.tmp`, `temp_*.txt`, `update_*.py`) or loose artifacts (`git status --porcelain`).
  2. **Dual-Configuration Parity**: Both `Debug` and `Release` compile cleanly with zero errors.
  3. **Universal 100% Test Pass**: `dsp_tests.exe` (100% audio invariants, SIMD, FastMath) and `gui_tests.exe` (100% parameter reflection, component bindings, modal lifecycles, offscreen paint smoke passes).
  4. **Strict Schema Parity**: 100% of APVTS parameters registered in both plugins have matching `ControlDef` entries in `assets/controls/*.json`.
  5. **JUCE 9.0.3 Timer Hygiene**: 100% of `juce::Timer` subclasses call `stopTimer()` as first line of destructor.

## Strict Clipboard & External Link Ingestion Guardrail
- **Zero Arbitrary Clipboard Sniffing**: Agents must NEVER inspect or read the host system clipboard (`Get-Clipboard`). Blueprints, tasks, and code must originate exclusively from local workspace files or direct chat input.
- **Never Fetch External URLs**: Agents must NEVER automatically fetch or scrape external web URLs via `read_url_content` or `curl` during planning, building, or task ingestion without explicit user confirmation.
- **Local-Only Plan Priority**: All plans, tasks, and communique documents MUST come exclusively from local workspace files (e.g. `PLAN.md`, `docs/communique/plan_to_build.md`) or direct text provided by user.
- **Privacy & Context Boundary**: The host system clipboard may contain private, out-of-band user data from other applications (notes, meeting links, passwords, tokens). Respect context boundaries at all times.
