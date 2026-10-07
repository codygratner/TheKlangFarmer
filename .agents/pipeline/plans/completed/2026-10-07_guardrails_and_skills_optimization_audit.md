# Implementation Plan: Milestone v0.3.2 Guardrails & Skills Optimization Overhaul

## Goal
Overhaul agent infrastructure per the `/grill-me` alignment: retire obsolete skills (`strict-plan` and `paste-plan`), modernize `read-plan`, `execute-task`, and `task-finish`, synchronize workspace (`.agents/skills/`) and global (`C:\Users\codyg\.gemini\config\skills\`) skill directories in 100% parity, close the communique state machine loop, and polish `GEMINI.md` guardrails.

---

## Technical Details & Phases

### Phase 1: Retire Obsolete Skills (`strict-plan` & `paste-plan`)
- **Target Directories**:
  - `.agents/skills/strict-plan/`
  - `.agents/skills/paste-plan/`
  - `C:\Users\codyg\.gemini\config\skills\paste-plan/`
  - `C:\Users\codyg\.gemini\config\skills\strict-plan/` (if present)
- **Action Items**:
  - [x] Delete `strict-plan` from both workspace and global skill stores.
  - [x] Delete `paste-plan` from both workspace and global skill stores.
  - [x] Verify zero stray references or broken dependencies.

### Phase 2: Modernize Core Skills (`read-plan`, `execute-task`, `task-finish`) & Sync Directories
- **Target Directories**:
  - `.agents/skills/read-plan/SKILL.md` & `C:\Users\codyg\.gemini\config\skills\read-plan/SKILL.md`
  - `.agents/skills/execute-task/SKILL.md` & `C:\Users\codyg\.gemini\config\skills\execute-task/SKILL.md`
  - `.agents/skills/task-finish/SKILL.md` & `C:\Users\codyg\.gemini\config\skills\task-finish/SKILL.md`
- **Action Items**:
  - [x] Modernize `read-plan` as the official Communiqué Dispatch Ingestor: reads `docs/communique/plan_to_build.md` (falling back to `PLAN.md`), verifies `READY_FOR_EXECUTION`, sets `IN_PROGRESS`, displays model advisory badge, and readies execution.
  - [x] Modernize `execute-task`: purge all Linear.app references and legacy ticket IDs (`THE-*`); source tasks strictly from `PLAN.md` or `docs/BACKLOG.md`.
  - [x] Modernize `task-finish`: purge all Linear comments/sync, automate archiving to `docs/completed_plans/` without blocking question modals, and update both `build_to_plan.md` and `plan_to_build.md` to `STATUS: COMPLETED`.
  - [x] Synchronize `.agents/skills/` and `C:\Users\codyg\.gemini\config\skills\` so both directories have identical, up-to-date versions of shared skills (`read-plan`, `execute-task`, `task-finish`, `context-extract`, `clean-plan`, `compat-check`, `audiothread-guard`, `build-validate`, `step-verify`, `cut-release`, `refresh-context`, `clear`).

### Phase 3: Inter-Chat Communiqué Tuning & State Machine Closure
- **Target Files**:
  - `docs/communique/plan_to_build.md`
  - `GEMINI.md`
- **Action Items**:
  - [x] Update `docs/communique/plan_to_build.md` from stale `READY_FOR_EXECUTION` to `STATUS: COMPLETED (Task: Top-Level Callout Parameter Controls)`.
  - [x] Formally codify the 4-stage lifecycle in `GEMINI.md`: `DRAFTING` -> `READY_FOR_EXECUTION` -> `IN_PROGRESS` -> `COMPLETED`.
  - [x] Require that Klang Industries marks `plan_to_build.md` as `STATUS: COMPLETED` alongside `build_to_plan.md` to eliminate stale pickup traps.

### Phase 4: Guardrail Polish & Inconsistency Resolution in `GEMINI.md`
- **Target Files**:
  - `GEMINI.md`
  - `C:\Users\codyg\.gemini\config\skills\build-validate\SKILL.md` (and workspace copy)
  - `C:\Users\codyg\.gemini\config\skills\compat-check\SKILL.md` (and workspace copy)
- **Action Items**:
  - [x] Fix truncated typos in `GEMINI.md` (`uild-validate` -> `build-validate`).
  - [x] Remove all references to retired `/pasteplan`, `/paste-plan`, and `/strict-plan` throughout `GEMINI.md`, updating rules to cite `/read-plan` and `/execute-task`.
  - [x] Align Section 7 in `build-validate/SKILL.md` with `GEMINI.md`'s **2-Strike Factory Floor Escalation** rule (replacing 3-attempt revert).
  - [x] Fix legacy path references in `compat-check/SKILL.md` (`TheKlangFarmer` -> `TheKlangSuite`).
  - [x] Refine the Clipboard Guardrail to strictly state: zero clipboard sniffing (`Get-Clipboard`), zero URL scraping; plans are sourced solely from local workspace files or direct chat text.

### Phase 5: Full Validation & Git Milestone Closeout
- **Target Files**:
  - Test suites & Git repository
- **Action Items**:
  - [x] Build Release test targets (`TheKlangEditor`, `gui_tests`, `dsp_tests`).
  - [x] Run `dsp_tests.exe` and `gui_tests.exe` verifying 100% assertions pass and exit code 0.
  - [x] Update `docs/BACKLOG.md` marking Item 1 of Milestone v0.3.2 as COMPLETED.
  - [x] Commit all changes to `0.3.2-dev` under `chore(audit): overhaul planning skills, synchronize skill directories, and streamline guardrails`.
