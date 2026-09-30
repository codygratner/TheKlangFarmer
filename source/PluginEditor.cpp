#include "PluginEditor.h"

// --- SAFE PARSING & FORMATTING HELPERS ---

static double parseNumberSafe(const juce::String& text, double fallback) {
    auto trimmed = text.trim();
    if (trimmed.isEmpty()) return fallback;

    // Check for note names like C4, A1, F#2, Bb3
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int oct = -2; oct <= 8; ++oct) {
        for (int semi = 0; semi < 12; ++semi) {
            juce::String noteStr = noteNames[semi] + juce::String(oct);
            if (trimmed.startsWithIgnoreCase(noteStr)) {
                int noteNum = (oct + 1) * 12 + semi;
                return static_cast<double>(std::clamp(noteNum, 0, 127));
            }
        }
    }

    // Bracketed note number like [33]
    int bStart = trimmed.indexOfChar('[');
    int bEnd = trimmed.indexOfChar(']');
    if (bStart >= 0 && bEnd > bStart) {
        auto inside = trimmed.substring(bStart + 1, bEnd).trim();
        if (inside.isNotEmpty()) {
            double v = inside.getDoubleValue();
            if (std::isfinite(v)) return v;
        }
    }

    bool isKhz = trimmed.containsIgnoreCase("k");

    juce::String numStr;
    bool hasDot = false;
    for (int i = 0; i < trimmed.length(); ++i) {
        juce::juce_wchar ch = trimmed[i];
        if (std::isdigit(ch)) {
            numStr += ch;
        } else if (ch == '-' || ch == '+') {
            if (numStr.isEmpty()) numStr += ch;
        } else if (ch == '.' && !hasDot) {
            numStr += ch;
            hasDot = true;
        } else if (!numStr.isEmpty() && !std::isspace(ch)) {
            break;
        }
    }

    if (numStr.isEmpty()) return fallback;
    double val = numStr.getDoubleValue();
    if (!std::isfinite(val)) return fallback;

    if (isKhz && std::abs(val) < 1000.0) {
        val *= 1000.0;
    }

    return val;
}

static juce::String getMidiNoteName(int noteNumber) {
    noteNumber = std::clamp(noteNumber, 0, 127);
    const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (noteNumber / 12) - 1;
    return juce::String(names[noteNumber % 12]) + juce::String(octave) + " [" + juce::String(noteNumber) + "]";
}

// --- ROTARY KNOB LOOK AND FEEL ---

RotaryKnobLookAndFeel::RotaryKnobLookAndFeel() {
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff161922));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff00d2ff));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xffe8edf5));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe8edf5));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff12141a));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2c3240));

    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff14161d));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2c3240));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8b95a8));

    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1a1d26));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe8edf5));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff00d2ff));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xff0f1115));
}

juce::Label* RotaryKnobLookAndFeel::createSliderTextBox(juce::Slider& slider) {
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        return new DiagramSliderLabel(*rks);
    }
    auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::FontOptions(11.5f, juce::Font::bold));
    l->setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xee11141a));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0x44303848));
    l->setJustificationType(juce::Justification::centred);
    return l;
}

void RotaryKnobLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal) {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    bool isBipolar = false;
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        isBipolar = rks->isBipolar;
    }

    // Outer track bounds
    float trackHeight = 6.0f;
    float trackY = static_cast<float>(y) + (static_cast<float>(height) - trackHeight) * 0.5f;
    float trackX = static_cast<float>(x) + 2.0f;
    float trackW = static_cast<float>(width) - 4.0f;
    juce::Rectangle<float> trackRect(trackX, trackY, trackW, trackHeight);

    // 1. Draw recessed track background
    g.setColour(juce::Colour(0xff161922));
    g.fillRoundedRectangle(trackRect, 3.0f);
    g.setColour(juce::Colour(0xff2a3040));
    g.drawRoundedRectangle(trackRect, 3.0f, 1.0f);

    // 2. Draw active value fill
    if (slider.isEnabled() && trackW > 2.0f) {
        float minX = trackX;
        float maxX = trackX + trackW;
        float clampedPos = std::clamp(sliderPos, minX, maxX);

        if (isBipolar) {
            float midX = trackX + trackW * 0.5f;
            float startX = std::min(midX, clampedPos);
            float endX = std::max(midX, clampedPos);
            float fillW = endX - startX;

            if (fillW > 0.5f) {
                juce::Rectangle<float> fillRect(startX, trackY + 1.0f, fillW, trackHeight - 2.0f);
                g.setColour(fillColour.withAlpha(0.85f));
                g.fillRoundedRectangle(fillRect, 2.0f);
            }

            // Center zero tick mark
            g.setColour(juce::Colour(0xff5a667d));
            g.drawVerticalLine(static_cast<int>(midX), trackY - 2.0f, trackY + trackHeight + 2.0f);
        } else {
            float fillW = clampedPos - minX;
            if (fillW > 0.5f) {
                juce::Rectangle<float> fillRect(minX + 1.0f, trackY + 1.0f, fillW - 1.0f, trackHeight - 2.0f);
                g.setColour(fillColour.withAlpha(0.85f));
                g.fillRoundedRectangle(fillRect, 2.0f);
            }
        }
    }

    // 3. Draw modern hardware fader thumb / handle
    float thumbW = 7.0f;
    float thumbH = std::min(static_cast<float>(height) - 2.0f, 16.0f);
    float thumbY = static_cast<float>(y) + (static_cast<float>(height) - thumbH) * 0.5f;
    float thumbX = std::clamp(sliderPos - thumbW * 0.5f, static_cast<float>(x), static_cast<float>(x + width) - thumbW);

    juce::Rectangle<float> thumbRect(thumbX, thumbY, thumbW, thumbH);

    // Subtle glow if mouse is over or dragging
    if (slider.isMouseOverOrDragging()) {
        g.setColour(fillColour.withAlpha(0.35f));
        g.drawRoundedRectangle(thumbRect.expanded(1.5f), 2.5f, 1.5f);
    }

    // Metallic thumb gradient
    juce::ColourGradient thumbGrad(juce::Colour(0xffeff3fa), thumbX, thumbY,
                                  juce::Colour(0xffb0bac9), thumbX, thumbY + thumbH, false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(thumbRect, 2.0f);

    // Thumb border
    g.setColour(juce::Colour(0xff12151c));
    g.drawRoundedRectangle(thumbRect, 2.0f, 1.0f);

    // Thumb center indicator line
    g.setColour(fillColour);
    g.drawVerticalLine(static_cast<int>(thumbRect.getCentreX()), thumbY + 2.5f, thumbY + thumbH - 2.5f);
}

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    // Well-proportioned dial radius for small knob footprint
    auto radius = juce::jlimit(11.0f, 22.0f, juce::jmin(bounds.getWidth() * 0.46f, bounds.getHeight() * 0.46f));
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = juce::jmax(2.4f, radius * 0.18f);
    auto arcRadius = radius - lineW * 0.5f;

    // Track background
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value Arc: Bipolar draws from center (12 o'clock); Unipolar draws from minimum (7 o'clock)
    bool isBipolar = false;
    if (auto* rks = dynamic_cast<RotaryKnobSlider*>(&slider)) {
        isBipolar = rks->isBipolar;
    }

    if (slider.isEnabled()) {
        juce::Path valueArc;
        if (isBipolar) {
            float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
            if (std::abs(toAngle - midAngle) > 0.005f) {
                if (toAngle >= midAngle) {
                    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                           0.0f, midAngle, toAngle, true);
                } else {
                    valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                           0.0f, toAngle, midAngle, true);
                }
                g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        } else {
            if (sliderPosProportional > 0.001f) {
                valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                       0.0f, rotaryStartAngle, toAngle, true);
                g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    // Inner dial disc
    auto innerRadius = arcRadius - lineW * 0.85f;
    if (innerRadius > 3.0f) {
        auto knobBounds = juce::Rectangle<float>(centre.x - innerRadius, centre.y - innerRadius,
                                                 innerRadius * 2.0f, innerRadius * 2.0f);
        juce::ColourGradient grad(juce::Colour(0xff2a2f3d), centre.x, centre.y - innerRadius,
                                  juce::Colour(0xff14161c), centre.x, centre.y + innerRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colour(0xff3b4354));
        g.drawEllipse(knobBounds, 1.0f);

        // Subtle glow when hovering or dragging
        if (slider.isMouseOverOrDragging()) {
            auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
            g.setColour(fillCol.withAlpha(0.30f));
            g.drawEllipse(knobBounds.expanded(1.5f), 1.0f);
        }

        // Pointer indicator
        juce::Path p;
        auto pointerLength = innerRadius * 0.68f;
        auto pointerThickness = juce::jmax(1.8f, innerRadius * 0.16f);
        p.addRoundedRectangle(-pointerThickness * 0.5f, -innerRadius + 1.0f, pointerThickness, pointerLength, 1.0f);
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.setColour(juce::Colour(0xffffffff));
        g.fillPath(p);
    }
}

// --- MINI OSCILLOSCOPE COMPONENT ---

MiniOscilloscopeComponent::MiniOscilloscopeComponent(juce::Colour traceColour)
    : traceCol(traceColour)
{
    setOpaque(false);
}

void MiniOscilloscopeComponent::updateData(const float* data, int numPoints) {
    if (!data || numPoints <= 0) return;
    points.assign(data, data + numPoints);
    repaint();
}

void MiniOscilloscopeComponent::setPlotMode(PlotMode mode) {
    if (plotMode != mode) {
        plotMode = mode;
        repaint();
    }
}

void MiniOscilloscopeComponent::updateFilterParams(int type, int slope, float cutoffHz, float resonance) {
    if (filterType != type || filterSlope != slope || filterCutoff != cutoffHz || filterResonance != resonance) {
        filterType = type;
        filterSlope = slope;
        filterCutoff = cutoffHz;
        filterResonance = resonance;
        repaint();
    }
}

void MiniOscilloscopeComponent::updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter) {
    if (eqFreq != freqHz || eqWidth != widthOct || eqGain != gainDb || eqDJ != djFilter) {
        eqFreq = freqHz;
        eqWidth = widthOct;
        eqGain = gainDb;
        eqDJ = djFilter;
        repaint();
    }
}

void MiniOscilloscopeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Dark background display
    g.setColour(juce::Colour(0xff0e1017));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle outline
    g.setColour(juce::Colour(0xff1d222e));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    if (plotMode == PlotMode::Oscilloscope) {
        // Centerline grid
        float midY = bounds.getCentreY();
        g.setColour(juce::Colour(0xff1b202c));
        g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

        if (points.empty()) return;

        // Draw waveform path
        juce::Path p;
        float dx = bounds.getWidth() / static_cast<float>(points.size() - 1);
        float halfH = (bounds.getHeight() - 4.0f) * 0.5f;

        for (size_t i = 0; i < points.size(); ++i) {
            float val = std::clamp(points[i], -1.5f, 1.5f);
            float y = midY - (val * (halfH / 1.15f));
            y = std::clamp(y, bounds.getY() + 1.0f, bounds.getBottom() - 1.0f);
            float x = bounds.getX() + static_cast<float>(i) * dx;

            if (i == 0) p.startNewSubPath(x, y);
            else p.lineTo(x, y);
        }

        // Glow line
        g.setColour(traceCol.withAlpha(0.25f));
        g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Crisp trace line
        g.setColour(traceCol);
        g.strokePath(p, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    // --- X-Y PLOT: AFFECTED FREQUENCIES (X, log) vs GAIN dB (Y) ---
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float left = bounds.getX();
    float top = bounds.getY();
    float right = bounds.getRight();
    float bottom = bounds.getBottom();

    // Log frequency scale: 20 Hz to 24000 Hz
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 24000.0f;
    float logRange = std::log(maxFreq / minFreq);

    auto freqToX = [&](float f) -> float {
        float norm = std::log(std::clamp(f, minFreq, maxFreq) / minFreq) / logRange;
        return left + norm * width;
    };

    // Draw subtle frequency grid lines at 100 Hz, 1 kHz, 10 kHz
    g.setColour(juce::Colour(0xff181c26));
    float x100 = freqToX(100.0f);
    float x1k  = freqToX(1000.0f);
    float x10k = freqToX(10000.0f);
    g.drawVerticalLine(static_cast<int>(x100), top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x1k),  top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x10k), top + 2.0f, bottom - 2.0f);

    // Subtle frequency axis markings
    g.setColour(juce::Colour(0xff333a4d));
    g.setFont(juce::FontOptions(7.5f));
    g.drawText("100", static_cast<int>(x100) - 10, static_cast<int>(bottom) - 10, 20, 9, juce::Justification::centred);
    g.drawText("1k",  static_cast<int>(x1k) - 8,   static_cast<int>(bottom) - 10, 16, 9, juce::Justification::centred);
    g.drawText("10k", static_cast<int>(x10k) - 10, static_cast<int>(bottom) - 10, 20, 9, juce::Justification::centred);

    // Compute Y mapping and curve path
    juce::Path curvePath;
    float yZero = top + height * 0.5f;

    if (plotMode == PlotMode::FilterXY) {
        // Filter: 0 dB line positioned at 35% height so negative dB roll-off has ample vertical room
        yZero = top + height * 0.35f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

        // Faint +12 dB line
        float yPlus12 = yZero - (12.0f / 36.0f) * (height * 0.55f);
        g.setColour(juce::Colour(0xff161a24));
        g.drawHorizontalLine(static_cast<int>(yPlus12), left + 2.0f, right - 2.0f);

        int numSteps = std::max(64, static_cast<int>(width));
        float fc = std::clamp(filterCutoff, 0.1f, 24000.0f);
        float qVal = 0.707f + filterResonance * 18.0f;
        if (filterSlope == 0) qVal = 0.5f + filterResonance * 5.0f;

        for (int i = 0; i < numSteps; ++i) {
            float u = static_cast<float>(i) / static_cast<float>(numSteps - 1);
            float f = minFreq * std::pow(maxFreq / minFreq, u);
            float px = left + u * width;

            float gainDb = 0.0f;
            if (filterType == 0) {
                // Off: flat 0 dB
                gainDb = 0.0f;
            } else {
                float r = f / fc;
                float d = std::sqrt((1.0f - r * r) * (1.0f - r * r) + (r / qVal) * (r / qVal));
                float mag = 1.0f;

                if (filterType == 1) { // LPF
                    if (filterSlope == 0)      mag = 1.0f / std::sqrt(1.0f + r * r);
                    else if (filterSlope == 1) mag = 1.0f / std::max(d, 1e-4f);
                    else if (filterSlope == 2) mag = (1.0f / std::max(d, 1e-4f)) * (1.0f / std::sqrt(1.0f + r * r));
                    else if (filterSlope == 3) mag = (1.0f / std::max(d, 1e-4f)) * (1.0f / std::max(d, 1e-4f));
                    else                       mag = std::pow(1.0f / std::max(d, 1e-4f), 3.0f);
                } else if (filterType == 2) { // BPF
                    float bMag = (r / qVal) / std::max(d, 1e-4f);
                    if (filterSlope <= 1) mag = bMag;
                    else                  mag = bMag * (1.0f / std::sqrt(1.0f + r * r));
                } else if (filterType == 3) { // HPF
                    if (filterSlope == 0)      mag = r / std::sqrt(1.0f + r * r);
                    else if (filterSlope == 1) mag = (r * r) / std::max(d, 1e-4f);
                    else if (filterSlope == 2) mag = ((r * r) / std::max(d, 1e-4f)) * (r / std::sqrt(1.0f + r * r));
                    else if (filterSlope == 3) mag = ((r * r) / std::max(d, 1e-4f)) * ((r * r) / std::max(d, 1e-4f));
                    else                       mag = std::pow((r * r) / std::max(d, 1e-4f), 3.0f);
                } else if (filterType == 4) { // BRF (Notch)
                    mag = std::abs(1.0f - r * r) / std::max(d, 1e-4f);
                }
                gainDb = 20.0f * std::log10(std::clamp(mag, 1e-4f, 16.0f));
            }

            float py = yZero - (gainDb / 36.0f) * (height * 0.55f);
            py = std::clamp(py, top + 1.0f, bottom - 1.0f);

            if (i == 0) curvePath.startNewSubPath(px, py);
            else curvePath.lineTo(px, py);
        }

        // Draw Cutoff indicator pip on the curve if active
        if (filterType != 0) {
            float xCutoff = freqToX(fc);
            g.setColour(traceCol.withAlpha(0.2f));
            g.drawVerticalLine(static_cast<int>(xCutoff), top + 2.0f, bottom - 2.0f);
        }
    } else if (plotMode == PlotMode::EqXY) {
        // Bell EQ: 0 dB line positioned at center
        yZero = top + height * 0.5f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

        // Faint +12 dB and -12 dB lines
        float yPlus12  = yZero - (12.0f / 24.0f) * (height * 0.42f);
        float yMinus12 = yZero + (12.0f / 24.0f) * (height * 0.42f);
        g.setColour(juce::Colour(0xff161a24));
        g.drawHorizontalLine(static_cast<int>(yPlus12), left + 2.0f, right - 2.0f);
        g.drawHorizontalLine(static_cast<int>(yMinus12), left + 2.0f, right - 2.0f);

        int numSteps = std::max(64, static_cast<int>(width));
        float f0 = std::clamp(eqFreq, 20.0f, 24000.0f);
        float bw = std::clamp(eqWidth, 0.1f, 10.0f);

        for (int i = 0; i < numSteps; ++i) {
            float u = static_cast<float>(i) / static_cast<float>(numSteps - 1);
            float f = minFreq * std::pow(maxFreq / minFreq, u);
            float px = left + u * width;

            // 1. Peaking Bell Response
            float octDist = std::log2(f / f0);
            float term = (2.0f * octDist) / bw;
            float bellDb = eqGain * (1.0f / (1.0f + term * term));

            // 2. DJ Filter Tilt Response
            float djDb = 0.0f;
            if (eqDJ < 0.49f) {
                float fCut = minFreq * std::pow(maxFreq / minFreq, eqDJ * 2.0f);
                djDb = 20.0f * std::log10(1.0f / std::sqrt(1.0f + (f / fCut) * (f / fCut)));
            } else if (eqDJ > 0.51f) {
                float fCut = minFreq * std::pow(maxFreq / minFreq, (eqDJ - 0.5f) * 2.0f);
                djDb = 20.0f * std::log10((f / fCut) / std::sqrt(1.0f + (f / fCut) * (f / fCut)));
            }

            float totalDb = bellDb + djDb;
            float py = yZero - (totalDb / 24.0f) * (height * 0.42f);
            py = std::clamp(py, top + 1.0f, bottom - 1.0f);

            if (i == 0) curvePath.startNewSubPath(px, py);
            else curvePath.lineTo(px, py);
        }

        // Draw EQ Bell center frequency marker dot
        float xPeak = freqToX(f0);
        float yPeak = yZero - (eqGain / 24.0f) * (height * 0.42f);
        yPeak = std::clamp(yPeak, top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol.withAlpha(0.25f));
        g.drawVerticalLine(static_cast<int>(xPeak), top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol);
        g.fillEllipse(xPeak - 2.5f, yPeak - 2.5f, 5.0f, 5.0f);
    }

    // Fill area between curve and 0 dB baseline
    juce::Path fillPath(curvePath);
    fillPath.lineTo(right, yZero);
    fillPath.lineTo(left, yZero);
    fillPath.closeSubPath();
    g.setColour(traceCol.withAlpha(0.12f));
    g.fillPath(fillPath);

    // Glow stroke
    g.setColour(traceCol.withAlpha(0.25f));
    g.strokePath(curvePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Crisp trace stroke
    g.setColour(traceCol);
    g.strokePath(curvePath, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// --- DIAGRAM SLIDER LABEL ---

DiagramSliderLabel::DiagramSliderLabel(RotaryKnobSlider& s)
    : slider(s)
{
    slider.addListener(this);
    setFont(juce::FontOptions(11.5f, juce::Font::bold));
    setJustificationType(juce::Justification::centred);
    setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    setColour(juce::Label::backgroundColourId, juce::Colour(0xee11141a));
    setColour(juce::Label::outlineColourId, juce::Colour(0x44303848));
}

DiagramSliderLabel::~DiagramSliderLabel() {
    slider.removeListener(this);
}

void DiagramSliderLabel::paint(juce::Graphics& g) {
    if (slider.diagramType == RotaryKnobSlider::DiagramType::None) {
        juce::Label::paint(g);
        return;
    }

    auto bounds = getLocalBounds().toFloat().reduced(1.5f, 1.0f);
    g.setColour(juce::Colour(0xee11141a));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colour(0x44303848));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    auto drawArea = bounds.reduced(5.0f, 2.5f);
    if (drawArea.getWidth() <= 4.0f || drawArea.getHeight() <= 4.0f) return;

    juce::Colour traceColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    float val = static_cast<float>(slider.getValue());

    if (slider.diagramType == RotaryKnobSlider::DiagramType::Waveform) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getCentreY()), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 32;
        for (int i = 0; i <= numPts; ++i) {
            float phase = static_cast<float>(i) / static_cast<float>(numPts);
            float waveY = TbdAudio::evaluateWaveform(phase, val);
            float px = drawArea.getX() + phase * drawArea.getWidth();
            float py = drawArea.getCentreY() - waveY * (drawArea.getHeight() * 0.44f);
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::EnvelopeSlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(1.0f - t, val);
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - y * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::VelocitySlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float y = TbdAudio::applyEnvelopeSlope(t, val);
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - y * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (slider.diagramType == RotaryKnobSlider::DiagramType::FilterSlope) {
        g.setColour(juce::Colour(0x22ffffff));
        g.drawHorizontalLine(static_cast<int>(drawArea.getBottom() - 1.0f), drawArea.getX(), drawArea.getRight());

        juce::Path p;
        constexpr int numPts = 24;
        float cutoffX = 0.55f;
        float exponent = 1.0f + val * 6.0f;
        for (int i = 0; i <= numPts; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numPts);
            float mag = 1.0f;
            if (t > cutoffX) {
                float f = (t - cutoffX) / (1.0f - cutoffX);
                mag = std::clamp(1.0f - std::pow(f, 1.0f / exponent), 0.0f, 1.0f);
            }
            float px = drawArea.getX() + t * drawArea.getWidth();
            float py = drawArea.getBottom() - mag * (drawArea.getHeight() * 0.88f) - 1.0f;
            if (i == 0) p.startNewSubPath(px, py);
            else p.lineTo(px, py);
        }
        g.setColour(traceColour);
        g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void DiagramSliderLabel::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        slider.openHoveringEditor();
        return;
    }
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseDown(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseDown(e);
}

void DiagramSliderLabel::mouseDrag(const juce::MouseEvent& e) {
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseDrag(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseDrag(e);
}

void DiagramSliderLabel::mouseUp(const juce::MouseEvent& e) {
    if (slider.diagramType != RotaryKnobSlider::DiagramType::None) {
        slider.mouseUp(e.getEventRelativeTo(&slider));
        return;
    }
    juce::Label::mouseUp(e);
}

void DiagramSliderLabel::mouseDoubleClick(const juce::MouseEvent& e) {
    slider.mouseDoubleClick(e);
}

void DiagramSliderLabel::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    slider.mouseWheelMove(e.getEventRelativeTo(&slider), wheel);
}

// --- ROTARY KNOB / HORIZONTAL SLIDER WITH RIGHT CLICK EDITING ---

RotaryKnobSlider::RotaryKnobSlider() {
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 20);
    setScrollWheelEnabled(true);
}

void RotaryKnobSlider::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (isScrollWheelEnabled()) {
        juce::Slider::mouseWheelMove(e, wheel);
    }
}

void RotaryKnobSlider::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        openHoveringEditor();
        return;
    }
    juce::Slider::mouseDown(e);
}

void RotaryKnobSlider::mouseDoubleClick(const juce::MouseEvent& e) {
    if (getDefaultValue) {
        setValue(getDefaultValue(), juce::sendNotificationAsync);
        return;
    }
    juce::Slider::mouseDoubleClick(e);
}

