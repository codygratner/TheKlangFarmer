# Implementation Plan: v0.2.0 Parity Audit

## Goal Description
To ensure the transition to the new JSON-driven architecture is 100% transparent to end-users, we need a comprehensive audit comparing the hardcoded C++ parameters, text formatting, and colors of `v0.2.0` against the current `assets/controls/*.json` data. 

Because manual visual inspection of 50+ parameters across two git branches is prone to human error, we will build an automated Python script that does the heavy lifting and generates a beautiful, printable PDF report highlighting **only the differences**.

## Proposed Strategy

### 1. The Automated Auditing Script
We will create a temporary Python tool: `tools/audit_legacy_parity.py`.
This script will:
1. **Extract Legacy Data**: Use `git show v0.2.0:source/PluginProcessor.cpp` and `UIComponents.cpp` to regex-parse the legacy `APVTS` defaults, `getTextFromValue` string formatters, tooltips, and `juce::Colour` hex codes.
2. **Extract Current Data**: Load all current JSON files in `assets/controls/`.
3. **Compare & Filter**: Cross-reference every parameter. If a parameter's Type, Range, Default, Double-Click, Formatting, and Colors are 100% identical, it will be silently dropped from the report to reduce noise.
    - **Exclusion Rule (Safe Additions)**: If a JSON array is a pure *superset* of the legacy array (e.g., the Filter Type choices match perfectly, but the JSON has a new `"APF"` tacked onto the end), this will be explicitly ignored. Appending new choices does not break backward compatibility for existing `0.2.0` save states, so it will not be flagged as a discrepancy.
4. **Highlight Differences**: Any discrepancies will be flagged for injection into the report.

### 2. PDF Generation via HTML Print CSS
To achieve your exact layout request ("1 card and 4 parameters per page, fill up the page, no tiny bullshit text"), the Python script will output a highly styled `tkf_parity_audit.html` file utilizing CSS Print Media Queries.

**CSS Layout Rules:**
- `@media print { .card-container { page-break-after: always; } }` guarantees exactly one Card per printed page.
- Massive font sizes (`font-size: 24px`) to fill the page.
- Differences will be wrapped in `<mark style="background-color: #ffcccc; font-weight: bold;">` to immediately draw your eye.

You will simply open the HTML file in Chrome/Edge, hit **Print -> Save as PDF**, and you'll have your perfect, readable audit document.

## Data Structure Additions
You requested a specific table format. I have expanded it slightly to ensure we capture string formatting and tooltips, which are critical for the 1:1 user experience:

| Card | Parameter (1..4): [Name] |
| :--- | :--- |
| **Type** | Legacy: `[Type]` <br> Current: `[Type]` |
| **Rendered Range** | Legacy: `[e.g., 20 Hz to 24 kHz]` <br> Current: `[e.g., 20 Hz to 24 kHz]` |
| **Default Value** | Legacy: `[Value]` <br> Current: `[Value]` |
| **Double Click** | Legacy: `[Value]` <br> Current: `[Value]` |
| **Rendered Examples** | Legacy: `[e.g., 50% / +3.0 dB]` <br> Current: `[e.g., 50% / +3.0 dB]` |
| **Colors (Hex)** | Legacy: `[0xAARRGGBB]` <br> Current: `[0xAARRGGBB]` |
| **Tooltip / Label** | Legacy: `[Tooltip Text]` <br> Current: `[Tooltip Text]` |

*(Note: The table will vertically stack inside the HTML grid to maximize readability, and identical rows will be collapsed or hidden).*

## Next Steps
This plan is queued up. As soon as you are completely satisfied with the completion of **The Klang Editor (TKE)**, let me know and I will immediately write the Python script, run the extraction, and hand you the final HTML/PDF report to review!
