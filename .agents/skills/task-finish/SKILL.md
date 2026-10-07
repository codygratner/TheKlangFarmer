---
name: task-finish
description: Closes out a completed feature from PLAN.md, validates full check-off, commits changes using conventional commits, auto-archives PLAN.md, and performs the dual-communique completion handshake. Triggers on `/taskfinish`, `/task-finish`, "finish task", "complete task", or "task done".
---

# Factory Task Closer & Communiqué Handshake (`task-finish`)

## Goal
Close out a completed engineering milestone with absolute discipline: verify 100% check-off in `PLAN.md`, stage clean changes, generate a structured conventional commit, autonomously archive `PLAN.md` to `docs/completed_plans/`, update both communiqué mailboxes to `COMPLETED`, and sound the Job's Done chime—with zero blocking modals or external tracker bloat.

---

## Operational Constraints
- **100% Check-off Required:** Never finish a task if unverified or unchecked (`- [ ]`) items remain in `PLAN.md`.
- **Zero Untracked Cruft:** Sweep and clean any temporary scratch scripts (`*.tmp`, `update_*.py`, `context_snapshot*.md`).
- **Autonomous Archival (No Modal):** Archive and reset `PLAN.md` automatically; never pop up an interactive modal asking whether to archive.
- **Dual Communiqué Synchronization:** Must update BOTH `build_to_plan.md` and `plan_to_build.md` to prevent stale dispatch pickups.

---

## Workflow

### 1. Pre-Flight Verification Audit
1. Open and parse [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) at project root:
   - Verify every phase checklist item is marked `[x]`.
   - If any item is unchecked, halt execution and report the unverified deliverable.
2. Inspect git status:
   ```powershell
   git status --short
   git diff --stat
   ```

---

### 2. Conventional Git Commit
1. Stage all verified changes:
   ```powershell
   git add <modified_files>
   ```
2. Commit adhering strictly to Conventional Commits:
   ```text
   <type>(<scope>): <concise description>

   - Completed all phases in PLAN.md
   - Real-time audio safety verified via audiothread-guard
   - 100% unit tests passing in gui_tests and dsp_tests
   ```
   *Allowed types:* `feat`, `fix`, `refactor`, `perf`, `test`, `chore`, `docs`.

---

### 3. Autonomous Archival & Clean Reset
1. Extract the plan title or slug and current date (`YYYY-MM-DD`).
2. Move [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) to:
   `docs/completed_plans/<YYYY-MM-DD>_<task_slug>.md`
3. Reset [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) at root:
   ```markdown
   # Implementation Plan

   *No active plan. Ready for next assignment from New Klang City.*
   ```
4. Clean temporary snapshot artifacts if present:
   ```powershell
   Remove-Item -Path "context_snapshot*.md", "PLAN_BACKUP*.md" -Force -ErrorAction SilentlyContinue
   ```

---

### 4. Dual Communiqué Mailbox Synchronization
If `docs/communique/` exists in the repository:
1. Update [`docs/communique/build_to_plan.md`](file:///c:/Dev/TheKlangSuite/docs/communique/build_to_plan.md):
   - Set `Status: COMPLETE ✅`
   - Include test metrics (GUI tests passed, DSP tests passed).
   - Link to the archived plan in `docs/completed_plans/`.
2. Update [`docs/communique/plan_to_build.md`](file:///c:/Dev/TheKlangSuite/docs/communique/plan_to_build.md):
   - Set `Status: COMPLETED` (with completion timestamp and link to archived plan).

---

### 5. The Job's Done Chime
Conclude your turn with the prominent handoff chime:

```markdown
# ✅ Task Completed, Verified & Archived

- **Git Commit:** Committed to branch `<branch>`.
- **Archived Plan:** [`docs/completed_plans/<archive_name>.md`](file:///docs/completed_plans/<archive_name>.md)
- **Communiqué Status:** Both `build_to_plan.md` and `plan_to_build.md` updated to `COMPLETED`.

> 🔔 **JOB'S DONE!** `<Task Name>` is fully built, tested, and deployed. Switch to New Klang City to review and advance!
```
