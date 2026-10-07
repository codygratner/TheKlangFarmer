# Communiqué: New Klang City (Planner) ➔ Klang Industries (Builder)

**Timestamp:** 2026-10-06 19:00  
**Active Milestone:** v0.3.1 "Editor Quality & Data Schema"  
**Task Name:** Restore Two-Line Status Bar Visibility & Universal Hover Feed  
**Status:** `READY_FOR_EXECUTION`  
**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  
**Quota Impact:** 🟢 SUSTAINABLE (Standard UI & test engineering)  

---

## 🎯 Executive Objective
Diagnose and eliminate the "missing status bar" perception in The Klang Planter and The Klang Farmer:
1. **Elevate Chassis Styling & Contrast**:
   - Change background to elevated `0xff121622` with a 1.5px top border `0xff2a3449`.
   - Add a subtle green status LED (`● READY`) when idle so the status bar is unmistakably recognized as an active display.
   - Boost Line 2 text luminance (`0xffcbd5e1` active, `0xff94a3b8` idle guide, `0xff64748b` tips-off) so text is never murky or near-black.
2. **Universal Header & Button Hover Feeds**:
   - Feed hover events from `INIT`, `TRIGGER`, `TIPS`, `SETTINGS`, and `Header Visualizer` into `statusBar.setHoveredControl()`, making the entire UI responsive.
3. **Z-Order Assurance**:
   - Ensure `statusBar.toFront(false)` is called in `resized()` so the bar is always pinned securely above all child components.
4. **Test & Deploy**:
   - Validate through `gui_tests` (all 177+ pass).
   - Rebuild Release binaries and deploy to `current_build/` and Program Files.
   - Conclude turn with the `🔔 JOB'S DONE!` chime.

## 📋 Active Plan Reference
The complete implementation checklist is published in [`PLAN.md`](file:///C:/Dev/TheKlangSuite/PLAN.md).

> ⚠️ **Model Selection Gate**: Do NOT pop up an `ask_question` modal before beginning Phase 1. Present your briefing with the Model Advisory banner at the very end, and pause in regular chat text for the user to adjust their model dropdown in the IDE footer and reply **`proceed`**.
