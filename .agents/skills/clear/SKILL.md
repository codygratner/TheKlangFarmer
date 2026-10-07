---
name: clear
description: Forcefully clears the active chat's context window by wiping the underlying transcript files. Triggers on `/clear`.
---

# Clear Context Skill

## Goal
To wipe the agent's memory of the current chat window by truncating the underlying transcript logs, effectively simulating a fresh chat without forcing the user to open a new UI tab.

## Workflow
1. **Execute Truncation**: Immediately run the truncation python script.
   ```powershell
   python C:\Users\codyg\.gemini\config\scripts\clear_transcript.py
   ```
2. **Output Success**: After the command succeeds, output a very brief confirmation message:
   > "Context cleared. I am ready for the next task."
3. **Resume (Optional)**: If the user previously used `/refresh-context`, they may follow up with "resume" to have you read `context_clues.md`.
