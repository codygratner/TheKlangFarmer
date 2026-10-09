---
name: Deep Research & Artifact Generation
description: Conducts extensive, multi-domain architectural and UX research by spawning Pro subagents and generating an interactive HTML research board, while aggressively protecting the main context window.
---

# 🕵️ The Deep Research Protocol

**Trigger:** `/deepresearch`, `/deep-research`, "do deep research", "research the backlog", "spin up a research board"

When the user requests a massive, multi-faceted research task (e.g., analyzing industry competitor UX, exploring embedded hardware architectures in secondary repositories, or evaluating DSP math constraints), execute the following protocol to prevent context-window bloat and deliver highly polished results.

## Phase 1: Artifact & Infrastructure Initialization
1. **The Research Hub (Dual Storage):** Immediately generate a standalone HTML/CSS/JS artifact (e.g., `ui_ux_research_board.html`) in BOTH:
   - `<appDataDir>\brain\<conversation-id>\<name>.html` (for Antigravity side-panel webview rendering).
   - `c:\Dev\TheKlangSuite\.agents\sidecar\<name>.html` (for local persistence).
2. **Mandatory Artifact Link (The Ready Handshake):** Output the prominent clickable link pointing strictly to the **Artifact Directory** URI:
   `👉 **[Open Research Board](file:///<appDataDir>/brain/<conversation-id>/<name>.html)**`
   *(CRITICAL: Antigravity opens workspace repository links in the code editor, but renders artifact links in the interactive side-panel Webview).*
3. **Neo-Slate Theming:** Style the artifact using the project's standard dark theme (Slate background, JetBrains Mono, Accent Blue/Neon).
4. **Dynamic Domain Tabs:** If the research focuses on a single topic, use a standard layout without macro-switches. If the user requests multiple distinct topics, dynamically generate a top-level JavaScript segmented toggle switch with relevant names (e.g., `[ DATABASE ]` vs `[ FRONTEND ]`) to separate concerns.
5. **Auto-Refresh Mechanism:** Since the artifact might be open side-by-side while you research, avoid aggressive `location.reload()`. Instead, implement a manual `[✨ New Updates Available]` glowing button. (Note: Beware of `file:///` CORS blocks on `fetch()`. If polling fails, instruct the user to click the link again).

## Phase 2: Tri-Engine Subagent Delegation (`research` [Pro], `explore` [Flash], & `contrarian` [Devil's Advocate])
Select the subagent model engine based on the research domain to maximize reasoning quality while protecting model quota:

1. **`research` Mode (Targeted Sprint / `Model="pro"` / Tier 1)**:
   - **Best For:** Complex DSP differential equations, non-linear analog filter proofs, lock-free ring-buffer concurrency, and deep multi-file architectural synthesis.
   - **Operational Profile:** High reasoning density over 1–3 short, surgical subagent turns. Slashes Pro quota burn by ~90% by only spawning Pro for exact technical proofs.
2. **`explore` Mode (Autonomous Marathon / `Model="flash"` / Tier 2 / `/goal`)**:
   - **Best For:** Broad UX benchmarking, community sentiment sweeps (Reddit, KVR, Gearspace), preset browser taxonomy, layout comparisons, and long-running `/goal` research.
   - **Operational Profile:** Self-guided, multi-turn iterative crawls with near-zero quota burn. Allows dozens of autonomous web searches and multi-site extractions without hitting tier rate limits.
