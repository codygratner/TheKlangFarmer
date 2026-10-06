# Communiqué: New Klang City (Planner) ➔ Klang Industries (Builder)

**Timestamp:** 2026-10-06 17:07  
**Active Milestone:** v0.3.1 "Editor Quality & Data Schema"  
**Task Name:** Planter Header Interactions & Two-Line Status Bar (Tasks 4 & 5)  
**Status:** `READY_FOR_EXECUTION`  
**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  

---

## 🎯 Executive Objective
Implement interactive Planter header controls (VU meter panic flush with visual flash, and right-click Limiter CalloutBox) and build a unified, two-line 36px bottom `StatusBarComponent` across both The Klang Farmer and The Klang Planter providing permanent value readouts, mouse shortcut badges, and a full-width tooltip feed.

## 📋 Active Plan Reference
The execution checklist is published in [`PLAN.md`](file:///C:/Dev/TheKlangSuite/PLAN.md).

### Phase 1: Planter Engine Panic & Header Interactions (Task 4)
- Add real-time safe `PlanterDrumEngine::panic()`.
- Add `meterArea` click for panic + 150ms meter bar flash.
- Add `limitArea` right-click for `PlanterLimiterCalloutComponent` (Enable, Gain, Ceiling, Release).

### Phase 2: Reusable Two-Line `StatusBarComponent` (Task 5)
- Create `StatusBarComponent` in `source/UIComponents.h/.cpp` (~36px height).
- Line 1: Bold control name + formatted value (left) & mouse shortcut badges (right).
- Line 2: Full-width tooltip description (toggled by Tooltips button).

### Phase 3: Integration into The Klang Farmer & The Klang Planter
- Integrate `statusBar` at the bottom of `FarmerEditor` and `PlanterEditor` (`removeFromBottom(36)`).
- Wire hover callbacks and tooltips button toggles.

### Phase 4: Automated GUI Test Verification
- Test Planter panic and CalloutBox opening in `test/gui_tests.cpp`.
- **Thoroughly test 100% of Limiter Callout controls**: sweep and assert APVTS synchronization on `planter_limiter_enable`, `planter_limiter_gain`, `planter_limiter_thresh`, and `planter_limiter_release`, along with non-empty tooltips and leak-free destruction.
- Test status bar hover updates and tooltip toggle.

### Phase 5: Verification, Archival & Handoff
- Full test pass (`gui_tests` + `dsp_tests`).
- Archive plan to `docs/completed_plans/` and reset `PLAN.md` to `# No Active Plan`.
- Report completion in `build_to_plan.md`.

---

## 🛑 Post-Execution Protocol for Klang Industries
Once all phases are complete and tests pass:
- Mark `build_to_plan.md` as `COMPLETE ✅`.
- Commit all changes cleanly to `0.3.1-dev`.
