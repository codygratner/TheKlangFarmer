# Communiqué: New Klang City (Planner) ➔ Klang Industries (Builder)

**Timestamp:** 2026-10-06 18:27  
**Active Milestone:** v0.3.1 "Editor Quality & Data Schema"  
**Task Name:** Planter Limiter Callout Typography & Short Labels Fix  
**Status:** `READY_FOR_EXECUTION`  
**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  
**Quota Impact:** 🟢 SUSTAINABLE (Standard UI & test engineering)  

---

## 🎯 Executive Objective
Fix ugly truncated text and missing labels in the Planter Limiter CalloutBox:
1. Increase callout size to `300x130`.
2. Draw bold, short labels above each control in `paint()`:
   - `LIMIT` (above Enable selector)
   - `GAIN` (above Gain knob)
   - `CEIL` (above Ceiling/Threshold knob)
   - `REL` (above Release knob)
3. Add concise `customFormatText` lambdas on all 3 knobs so values are short and readable without ellipses:
   - Gain: `+4.0 dB` (with +/- sign)
   - Ceil: `-0.2 dB` (or `0.0 dB`)
   - Rel: `150 ms`
4. Conclude turn with the `🔔 JOB'S DONE!` chime.

## 📋 Active Plan Reference
The execution checklist is published in [`PLAN.md`](file:///C:/Dev/TheKlangSuite/PLAN.md).

> ⚠️ **Model Selection Gate**: Do NOT pop up an `ask_question` modal before beginning Phase 1. Present your briefing with the Model Advisory banner at the very end, and pause in chat text for the user to adjust their model dropdown and reply **`proceed`**.

