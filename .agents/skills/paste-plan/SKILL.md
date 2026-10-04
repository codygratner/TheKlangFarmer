---
name: paste-plan
description: Ingests an implementation plan pasted by the user, writes it to PLAN.md, parses the phases, and coordinates execution through an automated audit, build, and task-finishing pipeline. Supports `--build` (-b) to run autonomously. Triggers on `/pasteplan`.
---

# Paste Plan Ingestor & Pipeline Orchestrator

## Goal
Capture an externally authored plan, persist it verbatim to `PLAN.md` at project root, and execute the verification loop across every phase, terminating with `/task-finish`.

## Workflow

### 1. Flag Detection & Ingest
- Check for `--build` or `-b`. Set `AUTO_BUILD = true` if present, else `false`.
- Strip `--build` and `-b` from the prompt text.
- Save the raw plan text directly to `PLAN.md` at project root.

### 2. Parse Plan Structure
Extract:
1. **Linear Issue ID:** Search for `[A-Z]+-[0-9]+` in the title or headers (e.g., `THE-7`).
2. **Core Objective & Phases:** List of numbered phases and acceptance criteria.
3. **Phase 1 Action Items:** Target files and verification targets.

### 3. Briefing & Gate
Print summary:
```markdown
# 📋 Plan Ingested & Saved to `PLAN.md`
**Linear Issue:** <Extracted ID or "None">
**Objective:** <Objective>
**Phases:** <N> Total Phases

### 🚀 Immediate Focus: Phase 1 — <Title>
**Target Files:** <Files>
**Verification:** <Criteria>
```

- If `AUTO_BUILD == true`: Print `[--build detected] Launching automated pipeline...` and begin Phase 1.
- If `AUTO_BUILD == false`: Prompt:
  > `PLAN.md` is locked and ready.
  > **Would you like to start the automated build pipeline for Phase 1 now?**
  > 1. Yes, start Phase 1 pipeline
  > 2. No, wait for manual instructions
  > Proceed immediately if user selects 1 or confirms.

### 4. Automated Execution Pipeline Loop (Per Phase)

For each phase in `PLAN.md`:

1. **Step 1: Code Edits:** Apply the changes specified for the active phase.
2. **Step 2: Audio-Thread Audit:** Run `/audiothread-guard` on changed DSP `.h`/`.cpp` files. Fix any dynamic allocations (`new`, `malloc`, `std::vector`), locks (`std::mutex`), or system calls/logging.
3. **Step 3: Build & Sanity Check:** Run `/build-validate` (CMake build and tests/pluginval). Autonomously diagnose and fix compiler/test errors.
4. **Step 4: Check-off & Stage:** Run `/step-verify` to update `PLAN.md` on disk (mark phase `[x]`) and stage clean files in git.
5. **Step 5: Progression Check:**
   - If uncompleted phases remain (`- [ ]`): Proceed to Phase N+1.
   - If ALL phases are marked complete (`- [x]`): **Immediately and automatically invoke `/task-finish`**.
