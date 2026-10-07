# Implementation Plan: Documentation Wiki & Agent Pipeline Separation

> **Status:** COMPLETED ✅  
> **Target Version:** v0.4.0 (Pre-Flight Infrastructure)  
> **Authority:** New Klang City (Ivory Tower)  

---

## 1. Executive Summary & Architectural Motivation

Currently, the `docs/` directory serves two completely conflicting purposes:
1. **Human Knowledge Base & Product Documentation:** Architecture guides, specifications, meeting notes, taxonomy, and dev history (which mirrors to `TheKlangVault` for Obsidian mobile viewing).
2. **AI Agent Working Machinery:** Inter-chat mailbox queues (`docs/communique/`), active draft buffers (`docs/draft_plans/`), and historical execution logs (`docs/completed_plans/`).

Furthermore, over 20 loose `*_plan.md` files clutter the `docs/` root—mixing already-shipped v0.3.x features with future roadmap blueprints. Finally, `[[wikilinks]]` written in Obsidian break on GitHub.com because GitHub repository Markdown only supports standard `[Label](path.md)` relative links.

This plan achieves **complete separation of concerns**, cleans `docs/` into a pristine 4-pillar documentation wiki, migrates agent pipes into `.agents/pipeline/`, introduces automated "fix-it-in-post" link normalization, and adds a dedicated **Phase 2.5: Pro High Documentation Polish** to `/cut-release`.

---

## 2. Target Directory & File Layout

### A. The Pure Human Documentation Tree (`docs/`)
```text
docs/
├── README.md                      # 🏛️ Central Wiki Hub & Map of Content (MOC)
├── BACKLOG.md                     # 🗺️ Master Product Roadmap & Feature Queue
├── GLOSSARY.md                    # 📖 Architectural Taxonomy (Cards, Modules, Sliders, Knobs)
├── architecture/                  # 📐 Deep Technical Systems Documentation
│   ├── audio_thread_invariants.md # Real-time DSP safety rules (Zero lock/alloc/IO)
│   ├── data_schema_layers.md      # 4-layer JSON schema separation
│   ├── headless_gui_testing.md    # JUCE headless component harness
│   ├── dev_logger_subsystem.md    # TKS_LOG rotating diagnostics
│   └── obsidian_sync_bridge.md    # Asymmetric sync architecture (migrated from OBSIDIAN_INTEGRATION.md)
├── specs/                         # 🚀 Active & Future Feature Specifications
│   ├── v040_neo_slate_ux.md       # v0.4.0 UX Overhaul & 4-Controls-Per-Card
│   ├── sqa_automation_hardening.md# v0.4.0 Timeout guardrails & failure snapshots
│   ├── gated_bass_glide.md        # v0.5.0 Voice articulation, note-off release, portamento
│   ├── fx_catalog_expansion.md    # v0.5.0 13 new DSP blocks & 5-column browser
│   ├── planter_bass_modes.md      # v0.5.0 Planter dual-oscillator hard sync & bass FM
│   ├── preset_system.md           # v0.6.0 JSON tag-based preset browser
│   ├── sample_players.md          # v0.6.0 Dual transient sample engines
│   ├── wav_sf2_export.md          # v0.6.0 Drag-and-drop export
│   ├── typography_engine.md       # JUCE 9 CSS-style typography
│   ├── crash_reporting.md         # Zero-server GitHub crash reporting
│   └── spinoffs/                  # Spinoff instrument blueprints (The Klang Mill, TKR-1)
├── history/                       # 📜 Historical Development Logs & Shipped Archives
│   ├── DEV_HISTORY.md             # Living development journal
│   └── archives/                  # Archived previous milestones (v0.3.0, completed plans)
├── briefings/                     # 🎙️ Stakeholder & Expert Consultation Notes
│   └── 2026-10-07_sqa_meeting.md  # SQA meeting briefing with Tom (migrated from SQA_MEETING_BRIEFING.md)
└── inbox/                         # 📥 Local-Only Staging Inbox (Gitignored, mirrors from Vault)
    └── README.md
```

### B. The Agent Working Machinery (`.agents/pipeline/`)
```text
.agents/
├── skills/                        # Existing slash commands and workflow skills
└── pipeline/                      # 🤖 Agent Inter-Process Communication & Planning Machinery
    ├── communique/                # Inter-chat mailboxes
    │   ├── plan_to_build.md       # Dispatch queue (New Klang City -> Klang Industries)
    │   └── build_to_plan.md       # Status feed (Klang Industries -> New Klang City)
    └── plans/
        ├── drafts/                # Staged planning buffer (drafts being grilled)
        │   └── README.md
        └── completed/             # Archived executed execution blueprints
```

---

## 3. Phased Execution Roadmap

### Phase 1: Agent Machinery Relocation (`.agents/pipeline/`) — ✅ COMPLETED
1. Created directories:
   - `.agents/pipeline/communique/`
   - `.agents/pipeline/plans/drafts/`
   - `.agents/pipeline/plans/completed/`
2. Migrated files via `git mv`:
   - `docs/communique/*` $\to$ `.agents/pipeline/communique/`
   - `docs/completed_plans/*` $\to$ `.agents/pipeline/plans/completed/`
   - `docs/draft_plans/*` $\to$ `.agents/pipeline/plans/drafts/`
