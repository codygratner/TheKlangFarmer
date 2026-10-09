---
name: Post-Mortem & Triage Lab
description: Conducts an interactive, methodical triage interview to process research findings or milestone completions, sorting concepts into the Backlog, Future Research, or the Strategy Graveyard.
---

# ⚖️ The Post-Mortem Protocol

**Trigger:** `/postmortem`, `/post-mortem`, `/triage`, "run a post-mortem", "triage the research"

When the user requests a post-mortem for a piece of research, a completed milestone, or an architectural review, execute this interactive lab to ensure no ideas are lost or left in limbo.

## Phase 1: Context Target
1. **Identify the Payload:** Determine exactly what is being triaged (e.g., "The Blue Sky Research Tab", "The v0.4.0 Milestone Completion", "The ToadTracker Integration Ideas").
2. **Compile the Roster:** Internally gather a list of the 3 to 7 distinct concepts, features, or architectural decisions that need to be reviewed.

## Phase 2: The Interactive Triage Interview & Sidecar Triage Canvas
Conduct a methodical, step-by-step interview. Do not overwhelm the user with the entire list at once unless requested.
For each concept, present it clearly and ask the user to assign it one of three strict verdicts:

*   **[ ✅ APPROVE ]:** The concept is validated. It must be formally scheduled into a specific phase of `docs/BACKLOG.md`.
*   **[ 🕰️ TABLE ]:** The concept is good but too ambitious or requires further R&D. It gets parked in the "Future Backlog / Blue Sky" section of the backlog.
*   **[ 💀 KILL ]:** The concept is rejected. It must be logged in the **Strategy Graveyard** (Negative Architecture) along with the explicit rationale for *why* it was killed, to prevent re-litigating the same bad idea next year.

> ⛔ **STRICT MODAL BAN (NEVER USE `ask_question`)**:
> Under NO circumstances should you EVER invoke the `ask_question` tool or spawn interactive IDE modals during a post-mortem or triage session. Interactive modals freeze chat context and create severe friction. All interactive triage cards, option breakdowns, and Decision Composers belong strictly inside the **Sidecar HTML Canvas** (e.g. `ui_ux_deep_research.html` or `post_mortem.html`). The user reviews the visual sidecar and simply types back in natural chat text.


## Phase 2.5: Deep Research Recursion (The Escape Hatch)
If during the triage interview the user proposes a spontaneous new idea or asks you to "do more research" on a concept:
1. **DO NOT** perform a shallow text-based web search.
2. **RECURSIVELY TRIGGER DEEP RESEARCH:** You must immediately invoke the rigorous `/deep-research` workflow (spawning Pro subagents, analyzing codebases, generating rich HTML mockups for the research board).
3. **INJECT & RESUME:** Once the deep research subagents return, inject their findings into the Active Workshop of the HTML board, present it to the user, and resume the triage interview loop.
This ensures all spontaneous ideas are vetted with the exact same extreme rigor as the original batch.

## Phase 2.8: The "Pause & Package" Workflow (Halting for Later)
If the user needs to stop the triage session, step away, or halt the post-mortem for a later time:
1. **Never Leave State in Limbo:** Do not simply acknowledge the pause in chat. You MUST snapshot the complete triage state into a machine-readable package.
2. **Snapshot Package:** Write `.agents/sidecar/packages/<YYYY-MM-DD>_<topic>_triage_package.json` recording:
   - `topic`, `timestamp`, `status: "PAUSED"`, `active_hero_id`.
   - `scorecard`: approved, tabled, killed, pending counts.
   - `triage_items`: all items with assigned verdicts, user notes, recommendations, and context sections.
   - `harvest_doc`: path to the permanent Markdown research harvest.
   - `board_artifact`: path to the HTML research board.
3. **Mirror to Vault:** Mirror the JSON package to `C:\Dev\TheKlangVault\Research\`.
4. **Resumption Receipt:** Output a concise chat receipt confirming the package is saved, displaying current scorecard stats, and providing the resumption shortcut: `👉 To resume at any time, run /resume-post-mortem or type "resume post-mortem"`.

## Phase 2.9: The "Resume" Workflow (Restoring a Paused Post-Mortem)
**Trigger:** `/resume-post-mortem`, `/resume-triage`, "resume post-mortem", "resume triage"
1. **Locate Package:** Find the latest (or user-specified) JSON package in `.agents/sidecar/packages/`.
2. **Restore State:** Ingest the active hero item ID, scorecard, and recorded verdicts from the JSON file.
3. **Render Handshake:** Output the clickable artifact link:
   `👉 **[Open Research Board & Post-Mortem Lab](file:///<appDataDir>/brain/<conversation-id>/<name>.html)**`
4. **Instant Engagement:** Jump straight into the pending triage item with zero lost progress, allowing seamless continuation even after `/clear` or across sessions.

## Phase 3: The Factory Pipeline Injection
Once the triage interview is complete, immediately update the official project documentation:
1. **Update `docs/BACKLOG.md`:** Inject the `[ ✅ APPROVE ]` items into their correct milestones and append the `[ 🕰️ TABLE ]` items to the bottom future section.
2. **Update the Graveyard:** If the target was an HTML research board, update its Strategy Graveyard section with the `[ 💀 KILL ]` items. If it was a general project review, document the kills in `docs/history/DEV_HISTORY.md`.
3. **Commit to Git:** Stage and commit the backlog changes with a conventional commit (e.g., `docs: post-mortem triage for TBD-16 research`).

## Phase 4: Sign-Off
Conclude the post-mortem by outputting a clean Markdown summary table of the verdicts and reminding the user to trigger `/clear` if the context window is getting heavy before jumping into factory execution.