void RotaryKnobSlider::openHoveringEditor() {
    auto editor = std::make_unique<juce::TextEditor>();
    editor->setSize(96, 26);
    editor->setFont(juce::FontOptions(13.0f, juce::Font::bold));
    editor->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12141a));
    editor->setColour(juce::TextEditor::textColourId, juce::Colour(0xffffffff));
    editor->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff00d2ff));
    editor->setText(getTextFromValue(getValue()), false);
    editor->selectAll();

    auto* edRaw = editor.get();
    edRaw->onReturnKey = [this, edRaw]() {
        juce::String text = edRaw->getText().trim();
        double newVal = getValueFromText(text);
        setValue(newVal, juce::sendNotificationAsync);
        if (auto* callout = edRaw->findParentComponentOfClass<juce::CallOutBox>()) {
            callout->dismiss();
        }
    };
    edRaw->onEscapeKey = [edRaw]() {
        if (auto* callout = edRaw->findParentComponentOfClass<juce::CallOutBox>()) {
            callout->dismiss();
        }
    };

    juce::CallOutBox::launchAsynchronously(std::move(editor), getScreenBounds(), nullptr);
}

juce::String RotaryKnobSlider::getTextFromValue(double val) {
    if (customFormatText) {
        return customFormatText(val);
    }
    return juce::Slider::getTextFromValue(val);
}

double RotaryKnobSlider::getValueFromText(const juce::String& text) {
    if (customParseText) {
        return customParseText(text);
    }
    return juce::Slider::getValueFromText(text);
}

// --- LED SELECTOR COMPONENT ---

LedSelectorComponent::LedSelectorComponent(juce::Colour activeAccent)
    : accent(activeAccent)
{
    setOpaque(false);
}

void LedSelectorComponent::setItems(const juce::StringArray& newItems, int numColumns) {
    items = newItems;
    columns = std::max(1, numColumns);
    repaint();
}

void LedSelectorComponent::setSelectedIndex(int newIndex, juce::NotificationType notification) {
    if (newIndex >= 0 && newIndex < items.size() && newIndex != selectedIndex) {
        selectedIndex = newIndex;
        repaint();
        if (notification != juce::dontSendNotification && onChange) {
            onChange(selectedIndex);
        }
    }
}

juce::Rectangle<int> LedSelectorComponent::getItemBounds(int index) const {
    int n = items.size();
    if (n == 0 || index < 0 || index >= n) return {};
    int rows = (n + columns - 1) / columns;
    float colW = static_cast<float>(getWidth()) / static_cast<float>(columns);
    float rowH = static_cast<float>(getHeight()) / static_cast<float>(rows);

    int col = index % columns;
    int row = index / columns;
    return juce::Rectangle<int>(static_cast<int>(col * colW),
                                static_cast<int>(row * rowH),
                                static_cast<int>(colW),
                                static_cast<int>(rowH));
}

int LedSelectorComponent::getItemIndexAt(juce::Point<int> pos) const {
    int n = items.size();
    if (n == 0) return -1;
    int rows = (n + columns - 1) / columns;
    float colW = static_cast<float>(getWidth()) / static_cast<float>(columns);
    float rowH = static_cast<float>(getHeight()) / static_cast<float>(rows);

    int col = static_cast<int>(pos.x / colW);
    int row = static_cast<int>(pos.y / rowH);

    if (col < 0 || col >= columns || row < 0 || row >= rows) return -1;
    int idx = row * columns + col;
    if (idx >= 0 && idx < n) return idx;
    return -1;
}

void LedSelectorComponent::paint(juce::Graphics& g) {
    int n = items.size();
    if (n == 0) return;

    for (int i = 0; i < n; ++i) {
        auto r = getItemBounds(i).toFloat().reduced(2.0f, 1.0f);
        bool isSel = (i == selectedIndex);
        bool isHov = (i == hoveredIndex);

        if (isSel) {
            g.setColour(accent.withAlpha(0.14f));
            g.fillRoundedRectangle(r, 3.0f);
            g.setColour(accent.withAlpha(0.35f));
            g.drawRoundedRectangle(r, 3.0f, 1.0f);
        } else if (isHov) {
            g.setColour(juce::Colour(0x15ffffff));
            g.fillRoundedRectangle(r, 3.0f);
        }

        // Draw LED dot
        float ledSize = 6.0f;
        float ledX = r.getX() + 4.5f;
        float ledY = r.getCentreY() - ledSize * 0.5f;
        auto ledBounds = juce::Rectangle<float>(ledX, ledY, ledSize, ledSize);

        if (isSel) {
            g.setColour(accent.withAlpha(0.40f));
            g.fillEllipse(ledBounds.expanded(2.0f));
            g.setColour(accent);
            g.fillEllipse(ledBounds);
            g.setColour(juce::Colours::white);
            g.fillEllipse(ledBounds.reduced(1.5f));
        } else {
            g.setColour(juce::Colour(0xff232733));
            g.fillEllipse(ledBounds);
            g.setColour(juce::Colour(0xff394152));
            g.drawEllipse(ledBounds, 0.8f);
        }

        // Draw Item Text
        auto textBounds = r.withTrimmedLeft(14.0f).withTrimmedRight(2.0f);
        g.setFont(juce::FontOptions(isSel ? 12.0f : 11.5f, juce::Font::bold));
        g.setColour(isSel ? juce::Colours::white : (isHov ? juce::Colour(0xffe6edf8) : juce::Colour(0xffb8c4d8)));
        g.drawFittedText(items[i], textBounds.toNearestInt(), juce::Justification::centredLeft, 1);
    }
}

void LedSelectorComponent::mouseMove(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx != hoveredIndex) {
        hoveredIndex = idx;
        repaint();
    }
}

void LedSelectorComponent::mouseExit(const juce::MouseEvent&) {
    if (hoveredIndex != -1) {
        hoveredIndex = -1;
        repaint();
    }
}

void LedSelectorComponent::mouseDown(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx >= 0 && idx < items.size()) {
        setSelectedIndex(idx, juce::sendNotification);
    }
}

// --- MODULE CARD COMPONENT ---

ModuleCardComponent::ModuleCardComponent(const juce::String& title, juce::Colour accentColour)
    : moduleTitle(title), accent(accentColour), oscilloscope(accentColour)
{
    addAndMakeVisible(oscilloscope);

    for (int i = 0; i < 4; ++i) {
        labels[i].setFont(juce::FontOptions(11.5f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        labels[i].setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(labels[i]);
    }
}

void ModuleCardComponent::setLedSelector(LedSelectorComponent* selector) {
    ledSelector = selector;
    if (ledSelector) addAndMakeVisible(ledSelector);
}

void ModuleCardComponent::setSecondLedSelector(LedSelectorComponent* selector) {
    secondLedSelector = selector;
    if (secondLedSelector) addAndMakeVisible(secondLedSelector);
}

void ModuleCardComponent::setSelector(juce::ComboBox* box) {
    selectorBox = box;
    if (selectorBox) addAndMakeVisible(selectorBox);
}

void ModuleCardComponent::setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider) {
    if (slotIndex >= 0 && slotIndex < 4) {
        knobs[slotIndex] = slider;
        labels[slotIndex].setText(label, juce::dontSendNotification);
        if (slider) addAndMakeVisible(slider);
    }
}

void ModuleCardComponent::setKnobLabel(int slotIndex, const juce::String& label) {
    if (slotIndex >= 0 && slotIndex < 4) {
        labels[slotIndex].setText(label, juce::dontSendNotification);
    }
}

void ModuleCardComponent::updateScope(const float* data, int numSamples) {
    oscilloscope.updateData(data, numSamples);
}

void ModuleCardComponent::setPlotMode(MiniOscilloscopeComponent::PlotMode mode) {
    oscilloscope.setPlotMode(mode);
}

void ModuleCardComponent::updateFilterParams(int type, int slope, float cutoffHz, float resonance) {
    oscilloscope.updateFilterParams(type, slope, cutoffHz, resonance);
}

void ModuleCardComponent::updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter) {
    oscilloscope.updateEqParams(freqHz, widthOct, gainDb, djFilter);
}

void ModuleCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Dark sleek card backdrop
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Subtle outline
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Header color strip
    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    // Title label
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText(moduleTitle.toUpperCase(), 8, 4, getWidth() - 16, 16, juce::Justification::left, true);
}

void ModuleCardComponent::resized() {
    auto area = getLocalBounds().reduced(5);
    area.removeFromTop(18); // Header title

    oscilloscope.setBounds(area.removeFromTop(28).reduced(2, 0));
    area.removeFromTop(4);

    int count = 4;
    if (ledSelector != nullptr && secondLedSelector != nullptr) {
        int selH1 = (ledSelector->getNumItems() > 4) ? 26 : 20;
        int selH2 = (secondLedSelector->getNumItems() > 4) ? 26 : 20;
        ledSelector->setBounds(area.removeFromTop(selH1));
        area.removeFromTop(2);
        secondLedSelector->setBounds(area.removeFromTop(selH2));
        area.removeFromTop(4);
        count = 2;
        labels[2].setVisible(false);
        labels[3].setVisible(false);
        if (knobs[2]) knobs[2]->setVisible(false);
        if (knobs[3]) knobs[3]->setVisible(false);
    } else if (ledSelector != nullptr) {
        int selH = (ledSelector->getNumItems() > 4) ? 32 : 22;
        ledSelector->setBounds(area.removeFromTop(selH));
        area.removeFromTop(4);
        count = 3;
        labels[3].setVisible(false);
        if (knobs[3]) knobs[3]->setVisible(false);
    }

    int rowH = area.getHeight() / count;
    int labelW = 64;

    for (int i = 0; i < count; ++i) {
        auto row = area.removeFromTop(rowH).reduced(0, 1);
        labels[i].setVisible(true);
        labels[i].setBounds(row.removeFromLeft(labelW));
        row.removeFromLeft(2);
        if (knobs[i]) {
            knobs[i]->setVisible(true);
            knobs[i]->setBounds(row);
        }
    }
}


// --- THE KLANG FARMER EDITOR ---

