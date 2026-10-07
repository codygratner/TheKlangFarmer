---
name: refresh-context
description: Harvests the active chat into DEV_HISTORY, and optionally creates a context clues artifact and triggers /clear. Triggers on `/refresh-context`.
---

# Context Refresh Workflow

## 1. Interactive Decision Modal
When this skill is triggered, you MUST immediately invoke the `ask_question` tool to present the user with an interactive modal containing these options:
**Question:** "How would you like to handle this context refresh?"
**Options:**
- `(Recommended) Harvest, Clear, and Resume`: Harvest the chat to DEV_HISTORY, write `context_clues.md`, and prompt for `/clear`.
- `Harvest Only`: Just append the chat summary to `DEV_HISTORY.md` and stop.

## 2. Execute Selected Workflow

### If "Harvest Only" is selected:
Read the current state of the active chat. Identify the major tasks accomplished, any unresolved issues, and the current goal.
Append a brief summary of this session's work into `docs/DEV_HISTORY.md` using the strict template below. 
**Do NOT** write `context_clues.md` and **Do NOT** prompt for `/clear`. Stop execution and yield to the user.

### If "Harvest, Clear, and Resume" is selected:
1. **Harvest**: Append the summary to `docs/DEV_HISTORY.md` using the strict template below.
2. **Context Clues**: Generate a lightweight file at the root of the project. If you are in the Planning Chat, name it `context_clues_plan.md`. If you are in the Implementation Chat, name it `context_clues_build.md`. This file must contain:
   - **Chat Role:** Explicitly state if this is the "Planning Chat" (New Klang City) or the "Implementation & Build Chat".
   - **Current Objective:** The active feature or milestone we are working on.
   - **Plan Status:** The exact step we were on in `PLAN.md`.
   - **Current State:** Any uncommitted code state, active bugs, or pending actions.
3. **The `/clear` Prompt**: Output a rich markdown block instructing the user to type `/clear` to wipe the context window.
   > 🧹 **Context harvested and saved.**
   > Please type **`/clear`** to wipe the context window in place.

## 3. Strict Template for DEV_HISTORY.md
Whenever writing to DEV_HISTORY, you MUST format the appended entry using this exact layout:
```markdown
### Session: YYYY-MM-DD HH:MM (<conversation_id>)
- **Chat Role:** [Implementation & Build | Planning (New Klang City) | General]
- **Primary Objectives:** <Brief summary of what was asked>
- **Files Modified/Created:** <List of files touched>
- **Key Decisions:** <Any architectural or design choices made>
```

## 4. Resume Trigger (Post-Refresh)
If the user's first prompt after a refresh is "resume" or "continue", you MUST immediately read your role-specific context clues file (`context_clues_plan.md` or `context_clues_build.md`) to restore your working state and pick up exactly where you left off.
