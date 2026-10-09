---
name: update-docs
description: Audits, synchronizes, and reconciles all project documentation (docs/BACKLOG.md, docs/specs/, docs/history/DEV_HISTORY.md, and docs/SYSTEM_MAP.md) with the active codebase, recent factory deliveries, and architectural decisions. Supports standard incremental mode and exhaustive cross-referencing mode (--deep). Triggers on `/updatedocs`, `/update-docs`, `/deep-docs-update`, "update docs", "deep docs update", "sync docs", or "reconcile docs".
---

# Documentation & Spec Synchronization Protocol

## Goal
Enforce total synchronization between C++ implementation realities, architectural design decisions, deep subagent research harvests, and project documentation across all directories defined in [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json).

## Operational Modes
- **Standard Mode (`/update-docs`)**: Fast, incremental reconciliation of recent factory deliveries, active milestone checks in `BACKLOG.md`, dev history logging, and system map updates for The Klang Suite.
- **Deep Docs Mode (`/update-docs --deep` or `/deep-docs-update`)**: Autonomous Goal-Oriented Gauntlet executed across all three ecosystem repositories (`TheKlangSuite`, `ToadTracker`, `nkai`) and the central `TheKlangVault`, linking specs, backlogs, research boards, subagent soul harvests, glossary taxonomy, and full Obsidian vault mirrors.

## Operational Constraints
- **Ivory Tower Only:** This skill is strictly executed by New Klang City. Klang Industries is forbidden from modifying backlogs or overarching architectural specs.
- **Zero Hallucination:** Only mark tasks complete that have verified passing test suites (`dsp_tests`, `gui_tests`) and git commits.
- **Single Source of Truth:** `docs/BACKLOG.md` remains the only official backlog in each repository. Never create rogue todo files.
- **Docs Catalog Grounding:** Always consult [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) as the canonical map for all documentation categories, file paths, and sync expectations.

---

## Standard Workflow (`/update-docs`)

