---
name: update-docs
description: Audits, synchronizes, and reconciles all project documentation (docs/BACKLOG.md, docs/specs/, docs/history/DEV_HISTORY.md, and docs/SYSTEM_MAP.md) with the active codebase, recent factory deliveries, and architectural decisions. Triggers on `/updatedocs`, `/update-docs`, "update docs", "sync docs", or "reconcile docs".
---

# Documentation & Spec Synchronization Protocol

## Goal
Enforce total synchronization between C++ implementation realities, architectural design decisions, and project documentation across `docs/BACKLOG.md`, `docs/specs/`, `docs/history/DEV_HISTORY.md`, and `docs/SYSTEM_MAP.md`.

## Operational Constraints
- **Ivory Tower Only:** This skill is strictly executed by New Klang City. Klang Industries is forbidden from modifying backlogs or overarching architectural specs.
- **Zero Hallucination:** Only mark tasks complete that have verified passing test suites (`dsp_tests`, `gui_tests`) and git commits.
- **Single Source of Truth:** `docs/BACKLOG.md` remains the only official backlog. Never create rogue todo files.

---

## Workflow

### 1. Ingest Recent Factory & Git Telemetry
1. Inspect `.agents/pipeline/communique/build_to_plan.md` for completed phases or tasks.
2. Review recent git commits:
   ```powershell
   git log -n 5 --oneline
   ```
3. Identify newly introduced classes, DSP blocks, parameter schemas, or architectural pivots.

### 2. Backlog Reconciliation (`docs/BACKLOG.md`)
1. Scan for items matching completed tasks.
2. Mark completed items with `— ✅ COMPLETED`.
3. Update any links referencing active `PLAN.md` to point to the permanent archived plan in `.agents/pipeline/plans/completed/<archive_name>.md`.
4. Ensure the `# 📑 Index` at the top of `docs/BACKLOG.md` is complete and up to date with all major milestone headers.

### 3. Spec Synchronization (`docs/specs/*.md`)
1. Cross-reference recent design agreements with the appropriate spec in `docs/specs/`:
   - UI/UX shifts & layout &rarr; `docs/specs/v040_neo_slate_ux.md`
   - New effects & algorithms &rarr; `docs/specs/fx_catalog_expansion.md`
   - Voice engine, gating, & articulation &rarr; `docs/specs/gated_bass_glide.md`
   - Testing & SQA &rarr; `docs/specs/sqa_automation_hardening.md`
2. If a major new subsystem was blueprinted without an existing spec, create a dedicated spec document in `docs/specs/<feature_slug>.md`.

### 4. Institutional Memory Capture (`docs/history/DEV_HISTORY.md`)
1. Append a concise bulleted summary of recent architectural takeaways, refactor outcomes, or gotchas under the active milestone heading in `docs/history/DEV_HISTORY.md`.
2. If agent skills, system guardrails, or workflow pipelines were modified, record the operational rationale in `docs/history/vibe_coding_field_guide.md`.

### 5. System Map Refresh (`docs/SYSTEM_MAP.md`)
1. Check `docs/SYSTEM_MAP.md`.
2. Add links to any new specs, tools, or major guidelines added since the last documentation pass.

### 6. Cleanliness & Link Verification
1. Verify that all markdown links within the touched files resolve to real disk paths.
2. Output a structured reconciliation report summarizing updated files.
