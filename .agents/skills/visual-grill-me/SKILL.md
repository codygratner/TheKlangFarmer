---
name: visual-grill-me
description: Conducts an interactive architectural & UI design interview paired with a live-updating, side-by-side HTML+CSS visual mockup artifact in the side panel. Triggers on `/grilllab`, `/grill-lab`, `/vlab`, `/vrill`, `/vrillme`, `/ggrill`, `/visualgrillme`, `/visual-grill-me`, `/grill-visual`, "grill lab", "visual grill me", or "visual interview".
---

# Visual Grill Me — Interactive Design Interview with Live HTML Sidecar

## Goal
Conduct a rigorous, interactive design interview (`/grill-me`) for UI/UX, layouts, and interactive components while maintaining an interactive, live-updating HTML+CSS visual artifact in the IDE's side panel. For each question asked via `ask_question`, the visual artifact immediately updates to render real-time comparative mockups of the proposed options (Option A vs Option B vs Option C) using Tailwind CSS and interactive widgets.

---

## Operational Guardrails & Architecture
1. **Frictionless Entry & Prominent Chat Link (MANDATORY)**:
   - If the user invokes `/grill-me` without specifying a mode, briefly ask if they want **Grill Lab** (live visual artifact) or **Classic Grill** (text-only).
   - If the user invokes `/grilllab`, `/vlab`, `/vrill`, etc., enter Grill Lab immediately.
   - **Always output a prominent, clickable file URI link in the chat response on kickoff**:
     `👉 **[Open Grill Lab Canvas](file:///<appDataDir>/brain/<conversation-id>/visual_grill_me_preview.html)**`
     so the user can immediately open it in the side panel with one click.
2. **Universal Scope & Dual Operating Modes**:
   - **UI Sandbox Mode**: Used for synth controls, sliders, cards, and meters. Displays real-time Web Audio audition synth, canvas oscilloscope, audio tick clicks, and theme swatches.
   - **Architecture & Knowledge Mode (Antigravity Dark Blue)**: Used for non-UI topics (documentation, schemas, ADRs, indexing, pipeline workflows). Strictly eliminates neon glows and fluorescent accents. Matches the native Antigravity IDE dark blue-tinged slate theme (Body: `#0e121a`, Cards: `#151b26`, Recessed: `#10141e`, Borders: 1px `#222c3d`, Accents: clean IDE blue `#3b82f6` and soft slate `#8b9cb5`). Uses clean Inter typography for prose and monospace for code. Automatically hides audio scope and synth widgets to maximize reading comfort.
3. **Conversational Interleaving & Live Tuning (CRITICAL)**:
   - The interview is never a rigid, lock-step rail. The user can pause anytime in chat to ask side questions, request visual/font/theming adjustments to the sidecar, or provide extra background context.
   - The agent responds directly to side questions and updates the canvas live, keeping the active question and decision state perfectly preserved.
4. **The "BAR-B-Q&A" Spirit**:
   - Also known affectionately as the **BAR-B-Q&A**! Keep the session engaging, visual, collaborative, and fun.
5. **Tier 2 Flash High Universal Baseline**:
   - Visual interviews, UI layout brainstorming, and HTML artifact authoring strictly use **Gemini 3.8 Flash (Thinking: High)**. Zero Pro quota burn required.
6. **Persistent Sidecar Artifact**:
   - The primary visual sidecar is written to: `<appDataDir>\brain\<conversation-id>\visual_grill_me_preview.html`.
   - Always provide `ArtifactMetadata` with `UserFacing: true` and a clear summary.
7. **Interactive Comparative Mockups & Sidecar Composer**:
   - Each interview question must correspond to a distinct visual section with live interactive controls styled with Tailwind CSS.
   - Provide an in-page **Decision & Write-In Composer** allowing the user to select options, write custom notes, and copy formatted markdown answers directly to their clipboard (`Ctrl+V` in chat).
8. **Web Worker & State Synchronization Architecture**:
   - Pair `visual_grill_me_preview.html` with a companion state token `visual_grill_me_state.json` (tracking `step`, `version`, `timestamp`).
   - Use an inline Blob Web Worker (`new Worker(URL.createObjectURL(blob))`) to monitor state and heartbeat pings off the main UI thread. Never use unthrottled `document.lastModified` loops.
   - Provide a prominent **Manual Refresh Button** (`[ ↻ Refresh View ]`) and Font Zoom Scaler (`[A-] [100%] [A+]`) in the page header.
   - Include an **Auto-Sync toggle** and a non-intrusive in-page toast banner (`#update-toast`) when new questions or layout updates arrive from chat.
9. **Modal-Free Flexibility**:
   - During Grill Lab sessions, prefer presenting questions in chat text and letting the user interact with the Sidecar Composer, avoiding modal deadlocks and keeping the IDE completely unlocked.

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