### 1. Ingest Catalog, Factory Telemetry & Subagents
1. Consult [`docs/DOCS_CATALOG.json`](file:///c:/Dev/TheKlangSuite/docs/DOCS_CATALOG.json) to index all tracked documentation targets and directories.
2. Inspect `.agents/pipeline/communique/build_to_plan.md` for completed phases or tasks.
3. Review recent git commits:
   ```powershell
   git log -n 5 --oneline
   ```
4. Check for completed research subagents. Extract their `send_message` or final outputs and compile them into a dedicated harvest document under `docs/history/research/YYYY-MM-DD_<topic>_Harvest.md` ("Soul Harvest") to permanently preserve equations and proofs without bloating dev logs.

### 2. Backlog Reconciliation (`docs/BACKLOG.md`)
1. Scan for items matching completed tasks or factory deliveries.
2. Mark completed items with `— ✅ COMPLETED`.
3. Update any links referencing active `PLAN.md` to point to the permanent archived plan in `.agents/pipeline/plans/completed/<archive_name>.md`.
4. Ensure the `# 📑 Index` at the top of `docs/BACKLOG.md` is complete and up to date with all major milestone headers.

### 3. Spec Synchronization (`docs/specs/*.md`)
1. Cross-reference recent design agreements with the appropriate spec in `docs/specs/`:
   - UI/UX shifts & layout &rarr; `docs/specs/v040_neo_slate_ux.md`
   - New effects & algorithms &rarr; `docs/specs/fx_catalog_expansion.md`
   - Voice engine, gating, & articulation &rarr; `docs/specs/gated_bass_glide.md`
   - Testing & SQA &rarr; `docs/specs/sqa_automation_hardening.md`
2. If a major new subsystem was blueprinted without an existing spec, create a dedicated spec document in `docs/specs/<feature_slug>.md` and register it in `docs/DOCS_CATALOG.json`.

### 4. Institutional Memory & Field Guides (`docs/history/`)
1. Append a concise bulleted summary of recent architectural takeaways, refactor outcomes, or gotchas under the active milestone heading in `docs/history/DEV_HISTORY.md`.
2. If agent skills, system guardrails, sidecars, or workflow pipelines were modified, record the operational rationale in `docs/history/vibe_coding_field_guide.md`.
3. If new DSP invariants, SIMD math routines, or audio safety rules were derived, record them in `docs/history/audio_dsp_field_guide.md`.

### 5. System Map & Catalog Refresh (`docs/SYSTEM_MAP.md` & `docs/DOCS_CATALOG.json`)
1. Check `docs/SYSTEM_MAP.md` and add links to any new specs, tools, research archives, or major guidelines added since the last documentation pass.
2. Update `docs/DOCS_CATALOG.json` if any new files or directories were added.

### 6. Cleanliness & Link Verification
1. Verify that all markdown links within the touched files resolve to real disk paths.
2. Output a structured reconciliation report summarizing updated files.

---

## Tri-Project Deep Ecosystem Mode (`/update-docs --deep`)

When explicitly invoked with `--deep` (or `/deep-docs-update`), this skill transforms into an **Autonomous Five-Pillar Gauntlet** executed by an on-demand Tier 1 Pro subagent (`invoke_subagent(Model="pro")`), sweeping across all four ecosystem repositories and the central Obsidian Vault:

1. **The Klang Suite (`c:\Dev\TheKlangSuite`)**: Synchronizes DSP specs, C++ audio invariants, architecture field guides, and the master backlog.
2. **ToadTracker (`c:\Dev\ToadTracker`)**: Modernizes tracker core specs, dadamachines TBD-16 HAL specifications, multi-target deployment profiles, and the Mayor Toad backlog.
3. **N'kai (`c:\Dev\nkai`)**: Maintains the Asymmetric Sidecar Framework, state machine architecture, UI extension bundle, and `schema.json` contracts.
4. **The Klang Research (`C:\Dev\Research`)**: Centralized RFCs, eternal DSP mathematics, research boards, subagent soul harvests, and novel QA autopsies.
5. **The Klang Vault (`C:\Dev\TheKlangVault`)**: Reconciles the root staging inbox (`Inbox/`) and lore archives (`Lore/`) while mirroring project-level documentation into dedicated project sub-portals (`The Klang Suite/`, `ToadTracker/`, `N'kai/`, `Research/`).

### Subagent Soul Harvest & QA Autopsy Curation Pipeline
To eliminate active coding friction while preventing the "knowledge cemetery" trap, `/update-docs --deep` acts as the master editorial curator for all subagent findings:
1. **Ingest Staged Receipts**: Reads [`.agents/pipeline/communique/subagent_receipt.md`](file:///c:/Dev/TheKlangSuite/.agents/pipeline/communique/subagent_receipt.md) to inspect all completed subagents since the last documentation pass.
2. **Curate High-Impact Discoveries**:
   - **Research Sprints**: Synthesizes mathematical proofs, circuit topologies, and competitive teardowns into publication-grade markdown in `C:\Dev\Research\subagents\<YYYY-MM-DD>_<slug>_Harvest.md`.
   - **Novel QA Autopsies**: Synthesizes non-obvious platform traps (e.g., Chromium webview sandboxes, loopback IPC, audio thread allocations, JUCE timer lifecycles) into `C:\Dev\Research\qa_autopsies\<YYYY-MM-DD>_<slug>_Autopsy.md`. Routine syntax, typo, and styling fixes are discarded to keep the signal-to-noise ratio high.
   - **Standard 5-Point Autopsy Format**:
     1. *Symptoms & Initial Defect*
     2. *Root Cause & Platform Trap*
     3. *Minimal Reproducible Proof*
     4. *Permanent Architectural Invariant / Rule*
     5. *Concrete Resolution & Code Snippet*
3. **Recompile Vault Research Index**: Executes `tools/sync_obsidian_vault.ps1` to mirror all research domains and recompile `TheKlangVault/Research/Master_Research_Index.md` (100% plain Markdown, zero Dataview dependencies).
4. **Receipt Staging Reset (Inbox Zero)**: Clears the processed entries in `.agents/pipeline/communique/subagent_receipt.md` so the receipt log stays clean for subsequent active coding sessions.
5. **Automated Link Verification**: Programmatically audits 100% of markdown hyperlinks across all five roots, asserting a 100% pass rate with zero dead anchors before completing.

