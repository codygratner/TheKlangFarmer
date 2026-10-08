---
name: visual-grill-me
description: Conducts an interactive architectural & UI design interview paired with a live-updating, side-by-side HTML+CSS visual mockup artifact in the side panel. Triggers on `/grilllab`, `/grill-lab`, `/vlab`, `/vrill`, `/vrillme`, `/ggrill`, `/visualgrillme`, `/visual-grill-me`, `/grill-visual`, "grill lab", "visual grill me", or "visual interview".
---

# Visual Grill Me — Interactive Design Interview with Live HTML Sidecar

## Goal
Conduct a rigorous, interactive design interview (`/grill-me`) for UI/UX, layouts, and interactive components while maintaining an interactive, live-updating HTML+CSS visual artifact in the IDE's side panel. For each question asked via `ask_question`, the visual artifact immediately updates to render real-time comparative mockups of the proposed options (Option A vs Option B vs Option C) using Tailwind CSS and interactive widgets.

---

## Operational Guardrails & Architecture
1. **Tier 2 Flash High Universal Baseline**:
   - Visual interviews, UI layout brainstorming, and HTML artifact authoring strictly use **Gemini 3.8 Flash (Thinking: High)**. Zero Pro quota burn required.
2. **Persistent Sidecar Artifact**:
   - The primary visual sidecar is written to: `<appDataDir>\brain\<conversation-id>\visual_grill_me_preview.html`.
   - Always provide `ArtifactMetadata` with `UserFacing: true` and a clear summary.
3. **Interactive Comparative Mockups**:
   - Each interview question must correspond to a distinct visual section or interactive toggle in the HTML artifact.
   - Mockups must use actual interactive controls (draggable sliders, clickable buttons, real color swatches, hover states) styled with Tailwind CSS and JetBrains Mono typography.
4. **Web Worker & State Synchronization Architecture**:
   - Pair `visual_grill_me_preview.html` with a companion state token `visual_grill_me_state.json` (tracking `step`, `version`, `timestamp`).
   - Use an inline Blob Web Worker (`new Worker(URL.createObjectURL(blob))`) to monitor state and heartbeat pings off the main UI thread. Never use unthrottled `document.lastModified` loops.
   - Provide a prominent **Manual Refresh Button** (`[ ↻ Refresh View ]`) and `Ctrl+R` / `r` shortcut in the page header.
   - Include an **Auto-Sync toggle** and a non-intrusive in-page toast banner (`#update-toast`) when new questions or layout updates arrive from chat.
5. **Interactive Modal Required**:
   - Use `ask_question` to pose design options to the user, with option text describing the user's choice and clear recommendations prefixed with `(Recommended)`.

---

## Workflow Steps

### Step 1: Initialize the Visual Canvas
1. Create `visual_grill_me_preview.html` in the conversation artifact directory.
2. Set up the baseline chassis container matching the active product aesthetic (e.g. Anthracite `#0d1117`, JetBrains Mono font, 1px hairline borders).
3. Present the initial state and provide the user with the clickable file URI to open in the side pane.

### Step 2: Pose Visual Questions & Iterate Mockups
1. For each design topic (e.g., Slider Styling, Header Layout, Modulation Cables):
   - Update `visual_grill_me_preview.html` to render the comparative options (Option A, Option B, Option C) side-by-side with live interaction.
   - Immediately invoke `ask_question` presenting the architectural tradeoffs, industry standards (Bitwig, Phase Plant, Vital, Ableton), and ergonomics.
2. Wait for the user's response from the interactive modal.

### Step 3: Bake In the Chosen Design & Advance
1. Highlight the chosen design in the HTML artifact (adding an "APPROVED ✅" badge and active glowing border).
2. Document the decision in the working notes or implementation plan (`PLAN.md`).
3. Move to the next component question until the interface is fully designed.

### Step 4: Final Blueprint Synthesis
1. Conclude the interview by synthesizing the approved design decisions into exact drop-in C++ component structures, APVTS parameter layouts, or JSON schemas.
2. Transition to Klang Industries for building.
