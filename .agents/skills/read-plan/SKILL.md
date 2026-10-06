---
name: read-plan
description: Scans the workspace for existing architectural plans (PLAN.md, backlog, or completed plans) and presents an interactive picker to load them for review or execution. Triggers on `/readplan` or `/read-plan`.
---

# Internal Plan Ingestor (`read-plan`)

## Goal
To locate and ingest internally stored plans (`PLAN.md`, `docs/*_plan.md`, `docs/completed_plans/*`, or tickets from `docs/BACKLOG.md`) and load them into the active workspace, while strictly honoring the "No Spontaneous Implementation" global guardrail.

## Workflow

### 1. File Discovery & Interactive Picker (If No Target Provided)
If the user triggers `/readplan` without specifying a specific file name or feature:
1. **Scan the Workspace:**
   - Look for `PLAN.md` at the project root.
   - Search the `docs/` directory for any active feature plans (`*_plan.md`).
   - Search `docs/completed_plans/` for archived plans (if they wish to resume/review).
   - Parse `docs/BACKLOG.md` for the current Top Priority tickets.
2. **Present the Picker via `ask_question`:**
   Invoke the `ask_question` tool with the list of discovered plans and backlog tickets:
   - **Question:** *"Which plan would you like to load into the workspace?"*
   - **Options:** Formatted as clickable choices (e.g., `(Recommended) [Active] PLAN.md - <Title>`, `[Backlog] docs/preset_system_plan.md - <Title>`, etc.)
3. **Execution Proceeds Directly:** The selection is handled immediately from the user's click.

### 2. File Ingest (If Target Provided or Selected)
If the user specifies a file (e.g., `/readplan preset_system_plan`) or selects one from the picker:
1. Locate the file on disk.
2. Read the contents.
3. If the selected file is NOT already `PLAN.md` at the project root, copy the contents and overwrite `PLAN.md` at the project root.

### 3. Briefing & Mandatory Decision Gate
Parse the loaded `PLAN.md` for its Objective and Phase count.
Print the summary:
```markdown
# 📖 Plan Loaded
**Objective:** <Objective>
**Phases:** <N> Total Phases
```

**CRITICAL GUARDRAIL:** You must NEVER automatically start implementation. You MUST invoke the `ask_question` tool to present the user with an interactive clickable modal:
- **Question:** *"How would you like to proceed with this plan?"*
- **Options:**
  - `(Recommended) Start Phase 1 pipeline: Check branch safety, bump version with feature tag, and begin automated engineering loop`
  - `Defer to Backlog: Move plan to docs/<slug>_plan.md and record as a priority in docs/BACKLOG.md`
  - `Review Only: Keep PLAN.md active at project root and wait for manual instructions`

### 4. Handoff
- If the user selects **1**: Coordinate the Git Branch Safety Gate, the Feature Version Bump, and begin the Automated Execution Pipeline (delegating to the logic in `paste-plan` / `execute-task`).
- If the user selects **2**: Execute the Backlog Deferment Flow.
- If the user selects **3**: Halt execution.
