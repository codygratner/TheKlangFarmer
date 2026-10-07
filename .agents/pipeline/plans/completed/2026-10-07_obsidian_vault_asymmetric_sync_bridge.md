# Implementation Plan: Obsidian Knowledge Base & Asymmetric Sync Bridge

> **Milestone:** v0.4.0 "The Interface & Experience Update"  
> **Task Name:** Item 1 — Obsidian Knowledge Base & Vault Integration  
> **Design Pattern:** Asymmetric Sync Bridge (Partitioned Ownership Model)  
> **Status:** COMPLETED ✅  
> **Target Complexity Tier:** Tier 2 (Universal Default / Automation Tooling)  

---

## 1. Objective & Architecture Overview

Create a dedicated, conflict-free **Obsidian Knowledge Base** at `C:\Dev\TheKlangVault\` backed by an automated **Asymmetric Sync Bridge** (`tools/sync_obsidian_vault.ps1`). 

This architecture allows the creator to use **Obsidian Sync** on mobile/desktop without sync collisions, while keeping the public C++ repository (`TheKlangSuite/docs/`) the version-controlled single source of truth on GitHub.

```
 ┌────────────────────────────────────────────────────────┐
 │           DEDICATED OBSIDIAN VAULT                     │
 │     (C:\Dev\TheKlangVault - Synced via Obsidian Sync)  │
 ├────────────────────────────────────────────────────────┤
 │ 📥 /Inbox          ──► [Mobile Writes]  ──► Syncs to Repo
 │ 📚 /Docs           ◄── [Git Writes]     ◄── Syncs from Repo
 │ 📊 /Telemetry      ◄── [Agent Writes]   ◄── Build & Error Logs
 │ 🎨 /Canvas         ◄── [User Visuals]   ◄── Architecture Maps
 └────────────────────────────────────────────────────────┘
```

---

## 2. Acceptance Criteria & Guardrails

1. **Partitioned Folder Isolation**:
   - `C:\Dev\TheKlangVault\Inbox\` syncs to `TheKlangSuite\docs\inbox\`. Mobile notes are never overwritten or deleted by Git.
   - `TheKlangSuite\docs\` mirrors to `C:\Dev\TheKlangVault\Docs\`. Repository documentation remains the immutable master.
   - `C:\Dev\TheKlangVault\Telemetry\` receives live build dashboards, test summaries, and parsed `dev.log` error reports.
2. **Git Cleanliness**:
   - `.gitignore` in `TheKlangSuite` ignores any `.obsidian/` directories and temporary sync cache files.
   - Public GitHub users only see official, curated Markdown documentation in `docs/`.
3. **Robust PowerShell Sync Engine (`tools/sync_obsidian_vault.ps1`)**:
   - Idempotent execution (safe to run repeatedly, on-demand, or via scheduler).
   - Generates a live heartbeat file `_sync_heartbeat.md` with timestamps and health status.
   - Generates a mobile-friendly dashboard `Telemetry/Dashboard.md` summarizing active Git branch, recent commits, and `dev.log` diagnostics.
4. **End-to-End Verification**:
   - Smoke test confirms bidirectional flow (test note from Inbox -> repo; docs from repo -> vault; telemetry generated).
   - Test artifacts cleanly scrubbed post-validation.

---

## 3. Phased Execution Blueprint

### Phase 1: Directory Scaffolding & Gitignore Hygiene
- [x] Update `TheKlangSuite/.gitignore` to ignore `.obsidian/` and `_sync_*.tmp`.
- [x] Create `TheKlangSuite/docs/inbox/.gitkeep` (via `docs/inbox/README.md`).
- [x] Initialize `C:\Dev\TheKlangVault\` folder structure:
  - `C:\Dev\TheKlangVault\Inbox\`
  - `C:\Dev\TheKlangVault\Docs\`
  - `C:\Dev\TheKlangVault\Telemetry\`
  - `C:\Dev\TheKlangVault\Canvas\`
- [x] Create `C:\Dev\TheKlangVault\Welcome to The Klang Vault.md` with setup instructions and folder role guides.

### Phase 2: Asymmetric Sync Engine (`tools/sync_obsidian_vault.ps1`)
- [x] Implement `tools/sync_obsidian_vault.ps1` with parameters:
  - `-VaultPath` (default: `C:\Dev\TheKlangVault`)
  - `-Once` (default run once)
  - `-Watch` (optional loop with configurable interval)
- [x] Implement Vault $\to$ Repo Inbox sync (copies new files from Vault Inbox to `docs/inbox/`).
- [x] Implement Repo $\to$ Vault Docs mirror (mirrors `docs/` to Vault `Docs/`, excluding `inbox/`).
- [x] Implement `_sync_heartbeat.md` status generation.

### Phase 3: Telemetry & Error Log Subsystem
- [x] Add Git repository inspection (branch, HEAD hash, recent commits, uncommitted changes).
- [x] Add DevLogger log parser: read `%LOCALAPPDATA%\TheKlangSuite\dev.log`, extracting any `[ERROR]` or `[WARN]` lines from the last 24 hours.
- [x] Format and write `C:\Dev\TheKlangVault\Telemetry\Dashboard.md` with high-contrast Obsidian callouts (`> [!INFO]`, `> [!WARNING]`).

### Phase 4: Verification & Live Smoke Test
- [x] Execute `tools/sync_obsidian_vault.ps1` and verify clean exit (code 0).
- [x] Verify `C:\Dev\TheKlangVault\Docs\` contains `BACKLOG.md`, `GLOSSARY.md`, etc.
- [x] Verify `C:\Dev\TheKlangVault\Telemetry\Dashboard.md` accurately displays branch and test status.
- [x] Create test note in `C:\Dev\TheKlangVault\Inbox\test_note.md`, run sync, verify `docs\inbox\test_note.md` is populated, then delete both.

### Phase 5: Backlog & Dispatch Closure
- [x] Update `docs/BACKLOG.md`: Mark Item 1 under Milestone v0.4.0 as `✅ COMPLETED`.
- [x] Document the Obsidian vault workflow in `docs/OBSIDIAN_INTEGRATION.md`.
- [x] Commit all changes to `0.4.0-dev` with conventional commit `feat(tools): add obsidian vault asymmetric sync bridge`.
- [x] Update `docs/communique/plan_to_build.md` with completion report.
