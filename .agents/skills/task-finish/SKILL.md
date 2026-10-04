---
name: task-finish
description: Closes out a completed feature from PLAN.md, validates full check-off, commits changes to git, and autonomously updates the corresponding Linear.app issue to Done with a completion comment. Triggers on `/taskfinish` or automatically via paste-plan.
---

# Task Finish & Linear Closer

## Goal
Automate final git commits and Linear issue closure without user friction once all phases in `PLAN.md` pass.

## Workflow

### 1. Verification Audit
Read `PLAN.md`. Verify that all phases are marked completed `[x]`. Halt if any tasks remain uncompleted.

### 2. Extract Issue Identifier
Scan `PLAN.md` for a Linear issue key (`[A-Z]+-[0-9]+`, e.g., `THE-7`). If missing, check the current git branch name.

### 3. Git Commit
- Run `git status` and `git diff --stat`.
- Stage all verified changes.
- Commit using conventional commits:
  ```text
  feat(<scope>): <summary of work> (<ISSUE-ID>)

  - Completed all phases in PLAN.md
  - Verified audio-thread safety via audiothread-guard
  - Clean CMake build and test pass
  ```

### 4. Linear Status & Comment Update (MCP)

If a Linear issue ID was found:

1. Call `linear_update_issue` to set status to **"Done"**.
2. Call `linear_add_comment` with:
```markdown
### Automated Completion Summary

**Verified by Antigravity Pipeline**
- **Phases Completed:** All milestones in `PLAN.md` executed.
- **DSP Safety:** Verified zero heap allocations, locks, or blocking I/O in the audio path.
- **Build & Test:** Clean CMake build and test validation pass.
- **Git Commit:** `<commit hash>` - `<commit title>`
```

### 5. Cleanup & Output

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
- **Linear:** Issue `<ISSUE-ID>` marked as **Done** with completion comment.
- **Plan:** `PLAN.md` verified and completed.
```