TheKlangFarmerAudioProcessorEditor::TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      carrier1TrackingSelector(juce::Colour(0xff00d2ff)),
      mod1TrackSelector(juce::Colour(0xffff7043)),
      mod1TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv1TargetSelector(juce::Colour(0xffffab00)),
      carrier2TrackingSelector(juce::Colour(0xff00d2ff)),
      mod2TrackSelector(juce::Colour(0xffff7043)),
      mod2TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv2TargetSelector(juce::Colour(0xffffab00)),
      driveLimiterSelector(juce::Colour(0xffff4081)),
      filterTypeSelector(juce::Colour(0xff7c4dff)),
      filterSlopeSelector(juce::Colour(0xff7c4dff)),
      waveFolderTypeSelector(juce::Colour(0xffff5252)),
      combTypeSelector(juce::Colour(0xff26a69a)),
      disperserTypeSelector(juce::Colour(0xffec407a)),
      ampLimiterSelector(juce::Colour(0xff00e5ff))
{
    setLookAndFeel(&knobLookAndFeel);

    // Setup Header Initialize Button
    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222736));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.onClick = [this]() {
        auto* alert = new juce::AlertWindow("Reset to Defaults",
                                           "Are you sure you want to reset all parameters to their default values?",
                                           juce::AlertWindow::QuestionIcon, this);
        alert->addButton("Reset", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        alert->setColour(juce::AlertWindow::backgroundColourId, juce::Colour(0xff151821));
        alert->setColour(juce::AlertWindow::textColourId, juce::Colour(0xffffffff));
        alert->setColour(juce::AlertWindow::outlineColourId, juce::Colour(0xff00d2ff));
        alert->enterModalState(true, juce::ModalCallbackFunction::create([this](int result) {
            if (result == 1) {
                resetToDefaults();
            }
        }), true);
    };
    addAndMakeVisible(initButton);

    // Setup Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    auto makeShapeFormat = [](double val) {
        float f = static_cast<float>(val);
        if (f <= 0.20f) return "Sine (" + juce::String(static_cast<int>(f * 500.0f)) + "%)";
        if (f <= 0.40f) return "Tri (" + juce::String(static_cast<int>((f - 0.20f) * 500.0f)) + "%)";
        if (f <= 0.60f) return "Saw (" + juce::String(static_cast<int>((f - 0.40f) * 500.0f)) + "%)";
        return "PWM (" + juce::String(static_cast<int>((f - 0.60f) * 250.0f)) + "%)";
    };
    auto makeShapeParse = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(p / 100.0, 0.0, 1.0);
    };

    auto makeDriveFormat = [](double val) {
        float db = TbdAudio::normToDriveDb(static_cast<float>(val));
        return (db >= 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
    };
    auto makeDriveParse = [](const juce::String& text) {
        double db = parseNumberSafe(text, 0.0);
        return TbdAudio::driveDbToNorm(static_cast<float>(db));
    };

    auto makeCarrierPitchFormat = [](LedSelectorComponent& sel, double val) -> juce::String {
        int track = sel.getSelectedIndex();
        if (track == 0) {
            float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        } else if (track == 1) {
            int note = static_cast<int>(std::round(val * 127.0));
            return getMidiNoteName(note);
        } else {
            int offset = static_cast<int>(std::round((val - 0.5) * 120.0));
            return (offset >= 0 ? "+" : "") + juce::String(offset) + " st";
        }
    };
    auto makeCarrierPitchParse = [](LedSelectorComponent& sel, RotaryKnobSlider& s, const juce::String& text) -> double {
        int track = sel.getSelectedIndex();
        double parsed = parseNumberSafe(text, s.getValue());
        if (track == 0) {
            double hz = std::clamp(parsed, 20.0, 24000.0);
            return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
        } else if (track == 1) {
            return std::clamp(parsed / 127.0, 0.0, 1.0);
        } else {
            return std::clamp(0.5 + parsed / 120.0, 0.0, 1.0);
        }
    };

    // 1. CARRIER 1 CARD
    auto cardCarrier1 = std::make_unique<ModuleCardComponent>("Carrier 1", juce::Colour(0xff00d2ff));
    bindSelector(carrier1TrackingSelector, carrier1TrackingBox, "carrier1_tracking", { "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 3);
    setupKnob(carrier1PitchSlider, juce::Colour(0xff00d2ff), false, 0.5);
    setupKnob(carrier1ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    setupKnob(carrier1DepthSlider, juce::Colour(0xff00d2ff), true, 0.5); // Bipolar, def 0.5

    carrier1PitchSlider.getDefaultValue = [this]() -> double {
        int track = carrier1TrackingSelector.getSelectedIndex();
        if (track == 0) return std::log(55.0 / 20.0) / std::log(24000.0 / 20.0);
        if (track == 1) return 33.0 / 127.0;
        return 0.5;
    };
    carrier1PitchSlider.customFormatText = [this, makeCarrierPitchFormat](double val) {
        return makeCarrierPitchFormat(carrier1TrackingSelector, val);
    };
    carrier1PitchSlider.customParseText = [this, makeCarrierPitchParse](const juce::String& text) {
        return makeCarrierPitchParse(carrier1TrackingSelector, carrier1PitchSlider, text);
    };
    carrier1ShapeSlider.customFormatText = makeShapeFormat;
    carrier1ShapeSlider.customParseText  = makeShapeParse;
    carrier1DepthSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 400.0));
        return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
    };
    carrier1DepthSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 400.0, 0.0, 1.0);
    };

    cardCarrier1->setLedSelector(&carrier1TrackingSelector);
    cardCarrier1->setKnob(0, "Pitch", &carrier1PitchSlider);
    cardCarrier1->setKnob(1, "Shape", &carrier1ShapeSlider);
    cardCarrier1->setKnob(2, "Mod Depth", &carrier1DepthSlider);
    cards.push_back(std::move(cardCarrier1));

    // Modulator helper lambdas
    auto makeModSpeedFormat = [](LedSelectorComponent& trackSel, LedSelectorComponent& typeSel, double val) -> juce::String {
        int type = typeSel.getSelectedIndex();
        int track = trackSel.getSelectedIndex();
        if (type == 1) { // Cyclic sine freq: 0.1 Hz to 24 kHz
            float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        }
        if (track == 0) { // Fixed
            float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        } else if (track == 1) { // Following
            int st = static_cast<int>(std::round((val - 0.5) * 128.0));
            return (st >= 0 ? "+" : "") + juce::String(st) + " st";
        } else { // FM Operator
            if (val <= 0.5) {
                float denom = 32.0f - static_cast<float>(val * 2.0 * 31.0);
                return "1:" + juce::String(denom, 1);
            } else {
                float num = 1.0f + static_cast<float>((val - 0.5) * 2.0 * 31.0);
                return juce::String(num, 1) + ":1";
            }
        }
    };
    auto makeModSpeedParse = [](LedSelectorComponent& trackSel, LedSelectorComponent& typeSel, const juce::String& text) -> double {
        int type = typeSel.getSelectedIndex();
        int track = trackSel.getSelectedIndex();
        if (type == 1 || track == 0) {
            double p = parseNumberSafe(text, 55.0);
            double hz = std::clamp(p, 0.1, 24000.0);
            return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
        } else if (track == 1) {
            double p = parseNumberSafe(text, 0.0);
            return std::clamp(0.5 + p / 128.0, 0.0, 1.0);
        } else {
            auto trimmed = text.trim();
            int colon = trimmed.indexOfChar(':');
            if (colon > 0) {
                double left = parseNumberSafe(trimmed.substring(0, colon), 1.0);
                double right = parseNumberSafe(trimmed.substring(colon + 1), 1.0);
                if (left > 1.01) {
                    return std::clamp(0.5 + (left - 1.0) / 62.0, 0.5, 1.0);
                } else if (right > 1.01) {
                    return std::clamp((32.0 - right) / 62.0, 0.0, 0.5);
                }
            }
            return 0.5;
        }
    };

    // 2. MODULATOR 1 CARD
    auto cardMod1 = std::make_unique<ModuleCardComponent>("Modulator 1", juce::Colour(0xffff7043));
    bindSelector(mod1TrackSelector, mod1TrackBox, "mod1_track", { "Fixed", "Following", "FM Operator" }, 3);
    bindSelector(mod1TypeSelector, mod1TypeBox, "mod1_type", { "Oscillator", "Cyclic", "Noise" }, 3);
    setupKnob(mod1ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    setupKnob(mod1SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);

    mod1SpeedSlider.getDefaultValue = [this]() -> double {
        int type = mod1TypeSelector.getSelectedIndex();
        int track = mod1TrackSelector.getSelectedIndex();
        if (type == 1) return 1.0; // Cyclic sine freq default 24 kHz
        if (track == 0) return std::log(55.0 / 0.1) / std::log(24000.0 / 0.1);
        if (track == 1) return 0.5;
        return 0.5;
    };
    mod1SpeedSlider.customFormatText = [this, makeModSpeedFormat](double val) {
        return makeModSpeedFormat(mod1TrackSelector, mod1TypeSelector, val);
    };
    mod1SpeedSlider.customParseText = [this, makeModSpeedParse](const juce::String& text) {
        return makeModSpeedParse(mod1TrackSelector, mod1TypeSelector, text);
    };
    mod1ShapeSlider.customFormatText = [this, makeShapeFormat](double val) {
        int type = mod1TypeSelector.getSelectedIndex();
        if (type == 0) return makeShapeFormat(val);
        if (type == 1) {
            if (val >= 0.49 && val <= 0.51) return juce::String("Flat (50%)");
            if (val < 0.49) {
                float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val / 0.49));
                return "LP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
            }
            float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>((val - 0.51) / 0.49));
            return "HP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
        }
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    mod1ShapeSlider.customParseText = [this, makeShapeParse](const juce::String& text) {
        int type = mod1TypeSelector.getSelectedIndex();
        if (type == 0) return makeShapeParse(text);
        if (type == 1) return 0.5;
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 0.1, 24000.0);
        return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
    };

    cardMod1->setLedSelector(&mod1TrackSelector);
    cardMod1->setSecondLedSelector(&mod1TypeSelector);
    cardMod1->setKnob(0, "Shape", &mod1ShapeSlider);
    cardMod1->setKnob(1, "Speed", &mod1SpeedSlider);
    cards.push_back(std::move(cardMod1));

    // Pitch Env helpers
    auto makePitchEnvSlopeFormat = [](double val) {
        if (val < 0.33) return "Exp (" + juce::String(static_cast<int>(val * 300.0)) + "%)";
        if (val < 0.67) return "Lin (" + juce::String(static_cast<int>((val - 0.33) * 300.0)) + "%)";
        return "Log (" + juce::String(static_cast<int>((val - 0.67) * 300.0)) + "%)";
    };
    auto makePitchEnvDepthFormat = [](double val) {
        float oct = static_cast<float>((val - 0.5) * 10.0);
        return (oct >= 0.0f ? "+" : "") + juce::String(oct, 1) + " Oct";
    };
    auto makePitchEnvDepthParse = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 10.0, 0.0, 1.0);
    };
    auto makePitchEnvDecayFormat = [](double val) {
        float sec = TbdAudio::warp5PointTime(static_cast<float>(val));
        return (sec < 1.0f) ? juce::String(static_cast<int>(std::round(sec * 1000.0f))) + " ms" : juce::String(sec, 2) + " s";
    };
    auto makePitchEnvDecayParse = [](const juce::String& text) {
        double v = parseNumberSafe(text, 333.0);
        double sec = (text.containsIgnoreCase("ms")) ? (v / 1000.0) : ((v > 60.0) ? (v / 1000.0) : v);
        return TbdAudio::unwarp5PointTime(static_cast<float>(sec));
    };

    // 3. PITCH ENVELOPE 1 CARD
    auto cardPitchEnv1 = std::make_unique<ModuleCardComponent>("Pitch Env 1", juce::Colour(0xffffab00));
    bindSelector(pitchEnv1TargetSelector, pitchEnv1TargetBox, "pitchenv1_target",
                 { "Off", "Carrier", "Mod", "Both" }, 4);
    setupKnob(pitchEnv1SlopeSlider, juce::Colour(0xffffab00), false, 0.0);
    pitchEnv1SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    setupKnob(pitchEnv1DepthSlider, juce::Colour(0xffffab00), true, 0.5); // Bipolar
    setupKnob(pitchEnv1DecaySlider, juce::Colour(0xffffab00), false, 0.3806);

    pitchEnv1SlopeSlider.customFormatText = makePitchEnvSlopeFormat;
    pitchEnv1DepthSlider.customFormatText = makePitchEnvDepthFormat;
    pitchEnv1DepthSlider.customParseText  = makePitchEnvDepthParse;
    pitchEnv1DecaySlider.customFormatText = makePitchEnvDecayFormat;
    pitchEnv1DecaySlider.customParseText  = makePitchEnvDecayParse;

    cardPitchEnv1->setLedSelector(&pitchEnv1TargetSelector);
    cardPitchEnv1->setKnob(0, "Slope", &pitchEnv1SlopeSlider);
    cardPitchEnv1->setKnob(1, "Depth", &pitchEnv1DepthSlider);
    cardPitchEnv1->setKnob(2, "Decay", &pitchEnv1DecaySlider);
    cards.push_back(std::move(cardPitchEnv1));

    // 4. CARRIER 2 CARD
    auto cardCarrier2 = std::make_unique<ModuleCardComponent>("Carrier 2", juce::Colour(0xff00d2ff));
    bindSelector(carrier2TrackingSelector, carrier2TrackingBox, "carrier2_tracking", { "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 3);
    setupKnob(carrier2PitchSlider, juce::Colour(0xff00d2ff), false, 0.5);
    setupKnob(carrier2ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    setupKnob(carrier2DepthSlider, juce::Colour(0xff00d2ff), true, 0.5);

    carrier2PitchSlider.getDefaultValue = [this]() -> double {
        int track = carrier2TrackingSelector.getSelectedIndex();
        if (track == 0) return std::log(55.0 / 20.0) / std::log(24000.0 / 20.0);
        if (track == 1) return 33.0 / 127.0;
        return 0.5;
    };
    carrier2PitchSlider.customFormatText = [this, makeCarrierPitchFormat](double val) {
        return makeCarrierPitchFormat(carrier2TrackingSelector, val);
    };
    carrier2PitchSlider.customParseText = [this, makeCarrierPitchParse](const juce::String& text) {
        return makeCarrierPitchParse(carrier2TrackingSelector, carrier2PitchSlider, text);
    };
    carrier2ShapeSlider.customFormatText = makeShapeFormat;
    carrier2ShapeSlider.customParseText  = makeShapeParse;
    carrier2DepthSlider.customFormatText = carrier1DepthSlider.customFormatText;
    carrier2DepthSlider.customParseText  = carrier1DepthSlider.customParseText;

    cardCarrier2->setLedSelector(&carrier2TrackingSelector);
    cardCarrier2->setKnob(0, "Pitch", &carrier2PitchSlider);
    cardCarrier2->setKnob(1, "Shape", &carrier2ShapeSlider);
    cardCarrier2->setKnob(2, "Mod Depth", &carrier2DepthSlider);
    cards.push_back(std::move(cardCarrier2));

    // 5. MODULATOR 2 CARD
    auto cardMod2 = std::make_unique<ModuleCardComponent>("Modulator 2", juce::Colour(0xffff7043));
    bindSelector(mod2TrackSelector, mod2TrackBox, "mod2_track", { "Fixed", "Following", "FM Operator" }, 3);
    bindSelector(mod2TypeSelector, mod2TypeBox, "mod2_type", { "Oscillator", "Cyclic", "Noise" }, 3);
    setupKnob(mod2ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    setupKnob(mod2SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);

    mod2SpeedSlider.getDefaultValue = [this]() -> double {
        int type = mod2TypeSelector.getSelectedIndex();
        int track = mod2TrackSelector.getSelectedIndex();
        if (type == 1) return 1.0; // Cyclic sine freq default 24 kHz
        if (track == 0) return std::log(55.0 / 0.1) / std::log(24000.0 / 0.1);
        if (track == 1) return 0.5;
        return 0.5;
    };
    mod2SpeedSlider.customFormatText = [this, makeModSpeedFormat](double val) {
        return makeModSpeedFormat(mod2TrackSelector, mod2TypeSelector, val);
    };
    mod2SpeedSlider.customParseText = [this, makeModSpeedParse](const juce::String& text) {
        return makeModSpeedParse(mod2TrackSelector, mod2TypeSelector, text);
    };
    mod2ShapeSlider.customFormatText = [this, makeShapeFormat](double val) {
        int type = mod2TypeSelector.getSelectedIndex();
        if (type == 0) return makeShapeFormat(val);
        if (type == 1) {
            if (val >= 0.49 && val <= 0.51) return juce::String("Flat (50%)");
            if (val < 0.49) {
                float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val / 0.49));
                return "LP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
            }
            float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>((val - 0.51) / 0.49));
            return "HP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
        }
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    mod2ShapeSlider.customParseText = [this, makeShapeParse](const juce::String& text) {
        int type = mod2TypeSelector.getSelectedIndex();
        if (type == 0) return makeShapeParse(text);
        if (type == 1) return 0.5;
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 0.1, 24000.0);
        return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
    };

    cardMod2->setLedSelector(&mod2TrackSelector);
    cardMod2->setSecondLedSelector(&mod2TypeSelector);
    cardMod2->setKnob(0, "Shape", &mod2ShapeSlider);
    cardMod2->setKnob(1, "Speed", &mod2SpeedSlider);
    cards.push_back(std::move(cardMod2));

    // 6. PITCH ENVELOPE 2 CARD
    auto cardPitchEnv2 = std::make_unique<ModuleCardComponent>("Pitch Env 2", juce::Colour(0xffffab00));
    bindSelector(pitchEnv2TargetSelector, pitchEnv2TargetBox, "pitchenv2_target",
                 { "Off", "Carrier", "Mod", "Both" }, 4);
    setupKnob(pitchEnv2SlopeSlider, juce::Colour(0xffffab00), false, 0.0);
    pitchEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    setupKnob(pitchEnv2DepthSlider, juce::Colour(0xffffab00), true, 0.5); // Bipolar
    setupKnob(pitchEnv2DecaySlider, juce::Colour(0xffffab00), false, 0.3806);

    pitchEnv2SlopeSlider.customFormatText = makePitchEnvSlopeFormat;
    pitchEnv2DepthSlider.customFormatText = makePitchEnvDepthFormat;
    pitchEnv2DepthSlider.customParseText  = makePitchEnvDepthParse;
    pitchEnv2DecaySlider.customFormatText = makePitchEnvDecayFormat;
    pitchEnv2DecaySlider.customParseText  = makePitchEnvDecayParse;

    cardPitchEnv2->setLedSelector(&pitchEnv2TargetSelector);
    cardPitchEnv2->setKnob(0, "Slope", &pitchEnv2SlopeSlider);
    cardPitchEnv2->setKnob(1, "Depth", &pitchEnv2DepthSlider);
    cardPitchEnv2->setKnob(2, "Decay", &pitchEnv2DecaySlider);
    cards.push_back(std::move(cardPitchEnv2));

    // 7. NOISE TRANSIENT CARD (4 Knobs)
    auto cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", juce::Colour(0xff00e676));
    setupKnob(noiseShRateSlider, juce::Colour(0xff00e676), false, 1.0);
    setupKnob(noiseFilterSlider, juce::Colour(0xff00e676), true, 0.5); // Bipolar
    setupKnob(noiseDriveSlider, juce::Colour(0xff00e676), false, 0.5);
    setupKnob(noiseDecaySlider, juce::Colour(0xff00e676), false, 0.3078);

    noiseShRateSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    noiseShRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 0.1, 24000.0);
        return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
    };
    auto makeDjFilterFormat = [](double val) {
        if (val >= 0.49 && val <= 0.51) return juce::String("Flat (50%)");
        if (val < 0.49) {
            float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val / 0.49));
            return "LP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
        }
        float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>((val - 0.51) / 0.49));
        return "HP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
    };
    noiseFilterSlider.customFormatText = makeDjFilterFormat;
    noiseDriveSlider.customFormatText  = makeDriveFormat;
    noiseDriveSlider.customParseText   = makeDriveParse;
    noiseDecaySlider.customFormatText = [](double val) {
        float sec = TbdAudio::warpNoiseDecayTime(static_cast<float>(val));
        return (sec < 1.0f) ? juce::String(static_cast<int>(std::round(sec * 1000.0f))) + " ms" : juce::String(sec, 2) + " s";
    };
    noiseDecaySlider.customParseText = [](const juce::String& text) {
        double v = parseNumberSafe(text, 100.0);
        double sec = (text.containsIgnoreCase("ms")) ? (v / 1000.0) : ((v > 60.0) ? (v / 1000.0) : v);
        return TbdAudio::unwarpNoiseDecayTime(static_cast<float>(sec));
    };

    cardNoise->setKnob(0, "S&H Rate", &noiseShRateSlider);
    cardNoise->setKnob(1, "Filter", &noiseFilterSlider);
    cardNoise->setKnob(2, "Drive", &noiseDriveSlider);
    cardNoise->setKnob(3, "Decay", &noiseDecaySlider);
    cards.push_back(std::move(cardNoise));

    // 8. MIXER CARD (4 Knobs)
    auto cardMixer = std::make_unique<ModuleCardComponent>("Mixer", juce::Colour(0xff40c4ff));
    setupKnob(mixerCarrier1LevelSlider, juce::Colour(0xff40c4ff), false, 0.5);
    setupKnob(mixerCarrier2LevelSlider, juce::Colour(0xff40c4ff), false, 0.0); // 0% default
    setupKnob(mixerRingModSlider, juce::Colour(0xff40c4ff), false, 0.0);       // 0% default
    setupKnob(mixerNoiseLevelSlider, juce::Colour(0xff40c4ff), false, 0.0);     // 0% default

    mixerCarrier2LevelSlider.getDefaultValue = []() { return 0.5; };
    mixerCarrier2LevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerRingModSlider.getDefaultValue = []() { return 0.5; };
    mixerRingModSlider.setDoubleClickReturnValue(true, 0.5);
    mixerNoiseLevelSlider.getDefaultValue = []() { return 0.5; };
    mixerNoiseLevelSlider.setDoubleClickReturnValue(true, 0.5);

    auto format0to400Pct = [](double val) {
        float pct = (val <= 0.5) ? static_cast<float>(val * 200.0) : static_cast<float>(100.0 + (val - 0.5) * 600.0);
        return juce::String(static_cast<int>(std::round(pct))) + "%";
    };
    auto parse0to400Pct = [](const juce::String& text) {
        double p = parseNumberSafe(text, 100.0);
        return (p <= 100.0) ? std::clamp(p / 200.0, 0.0, 0.5) : std::clamp(0.5 + (p - 100.0) / 600.0, 0.5, 1.0);
    };

    mixerCarrier1LevelSlider.customFormatText = format0to400Pct;
    mixerCarrier1LevelSlider.customParseText  = parse0to400Pct;
    mixerCarrier2LevelSlider.customFormatText = format0to400Pct;
    mixerCarrier2LevelSlider.customParseText  = parse0to400Pct;
    mixerRingModSlider.customFormatText       = format0to400Pct;
    mixerRingModSlider.customParseText        = parse0to400Pct;
    mixerNoiseLevelSlider.customFormatText    = format0to400Pct;
    mixerNoiseLevelSlider.customParseText     = parse0to400Pct;

    cardMixer->setKnob(0, "Carrier 1", &mixerCarrier1LevelSlider);
    cardMixer->setKnob(1, "Carrier 2", &mixerCarrier2LevelSlider);
    cardMixer->setKnob(2, "RingMod",   &mixerRingModSlider);
    cardMixer->setKnob(3, "Noise Lvl", &mixerNoiseLevelSlider);
    cards.push_back(std::move(cardMixer));

    // 9. DRIVE CARD (Limiter selector + 3 Knobs)
    auto cardDrive = std::make_unique<ModuleCardComponent>("Drive", juce::Colour(0xffff4081));
    bindSelector(driveLimiterSelector, driveLimiterBox, "drive_limiter", { "Off", "On" }, 2);
    setupKnob(driveAmountSlider, juce::Colour(0xffff4081), false, 0.5);
    setupKnob(driveBiasSlider, juce::Colour(0xffff4081), true, 0.5); // Bipolar
    setupKnob(driveFilterSlider, juce::Colour(0xffff4081), true, 0.5); // Bipolar

    driveAmountSlider.customFormatText = [](double val) {
        float db = (val <= 0.5) ? static_cast<float>(-6.0 + val * 12.0) : static_cast<float>((val - 0.5) * 48.0);
        return (db >= 0 ? "+" : "") + juce::String(db, 1) + " dB";
    };
    driveAmountSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return (p <= 0.0) ? std::clamp((p + 6.0) / 12.0, 0.0, 0.5) : std::clamp(0.5 + p / 48.0, 0.5, 1.0);
    };
    driveBiasSlider.customFormatText = [](double val) {
        float b = static_cast<float>((val - 0.5) * 2.0);
        return (b >= 0 ? "+" : "") + juce::String(b, 2);
    };
    driveBiasSlider.customParseText = [](const juce::String& text) {
        double b = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + b * 0.5, 0.0, 1.0);
    };
    driveFilterSlider.customFormatText = makeDjFilterFormat;

    cardDrive->setLedSelector(&driveLimiterSelector);
    cardDrive->setKnob(0, "Drive", &driveAmountSlider);
    cardDrive->setKnob(1, "Bias", &driveBiasSlider);
    cardDrive->setKnob(2, "Post-Filter", &driveFilterSlider);
    cards.push_back(std::move(cardDrive));

    // 10. FILTER CARD
    auto cardFilter = std::make_unique<ModuleCardComponent>("Filter", juce::Colour(0xff7c4dff));
    bindSelector(filterTypeSelector, filterTypeBox, "filter_type",
                 { "Off", "LPF", "BPF", "HPF", "BRF" }, 5);
    bindSelector(filterSlopeSelector, filterSlopeBox, "filter_slope",
                 { "-6dB", "-12dB", "-18dB", "-24dB", "-36dB" }, 5);
    setupKnob(filterCutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    setupKnob(filterResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0); // 0% unipolar

    filterCutoffSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    filterCutoffSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 0.1, 24000.0);
        return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
    };
    filterResonanceSlider.customFormatText = [](double val) {
        return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
    };
    filterResonanceSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(p / 100.0, 0.0, 1.0);
    };

    cardFilter->setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
    cardFilter->setLedSelector(&filterTypeSelector);
    cardFilter->setSecondLedSelector(&filterSlopeSelector);
    cardFilter->setKnob(0, "Cutoff", &filterCutoffSlider);
    cardFilter->setKnob(1, "Resonance", &filterResonanceSlider);
    cards.push_back(std::move(cardFilter));

    // 11. FILTER ENVELOPE CARD (4 Knobs)
    auto cardFilterEnv = std::make_unique<ModuleCardComponent>("Filter Env", juce::Colour(0xffb388ff));
    setupKnob(filterEnvSlopeSlider, juce::Colour(0xffb388ff), false, 0.0);
    filterEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    setupKnob(filterEnvDepthSlider, juce::Colour(0xffb388ff), true, 0.5); // Bipolar, def 0 octaves
    setupKnob(filterEnvDecaySlider, juce::Colour(0xffb388ff), false, 0.3806);
    setupKnob(filterEnvPostDriveSlider, juce::Colour(0xffb388ff), false, 0.5);

    filterEnvSlopeSlider.customFormatText = makePitchEnvSlopeFormat;
    filterEnvDepthSlider.customFormatText = [](double val) {
        float oct = static_cast<float>((val - 0.5) * 20.0);
        return (oct >= 0.0f ? "+" : "") + juce::String(oct, 1) + " Oct";
    };
    filterEnvDepthSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 20.0, 0.0, 1.0);
    };
    filterEnvDecaySlider.customFormatText = makePitchEnvDecayFormat;
    filterEnvDecaySlider.customParseText = makePitchEnvDecayParse;
    filterEnvPostDriveSlider.customFormatText = makeDriveFormat;
    filterEnvPostDriveSlider.customParseText = makeDriveParse;

    cardFilterEnv->setKnob(0, "Slope", &filterEnvSlopeSlider);
    cardFilterEnv->setKnob(1, "Depth", &filterEnvDepthSlider);
    cardFilterEnv->setKnob(2, "Decay", &filterEnvDecaySlider);
    cardFilterEnv->setKnob(3, "Post-Drive", &filterEnvPostDriveSlider);
    cards.push_back(std::move(cardFilterEnv));

    // 12. WAVE FOLDER CARD (Type selector + 3 Knobs)
    auto cardWaveFolder = std::make_unique<ModuleCardComponent>("Wave Folder", juce::Colour(0xffff5252));
    bindSelector(waveFolderTypeSelector, waveFolderTypeBox, "wavefolder_type", { "Off", "On" }, 2);
    setupKnob(waveFolderFoldSlider, juce::Colour(0xffff5252), false, 0.0);
    setupKnob(waveFolderBiasSlider, juce::Colour(0xffff5252), true, 0.5); // Bipolar
    setupKnob(waveFolderFilterSlider, juce::Colour(0xffff5252), true, 0.5); // Bipolar

    waveFolderFoldSlider.customFormatText = [](double val) {
        return juce::String(val * 8.0, 1) + " folds";
    };
    waveFolderFoldSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(p / 8.0, 0.0, 1.0);
    };
    waveFolderBiasSlider.customFormatText = [](double val) {
        float b = static_cast<float>((val - 0.5) * 2.0);
        return (b >= 0 ? "+" : "") + juce::String(b, 2);
    };
    waveFolderBiasSlider.customParseText = [](const juce::String& text) {
        double b = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + b * 0.5, 0.0, 1.0);
    };
    waveFolderFilterSlider.customFormatText = makeDjFilterFormat;

    cardWaveFolder->setLedSelector(&waveFolderTypeSelector);
    cardWaveFolder->setKnob(0, "Fold", &waveFolderFoldSlider);
    cardWaveFolder->setKnob(1, "Bias", &waveFolderBiasSlider);
    cardWaveFolder->setKnob(2, "Post-Filter", &waveFolderFilterSlider);
    cards.push_back(std::move(cardWaveFolder));

    // 13. RINGMOD CARD (4 Knobs)
    auto cardRingMod = std::make_unique<ModuleCardComponent>("RingMod", juce::Colour(0xffff4081));
    setupKnob(ringModShapeSlider, juce::Colour(0xffff4081), false, 0.0);
    ringModShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    setupKnob(ringModRateSlider, juce::Colour(0xffff4081), false, 0.50934);
    setupKnob(ringModAmountSlider, juce::Colour(0xffff4081), false, 0.0);
    setupKnob(ringModWidthSlider, juce::Colour(0xffff4081), true, 0.5); // Bipolar

    ringModShapeSlider.customFormatText = makeShapeFormat;
    ringModShapeSlider.customParseText  = makeShapeParse;
    ringModRateSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    ringModRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 55.0), 0.1, 24000.0);
        return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
    };
    ringModAmountSlider.customFormatText = [](double val) {
        return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
    };
    ringModAmountSlider.customParseText = [](const juce::String& text) {
        return std::clamp(parseNumberSafe(text, 0.0) / 100.0, 0.0, 1.0);
    };
    ringModWidthSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 200.0));
        return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
    };
    ringModWidthSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
    };

    cardRingMod->setKnob(0, "Shape", &ringModShapeSlider);
    cardRingMod->setKnob(1, "Rate", &ringModRateSlider);
    cardRingMod->setKnob(2, "Amount", &ringModAmountSlider);
    cardRingMod->setKnob(3, "Width", &ringModWidthSlider);
    cards.push_back(std::move(cardRingMod));

    // 14. FREQUENCY SHIFTER CARD (4 Knobs)
    auto cardFreqShift = std::make_unique<ModuleCardComponent>("Freq Shifter", juce::Colour(0xff69f0ae));
    setupKnob(freqShiftShiftSlider, juce::Colour(0xff69f0ae), true, 0.5); // Bipolar
    setupKnob(freqShiftRangeSlider, juce::Colour(0xff69f0ae), false, TbdAudio::rangeHzToNorm(3.0f));
    setupKnob(freqShiftBlendSlider, juce::Colour(0xff69f0ae), true, 0.5); // Bipolar
    setupKnob(freqShiftWidthSlider, juce::Colour(0xff69f0ae), true, 0.5); // Bipolar

    freqShiftRangeSlider.customFormatText = [](double val) {
        float hz = TbdAudio::normToRangeHz(static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    freqShiftRangeSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 3.0), 0.0, 5000.0);
        return TbdAudio::rangeHzToNorm(static_cast<float>(hz));
    };
    freqShiftShiftSlider.customFormatText = [this](double val) {
        float r = TbdAudio::normToRangeHz(static_cast<float>(freqShiftRangeSlider.getValue()));
        float shiftHz = static_cast<float>((val - 0.5) * 2.0) * r;
        return (shiftHz >= 0 ? "+" : "") + juce::String(shiftHz, 1) + " Hz";
    };
    freqShiftShiftSlider.customParseText = [this](const juce::String& text) {
        float r = TbdAudio::normToRangeHz(static_cast<float>(freqShiftRangeSlider.getValue()));
        if (r < 0.0001f) return 0.5;
        double s = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + (s / r) * 0.5, 0.0, 1.0);
    };
    freqShiftRangeSlider.onValueChange = [this]() {
        freqShiftShiftSlider.updateText();
    };
    freqShiftBlendSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (pct == 0) return juce::String("Dry (0%)");
        if (pct < 0) return juce::String(pct) + "% (Inv)";
        return "+" + juce::String(pct) + "%";
    };
    freqShiftBlendSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
    };
    freqShiftWidthSlider.customFormatText = ringModWidthSlider.customFormatText;
    freqShiftWidthSlider.customParseText = ringModWidthSlider.customParseText;

    cardFreqShift->setKnob(0, "Shift", &freqShiftShiftSlider);
    cardFreqShift->setKnob(1, "Range", &freqShiftRangeSlider);
    cardFreqShift->setKnob(2, "Blend", &freqShiftBlendSlider);
    cardFreqShift->setKnob(3, "Width", &freqShiftWidthSlider);
    cards.push_back(std::move(cardFreqShift));

    // 15. GRIT FX CARD (4 Knobs)
    auto cardGrit = std::make_unique<ModuleCardComponent>("Grit FX", juce::Colour(0xffffd740));
    setupKnob(gritBitsSlider, juce::Colour(0xffffd740), false, 1.0);
    setupKnob(gritRateSlider, juce::Colour(0xffffd740), false, 1.0);
    setupKnob(gritLowSlider, juce::Colour(0xffffd740), true, 0.5); // Bipolar, def 0 dB
    setupKnob(gritHighSlider, juce::Colour(0xffffd740), true, 0.5); // Bipolar, def 0 dB

    gritBitsSlider.customFormatText = [](double val) {
        return juce::String(1.0 + val * 15.0, 1) + " Bits";
    };
    gritBitsSlider.customParseText = [](const juce::String& text) {
        double b = std::clamp(parseNumberSafe(text, 16.0), 1.0, 16.0);
        return (b - 1.0) / 15.0;
    };
    gritRateSlider.customFormatText = [](double val) {
        float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 0) + " Hz";
    };
    gritRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 20.0, 24000.0);
        return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
    };
    auto formatShelfDb = [](double val) {
        double db = (val - 0.5) * 48.0;
        return (db >= 0.0 ? "+" : "") + juce::String(db, 1) + " dB";
    };
    auto parseShelfDb = [](const juce::String& text) {
        double db = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + db / 48.0, 0.0, 1.0);
    };
    gritLowSlider.customFormatText = formatShelfDb;
    gritLowSlider.customParseText  = parseShelfDb;
    gritHighSlider.customFormatText = formatShelfDb;
    gritHighSlider.customParseText  = parseShelfDb;

    cardGrit->setKnob(0, "Bit Crush", &gritBitsSlider);
    cardGrit->setKnob(1, "Downsample", &gritRateSlider);
    cardGrit->setKnob(2, "Low Shelf", &gritLowSlider);
    cardGrit->setKnob(3, "High Shelf", &gritHighSlider);
    cards.push_back(std::move(cardGrit));

    // 16. COMB FILTER CARD
    auto cardComb = std::make_unique<ModuleCardComponent>("Comb Filter", juce::Colour(0xff26a69a));
    bindSelector(combTypeSelector, combTypeBox, "comb_type", { "Off", "On" }, 2);
    setupKnob(combDampeningSlider, juce::Colour(0xff26a69a), false, 1.0);
    setupKnob(combCutoffSlider, juce::Colour(0xff26a69a), false, 1.0);
    setupKnob(combResonanceSlider, juce::Colour(0xff26a69a), true, 0.5); // Bipolar, 0% default

    combDampeningSlider.customFormatText = filterCutoffSlider.customFormatText;
    combDampeningSlider.customParseText  = filterCutoffSlider.customParseText;
    combCutoffSlider.customFormatText    = filterCutoffSlider.customFormatText;
    combCutoffSlider.customParseText     = filterCutoffSlider.customParseText;

    combResonanceSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 200.0));
        return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
    };
    combResonanceSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
    };

    cardComb->setLedSelector(&combTypeSelector);
    cardComb->setKnob(0, "Dampening", &combDampeningSlider);
    cardComb->setKnob(1, "Cutoff", &combCutoffSlider);
    cardComb->setKnob(2, "Resonance", &combResonanceSlider);
    cards.push_back(std::move(cardComb));

    // 17. DISPERSER CARD
    auto cardDisperser = std::make_unique<ModuleCardComponent>("Disperser", juce::Colour(0xffec407a));
    bindSelector(disperserTypeSelector, disperserTypeBox, "disperser_type", { "Off", "On" }, 2);
    setupKnob(disperserAmountSlider, juce::Colour(0xffec407a), false, 4.0 / 32.0); // 4 APFs default
    setupKnob(disperserCutoffSlider, juce::Colour(0xffec407a), false, 0.62124); // 220 Hz default!
    setupKnob(disperserResonanceSlider, juce::Colour(0xffec407a), true, 0.5); // Bipolar, 0% default
    disperserCutoffSlider.getDefaultValue = []() { return 0.62124; };

    disperserAmountSlider.customFormatText = [](double val) {
        int stages = static_cast<int>(std::round(val * 32.0));
        return juce::String(stages) + " APFs";
    };
    disperserAmountSlider.customParseText = [](const juce::String& text) {
        double s = parseNumberSafe(text, 4.0);
        return std::clamp(s / 32.0, 0.0, 1.0);
    };
    disperserCutoffSlider.customFormatText = filterCutoffSlider.customFormatText;
    disperserCutoffSlider.customParseText  = filterCutoffSlider.customParseText;
    disperserResonanceSlider.customFormatText = combResonanceSlider.customFormatText;
    disperserResonanceSlider.customParseText  = combResonanceSlider.customParseText;

    cardDisperser->setLedSelector(&disperserTypeSelector);
    cardDisperser->setKnob(0, "Amount", &disperserAmountSlider);
    cardDisperser->setKnob(1, "Cutoff", &disperserCutoffSlider);
    cardDisperser->setKnob(2, "Resonance", &disperserResonanceSlider);
    cards.push_back(std::move(cardDisperser));

    // 18. EQ CARD (4 Knobs)
    auto cardEQ = std::make_unique<ModuleCardComponent>("Bell EQ", juce::Colour(0xff40c4ff));
    setupKnob(eqFreqSlider, juce::Colour(0xff40c4ff), false, 1.0);
    setupKnob(eqWidthSlider, juce::Colour(0xff40c4ff), false, 0.0);
    setupKnob(eqGainSlider, juce::Colour(0xff40c4ff), true, 0.5);
    setupKnob(eqFilterSlider, juce::Colour(0xff40c4ff), true, 0.5);

    eqFreqSlider.customFormatText = [](double val) {
        float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 0) + " Hz";
    };
    eqFreqSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 24000.0), 20.0, 24000.0);
        return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
    };
    eqWidthSlider.customFormatText = [](double val) {
        float oct = 0.1f * std::pow(10.0f / 0.1f, static_cast<float>(val));
        return juce::String(oct, 2) + " oct";
    };
    eqWidthSlider.customParseText = [](const juce::String& text) {
        double oct = std::clamp(parseNumberSafe(text, 0.1), 0.1, 10.0);
        return std::log(oct / 0.1) / std::log(10.0 / 0.1);
    };
    eqGainSlider.customFormatText = formatShelfDb;
    eqGainSlider.customParseText  = parseShelfDb;
    eqFilterSlider.customFormatText = makeDjFilterFormat;

    cardEQ->setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
    cardEQ->setKnob(0, "Frequency", &eqFreqSlider);
    cardEQ->setKnob(1, "Width",     &eqWidthSlider);
    cardEQ->setKnob(2, "Gain",      &eqGainSlider);
    cardEQ->setKnob(3, "DJ Filter", &eqFilterSlider);
    cards.push_back(std::move(cardEQ));

    // 19. AMP CARD
    auto cardAmp = std::make_unique<ModuleCardComponent>("Amp", juce::Colour(0xff00e5ff));
    bindSelector(ampLimiterSelector, ampLimiterBox, "amp_limiter", { "Off", "On" }, 2);
    setupKnob(ampLevelSlider, juce::Colour(0xff00e5ff), false, 1.0);
    setupKnob(ampPanSlider, juce::Colour(0xff00e5ff), true, 0.5); // Bipolar
    setupKnob(ampDriveSlider, juce::Colour(0xff00e5ff), false, 0.5);

    ampLevelSlider.customFormatText = [](double val) {
        return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
    };
    ampLevelSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 100.0);
        return std::clamp(p / 100.0, 0.0, 1.0);
    };
    ampPanSlider.customFormatText = [](double val) {
        int p = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (p == 0) return juce::String("Center");
        if (p < 0) return juce::String(-p) + "% L";
        return juce::String(p) + "% R";
    };
    ampPanSlider.customParseText = [](const juce::String& text) {
        if (text.containsIgnoreCase("c")) return 0.5;
        double p = parseNumberSafe(text, 0.0);
        if (text.containsIgnoreCase("l")) p = -std::abs(p);
        return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
    };
    ampDriveSlider.customFormatText = makeDriveFormat;
    ampDriveSlider.customParseText  = makeDriveParse;

    cardAmp->setLedSelector(&ampLimiterSelector);
    cardAmp->setKnob(0, "Level", &ampLevelSlider);
    cardAmp->setKnob(1, "Pan",   &ampPanSlider);
    cardAmp->setKnob(2, "Drive", &ampDriveSlider);
    cards.push_back(std::move(cardAmp));

    // 20. AMP ENVELOPE CARD (4 Knobs)
    auto cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Env", juce::Colour(0xff64ffda));
    setupKnob(ampEnvClapsSlider, juce::Colour(0xff64ffda), false, 0.0);
    setupKnob(ampEnvClapSpeedSlider, juce::Colour(0xff64ffda), false, 0.1429);
    setupKnob(ampEnvSlopeSlider, juce::Colour(0xff64ffda), false, 0.0);
    ampEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    setupKnob(ampEnvDecaySlider, juce::Colour(0xff64ffda), false, 0.3806);

    ampEnvClapsSlider.customFormatText = [](double val) {
        int c = static_cast<int>(std::round(val * 32.0));
        return juce::String(c) + " claps";
    };
    ampEnvClapsSlider.customParseText = [](const juce::String& text) {
        return std::clamp(parseNumberSafe(text, 0.0) / 32.0, 0.0, 1.0);
    };
    ampEnvClapSpeedSlider.customFormatText = [](double val) {
        return juce::String(1.0 + val * 14.0, 1) + " ms";
    };
    ampEnvClapSpeedSlider.customParseText = [](const juce::String& text) {
        double v = std::clamp(parseNumberSafe(text, 3.0), 1.0, 15.0);
        return (v - 1.0) / 14.0;
    };
    ampEnvSlopeSlider.customFormatText = makePitchEnvSlopeFormat;
    ampEnvDecaySlider.customFormatText = makePitchEnvDecayFormat;
    ampEnvDecaySlider.customParseText  = makePitchEnvDecayParse;

    cardAmpEnv->setKnob(0, "Claps", &ampEnvClapsSlider);
    cardAmpEnv->setKnob(1, "Clap Speed", &ampEnvClapSpeedSlider);
    cardAmpEnv->setKnob(2, "Slope", &ampEnvSlopeSlider);
    cardAmpEnv->setKnob(3, "Decay", &ampEnvDecaySlider);
    cards.push_back(std::move(cardAmpEnv));

    // 21. VELOCITY CARD (4 Knobs)
    auto cardVel = std::make_unique<ModuleCardComponent>("Velocity", juce::Colour(0xff80cbc4));
    setupKnob(velSlopeSlider, juce::Colour(0xff80cbc4), false, 0.0); // Exponential default
    velSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    setupKnob(velDecaySlider, juce::Colour(0xff80cbc4), true, 0.5);  // 0% default (bipolar)
    setupKnob(velDepthSlider, juce::Colour(0xff80cbc4), true, 0.5);  // 0% default (bipolar)
    setupKnob(velVolumeSlider, juce::Colour(0xff80cbc4), false, 0.0); // 0% default (unipolar)

    velSlopeSlider.customFormatText = [](double val) {
        if (val < 0.33) return "Exp (" + juce::String(static_cast<int>(val * 300.0)) + "%)";
        if (val < 0.67) return "Lin (" + juce::String(static_cast<int>((val - 0.33) * 300.0)) + "%)";
        return "Log (" + juce::String(static_cast<int>((val - 0.67) * 300.0)) + "%)";
    };
    auto formatBipolarVel = [](double val) {
        float eff = TbdAudio::warpBipolarExp(static_cast<float>(val));
        if (std::abs(eff) <= 0.0001f) return juce::String("0%");
        float pct = eff * 100.0f;
        juce::String sign = (pct >= 0.0f) ? "+" : "";
        if (std::abs(eff) < 0.0999f) {
            return sign + juce::String(pct, 1) + "%";
        }
        return sign + juce::String(static_cast<int>(std::round(pct))) + "%";
    };
    auto parseBipolarVel = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0) / 100.0;
        return static_cast<double>(TbdAudio::unwarpBipolarExp(static_cast<float>(p)));
    };

    velDecaySlider.customFormatText = formatBipolarVel;
    velDecaySlider.customParseText  = parseBipolarVel;
    velDepthSlider.customFormatText = formatBipolarVel;
    velDepthSlider.customParseText  = parseBipolarVel;

    velVolumeSlider.customFormatText = [](double val) {
        float eff = TbdAudio::warpUnipolarExp(static_cast<float>(val));
        if (eff <= 0.0001f) return juce::String("0%");
        if (eff < 0.0999f) {
            return "-" + juce::String(eff * 100.0f, 1) + "%";
        }
        return "-" + juce::String(static_cast<int>(std::round(eff * 100.0f))) + "%";
    };
    velVolumeSlider.customParseText = [](const juce::String& text) {
        double p = std::clamp(std::abs(parseNumberSafe(text, 0.0)) / 100.0, 0.0, 1.0);
        return static_cast<double>(TbdAudio::unwarpUnipolarExp(static_cast<float>(p)));
    };

    cardVel->setKnob(0, "Slope", &velSlopeSlider);
    cardVel->setKnob(1, "Decay", &velDecaySlider);
    cardVel->setKnob(2, "Depth", &velDepthSlider);
    cardVel->setKnob(3, "Volume", &velVolumeSlider);
    cards.push_back(std::move(cardVel));

    // 22. SLOP CARD (4 Knobs)
    auto cardSlop = std::make_unique<ModuleCardComponent>("Slop", juce::Colour(0xffffa726));
    setupKnob(slopFreqSlider, juce::Colour(0xffffa726), false, 0.0); // 0% default (unipolar)
    setupKnob(slopDepthSlider, juce::Colour(0xffffa726), false, 0.0); // 0% default (unipolar)
    setupKnob(slopDecaySlider, juce::Colour(0xffffa726), false, 0.0); // 0% default (unipolar)
    setupKnob(slopPanSlider, juce::Colour(0xffffa726), false, 0.0);   // 0% default (unipolar)

    auto formatSlop = [](double val) {
        float eff = TbdAudio::warpUnipolarExp(static_cast<float>(val));
        if (eff <= 0.0001f) return juce::String("0%");
        if (eff < 0.0999f) {
            return "+/-" + juce::String(eff * 100.0f, 1) + "%";
        }
        return "+/-" + juce::String(static_cast<int>(std::round(eff * 100.0f))) + "%";
    };
    auto parseSlop = [](const juce::String& text) {
        double p = std::clamp(std::abs(parseNumberSafe(text, 0.0)) / 100.0, 0.0, 1.0);
        return static_cast<double>(TbdAudio::unwarpUnipolarExp(static_cast<float>(p)));
    };

    slopFreqSlider.customFormatText = formatSlop;
    slopFreqSlider.customParseText  = parseSlop;

    slopDepthSlider.customFormatText = formatSlop;
    slopDepthSlider.customParseText  = parseSlop;

    slopDecaySlider.customFormatText = formatSlop;
    slopDecaySlider.customParseText  = parseSlop;

    slopPanSlider.customFormatText = formatSlop;
    slopPanSlider.customParseText  = parseSlop;

    cardSlop->setKnob(0, "Freq", &slopFreqSlider);
    cardSlop->setKnob(1, "Depth", &slopDepthSlider);
    cardSlop->setKnob(2, "Decay", &slopDecaySlider);
    cardSlop->setKnob(3, "Pan", &slopPanSlider);
    cards.push_back(std::move(cardSlop));

    // Add all 22 cards to editor
    for (auto& c : cards) {
        addAndMakeVisible(c.get());
    }

    // Attach all APVTS parameters
    // 1. Carrier 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier1_tracking", carrier1TrackingBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier1_pitch", carrier1PitchSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier1_shape", carrier1ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier1_depth", carrier1DepthSlider));

    // 2. Modulator 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_track", mod1TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_type", mod1TypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod1_shape", mod1ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod1_speed", mod1SpeedSlider));

    // 3. Pitch Envelope 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv1_target", pitchEnv1TargetBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv1_slope", pitchEnv1SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv1_depth", pitchEnv1DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv1_decay", pitchEnv1DecaySlider));

    // 4. Carrier 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier2_tracking", carrier2TrackingBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_pitch", carrier2PitchSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_shape", carrier2ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_depth", carrier2DepthSlider));

    // 5. Modulator 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_track", mod2TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_type", mod2TypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod2_shape", mod2ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod2_speed", mod2SpeedSlider));

    // 6. Pitch Envelope 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv2_target", pitchEnv2TargetBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_slope", pitchEnv2SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_depth", pitchEnv2DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_decay", pitchEnv2DecaySlider));

    // 7. Noise Transient
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_sh_rate", noiseShRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_filter", noiseFilterSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_drive", noiseDriveSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_decay", noiseDecaySlider));

    // 8. Mixer
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_carrier1_level", mixerCarrier1LevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_carrier2_level", mixerCarrier2LevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_ringmod", mixerRingModSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_noise_level", mixerNoiseLevelSlider));

    // 9. Drive
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_amount", driveAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_bias", driveBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_filter", driveFilterSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "drive_limiter", driveLimiterBox));

    // 10. Filter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter_type", filterTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter_slope", filterSlopeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_cutoff", filterCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_resonance", filterResonanceSlider));

    // 11. Filter Envelope
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_slope", filterEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_depth", filterEnvDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_decay", filterEnvDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_postdrive", filterEnvPostDriveSlider));

    // 12. Wave Folder
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "wavefolder_type", waveFolderTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_fold", waveFolderFoldSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_bias", waveFolderBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_filter", waveFolderFilterSlider));

    // 13. RingMod
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_shape", ringModShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_rate", ringModRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_amount", ringModAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_width", ringModWidthSlider));

    // 14. Frequency Shifter
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_shift", freqShiftShiftSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_range", freqShiftRangeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_blend", freqShiftBlendSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_width", freqShiftWidthSlider));

    // 15. Grit FX
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_bits", gritBitsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_rate", gritRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_low", gritLowSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_high", gritHighSlider));

    // 16. Comb Filter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "comb_type", combTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_dampening", combDampeningSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_cutoff", combCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_resonance", combResonanceSlider));

    // 17. Disperser
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "disperser_type", disperserTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_amount", disperserAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_cutoff", disperserCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_resonance", disperserResonanceSlider));

    // 18. EQ (Bell EQ)
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_freq", eqFreqSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_width", eqWidthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_gain", eqGainSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_filter", eqFilterSlider));

    // 19. Amp
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_level", ampLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_pan", ampPanSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_drive", ampDriveSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_limiter", ampLimiterBox));

    // 20. Amp Envelope
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_claps", ampEnvClapsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_clapspeed", ampEnvClapSpeedSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_slope", ampEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_decay", ampEnvDecaySlider));

    // 21. Velocity
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_slope", velSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_decay", velDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_depth", velDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_volume", velVolumeSlider));

    // 22. Slop
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_freq", slopFreqSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_depth", slopDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_decay", slopDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_pan", slopPanSlider));

    updateDynamicControls();
    setSize(1840, 760);
    setResizable(true, true);
    setResizeLimits(1200, 520, 2880, 1600);
    startTimerHz(30); // 30 FPS oscilloscope & GUI update
}

