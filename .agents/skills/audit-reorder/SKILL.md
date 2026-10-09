---
name: audit-reorder
description: Intelligently audits, consolidates, and topologically re-orders system sanity audit & triage topics using a Pro subagent, calculating cumulative allowance impact and sequencing prerequisite-first flight paths.
---

# 🛫 The Audit & Itinerary Re-Order Protocol

**Trigger:** `/audit-reorder`, `/reorder-audit`, `/reorder-itinerary`, `/audit-plan`, "reorder topics", "reorder itinerary", "audit and reorder", "check topics to reorder"

When the user requests an audit or re-ordering of in-flight research/sanity topics, execute this skill to synthesize all active topics, eliminate redundancies, establish topological prerequisites, and compute cumulative token/allowance impact.

## Phase 1: Topic Inventory & State Sweep
1. **Sweep Active Topics**: Read the active sidecar canvas (e.g. `.agents/sidecar/nkai_deep_research.html` or `ui_ux_deep_research.html`), `.agents/pipeline/plans/`, and `docs/BACKLOG.md`.
2. **Collect Current Verdicts**: Identify all items marked `APPROVED`, `TABLED`, `KILLED`, or `PENDING`.
3. **Capture New Spontaneous Proposals**: Extract any newly proposed topics, user suggestions, or architectural forks mentioned in the recent chat turns.

## Phase 2: Autonomous Pro Subagent Synthesis (`invoke_subagent(Model="pro")`)
Delegate the heavy multi-topic topological sorting and token economics to a Pro subagent:
- **Role**: `Audit & Itinerary Synthesis Architect`
- **Model**: `pro` (Tier 1 High Reasoning)
- **Directives**:
  1. **Consolidation & Merge Audit**: Review all topics for overlapping scope or duplicate concerns. Recommend specific combinations/merges to reduce cognitive friction while preserving all distinct decisions.
  2. **Topological Dependency Graph**: Sequence topics strictly prerequisite-first:
     - Foundational Infrastructure & Protocols (Bridge IPC, Data Persistence, Cross-Host Runtimes) MUST precede...
     - UI/UX Surfaces & Layouts (Headers, Tab Density, Utility Drawers) MUST precede...
     - Specific Workflows & View Modes (Ongoing Autopsy, Mail Inbox, Skill Defaults) MUST precede...
     - Meta & Optimization (Harness Shortcuts, Parallel Subagents, Allowance Ledgers).
  3. **Cumulative Allowance & Quota Impact Analysis**:
     - Classify each approved and pending topic into Tier 1 (High Reasoning / Heavy DSP), Tier 2 (Balanced UI/TS), or Tier 3 (Minimal Config/JSON).
     - Calculate the cumulative estimated token burn and highlight potential quota risks.
  4. **The Optimized Flight Path**: Output the clean, re-sequenced flight itinerary (Flight 1 to Flight N) with clear rationale for each position and the recommended next Active Workshop focus.

## Phase 3: Sidecar & State Synchronization
1. **Update Sidecar Data**: Update `workshopTopicsData`, `auditVerdicts`, and `#itinerary-drawer` in the sidecar HTML files (`.agents/sidecar/` and brain artifact).
2. **Run Sidecar QA**: Validate with `python c:\Dev\nkai\tools\audit_sidecar.py` to guarantee 100% HTML tag balance and zero JavaScript errors.
3. **Mirror & Push**: Mirror updated sidecar to `TheKlangResearch` (`C:\Dev\Research\ui_ux\`) and commit.

## Phase 4: Terse Reporting & Handshake
1. Deliver a concise summary (1–3 sentences) in chat highlighting:
   - Any proposed topic merges.
   - The newly sequenced next flight in the Active Workshop.
   - Cumulative token impact summary.
2. Provide the deep link to the sidecar canvas.
