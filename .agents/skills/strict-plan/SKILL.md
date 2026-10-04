---
name: strict-plan
description: Conducts strict, read-only, single-threaded architectural planning for complex audio DSP features and backlog tasks. Writes the final plan directly to PLAN.md without modifying source code or spawning sub-agents. Triggers on `/strict-plan` or `/strictplan`.
---

# In-IDE Strict Architect & Single-Threaded Planner

## Goal
Inspect local codebase architecture, header interfaces, and task specifications, design a phased technical plan, and write it directly to `PLAN.md` at project root—all within a single execution thread with zero source file edits.

## Hard Operational Guardrails (CRITICAL)
- **Strictly Read-Only:** NEVER create, edit, or delete source files (`.h`, `.cpp`, `.cmake`, etc.). The ONLY file you are permitted to write or overwrite is `PLAN.md` at project root.
- **Single-Threaded Execution:** Do NOT spawn sub-agents, background processes, or parallel workers. Perform all discovery and reasoning directly in this primary thread.
- **No Premature Compiles or Tests:** Do NOT run builds, test harnesses, or terminal mutating commands.
- **Audio-Thread Real-Time Safety:** All DSP plans must explicitly account for zero dynamic heap allocations, zero mutex locks, and zero blocking I/O on the audio thread.

## Workflow

### 1. Ingest Task & Scope
- Parse the input following `/strict-plan` (e.g., `/strict-plan Add APF cascade mode` or `/strict-plan Issue #16`).
- If an issue key or backlog reference is provided, inspect `docs/BACKLOG.md` or task context.
- Identify the feature scope and target subsystems (DSP blocks, APVTS parameters, GUI visualizers, editors).

### 2. Targeted Codebase Discovery (Read-Only)
- Inspect `*SPEC*.md` or `docs/` for overarching architecture.
- Use file-read tools to inspect ONLY the relevant headers, parameter IDs, and base classes.
- Keep context lean: read interface definitions rather than thousands of lines of implementation boilerplate.

### 3. Synthesize Technical Architecture & Write `PLAN.md`
Synthesize the approach and write the result directly to `PLAN.md` at project root using this exact structure:

```markdown
# Feature: <Feature Name / Title>
## Objective: <High-level summary>

### Invariants & Technical Constraints
- Real-time safety: zero heap allocation / locks / I/O in DSP rendering path.
- Parameter handling: APVTS IDs, ranges, and normalization curves.
- Build targets: <TKF / TKP / Common>.

---

### Phase 1: <Foundation / Parameter Interfaces>
- **Target Files:** `<path/to/file>`
- **Action Items:**
  - [ ] Item 1
  - [ ] Item 2
- **Verification Condition:** <Specific compiler/assertion check>

### Phase 2: <Core DSP Implementation>
- **Target Files:** `<path/to/file>`
- **Action Items:**
  - [ ] Item 1
  - [ ] Item 2
- **Verification Condition:** <Specific test/run criteria>

### Phase 3: <GUI / Integration Polish>
- **Target Files:** `<path/to/file>`
- **Action Items:**
  - [ ] Item 1
  - [ ] Item 2
- **Verification Condition:** <pluginval / manual audio test>
```

### 4. Present Briefing & Await Approval
Once `PLAN.md` is written to disk, do NOT begin coding. Output an executive summary in the chat pane:

```markdown
# 📐 Plan Drafted & Saved to `PLAN.md`

**Target:** <Objective>  
**Total Phases:** <N> Phases

### Architectural Summary
<2-3 concise sentences on how the design works>

---

### Ready for Review
Review `PLAN.md` on disk. When satisfied, proceed with:
- **`/readplan --build`** (or `/pasteplan --build` — fully autonomous audit and build loop)
- **`/readplan`** (interactive decision gate: start Phase 1, defer, or review)
- **`/readplan --backlog`** (defer to backlog for later)
- **`Execute Phase 1`** (step-by-step with checkpoints)
```