TheKlangFarmerAudioProcessorEditor::~TheKlangFarmerAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void TheKlangFarmerAudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar, double defaultVal) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 20);
    slider.setScrollWheelEnabled(true);
    slider.setBipolar(isBipolar);
    slider.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
    slider.setColour(juce::Slider::trackColourId, trackColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff161922));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe8edf5));
    slider.setRange(0.0, 1.0, 0.0005);
    slider.setDoubleClickReturnValue(true, defaultVal);
    slider.getDefaultValue = [defaultVal]() { return defaultVal; };
}

void TheKlangFarmerAudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setVisible(false);
}

void TheKlangFarmerAudioProcessorEditor::resetToDefaults() {
    for (auto* param : audioProcessor.getParameters()) {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            rangedParam->setValueNotifyingHost(rangedParam->getDefaultValue());
        }
    }
    updateDynamicControls();
    repaint();
}

void TheKlangFarmerAudioProcessorEditor::bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                                              const juce::String& paramId, const juce::StringArray& items, int numColumns) {
    box.clear();
    box.addItemList(items, 1);
    box.setVisible(false);
    selector.setItems(items, numColumns);
    selector.onChange = [this, &box, paramId](int idx) {
        box.setSelectedId(idx + 1, juce::sendNotification);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        }
    };
}

