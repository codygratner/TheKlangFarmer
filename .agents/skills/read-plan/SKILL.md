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
   <!-- [Strategy Experiment: Multi-Chat Communiqué Awareness] -->
   - Check `docs/communique/plan_to_build.md`: If it exists and status is `READY_FOR_EXECUTION`, prioritize it as the active handoff dispatch!
   - Look for `PLAN.md` at the project root (if non-empty).
   - Search the `docs/` directory for any active feature plans (`*_plan.md`).
   - Search `docs/completed_plans/` for archived plans (if they wish to resume/review).
   - Parse `docs/BACKLOG.md` for the current Top Priority tickets.
2. **Present the Picker via `ask_question`:**
   Invoke the `ask_question` tool with the list of discovered plans and backlog tickets:
   - **Question:** *"Which plan would you like to load into the workspace?"*
   - **Options:** Formatted as clickable choices (e.g., `(Recommended) [Communiqué Dispatch] plan_to_build.md - <Task>`, `[Active] PLAN.md - <Title>`, `[Backlog] docs/preset_system_plan.md - <Title>`, etc.)
3. **Execution Proceeds Directly:** The selection is handled immediately from the user's click.

### 2. File Ingest (If Target Provided or Selected)
If the user specifies a file (e.g., `/readplan preset_system_plan`) or selects one from the picker:
1. Locate the file on disk.
2. Read the contents.
3. If the selected file is NOT already `PLAN.md` at the project root, copy the contents and overwrite `PLAN.md` at the project root.

### 3. Briefing & Mandatory Decision Gate
Parse the loaded `PLAN.md` for its Objective, Phase count, and **Recommended Model & Thinking Budget**.
Print the summary and Model Advisory:
```markdown
# 📖 Plan Loaded
**Objective:** <Objective>
**Phases:** <N> Total Phases

> 🧠 **MODEL ADVISORY: Tier <1 | 2 | 3>**
> - **Recommended Setting:** <Recommended Model & Thinking Budget from PLAN.md>
> - **Quota Impact:** <⚠️ HIGH IMPACT | 🟢 SUSTAINABLE | ⚡ MINIMAL>
> - **Active Model Check:** Please verify your model dropdown in the IDE footer matches this tier before proceeding!
```

**CRITICAL GUARDRAIL (NO MODAL ON PLAN EXECUTION START):** You must NEVER automatically start implementation, and you MUST NEVER pop up an `ask_question` modal here (interactive modals freeze the IDE interface and completely prevent the user from changing their model dropdown in the IDE footer).
Instead, conclude your response with the Model Advisory banner at the very bottom, and pause in regular chat text:
> Please verify or adjust your model dropdown in the IDE footer to match the advisory above, then reply **`proceed`** (or type `/pasteplan --backlog` to defer) to begin Phase 1.

### 4. Handoff
- When the user replies **`proceed`** (or confirms execution): Coordinate the Git Branch Safety Gate, the Feature Version Bump, and begin the Automated Execution Pipeline (delegating to the logic in `paste-plan` / `execute-task`).
- If the user replies to defer to backlog (e.g. `defer`, `backlog`, `--backlog`): Execute the Backlog Deferment Flow.
- If the user provides other instructions or requests changes: Comply without starting code execution until explicitly instructed.

