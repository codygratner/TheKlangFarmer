# Bugfix Plan: Planter Standalone State Reset & Limiter Callout Parentage

**Goal:** Resolve the two user-reported issues in The Klang Planter:
1. Eliminate stale cached state in Standalone by ensuring clean parameter defaults and documenting the `%APPDATA%` standalone cache behavior.
2. Fix the Limiter CalloutBox so it renders reliably inside the plugin editor (`parent = this` instead of `nullptr`), expands the click target of the header `LIMIT` badge, and enables right-clicking on Card 6's title bar as an alternate trigger.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)
**Quota Impact:** 🟢 SUSTAINABLE (Economical — standard UI & test engineering)

---

## Phase 1: Robust Limiter Callout In-Window Parenting & Hit Targets (`source/PlanterEditor.cpp`)
- [x] **Fix Parent Component on CalloutBox**:
  - In `headerViz.onLimiterCalloutRequested`:
    - Instead of `launchAsynchronously(std::move(callout), limitScreenArea, nullptr);`, pass `this` (the editor) as the parent component!
    - When `parentComponent = this`, JUCE renders the `CallOutBox` directly inside the editor bounds on top of all children, avoiding Windows OS desktop z-ordering issues where the popup spawns behind the standalone window.
    - Set the anchor area relative to `this`:
      `auto limitEditorArea = area + headerViz.getPosition();`
      `juce::CallOutBox::launchAsynchronously(std::move(callout), limitEditorArea, this);`
- [x] **Enlarge Header Hit Target**:
  - Ensure `PlanterHeaderVisualizer::getLimitArea()` hit target uses the full unreduced height (full 26px) for mouse detection so clicking anywhere on or near the `LIMIT` badge triggers the callout reliably.
- [x] **Add Secondary Trigger on Card 6 (Limiter Card)**:
  - If the user right-clicks Card 6 (`cardLimiter`), also offer the callout or details, making discovery intuitive regardless of whether they click the header or the card.

---

## Phase 2: Standalone State Cache & Default Initialization Verification
- [x] **Verify Default Fallbacks in `PlanterProcessor.cpp`**:
  - Double-check that all 3 limiter parameters in `assets/controls/planter.json` have `"defaultFloat"` and `"default"` populated identically so `ParameterManager` parses both keys without relying on fallbacks.
- [x] **Verify `INIT` Button Reset**:
  - Ensure clicking `INIT` in the header resets 100% of APVTS parameters to their `ControlDef` / APVTS default values and flushes engine state.
- [x] **Automated Headless Test Coverage (`test/PluginIntensiveTestSuite.h`)**:
  - Add test case verifying `PlanterLimiterCalloutComponent` launches successfully when parent is `editor.get()`.
  - Assert that default values on a fresh instance match `assets/controls/planter.json` 100%.

---

## Phase 3: Verification & Build Validation
- [x] Run `build\Debug\gui_tests.exe` and `build\Debug\dsp_tests.exe`.
- [x] Rebuild and deploy to `current_build\Standalone\` and `C:\Program Files\Common Files\VST3\`.
