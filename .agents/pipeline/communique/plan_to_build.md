# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.4.0 "The Interface & Experience Update"  
**Task Name:** Obsidian Knowledge Base & Asymmetric Sync Bridge  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** COMPLETED ✅  
**Source Plan:** [`docs/completed_plans/2026-10-07_obsidian_vault_asymmetric_sync_bridge.md`](file:///c:/Dev/TheKlangSuite/docs/completed_plans/2026-10-07_obsidian_vault_asymmetric_sync_bridge.md)  

---

## Strategic Objective
Initialize a dedicated, conflict-free Obsidian Vault at `C:\Dev\TheKlangVault\` (with `Inbox/`, `Docs/`, `Telemetry/`, and `Canvas/` partitions) and implement a robust PowerShell sync bridge (`tools/sync_obsidian_vault.ps1`). This decouples mobile Obsidian Sync from Git, enables frictionless idea capture on mobile, mirrors project documentation for offline reading, and streams build and error telemetry directly into mobile-accessible Obsidian notes.

---

## Acceptance Criteria & Execution Guardrails
1. **Partitioned Folder Isolation**:
   - `C:\Dev\TheKlangVault\Inbox\` syncs to `TheKlangSuite\docs\inbox\`. Mobile notes are never overwritten or deleted by Git.
   - `TheKlangSuite\docs\` mirrors to `C:\Dev\TheKlangVault\Docs\`. Repository documentation remains the immutable master.
   - `C:\Dev\TheKlangVault\Telemetry\` receives live build dashboards, test summaries, and parsed `dev.log` error reports.
2. **Git Cleanliness**:
   - `.gitignore` in `TheKlangSuite` ignores any `.obsidian/` directories and temporary sync cache files (`_sync_*.tmp`).
   - Create `docs/inbox/.gitkeep` in the repo.
3. **Robust PowerShell Sync Engine (`tools/sync_obsidian_vault.ps1`)**:
   - Idempotent execution (safe to run repeatedly, on-demand, or via scheduler).
   - Generates a live heartbeat file `_sync_heartbeat.md` with timestamps and health status.
   - Generates a mobile-friendly dashboard `Telemetry/Dashboard.md` summarizing active Git branch, recent commits, and `dev.log` diagnostics.
4. **End-to-End Verification**:
   - Smoke test confirms bidirectional flow (test note from Inbox -> repo; docs from repo -> vault; telemetry generated).
   - Test artifacts cleanly scrubbed post-validation.
