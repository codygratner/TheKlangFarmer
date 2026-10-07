# Architecture Plan: Editor GUI Test Suite & Tree UX (v0.3.1)

**Goal:** Expand `gui_tests` to fully validate `The Klang Editor` through headless component testing, and improve the Tree View's user experience with global and contextual expand/collapse controls.

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.1 Pro (Medium) or Gemini 3.8 Flash (High). Balanced reasoning for JUCE UI wiring and unit test assertions.

## Phase 1: Tree View UX Upgrades
- [x] **Global Toolbar**: 
  - Add a small `juce::Toolbar` (or equivalent layout) above the Tree View component in the Editor.
  - Add two icon buttons: `Expand All` and `Collapse All`.
  - Wire them to iterate and expand/collapse all root and child nodes in the active `juce::TreeView`.
- [x] **Contextual Menu**:
  - Implement a right-click `juce::PopupMenu` on the Tree View items.
  - Add options: `Expand All`, `Collapse All`, and `Collapse Others`.
  - `Collapse Others`: Iterate all sibling/peer nodes at the same depth as the clicked node and collapse them, leaving only the clicked branch open.

## Phase 2: Editor GUI Test Suite Expansion
- [x] **Test Definition**: Add a new `juce::UnitTest` module for the Editor in `test/gui_tests.cpp`.
- [x] **Headless Data Sync Test**:
  - Programmatically instantiate the Editor's Tree View model and the target Property/Inspector component.
  - Iterate through 100% of the loaded JSON node items.
  - Simulate a `juce::TreeView` selection event for each node.
  - Assert that the central Property/Inspector component correctly receives the active JSON object and reflects the right parameter keys.
- [x] **Null/Crash Safety Test**:
  - Simulate selecting root nodes, empty folders, and invalid bounds to ensure the Editor does not crash or throw memory exceptions.

## Phase 3: Test Parity Verification
- [x] Build the project and run `gui_tests`. 
- [x] Ensure the tests catch any broken linkages between the Tree and the Inspector before marking complete.
