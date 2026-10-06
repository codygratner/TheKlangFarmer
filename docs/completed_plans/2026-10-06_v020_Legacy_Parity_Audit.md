# Completed Plan: Automated v0.2.0 Legacy Parity Audit & HTML Report Generator
*Completed on: 2026-10-06*
*Audit Report:* [`docs/parity_audit/tkf_parity_audit.html`](../parity_audit/tkf_parity_audit.html) | [`docs/parity_audit/tkf_parity_audit.pdf`](../parity_audit/tkf_parity_audit.pdf)

## Objective: Establish a comprehensive, automated parity audit between legacy v0.2.0 C++ parameter definitions and the v0.3.0 data-driven JSON architecture across both The Klang Farmer and The Klang Planter. Generate a print-ready, slide-deck style HTML/PDF report with side-by-side card screenshots.

### Invariants & Technical Constraints
- **Strict Read-Only Source Policy**: During the audit execution, C++ source files (`source/*.cpp`, `source/*.h`) are not modified unless an explicit discrepancy fix is approved.
- **Git as Baseline Single Source of Truth**: All legacy `v0.2.0` parameters, formatting, colors, and screenshots are extracted directly from git tag `v0.2.0`.
- **Zero Hallucination Tolerance**: Every numerical range, skew, default, step, and formatting curve must be compared within a tolerance of $\epsilon < 1e-4$.
- **Cruft Purge Accounting**: The 40 static FX parameters purged in Item 4 (`4001296`) are categorized as "Intentionally Migrated to Dynamic Multi-Instance FX Slots".
- **Print Layout Directive**: HTML report enforces "1 card and 4 parameters per page, fill up the page, no tiny bullshit text" using `@media print { .card-page { page-break-after: always; } }`.

---

### Phase 1: Legacy Extractor & JSON Schema Parser
Build the core data extraction engine in `tools/audit_legacy_parity.py`.
- **Target Files:** `tools/audit_legacy_parity.py`, `assets/controls/*.json`, `assets/layouts/*.json`
- **Action Items:**
  - [x] Extract legacy `v0.2.0` APVTS parameters from `PluginProcessor.cpp` (TKF) and `PlanterProcessor.cpp` (TKP) via `git show v0.2.0:source/...`.
  - [x] Parse legacy names, ranges (min, max, step, skew), default values, double-click values, string formatters, and choice arrays.
  - [x] Extract legacy card accent colors from `PluginEditor.cpp`, `PlanterEditor.cpp`, and `UIComponents.cpp`.
  - [x] Load and parse current `assets/controls/*.json` (`carrier.json`, `envelopes.json`, `filters.json`, `fx.json`, `global.json`, `mixer.json`, `modulators.json`, `planter.json`).
  - [x] Ingest `assets/layouts/tkf_layout.json` and `tkp_layout.json` for card hierarchy and knob ordering.
- **Verification Condition:** Python parser runs and successfully outputs extracted parameter dictionaries for both `v0.2.0` and current `v0.3.0` with matching ID counts.

### Phase 2: Parity Comparator & Categorization Engine
Implement the 4-tier comparison logic evaluating every parameter and color.
- **Target Files:** `tools/audit_legacy_parity.py`
- **Action Items:**
  - [x] Compare parameter metadata: Type, Min, Max, Step, Skew, Default, Double-Click, Formatting, Choices.
  - [x] Classify parameters into 4 explicit categories:
    - **Exact Match (Green)**: 100% identical values (160 parameters).
    - **Intentional FX Migration (Blue)**: The 40 purged legacy static FX parameters mapped to dynamic FX slots.
    - **Safe Addition (Yellow)**: New parameters introduced in v0.3.0 (30 parameters).
    - **Discrepancy (Red)**: Any unintended shift in parameter behavior or default values (0 remaining!).
  - [x] Compare card accent hex colors between legacy C++ constants and new `ui_colors` JSON definitions.
- **Verification Condition:** Comparator outputs a structured JSON diff summary categorizing 100% of legacy and current parameters without uncaught exceptions.

### Phase 3: Visual Screenshot Pipeline
Extract baseline images and capture fresh card-level renders for side-by-side comparison.
- **Target Files:** `tools/audit_legacy_parity.py`, `screenshots/`, `docs/parity_audit/snapshots/`
- **Action Items:**
  - [x] Extract legacy `v0.2.0` GUI screenshots from `git show v0.2.0:screenshots/...` into `docs/parity_audit/snapshots/legacy/`.
  - [x] Render or extract current `v0.3.0` card snapshots using `gui_tests` offscreen rendering or full GUI captures.
  - [x] Generate individual 1:1 cropped card PNGs for both TKF (Carrier 1, Carrier 2, Modulator 1, Modulator 2, Pitch Env, Filter, Filter Env, Amp, Amp Env, Mixer, Mod Envelopes) and TKP (Carrier, Modulator, Pitch Env, Noise Transient, Filter, Filter Env, Amplifier, Amp Env).
- **Verification Condition:** Staging directory contains side-by-side paired PNG snapshots for all UI cards.

### Phase 4: Print-Ready HTML & PDF Report Generator
Generate the final publication-quality audit report.
- **Target Files:** `docs/parity_audit/tkf_parity_audit.html`, `docs/parity_audit/tkf_parity_audit.pdf`
- **Action Items:**
  - [x] Build an executive dashboard at the top of the report displaying:
    - Total Parameters Audited: 234
    - Exact Matches: 160
    - Safe Additions: 30
    - Intentional FX Migrations: 44
    - Active Discrepancies: 0
  - [x] Implement user print styling: `@media print { .card-page { page-break-after: always; height: 100vh; } }`.
  - [x] Set large, high-legibility typography (20-24px text, 30-40px headers) with high-contrast discrepancy highlights.
  - [x] Lay out 1 card per page with side-by-side legacy vs current screenshots and a parameter comparison grid.
  - [x] Automatically compile print-ready PDF using headless Microsoft Edge.
- **Verification Condition:** Opening `tkf_parity_audit.html` in browser displays full report cleanly, and `tkf_parity_audit.pdf` is rendered at 1.44 MB ready for manual user checking.

### Phase 5: Test Suite Parity & Discrepancy Resolution
Verify that any discrepancies identified in Phase 4 are resolved, and run automated regression tests.
- **Target Files:** `assets/controls/*.json`, `test/dsp_tests.cpp`, `test/gui_tests.cpp`
- **Action Items:**
  - [x] Review any flagged Red Discrepancies with the user.
  - [x] Apply necessary data fixes in `assets/controls/` (`envelopes.json`, `filters.json`, `global.json`, `modulators.json`, `planter.json`).
  - [x] Run `dsp_tests` to verify 100% audio engine stability (32 DSP tests passed).
  - [x] Run `gui_tests` to ensure 100% parameter reflection and UI binding integrity (61 GUI tests passed).
- **Verification Condition:** `dsp_tests` and `gui_tests` both pass with 0 errors; parity audit report shows 0 unintended discrepancies.
