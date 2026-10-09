# Implementation Chat Context Clues
- **Chat Role:** Implementation & Build Chat (Klang Industries)
- **Current Objective:** Factory Floor Idle (Awaiting v0.4.0 release cut and v0.4.1 dispatch)
- **Plan Status:** Idle 💤 (`PLAN.md` archived, `v0.4.0` completed and verified)
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
- **Subagent Implementation Delegation Protocol (The 20-Line / 1-File Rule):**
  - When in a research or triage session and implementation is requested:
    - *< 20 lines, single file*: Execute directly in-chat (Tier 2 Flash active).
    - *> 20 lines, multi-file, or running builds/tests*: Delegate to Flash subagent (`invoke_subagent(Model="flash")`) to protect main context.
    - *2-Strike Escalation*: If Flash fails 2 compilation/test attempts, pause and escalate to Pro (`Model="pro"`).
- **Product Vision & Taxonomy References:**
  - Glossary & Acronyms: [`docs/GLOSSARY.md`](file:///c:/Dev/TheKlangSuite/docs/GLOSSARY.md) (`TKS`, `TKF`, `TKP`, `TKM`, `TKE`, `TKB`, `TKR`, `TKC`)
  - Product Intent Matrix: [`docs/architecture/product_lineup.md`](file:///c:/Dev/TheKlangSuite/docs/architecture/product_lineup.md)

