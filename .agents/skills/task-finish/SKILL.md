---
name: task-finish
description: Closes out a completed feature from PLAN.md, validates full check-off, commits changes to git, Triggers on `/taskfinish` or automatically via paste-plan.
---

# Task Finish & Linear Closer

## Goal
Automate final git commits and Linear issue closure without user friction once all phases in `PLAN.md` pass.

## Workflow

### 1. Verification Audit
Read `PLAN.md`. Verify that all phases are marked completed `[x]`. Halt if any tasks remain uncompleted.

### 2.5 Strip Feature Tag & Finalize Clean Release
Before committing verified changes:
1. In `CMakeLists.txt`, clear `TKF_FEATURE_TAG`:
   ```cmake
   set(TKF_FEATURE_TAG "" CACHE STRING "Prerelease feature tag for development builds")
   ```
2. Run `/build-validate` or compile locally to deploy the clean release VST3 binary (`vX.Y.Z`).
3. Log: `[Version Finalize] Stripped feature tag -> Clean release vX.Y.Z ready for master`.

### 3. Git Commit
- Run `git status` and `git diff --stat`.
- Stage all verified changes.
- Commit using conventional commits:
  ```text
  feat(<scope>): <summary of work>

  - Completed all phases in PLAN.md
  - Verified audio-thread safety via audiothread-guard
  - Clean CMake build and test pass
  ```

### 4. Cleanup & Output

- Delete or clean `context_snapshot.md`.
- Conclude by prompting the user via `ask_question` whether to run `clean-plan`:
  - **Question:** *"Task completed and verified! Would you like to run `clean-plan` to tidy up planning artifacts?"*
  - **Options:**
    - `(Recommended) Yes, clean up planning files (archive PLAN.md to docs/completed_plans/ and clean snapshots)`
    - `No, keep PLAN.md in the root directory`
  If selected, invoke the `clean-plan` skill.
- Print confirmation:
```markdown
# ✅ Task Completed & Closed

- **Git:** Changes committed to `<branch>`.
- **Plan:** `PLAN.md` verified and completed.
```
