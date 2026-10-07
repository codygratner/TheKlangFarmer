# Bugfix & Ergonomics Plan: Restore Two-Line Status Bar Visibility & Universal Hover Feed

**Goal:** Ensure the Kilohearts/Ableton-style 36px bottom status bar is unmistakably visible, legible, and responsive across The Klang Planter and The Klang Farmer:
1. Elevate the visual chassis and contrast in `StatusBarComponent::paint()`: distinctive background, crisp top border, high-contrast typography, and a prominent status indicator dot (`● READY`).
2. Upgrade Line 2 idle/tooltip text luminance so it is immediately legible (never murky or hidden).
3. Connect universal hover feeds for all header controls (`INIT`, `TRIGGER`, `TIPS`, `SETTINGS`, and `Header Visualizer / Panic`) so the bar responds to the whole interface, not just card knobs.
4. Guarantee z-order prominence (`toFront(false)`) in both Planter and Farmer editors.
5. Validate via headless assertions in `gui_tests` (177+ passing) and deploy Release builds.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  
**Quota Impact:** 🟢 SUSTAINABLE (Standard UI & test engineering)  

---

## Phase 1: Status Bar Visual Elevation & High-Contrast Typography (`source/UIComponents.cpp`)
- [x] In `StatusBarComponent::paint(juce::Graphics& g)`:
  - **Chassis Background & Border**:
    - Change background fill from dark `0xff0d1117` to elevated chassis `0xff121622`.
    - Draw a crisp 1.5px top border in `0xff2a3449` to cleanly demarcate the status bar from the card area.
  - **Line 1 (Permanent Top Line - 16px)**:
    - When `currentName.isNotEmpty()`:
      - Display control name in crisp bold (`0xfff8fafc`).
      - Value in bright cyan (`0xff38bdf8`).
      - Shortcuts in right-aligned rounded pill badges (`0xff1a2333` fill, `0xff38bdf8` text).
    - When idle (`currentName.isEmpty()`):
      - Draw a subtle green status LED dot (`●`, `0xff22c55e`, radius 3.5px) at left margin.
      - Draw bold label `"SYSTEM READY"` in `0xff94a3b8` next to the LED.
      - On the right: display subtle badge `"The Klang Suite v0.3.1"`.
  - **Line 2 (Dynamic Tooltip & Guide Feed - 14px)**:
    - When `tooltipsEnabled` is true:
      - Active description: rendered in `0xffcbd5e1` (crisp light slate, 100% legible).
      - Idle guide text: `"✦ Hover any knob, button, or header meter for parameter details and shortcuts."` in `0xff94a3b8`.
    - When `tooltipsEnabled` is false:
      - Idle guide text: `"TIPS DISABLED — Click 'TIPS: OFF' in header to activate full parameter guides."` in `0xff64748b` (clearly readable, not blacked out).

---

## Phase 2: Universal Header & Button Hover Feeds (`source/KlangCoreEditor.cpp`, `PlanterEditor.cpp`, `FarmerEditor.cpp`)
- [x] In `KlangCoreEditor.h` & `KlangCoreEditor.cpp`:
  - Add helper `wireHeaderButtonHover(juce::Button& btn, const juce::String& name, const juce::String& desc, const juce::String& clickHint)`:
    - Uses mouse enter/exit listeners or custom button subclasses to feed `statusBar.setHoveredControl()`.
  - Wire hover callbacks for shared header controls:
    - `initButton`: Name: `"INIT"`, Value: `"Patch Reset"`, Desc: `"Emergency panic audio flush & restore factory default parameters."`, Click: `"Click: Reset"`.
    - `triggerButton`: Name: `"TRIGGER"`, Value: `"Audition Hit"`, Desc: `"Fire manual audition drum strike at full 1.0 velocity."`, Click: `"Click: Trigger"`.
    - `tooltipsButton`: Name: `"TOOLTIPS"`, Value: `(tooltipsEnabled ? "ON" : "OFF")`, Desc: `"Toggle live dynamic two-line parameter guidance feed."`, Click: `"Click: Toggle"`.
    - `settingsButton`: Name: `"SETTINGS"`, Value: `"Configuration"`, Desc: `"Check for GitHub updates and view system telemetry."`, Click: `"Click: Open"`.
    - `guideButton` (Farmer): Name: `"QUICKSTART"`, Value: `"Interactive Guide"`, Desc: `"Display modal walkthrough and workflow shortcuts."`, Click: `"Click: Open"`.
- [x] In `PlanterEditor.cpp`:
  - Wire `headerViz` mouse enter/exit:
    - Name: `"HEADER DISPLAY"`, Value: `"Scope & Meters"`, Desc: `"Click meters to panic flush. Right-click LIMIT badge to summon master limiter."`, Shortcuts: `"L-Click: Panic | R-Click: Limiter"`.
  - In `resized()`: Ensure `statusBar.toFront(false)` is invoked so no card can ever obscure the status bar.
- [x] In `FarmerEditor.cpp`:
  - In `resized()`: Ensure `statusBar.toFront(false)` is invoked.

---

## Phase 3: Headless Verification & Deployment
- [x] In `test/PluginIntensiveTestSuite.h`:
  - Expand `runStatusBarIntegrityTest()`:
    - Assert `statusBar.isOpaque() == true`.
    - Assert `statusBar.getY() == editor->getHeight() - 36`.
    - Simulate mouse enter on `initButton` and assert `statusBar.getActiveName() == "INIT"`.
    - Simulate mouse enter on `triggerButton` and assert `statusBar.getActiveName() == "TRIGGER"`.
    - Assert idle text is non-empty and contains `"SYSTEM READY"`.
- [x] Run `build\Debug\gui_tests.exe` (all 191 tests passed).
- [x] Build Release targets: `TheKlangPlanter_Standalone`, `TheKlangFarmer_Standalone`, `TheKlangPlanter_VST3`, `TheKlangFarmer_VST3`, and `TheKlangEditor`.
- [x] Deploy binaries to `current_build\` and `C:\Program Files\Common Files\VST3\`.
- [x] Sound the `🔔 JOB'S DONE!` chime for New Klang City.