3. **`contrarian` Mode (The Devil's Advocate / `Model="flash"` / Tier 2)**:
   - **Best For:** Adversarial analysis, finding hidden architectural traps, spotting edge-case link rot, dependency bloat, and challenging optimistic assumptions.
   - **Operational Profile:** Always paired with exploratory sprints. Explicitly tasked with finding the "Why We Shouldn't Do This", exposing failure modes, and formulating the pragmatic 80/20 middle ground to prevent AI echo chambers.

### Delegation Workflow & Access Policies:
- **Pre-Authorized Web Access Invariant (Default)**: By default, all subagents (`research`, `explore`, `contrarian`) are strictly pre-authorized to use `search_web` and `read_url_content` to sweep competitor architectures, forums, and technical literature.
- **The `--no-web` Air-Gapped Override**: If the user invokes `/deep-research --no-web` or requests offline research, all web access tools are strictly prohibited. Subagents MUST be explicitly instructed: *"Negative Constraint: Web tools disabled. Conduct research strictly using local workspace files, `TheKlangVault/`, and `TheKlangResearch/`."*
- **Step 0 Subagent Context Bootstrapping**: Every spawned subagent charter MUST include a pointer to [`context_clues_subagent.md`](file:///c:/Dev/TheKlangSuite/context_clues_subagent.md) so subagents immediately index project taxonomy (`docs/GLOSSARY.md`), the master docs catalog (`docs/DOCS_CATALOG.json`), and system architecture without prompt bloat.

1. **Spawn Subagent:** Invoke `invoke_subagent` using either `Model="pro"` (Sprint), `Model="flash"` (Marathon), or pair with a Devil's Advocate Flash explorer.
2. **Clear Boundaries:** Provide a tightly scoped mission charter with explicit negative constraints (e.g., "Research UI layouts and preset browsers; do NOT write code and do NOT research DSP math").
3. **Yield & Await:** Let the subagents autonomously explore and report back their structured payloads via `send_message`.


## Phase 3: The Strategy Graveyard (Negative Architecture)
1. **Document Anti-Patterns:** During research, actively look for bad UX/DSP patterns in competitor products or legacy code.
2. **Graveyard Section:** Dedicate a specific section of the HTML artifact to "Negative Architecture." Explicitly state *why* a competitor's feature (e.g., "UVI Falcon Spreadsheet Trees") is rejected.

## Phase 4: In-Flight Triage & Executive Ledger Ingestion
1. **Continuous In-Flight Triage**: Every generated research board MUST include live `[ ✅ APPROVE ]`, `[ ⏳ TABLE ]`, `[ 💀 KILL ]` action chips directly on each concept card, coupled with a live scorecard and Decision Composer.
2. **The Executive Synthesis Handoff**: Once the user has marked their verdicts on the board, triggering `/post-mortem` executes the 1-click **Executive Ledger Ingestion**: ingesting all approved items directly into `docs/BACKLOG.md`, logging killed items into the Strategy Graveyard, and generating the executive summary report in one swift pass.
3. **Pause & Resume Support**: If the user steps away before concluding all items, snapshot packages in `.agents/sidecar/packages/` record current scorecard states, allowing instant 1-click resumption via `/resume-post-mortem`.

## Phase 5: Capstone Harvest, Soul Reaping & Centralized Research Archival
Deep research burns through Pro quota and context tokens rapidly. Once triage is complete:
1. **Subagent Soul Harvest:** Extract the full raw findings, mathematical derivations, circuit schematics, and code snippets from all subagent transcripts/`send_message` payloads and compile them into `C:\Dev\Research\research\subagents\<YYYY-MM-DD>_Subagent_Deep_Research_Harvest.md`.
2. **Artifact Archival:** Save the final HTML artifact to `C:\Dev\Research\research\ui_ux\<YYYY-MM-DD>_<topic>.html` and mirror to `C:\Dev\TheKlangVault\Research\`.
3. **Lean Codebase Guarantee:** Keep the primary C++ code repository lean by storing only active implementation specs in `docs/specs/*.md`. Avoid checking massive 300KB+ HTML research boards into `TheKlangSuite` Git history.
4. **Static Index Compilation:** Ensure `tools/sync_obsidian_vault.ps1` runs to compile the master static markdown index table in `C:\Dev\TheKlangVault\Research\Master_Research_Index.md` (no Dataview required).
5. **Cross-Linking:** Update `docs/history/DEV_HISTORY.md` and `docs/DOCS_CATALOG.json` with permalinks pointing to the centralized research.
6. **Harvest:** Append a concise summary of the decisions and findings directly into `docs/history/DEV_HISTORY.md`.
7. **Handoff:** Tell the user the artifact is complete and explicitly instruct them to run `/clear` or "Replace with New" to begin a fresh, lightning-fast session on Flash High for implementation.


## ⚠️ Key Guardrails
- **The Web Access State Machine:** Deep research often requires external competitive analysis or API docs via the `search_web` tool. You MUST adhere to the Web Access State Machine defined in `GEMINI.md`. If the state is `RESTRICTED`, you must pause and ask for consent ("Reply 'ok for this session' to authorize web access"). If the user authorizes a session-wide override, you MUST append the `[ 🌐 Web Access: ACTIVE (SKILL_SESSION) ]` security footer to every message until the skill completes.
- **NEVER** overwrite the active `PLAN.md` with research data. Research belongs in the HTML artifact and the `BACKLOG.md`.
- **ALWAYS** decouple visual UI research from heavy DSP math research in the presentation layer.