void TheKlangFarmerAudioProcessorEditor::updateDynamicControls() {
    auto syncSelector = [this](juce::ComboBox& box, LedSelectorComponent& selector, const juce::String& paramId, int& lastVal) {
        int idx = box.getSelectedItemIndex();
        if (idx < 0) {
            if (auto* param = audioProcessor.apvts.getRawParameterValue(paramId)) {
                idx = static_cast<int>(param->load());
            }
        }
        if (idx >= 0 && idx != lastVal) {
            selector.setSelectedIndex(idx, juce::dontSendNotification);
            lastVal = idx;
        }
        return idx;
    };

    int curCarrier1Track = syncSelector(carrier1TrackingBox, carrier1TrackingSelector, "carrier1_tracking", lastCarrier1Track);
    if (curCarrier1Track >= 0) {
        carrier1PitchSlider.setBipolar(curCarrier1Track == 2);
        carrier1PitchSlider.updateText();
    }

    int curMod1Track = syncSelector(mod1TrackBox, mod1TrackSelector, "mod1_track", lastMod1Track);
    int curMod1Type  = syncSelector(mod1TypeBox, mod1TypeSelector, "mod1_type", lastMod1Type);
    if (curMod1Type >= 0 || curMod1Track >= 0) {
        if (curMod1Type == 0) {
            cards[1]->setKnobLabel(0, "Shape");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod1ShapeSlider.setBipolar(false);
        } else if (curMod1Type == 1) {
            cards[1]->setKnobLabel(0, "DJ Filter");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(true);
        } else {
            cards[1]->setKnobLabel(0, "S&H Rate");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(false);
        }
        mod1ShapeSlider.repaint();
        mod1ShapeSlider.updateText();
        mod1SpeedSlider.updateText();
    }

    syncSelector(pitchEnv1TargetBox, pitchEnv1TargetSelector, "pitchenv1_target", lastPitchEnv1Target);

    int curCarrier2Track = syncSelector(carrier2TrackingBox, carrier2TrackingSelector, "carrier2_tracking", lastCarrier2Track);
    if (curCarrier2Track >= 0) {
        carrier2PitchSlider.setBipolar(curCarrier2Track == 2);
        carrier2PitchSlider.updateText();
    }

    int curMod2Track = syncSelector(mod2TrackBox, mod2TrackSelector, "mod2_track", lastMod2Track);
    int curMod2Type  = syncSelector(mod2TypeBox, mod2TypeSelector, "mod2_type", lastMod2Type);
    if (curMod2Type >= 0 || curMod2Track >= 0) {
        if (curMod2Type == 0) {
            cards[4]->setKnobLabel(0, "Shape");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod2ShapeSlider.setBipolar(false);
        } else if (curMod2Type == 1) {
            cards[4]->setKnobLabel(0, "DJ Filter");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(true);
        } else {
            cards[4]->setKnobLabel(0, "S&H Rate");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(false);
        }
        mod2ShapeSlider.repaint();
        mod2ShapeSlider.updateText();
        mod2SpeedSlider.updateText();
    }

    syncSelector(pitchEnv2TargetBox, pitchEnv2TargetSelector, "pitchenv2_target", lastPitchEnv2Target);

    syncSelector(driveLimiterBox, driveLimiterSelector, "drive_limiter", lastDriveLimiter);

    int curFilterType  = syncSelector(filterTypeBox, filterTypeSelector, "filter_type", lastFilterType);
    int curFilterSlope = syncSelector(filterSlopeBox, filterSlopeSelector, "filter_slope", lastFilterSlope);

    // Update Filter X-Y Frequency vs Gain response plot
    float fCutNorm = static_cast<float>(filterCutoffSlider.getValue());
    float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
    float fRes = static_cast<float>(filterResonanceSlider.getValue());
    cards[9]->updateFilterParams(curFilterType, curFilterSlope, fCutHz, fRes);

    // Update Bell EQ X-Y Frequency vs Gain response plot
    float eqFNorm = static_cast<float>(eqFreqSlider.getValue());
    float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
    float eqWNorm = static_cast<float>(eqWidthSlider.getValue());
    float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
    float eqGNorm = static_cast<float>(eqGainSlider.getValue());
    float eqGDb = (eqGNorm - 0.5f) * 48.0f;
    float eqDjNorm = static_cast<float>(eqFilterSlider.getValue());
    cards[17]->updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);

    syncSelector(waveFolderTypeBox, waveFolderTypeSelector, "wavefolder_type", lastWaveFolderType);

    syncSelector(combTypeBox, combTypeSelector, "comb_type", lastCombType);
    syncSelector(disperserTypeBox, disperserTypeSelector, "disperser_type", lastDisperserType);
    syncSelector(ampLimiterBox, ampLimiterSelector, "amp_limiter", lastAmpLimiter);
}

