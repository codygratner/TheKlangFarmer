# Implementation Plan: The Klang Editor CalloutBox Preview & Live Theming Harness

## Goal
Integrate visual inspection and live JSON-driven theming for modal CalloutBoxes (Master Limiter callout and Slider Modulation callout) directly into **The Klang Editor**, backed by automated GUI test coverage in `test/EditorTestSuite.h`.

---

## Technical Architecture & Implementation Phases

### Phase 1: Callout Styling Schema (`assets/controls/global_ui.json`)
- Add a `"callout_styles"` object to `assets/controls/global_ui.json` with dedicated schemas:
  - `"planter_limiter"`:
    - `title`: `"MASTER LIMITER"`
    - `width`: `300`
    - `height`: `130`
    - `background_colour`: `"0xff1e222b"`
    - `border_colour`: `"0xffe53935"`
    - `corner_radius`: `6.0`
  - `"slider_modulation"`:
    - `title`: `"PARAM MODULATION"`
    - `width`: `260`
    - `height`: `140`
    - `background_colour`: `"0xff1a1e24"`
    - `border_colour`: `"0xff00d2ff"`
    - `corner_radius`: `5.0`
- Maintain 100% JSON validity and formatting.

---

### Phase 2: The Klang Editor Integration (`tools/editor/MainComponent.h`, `tools/editor/MainComponent.cpp`)
- **Tree Population (`buildTree()`)**:
  - In `buildTree()`, append a `"Callouts & Overlays"` root category node to the tree.
  - Add child `EditorTreeItem` nodes:
    - `"Planter Master Limiter"` (`itemType = "callout_preview"`, `cardId = "planter_limiter"`, `productId = "tkp"`)
    - `"Slider Modulation & Snaps"` (`itemType = "callout_preview"`, `cardId = "slider_modulation"`, `productId = "tkf"`)
- **Inspection & Preview (`onTreeItemSelected()`, `syncJsonToPreview()`)**:
  - When an `itemType == "callout_preview"` item is selected:
    - Load the style properties from `global_ui.json` into `formEditor` (Width, Height, Background Color, Border Color, Corner Radius).
    - In `previewWrapper`, clear previous children and instantiate a centered preview card displaying:
      - Stylized header badge with title and accent border.
      - Mock rotary sliders / toggle controls matching the callout geometry.
      - Real-time response to color/dimension property changes.

---

### Phase 3: Automated GUI Test Coverage (`test/EditorTestSuite.h`)
- Expand `EditorTestSuite::runSuite(reporter)`:
  - Verify the `"Callouts & Overlays"` node is present in the tree hierarchy.
  - Programmatically simulate selecting both callout items.
  - Assert that `previewWrapper` is populated with child components.
  - Assert that `formEditor` exposes the expected styling properties.
  - Perform an offscreen smoke paint check on `previewWrapper` to verify zero division-by-zero or clip bounds crashes.

---

### Phase 4: Compilation, Validation & Deployment
- Build `TheKlangEditor` and `gui_tests` targets in Release configuration.
- Run `gui_tests.exe` and confirm 100% assertions pass with 0 failures.
- Deploy updated `The Klang Editor.exe` via `deploy.ps1`.
- Update `docs/communique/build_to_plan.md` with completion report.

---

## Acceptance Criteria
- [x] `"callout_styles"` added cleanly to `assets/controls/global_ui.json`.
- [x] The Klang Editor displays Callout preview items in its navigation tree.
- [x] Selecting a callout item displays its live preview card and property inspector.
- [x] `EditorTestSuite` validates callout tree navigation and rendering.
- [x] All `gui_tests` pass with zero failures.