3. Removed old empty directories from `docs/`.

### Phase 2: Skills & Guardrails Path Synchronization — ✅ COMPLETED
1. Updated skill definitions across both `.agents/skills/` and `~/.gemini/config/skills/`:
   - `clean-plan/SKILL.md`: Updated target archive paths to `.agents/pipeline/plans/completed/` and mailboxes to `.agents/pipeline/communique/`.
   - `execute-task/SKILL.md`: Sourced from `docs/BACKLOG.md` or `.agents/pipeline/plans/`.
   - `read-plan/SKILL.md`: Read dispatch from `.agents/pipeline/communique/plan_to_build.md`.
   - `task-finish/SKILL.md`: Updated mailboxes in `.agents/pipeline/communique/` and archive to `.agents/pipeline/plans/completed/`.
   - `cut-release/SKILL.md`: Integrated **Phase 2.5: Pro High Documentation Polish & Wiki Sync**.
2. Updated `GEMINI.md`:
   - Mailbox Protocol paths $\to$ `.agents/pipeline/communique/`.
   - Staged Planning Buffer $\to$ `.agents/pipeline/plans/drafts/`.
   - Completed plans $\to$ `.agents/pipeline/plans/completed/`.
   - Codified **Phase 2.5 Pre-Flight Doc Polish** in the Pre-Release Gauntlet.

### Phase 3: Human Documentation Reorganization (`docs/`) — ✅ COMPLETED
1. Created 4-pillar directories:
   - `docs/architecture/`
   - `docs/specs/`
   - `docs/specs/spinoffs/`
   - `docs/history/`
   - `docs/briefings/`
2. Triaged & Moved Loose Plans:
   - **Completed Shipped Features (v0.3.x):** Moved to `.agents/pipeline/plans/completed/` (e.g. `gui_test_harness_plan.md`, `json_editor_tool_plan.md`, `v020_parity_audit_plan.md`, `post_v030_release_and_archive_plan.md`).
   - **Technical Guides:** Authored and organized `docs/architecture/` (`audio_thread_invariants.md`, `data_schema_layers.md`, `headless_gui_testing.md`, `dev_logger_subsystem.md`, `obsidian_sync_bridge.md`).
   - **Meeting Notes:** Moved `SQA_MEETING_BRIEFING.md` $\to$ `docs/briefings/2026-10-07_sqa_meeting.md`.
   - **Active & Future Feature Specs:** Cleanly renamed and placed in `docs/specs/`:
     - `v040_neo_slate_ux.md`
     - `sqa_automation_hardening.md`
     - `gated_bass_glide.md`
     - `fx_catalog_expansion.md`
     - `preset_system.md`
     - `wav_sf2_export.md`
     - `typography_engine.md`
     - `crash_reporting.md`
     - `one_time_quick_tour.md`
     - `pre_v1_sound_and_workflow_expansion.md`
     - `plugin_starter_template.md`
     - `tbd16_klang_seed_effects.md`
     - `spinoffs/the_klang_mill.md`
     - `spinoffs/the_klang_r1.md`
     - `spinoffs/zynthian_port.md`
     - `spinoffs/toadtracker_migration.md`
   - **History:** Moved `DEV_HISTORY.md`, `BACKLOG_ARCHIVE.md`, and `archives/` into `docs/history/`.
3. Authored `docs/README.md` (Wiki Hub / Map of Content):
   - Clickable table of contents linking across all 4 pillars using standard Markdown links `[Title](path.md)`.
4. Updated root `README.md`:
   - Added prominent callout linking to `[Documentation Wiki](docs/README.md)`.

### Phase 4: Asymmetric Sync Bridge & Link Normalization (`tools/sync_obsidian_vault.ps1`) — ✅ COMPLETED
1. Updated `tools/sync_obsidian_vault.ps1`:
   - Pointed Docs mirror source strictly to `docs/`.
   - Added regex normalizer `Convert-WikiLinksToMarkdown`: automatically converts `[[TargetDoc]]` or `[[TargetDoc|Label]]` into standard `[Label](TargetDoc.md)`.
   - Added automated pruning of stale deleted/moved files in `TheKlangVault/Docs/`.
2. Ran full synchronization pass:
   - Synchronized all 26 clean docs, pruned 38 stale robot plans and duplicate files from Vault.
3. Verified that `TheKlangVault/Docs/` reflects the pristine 4-pillar layout with working links and zero robot files.

---

## 4. Verification & Acceptance Criteria
- [x] Zero loose `*_plan.md` files in `docs/` root.
- [x] `docs/` contains only `README.md`, `BACKLOG.md`, `GLOSSARY.md`, and the 4 pillars (`specs/`, `architecture/`, `history/`, `briefings/`, plus `inbox/`).
- [x] `.agents/pipeline/` contains all mailboxes and draft/completed plan archives.
- [x] 100% of skills (`clean-plan`, `execute-task`, `read-plan`, `task-finish`, `cut-release`) reference the new paths and operate cleanly.
- [x] `tools/sync_obsidian_vault.ps1` runs in < 600ms with 0 errors.
- [x] 100% of links in `docs/README.md` resolve on GitHub.com and inside Obsidian.