void TheKlangFarmerAudioProcessorEditor::timerCallback() {
    updateDynamicControls();

    // Fetch and display synchronized oscilloscope buffers across modules (Filter & EQ use XY plots)
    float scopeBuffer[128];
    for (int b = 0; b < static_cast<int>(cards.size()); ++b) {
        if (b == 9 || b == 17) continue; // 9 = BLK_FILTER, 17 = BLK_EQ
        audioProcessor.getEngine().getScopeData(b, scopeBuffer, 128);
        cards[b]->updateScope(scopeBuffer, 128);
    }
}

void TheKlangFarmerAudioProcessorEditor::paint(juce::Graphics& g) {
    // Top-to-bottom subtle gradient
    juce::ColourGradient bgGrad(juce::Colour(0xff12141a), 0, 0,
                                juce::Colour(0xff0a0b0e), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Header bar
    g.setColour(juce::Colour(0xff171a22));
    g.fillRect(0, 0, getWidth(), 36);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(36, 0.0f, static_cast<float>(getWidth()));

    // Title branding
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER", 14, 0, 165, 36, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("22-MODULE DUAL FM HARDWARE SYNTHESIS DRUM VOICE", 182, 0, 480, 36, juce::Justification::centredLeft);

    // Subtle rack-mount placeholder frames for the 2 blank slots
    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;
    int numCols = 8;
    int numRows = 3;
    int cardW = (totalW - (numCols - 1) * margin) / numCols;
    int rowH  = (totalH - (numRows - 1) * margin) / numRows;

    static const std::pair<int, int> blankSlots[2] = { { 2, 6 }, { 2, 7 } };
    for (const auto& slot : blankSlots) {
        int x = margin + slot.second * (cardW + margin);
        int y = topOffset + slot.first * (rowH + margin);
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                            static_cast<float>(cardW), static_cast<float>(rowH));
        g.setColour(juce::Colour(0xff12141a));
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(juce::Colour(0xff1a1e28));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
    }
}

