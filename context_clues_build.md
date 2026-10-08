# Implementation Chat Context Clues
- **Chat Role:** Implementation & Build Chat (Klang Industries)
- **Current Objective:** v0.4.0 Interaction & UI Polish Patch (Phase 1–4 from `PLAN.md`)
- **Plan Status:** Ingested new dispatch from New Klang City (`STATUS: IN_PROGRESS`), ready for Phase 1 execution
- **Current State:**
  - Branch: `0.4.0-dev` (clean working tree)
  - Active Blueprint: [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md)
  - Strategic Directives:
    1. Double-Click vs Alt-Click: Fix `RotaryKnobSlider` so double-click opens text entry without resetting parameter; Alt-click cleanly resets to default.
    2. Audition Trigger Audio Path: Wire thread-safe atomics `triggerAudition(velocity, note)` in `FarmerProcessor` so clicking `TRIGGER` fires an audible drum hit on the audio thread.
    3. Vector 6-Sided Die: Replace text `"d6"` with a custom-painted `DiceButton` vector die face with pips.
    4. Popover Callout Layer: Replace OS-style PopupMenu with sleek `juce::CallOutBox` popovers on right-click for sliders and module cards.
    5. Text Bounds & Truncation: Eliminate text clipping in custom-drawn meter sliders and card headers; expand `TextTruncationAuditSuite`.
    6. Dual-Configuration Gauntlet: 100% test pass in both **Debug** and **Release** builds with fresh deployment.
  - Verification Target: `dsp_tests.exe` and `gui_tests.exe` (all assertions green across Debug and Release)
  - Recommended Model Tier: Tier 2 (Gemini 3.8 Flash High)
