# [THE-7] The Klang Planter UI Color Routing & "Vel Min Level" Label Polish
**Linear URL:** https://linear.app/the-klang-farmer/issue/THE-7/the-klang-planter-ui-color-routing-and-vel-min-level-label-polish

---

# Implementation Plan & Technical Audit: The Klang Planter UI Color Routing & Label Polish (`THE-7`)

**Repository:** `https://github.com/codygratner/TheKlangFarmer.git`
**Branch:** `codygratner/the-6-the-7-fx-alphabetical-and-planter-color`
**Linear Issue:** `THE-7`

---

## 1. Executive Summary & Design Decisions

Implement targeted front-panel visual refinements on **The Klang Planter** (TKP) to enhance signal flow communication, introduce dual-accent tactile destination buttons, align parameter typography, and synchronize Section 6 of `spec.md`:
1. **Carrier Card — Mod Depth Slider (Cyan)**: Route Modulator Cyan (`colCyan` / `0xff00d2ff`) to `carrierDepthSlider` on the Carrier card to visually communicate that this slider attenuates incoming modulation from the Modulator module.
2. **Pitch Env Card — Multi-Color Destination Buttons (`Car`, `Mod`, `Both`, `Opp`)**: Extend `LedSelectorComponent` in `source/UIComponents.h` and `source/UIComponents.cpp` to support per-button primary/secondary accents, dual-halo LEDs, and customized text coloring with readable unselected state tinting:
   - `Car`: Carrier Red accent (`colRed`).
   - `Mod`: Modulator Cyan accent (`colCyan`).
   - `Both`: Carrier Red text (`colRed`) with Modulator Cyan outline (`colCyan`) and dual Red/Cyan LED halo.
   - `Opp`: Modulator Cyan text (`colCyan`) with Carrier Red outline (`colRed`) and dual Cyan/Red LED halo.
   - Unselected State Readability: Blend 28% border tint and 38% text tint against standard neutral steel-blue (`#b0bdd0`), ensuring crisp readability at all times without washing out.
3. **Filter Env Card — Pre-Filter Drive Slider (Blue)**: Route Filter Blue (`colBlue` / `0xff2979ff`) to `filterEnvDriveSlider` on the Filter Env card to visually communicate that this drive stage feeds into the Filter module.
4. **Amplifier Card — Velocity Floor Label Polish**: Update Knob 4's label from `"Velocity"` to confirmed choice: **`"Vel Min Level"`** (aligning with `"Vel Slope"` directly above it on Knob 3).
5. **Master Specification Synchronization**: Update the 2x4 ASCII rack diagram and Signal Flow documentation in Section 6 of `spec.md`.

---

## 2. Step-by-Step Implementation Plan

### Phase 1: `LedSelectorComponent` Dual-Accent & Styling Support (`source/UIComponents.h` & `source/UIComponents.cpp`) — [x] COMPLETED
- [x] **Step 1.1**: In `source/UIComponents.h`, define `struct ItemStyle { juce::Colour primaryAccent; std::optional<juce::Colour> secondaryAccent = std::nullopt; std::optional<juce::Colour> textColour = std::nullopt; };` and declare `setItemStyle(int index, const ItemStyle& style)` and `clearItemStyles()`.
- [x] **Step 1.2**: In `source/UIComponents.cpp`, implement `setItemStyle`, `clearItemStyles`, update `setItems` to reset itemStyles, and update `paint()` to render dual gradients, custom primary/secondary halos, and unselected text tinting.
- [x] **Step 1.3**: Verify compilation with `cmake --build build --config Release --target TheKlangFarmer`.

---

### Phase 2: The Klang Planter UI Wiring (`source/PlanterEditor.cpp`) — [x] COMPLETED
- [x] **Step 2.1**: Update `carrierDepthSlider` accent to `colCyan` in `setupKnob` and `cardCarrier->setKnob(2, "Mod Depth", &carrierDepthSlider, colCyan);`.
- [x] **Step 2.2**: Configure `pitchEnvTargetSelector` per-button item styles:
  - Index 0 (`Car`): `{ colRed, std::nullopt, std::nullopt }`
  - Index 1 (`Mod`): `{ colCyan, std::nullopt, std::nullopt }`
  - Index 2 (`Both`): `{ colCyan, colRed, colRed }`
  - Index 3 (`Opp`): `{ colRed, colCyan, colCyan }`
- [x] **Step 2.3**: Update `filterEnvDriveSlider` accent to `colBlue` in `setupKnob` and `cardFilterEnv->setKnob(3, "Pre-Filter Drive", &filterEnvDriveSlider, colBlue);`.
- [x] **Step 2.4**: Update Knob 4 in `cardAmp` from `"Velocity"` to `"Vel Min Level"`.
- [x] **Step 2.5**: Verify compilation of `TheKlangPlanter_Standalone` and `TheKlangPlanter_VST3`.

---

### Phase 3: Specification Synchronization & Visual Verification (`spec.md`) — [x] COMPLETED
- [x] **Step 3.1**: Update Section 6 in `spec.md` with revised ASCII rack diagram and color/label notes.
- [x] **Step 3.2**: Run `dsp_tests.exe` (confirm 33 passing tests) and `capture_planter_screenshot.exe`.
- [x] **Step 3.3**: Inspect generated screenshot to visually confirm colors, dual halos, and labels.

---

### Phase 4: Stage, Commit & Linear Sync — [x] COMPLETED
- [x] **Step 4.1**: Stage all modified files in git.
- [x] **Step 4.2**: Commit with conventional commit message: `feat(planter): update ui color routing, dual-accent destination buttons, and vel min label (THE-7)`.
- [x] **Step 4.3**: Sync completion comment to Linear ticket `THE-7`.
