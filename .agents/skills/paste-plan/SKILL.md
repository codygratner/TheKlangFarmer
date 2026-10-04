---
name: paste-plan
description: Ingests an implementation plan pasted by the user, writes it to PLAN.md, parses the phases, and either coordinates automated build execution, prompts for interactive action, or defers the plan directly to the project backlog. Supports `--build` (-b) to run autonomously and `--backlog` (-l) to defer to the backlog. Triggers on `/pasteplan`.
---

# Paste Plan Ingestor & Pipeline Orchestrator

## Goal
Capture an externally authored plan, persist it verbatim to `PLAN.md` at project root, guard against unintended execution on `master`, bump the patch version with a prerelease feature tag for DAW cache busting, and execute the verification loop across every phase, terminating with `/task-finish`.

## Workflow

### 1. Flag Detection & Command Parsing
Inspect the invocation:
- Flag `--build` or `-b`: Autonomous Build Mode (`AUTO_BUILD = true`).
- Flag `--backlog` or `-l`: Backlog Deferment Mode (`DEFER_BACKLOG = true`).
- Strip `--build`, `-b`, `--backlog`, and `-l` from the plan text.
- Save the raw plan text directly to `PLAN.md` at project root.

### 2. Parse Plan Structure
Extract:
1. **Feature Slug:** Derive a short url-safe slug from the plan title (e.g., `tooltips`, `mix-knob`, `sf2-export`).
2. **Core Objective & Phases:** List of numbered phases and acceptance criteria.
3. **Phase 1 Action Items:** Target files and verification targets.

### 2.5 Backlog Deferment Flow (If DEFER_BACKLOG == true)
If the user supplied `--backlog` (`-l`) or selected Backlog Deferment in the decision gate:
1. **Zero Premature Implementation:** Do NOT create a feature branch, do NOT bump versions in `CMakeLists.txt`, and do NOT modify source code or run builds.
2. **Archive Plan:** Save the full plan text to `docs/<slug>_plan.md` so the complete architecture, structs, and steps are permanently preserved.
3. **Update Backlog:** Add the feature to `docs/BACKLOG.md` and `future_backlog_and_reminders.md` under **Top Priorities for Upcoming Sessions** (at the priority requested or as the next active top priority).
4. **Log Confirmation:**
   ```markdown
   # 📋 Plan Saved & Deferred to Backlog
   - **Plan Preserved:** `docs/<slug>_plan.md`
   - **Backlog Priority:** Added to `docs/BACKLOG.md` as Priority #<N>
   - **Status:** Recorded only (no code modified, no builds executed).
   ```
5. **HALT Execution.** Stop calling tools and wait for explicit user direction.

### 3. Git Branch Safety Gate & Feature Version Bump
Check current active branch (`git branch --show-current`):
- If the current branch is `master` or `main`:
  - Determine a clean branch name: `feature/<slug>`.
  - **HALT before touching any source code**, even if `--build` was passed.
  - Prompt the user:
    > ⚠️ **BRANCH GUARDRAIL ALERT** ⚠️  
    > You are currently on the **`master`** branch.
    >
    > How would you like to proceed?  
    > 1. **Make a new branch** (Recommended: `feature/<slug>`)  
    > 2. **No, do this in master, I'm feeling fucking feisty**
  - If user selects 1: execute `git checkout -b feature/<slug>` and continue to Version Bump.
  - If user selects 2: log confirmation and continue on `master`.

#### Automated Feature Version Bump:
1. Derive `<slug>` from the task or feature title (e.g., `tooltips`, `mix-knob`).
2. Inspect `CMakeLists.txt` for `project(TheKlangFarmer VERSION X.Y.Z LANGUAGES C CXX)`.
3. Increment patch version: `Z -> Z+1` (e.g. `0.1.8` -> `0.1.9`).
4. Set feature tag in `CMakeLists.txt`:
   `set(TKF_FEATURE_TAG "-<slug>" CACHE STRING "Prerelease feature tag for development builds")`
5. Print notice:
   `📦 [Version Bump] Set development build to vX.Y.(Z+1)-<slug> (forces DAW rescan and UI header badge update).`

### 4. Briefing & Decision Gate
Print summary:
```markdown
# 📋 Plan Ingested & Saved to `PLAN.md`
**Branch:** <Current Active Branch>
**Version:** vX.Y.(Z+1)-<slug>
**Objective:** <Objective>
**Phases:** <N> Total Phases

### 🚀 Immediate Focus: Phase 1 — <Title>
**Target Files:** <Files>
**Verification:** <Criteria>
```

- If `AUTO_BUILD == true`: Print `[--build detected] Launching automated pipeline...` and begin Phase 1.
- If `DEFER_BACKLOG == true`: Execute **Section 2.5 (Backlog Deferment Flow)**.
- If neither flag was supplied: Present the user with an interactive decision:
  > **How would you like to proceed with this plan?**
  > 1. `(Recommended) Start Phase 1 pipeline`: Check branch safety, bump version with feature tag, and begin automated engineering loop.
  > 2. `Defer to Backlog`: Save plan to `docs/<slug>_plan.md`, add as an active priority in `docs/BACKLOG.md`, and record without writing code or building.
  > 3. `Review Only`: Keep `PLAN.md` at project root and wait for manual instructions.
  >
  > Proceed according to user selection.

### 5. Automated Execution Pipeline Loop (Per Phase)

For each phase in `PLAN.md`:

1. **Step 1: Code Edits:** Apply the changes specified for the active phase.
2. **Step 2: Audio-Thread Audit:** Run `/audiothread-guard` on changed DSP `.h`/`.cpp` files. Fix dynamic allocations (`new`, `malloc`, `std::vector`), locks (`std::mutex`), or system calls/logging.
3. **Step 3: Build & Sanity Check:** Run `/build-validate` (CMake build and tests/pluginval). Autonomously diagnose and fix compiler/test errors.
4. **Step 4: Check-off & Stage:** Run `/step-verify` to update `PLAN.md` on disk (mark phase `[x]`) and stage clean files in git.
5. **Step 5: Progression Check:**
   - If uncompleted phases remain (`- [ ]`): Proceed to Phase N+1.
   - If ALL phases are marked complete (`- [x]`): **Immediately and automatically invoke `/task-finish`**.