void TheKlangFarmerAudioProcessorEditor::resized() {
    initButton.setBounds(getWidth() - 232, 5, 84, 26);
    triggerButton.setBounds(getWidth() - 138, 5, 124, 26);

    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;

    int numCols = 8;
    int numRows = 3;
    int cardW = (totalW - (numCols - 1) * margin) / numCols;
    int rowH  = (totalH - (numRows - 1) * margin) / numRows;

    // 3 Rows x 8 Columns Grid Layout (22 modules + 2 blank slots):
    // Row 0: Carrier 1 (0), Mod 1 (1), Pitch Env 1 (2), Carrier 2 (3), Mod 2 (4), Pitch Env 2 (5), Noise Transient (6), Mixer (7)
    // Row 1: Drive (8), Filter (9), Filter Env (10), Wave Folder (11), RingMod (12), Freq Shift (13), Grit FX (14), Comb Filter (15)
    // Row 2: Disperser (16), Bell EQ (17), Amp (18), Amp Env (19), Velocity (20), Slop (21), [Blank] (22), [Blank] (23)

    static const std::pair<int, int> cardGridPositions[22] = {
        { 0, 0 }, // 0: Carrier 1
        { 0, 1 }, // 1: Modulator 1
        { 0, 2 }, // 2: Pitch Env 1
        { 0, 3 }, // 3: Carrier 2
        { 0, 4 }, // 4: Modulator 2
        { 0, 5 }, // 5: Pitch Env 2
        { 0, 6 }, // 6: Noise Transient
        { 0, 7 }, // 7: Mixer
        { 1, 0 }, // 8: Drive
        { 1, 1 }, // 9: Filter
        { 1, 2 }, // 10: Filter Env
        { 1, 3 }, // 11: Wave Folder
        { 1, 4 }, // 12: RingMod
        { 1, 5 }, // 13: Freq Shifter
        { 1, 6 }, // 14: Grit FX
        { 1, 7 }, // 15: Comb Filter
        { 2, 0 }, // 16: Disperser
        { 2, 1 }, // 17: Bell EQ
        { 2, 2 }, // 18: Amp
        { 2, 3 }, // 19: Amp Env
        { 2, 4 }, // 20: Velocity
        { 2, 5 }  // 21: Slop
    };

    for (int i = 0; i < static_cast<int>(cards.size()) && i < 22; ++i) {
        int row = cardGridPositions[i].first;
        int col = cardGridPositions[i].second;
        int x = margin + col * (cardW + margin);
        int y = topOffset + row * (rowH + margin);
        cards[i]->setBounds(x, y, cardW, rowH);
    }
}

