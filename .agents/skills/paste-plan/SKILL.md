---
name: paste-plan
description: Ingests an implementation plan pasted by the user. If empty, falls back to PLAN.md. Parses the phases and always prompts for user confirmation before execution. Supports `--backlog` (-l) to bypass implementation entirely and defer to the backlog. Triggers on `/pasteplan` or `/paste-plan`.
---

# Paste Plan Ingestor & Pipeline Orchestrator

## Goal
Capture an externally authored plan, persist it verbatim to `PLAN.md` at project root, prompt the user for confirmation (honoring the global "No Spontaneous Implementation" guardrail), and either coordinate execution or defer the plan to the backlog.

## Workflow

### 1. Flag Detection & Command Parsing
Inspect the invocation:
- Flag `--backlog` or `-l`: Backlog Deferment Mode (`DEFER_BACKLOG = true`).
- Strip `--backlog` and `-l` from the input arguments.

#### Input Ingest & Existing `PLAN.md` Fallback:
1. **If New Plan Text is Provided**:
   - Write the provided plan text directly to `PLAN.md` at project root.
2. **If Input is Empty**:
   - Inspect `PLAN.md` at project root.
   - If `PLAN.md` exists and contains non-empty content:
     - Read the plan directly from `PLAN.md` on disk.
     - **CRITICAL GUARDRAIL:** You must ask the user: *"Is this the plan you intend to work on?"* and await confirmation before proceeding.
   - If `PLAN.md` does not exist or is empty:
     - Prompt: `No plan text was provided and PLAN.md is empty. Please paste a plan or run /strict-plan <feature> to architect one.`
     - Halt execution.

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
*(Only executed if the user explicitly approves starting Phase 1 from the Decision Gate)*
Check current active branch (`git branch --show-current`):
- If the current branch is `master` or `main`:
  - Determine a clean branch name: `feature/<slug>`.
  - **HALT before touching any source code**.
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
1. Derive `<slug>` from the task or feature title.
2. Inspect `CMakeLists.txt` for `project(TheKlangFarmer ...)` and increment patch version: `Z -> Z+1`.
3. Set feature tag: `set(TKF_FEATURE_TAG "-<slug>" CACHE STRING "Prerelease feature tag for development builds")`.

### 4. Briefing & Mandatory Decision Gate
Print summary:
```markdown
# 📋 Plan Ingested & Saved to `PLAN.md`
**Objective:** <Objective>
**Phases:** <N> Total Phases

### 🚀 Immediate Focus: Phase 1 — <Title>
**Target Files:** <Files>
**Verification:** <Criteria>
```

- If `DEFER_BACKLOG == true`: Execute **Section 2.5 (Backlog Deferment Flow)**.
- Otherwise, strictly enforce the Global Planning Guardrail. You MUST present the user with this interactive decision:
  > **How would you like to proceed with this plan?**
  > 1. `Start Phase 1 pipeline`: Check branch safety, bump version with feature tag, and begin automated engineering loop.
  > 2. `Defer to Backlog`: Save plan to `docs/<slug>_plan.md`, add as an active priority in `docs/BACKLOG.md`, and record without writing code or building.
  > 3. `Review Only`: Keep `PLAN.md` at project root and wait for manual instructions.
  >
  > Proceed according to user selection.

### 5. Automated Execution Pipeline Loop (Per Phase)
*(Only if User Selected Option 1)*
1. **Step 1: Code Edits:** Apply the changes specified for the active phase.
2. **Step 2: Audio-Thread Audit:** Run `/audiothread-guard`.
3. **Step 3: Build & Sanity Check:** Run `/build-validate` (CMake build and tests/pluginval).
4. **Step 4: Check-off & Stage:** Run `/step-verify` to update `PLAN.md` on disk (mark phase `[x]`) and stage clean files in git.
5. **Step 5: Progression Check:** If uncompleted phases remain, proceed to Phase N+1. If ALL phases are marked complete, immediately invoke `/task-finish`.
