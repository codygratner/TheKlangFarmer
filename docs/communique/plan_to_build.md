# Communiqué: New Klang City (Planner) ➔ Klang Industries (Builder)

**Timestamp:** 2026-10-06 17:58  
**Active Milestone:** v0.3.1 "Editor Quality & Data Schema"  
**Task Name:** Planter Standalone State Reset & Limiter Callout Fixes  
**Status:** `READY_FOR_EXECUTION`  
**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  
**Quota Impact:** 🟢 SUSTAINABLE (Standard UI & test engineering)  

---

## 🎯 Executive Objective
Fix two user-reported issues in The Klang Planter:
1. Ensure the Limiter CalloutBox renders reliably inside the plugin editor window (`parent = this` instead of `nullptr`), expand the click target on the header `LIMIT` badge, and support right-clicking on Card 6.
2. Ensure parameter defaults match `planter.json` and document the `%APPDATA%\The Klang Planter\The Klang Planter.settings` standalone caching behavior.

## 📋 Active Plan Reference
The execution checklist is published in [`PLAN.md`](file:///C:/Dev/TheKlangSuite/PLAN.md).

### Directives for Klang Industries:
- In `PlanterEditor.cpp`: Replace `launchAsynchronously(std::move(callout), limitScreenArea, nullptr)` with `launchAsynchronously(std::move(callout), area + headerViz.getPosition(), this)`.
- Enlarge the header mouse click target in `PlanterHeaderVisualizer::getLimitArea()` so it spans the full 26px height.
- Update `assets/controls/planter.json` to ensure `planter_limiter_gain`, `planter_limiter_thresh`, and `planter_limiter_release` have `"defaultFloat"` matching `"default"`.
- Run `gui_tests` and deploy fresh binaries to `current_build\Standalone\` and `C:\Program Files\Common Files\VST3\`.
