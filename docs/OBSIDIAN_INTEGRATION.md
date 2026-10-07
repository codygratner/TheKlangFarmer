# Obsidian Knowledge Base & Asymmetric Sync Bridge

The Klang Suite features a decoupled **Obsidian Knowledge Base** architecture designed for solo developers using **Obsidian Sync** across desktop and mobile devices.

---

## 🏛️ Architectural Overview

To eliminate file contention between **Git** (which rewrites and deletes files during branch switches) and **Obsidian Sync** (which cloud-syncs file system events), the repository and personal vault are physically separated on disk:

```
 ┌────────────────────────────────────────────────────────┐
 │           THE KLANG VAULT (Obsidian Sync)              │
 │                 Path: C:\Dev\TheKlangVault             │
 ├────────────────────────────────────────────────────────┤
 │ 📥 /Inbox          ──► [Mobile Writes]  ──► Syncs to Repo
 │ 📚 /Docs           ◄── [Git Writes]     ◄── Syncs from Repo
 │ 📊 /Telemetry      ◄── [Agent Writes]   ◄── Build & Error Logs
 │ 🎨 /Canvas         ◄── [User Visuals]   ◄── Architecture Maps
 └────────────────────────────────────────────────────────┘
```

- **Repository Docs (`TheKlangSuite/docs/`)**: The version-controlled, public single source of truth hosted on GitHub.
- **Dedicated Obsidian Vault (`C:\Dev\TheKlangVault/`)**: Your private, personal workspace with Obsidian Sync enabled.

---

## 🗂️ Partition Rules

| Directory | Owner / Authority | Direction | Description |
| :--- | :--- | :--- | :--- |
| `Inbox/` | **Mobile / Obsidian** 👑 | Vault $\to$ Repo | Quick notes, preset ideas, and audio math jotted on mobile. Safely copied to `TheKlangSuite/docs/inbox/`. Never deleted or overwritten by Git. |
| `Docs/` | **Git / Repository** 👑 | Repo $\to$ Vault | Automatically mirrored from `docs/`. Read [`GLOSSARY.md`](GLOSSARY.md), [`BACKLOG.md`](BACKLOG.md), and master blueprints offline on your phone. |
| `Telemetry/` | **Agent / Sync Script** 📊 | Script $\to$ Vault | Live status dashboards (`Dashboard.md`) and error reports (`Active_Errors.md`) parsed from `%LOCALAPPDATA%\TheKlangSuite\dev.log`. |
| `Canvas/` | **Obsidian Canvases** 🎨 | Vault Only | Visual signal flow graphs, FM routing diagrams, and card layout mockups. |

---

## ⚡ Sync Tool: `tools/sync_obsidian_vault.ps1`

### Running On-Demand
Run a single synchronization pass at any time:
```powershell
./tools/sync_obsidian_vault.ps1
```

### Running as a Live Watcher
Run continuous background synchronization (polls every 10 seconds):
```powershell
./tools/sync_obsidian_vault.ps1 -Watch -IntervalSeconds 10
```

### Heartbeat & Health Check
Every sync pass updates `_sync_heartbeat.md` at the root of the vault:
```markdown
# Vault Sync Heartbeat
- **Status:** ACTIVE & IN SYNC
- **Last Sync:** 2026-10-07 11:51:14
- **Duration:** 456.6 ms
- **Branch:** `0.4.0-dev`
- **Inbox Ingested:** 1
- **Docs Mirrored:** 61
- **Active Errors:** 0
```
