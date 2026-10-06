# Bugfix Plan: Planter Limiter Callout Typography, Short Labels & Formatted Values

**Goal:** Eliminate ugly truncated text and unlabelled knobs in the Planter Limiter CalloutBox:
1. Increase callout dimensions from `260x110` to `300x130` for comfortable knob breathing room.
2. In `PlanterLimiterCalloutComponent::paint()`, draw high-contrast, bold column header labels above each control:
   - Column 1: `LIMIT` (above Enable selector)
   - Column 2: `GAIN` (above Gain knob)
   - Column 3: `CEIL` (above Ceiling/Threshold knob)
   - Column 4: `REL` (above Release knob)
3. Attach concise `customFormatText` lambdas to all 3 knobs:
   - `gainSlider`: Decibels with sign, e.g. `+4.0 dB`, `0.0 dB`, `+12.0 dB` ($\le 8$ chars)
   - `threshSlider`: Decibels down from zero or percent, e.g. `0.0 dB`, `-0.2 dB`, `-3.0 dB` ($\le 7$ chars)
   - `releaseSlider`: Milliseconds, e.g. `150 ms`, `630 ms` ($\le 6$ chars)
4. Ensure text boxes never display ellipses and have clean, readable two-line visual separation (Label on top line, formatted value on bottom text box).

**Recommended Model & Thinking Budget:** Tier 2 — Gemini 3.8 Flash (Thinking: High)  
**Quota Impact:** 🟢 SUSTAINABLE (Economical — standard UI & test engineering)  

---

## Phase 1: Callout Geometry & Column Header Labels (`source/PlanterEditor.cpp`)
- [x] In `PlanterLimiterCalloutComponent`:
  - Component dimensions: `setSize(300, 130)`.
  - In `paint(juce::Graphics& g)`:
    - Paint column header labels above each control in bold font (`11.0f` bold, `0xffcfd8dc`):
      - Column 1: `LIMIT` centered above `enableSelector`.
      - Column 2: `GAIN` centered above `gainSlider`.
      - Column 3: `CEIL` centered above `threshSlider`.
      - Column 4: `REL` centered above `releaseSlider`.
  - In `resized()`:
    - Set `topY = 46` for header labels.
    - Layout:
      - `enableSelector`: X=10, Y=46, W=46, H=74.
      - 3 knobs: X starting at 64, W=72 each, gap=(300 - 64 - 216) / 3 = 6px.
      - Ample room for 72px wide knobs with zero text box clipping!

---

## Phase 2: Short & Punchy Value Formatters
- [x] In `PlanterLimiterCalloutComponent` constructor:
  - Wire `gainSlider.customFormatText`: `formatLimiterGain` (`+4.0 dB`, `0.0 dB`, etc.)
  - Wire `threshSlider.customFormatText`: `formatLimiterThresh` (`0.0 dB`, `-3.0 dB`, etc.)
  - Wire `releaseSlider.customFormatText`: `formatLimiterRelease` (`50 ms`, `150 ms`, etc.)
  - Wire custom parser lambdas for double-click/manual numeric entry (`parseLimiterGain`, `parseLimiterThresh`, `parseLimiterRelease`).
  - Wire `limiterGainSlider`, `limiterThreshSlider`, `limiterReleaseSlider` on Card 6.
  - Enhanced `RotaryKnobSlider::paint` to cleanly center value strings when `label.isEmpty()`.

---

## Phase 3: Headless Verification & Deployment
- [x] Run `build\Debug\gui_tests.exe` to assert all 177 tests pass.
- [x] Rebuild Release standalone & VST3 and deploy to `current_build\` and `C:\Program Files\Common Files\VST3\`.
- [x] Output the `🔔 JOB'S DONE!` chime for New Klang City.
