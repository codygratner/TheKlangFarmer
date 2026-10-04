---
name: paste-plan
description: Ingests an implementation plan pasted by the user, writes it to PLAN.md, parses the phases, verifies Git branch safety against master/main, bumps the patch version with a prerelease -[feature] tag, and coordinates execution through an automated audit, build, and task-finishing pipeline. Supports `--build` (-b) to run autonomously. Triggers on `/pasteplan`.
---

# Paste Plan Ingestor & Pipeline Orchestrator

## Goal
Capture an externally authored plan, persist it verbatim to `PLAN.md` at project root, guard against unintended execution on `master`, bump the patch version with a prerelease feature tag for DAW cache busting, and execute the verification loop across every phase, terminating with `/task-finish`.

## Workflow

### 1. Flag Detection & Ingest
- Check for `--build` or `-b`. Set `AUTO_BUILD = true` if present, else `false`.
- Strip `--build` and `-b` from the prompt text.
- Save the raw plan text directly to `PLAN.md` at project root.

### 2. Parse Plan Structure
Extract:
1. **Linear Issue ID:** Search for `[A-Z]+-[0-9]+` in the title or headers (e.g., `THE-9`).
2. **Core Objective & Phases:** List of numbered phases and acceptance criteria.
3. **Phase 1 Action Items:** Target files and verification targets.

### 3. Git Branch Safety Gate & Feature Version Bump
Check current active branch (`git branch --show-current`):
- If the current branch is `master` or `main`:
  - Determine a clean branch name:
    - If a Linear Issue ID was found: `feature/<ISSUE-ID>-<slug>` (e.g., `feature/THE-9-tooltips`).
    - If no issue ID: `feature/<task-slug>`.
  - **HALT before touching any source code**, even if `--build` was passed.
  - Prompt the user:
    > ⚠️ **BRANCH GUARDRAIL ALERT** ⚠️  
    > You are currently on the **`master`** branch.
    >
    > How would you like to proceed?  
    > 1. **Make a new branch** (Recommended: `<suggested-branch-name>`)  
    > 2. **No, do this in master, I'm feeling fucking feisty**
  - If user selects 1: execute `git checkout -b <suggested-branch-name>` and continue to Version Bump.
  - If user selects 2: log confirmation and continue on `master`.

#### Automated Feature Version Bump:
1. Derive `<slug>` from the task or issue key (e.g., `tooltips`, `mix-knob`).
2. Inspect `CMakeLists.txt` for `project(TheKlangFarmer VERSION X.Y.Z LANGUAGES C CXX)`.
3. Increment patch version: `Z -> Z+1` (e.g. `0.1.8` -> `0.1.9`).
4. Set feature tag in `CMakeLists.txt`:
   `set(TKF_FEATURE_TAG "-<slug>" CACHE STRING "Prerelease feature tag for development builds")`
5. Print notice:
   `📦 [Version Bump] Set development build to vX.Y.(Z+1)-<slug> (forces DAW rescan and UI header badge update).`

### 4. Briefing & Execution Trigger
Print summary:
```markdown
# 📋 Plan Ingested & Saved to `PLAN.md`
**Branch:** <Current Active Branch>
**Version:** vX.Y.(Z+1)-<slug>
**Linear Issue:** <Extracted ID or "None">
**Objective:** <Objective>
**Phases:** <N> Total Phases

### 🚀 Immediate Focus: Phase 1 — <Title>
**Target Files:** <Files>
**Verification:** <Criteria>
```

- If `AUTO_BUILD == true`: Print `[--build detected] Launching automated pipeline...` and begin Phase 1.
- If `AUTO_BUILD == false`: Prompt:
  > `PLAN.md` is locked and ready on `<branch>`.
  > **Would you like to start the automated build pipeline for Phase 1 now?**
  > 1. Yes, start Phase 1 pipeline
  > 2. No, wait for manual instructions
  > Proceed immediately if user selects 1 or confirms.

### 5. Automated Execution Pipeline Loop (Per Phase)

For each phase in `PLAN.md`:

1. **Step 1: Code Edits:** Apply the changes specified for the active phase.
2. **Step 2: Audio-Thread Audit:** Run `/audiothread-guard` on changed DSP `.h`/`.cpp` files. Fix dynamic allocations (`new`, `malloc`, `std::vector`), locks (`std::mutex`), or system calls/logging.
3. **Step 3: Build & Sanity Check:** Run `/build-validate` (CMake build and tests/pluginval). Autonomously diagnose and fix compiler/test errors.
4. **Step 4: Check-off & Stage:** Run `/step-verify` to update `PLAN.md` on disk (mark phase `[x]`) and stage clean files in git.
5. **Step 5: Progression Check:**
   - If uncompleted phases remain (`- [ ]`): Proceed to Phase N+1.
   - If ALL phases are marked complete (`- [x]`): **Immediately and automatically invoke `/task-finish`**.
