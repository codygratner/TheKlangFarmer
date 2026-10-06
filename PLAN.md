# Architectural Plan: The "Context Refresh" Skill

**Goal**: Implement the `/refresh-context` workflow to allow long-running builder chats (like "Klang Industries") to cleanly wipe their context windows (`/clear`) without losing track of their current state or objectives, simulating an ephemeral chat while maintaining UX continuity.

---

## Phase 1: Skill Definition (`SKILL.md`)
- [x] Create a new skill directory: `C:\Users\codyg\.gemini\config\skills\refresh-context\`
- [x] Create `SKILL.md` inside the directory with the YAML frontmatter:
  ```yaml
  ---
  name: refresh-context
  description: Harvests the active chat into DEV_HISTORY, writes a context clues artifact, and prompts the user to /clear the session. Triggers on `/refresh-context`, `/refreshcontext`, or "refresh context".
  ---
  ```
- [x] Write the workflow instructions inside `SKILL.md` enforcing the following sequence:
  1. **Harvest**: The agent must read the current state and append a brief summary of the session's work so far into `docs/DEV_HISTORY.md`.
  2. **Context Clues**: The agent must generate a lightweight `context_clues.md` file in the repository root. This file must contain:
     - The current active feature or milestone.
     - The exact step we were on in `PLAN.md`.
     - Any uncommitted code state or active bugs.
  3. **The `/clear` Prompt**: The agent must stop and output a rich markdown block instructing the user to type `/clear` to wipe the context window.
  4. **The Resume Trigger**: Add instructions that if the user's first prompt after a refresh is "resume" or "continue", the agent should immediately read `context_clues.md` and pick up exactly where it left off.

## Phase 2: Test & Validate
- [x] Execute the skill locally in the Klang Industries chat by typing `/refresh-context`.
- [x] Verify `context_clues.md` is generated accurately.
- [x] Click "Replace with New" in the sidebar.
- [x] Type "resume" and verify the agent seamlessly continues the workflow without prompt drift.

---
*Note: This is a pure prompt-engineering skill. No Python harvester scripts are needed since the agent already has the active chat in its context window and can write the files directly.*
