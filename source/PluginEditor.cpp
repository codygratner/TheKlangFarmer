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

// Formatters and parsers
static juce::String formatMidiNote(double val) {
    int note = static_cast<int>(std::round(val * 127.0));
    return getMidiNoteName(note);
}
static double parseMidiNote(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 36.0) / 127.0, 0.0, 1.0);
}

static juce::String formatSemi(double val) {
    float semi = static_cast<float>((val - 0.5) * 48.0);
    return juce::String(semi, 1) + " st";
}
static double parseSemi(const juce::String& text) {
    double semi = parseNumberSafe(text, 0.0);
    return std::clamp((semi / 48.0) + 0.5, 0.0, 1.0);
}

static juce::String formatRatio(double val) {
    float r = static_cast<float>(val) * 16.0f;
    return juce::String(r, 2) + "x";
}
static double parseRatio(const juce::String& text) {
    double r = parseNumberSafe(text, 1.0);
    return std::clamp(r / 16.0, 0.0, 1.0);
}

static juce::String formatPercent(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
static double parsePercent(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 50.0) / 100.0, 0.0, 1.0);
}

static juce::String formatBipolarPercent(double val) {
    int p = static_cast<int>(std::round((val - 0.5) * 200.0));
    return (p > 0 ? "+" : "") + juce::String(p) + "%";
}
static double parseBipolarPercent(const juce::String& text) {
    double p = parseNumberSafe(text, 0.0);
    return std::clamp((p / 200.0) + 0.5, 0.0, 1.0);
}

static juce::String formatTimeMs(double val) {
    float ms = 5.0f * std::pow(60000.0f / 5.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 333.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 5.0, 60000.0);
    return std::log(ms / 5.0) / std::log(60000.0 / 5.0);
}

static juce::String formatNoiseTimeMs(double val) {
    float ms = 1.0f * std::pow(60000.0f / 1.0f, static_cast<float>(val));
    if (ms >= 1000.0f) return juce::String(ms / 1000.0f, 2) + " s";
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseNoiseTimeMs(const juce::String& text) {
    double ms = parseNumberSafe(text, 100.0);
    if (text.containsIgnoreCase("s") && !text.containsIgnoreCase("ms")) ms *= 1000.0;
    ms = std::clamp(ms, 1.0, 60000.0);
    return std::log(ms / 1.0) / std::log(60000.0 / 1.0);
}

static juce::String formatFreqHz(double val) {
    float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    if (hz >= 10.0f) return juce::String(hz, 1) + " Hz";
    return juce::String(hz, 2) + " Hz";
}
static double parseFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 0.1, 24000.0);
    return std::log(hz / 0.1) / std::log(24000.0 / 0.1);
}

static juce::String formatEqFreqHz(double val) {
    float hz = 20.0f * std::pow(24000.0f / 20.0f, static_cast<float>(val));
    if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
    return juce::String(static_cast<int>(std::round(hz))) + " Hz";
}
static double parseEqFreqHz(const juce::String& text) {
    double hz = parseNumberSafe(text, 1000.0);
    hz = std::clamp(hz, 20.0, 24000.0);
    return std::log(hz / 20.0) / std::log(24000.0 / 20.0);
}

static juce::String formatDb(double val) {
    float db = -6.0f + static_cast<float>(val) * 30.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 6.0) / 30.0, 0.0, 1.0);
}

static juce::String formatBipolarDb(double val) {
    float db = static_cast<float>((val - 0.5) * 48.0);
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseBipolarDb(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db / 48.0) + 0.5, 0.0, 1.0);
}

static juce::String formatMixerLevel(double val) {
    float pct = (val <= 0.5) ? static_cast<float>(val * 200.0) : static_cast<float>(100.0 + (val - 0.5) * 600.0);
    return juce::String(static_cast<int>(std::round(pct))) + "%";
}
static double parseMixerLevel(const juce::String& text) {
    double pct = parseNumberSafe(text, 100.0);
    pct = std::clamp(pct, 0.0, 400.0);
    if (pct <= 100.0) return pct / 200.0;
    return 0.5 + (pct - 100.0) / 600.0;
}

static juce::String formatOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 10.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
static double parseOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 10.0) + 0.5, 0.0, 1.0);
}

static juce::String formatFilterOctaves(double val) {
    float oct = static_cast<float>((val - 0.5) * 20.0);
    return (oct > 0 ? "+" : "") + juce::String(oct, 1) + " oct";
}
static double parseFilterOctaves(const juce::String& text) {
    double oct = parseNumberSafe(text, 0.0);
    return std::clamp((oct / 20.0) + 0.5, 0.0, 1.0);
}

static juce::String formatBits(double val) {
    float b = 1.0f + static_cast<float>(val) * 15.0f;
    return juce::String(b, 1) + " bit";
}
static double parseBits(const juce::String& text) {
    double b = parseNumberSafe(text, 16.0);
    return std::clamp((b - 1.0) / 15.0, 0.0, 1.0);
}

static juce::String formatWavefolds(double val) {
    float f = static_cast<float>(val) * 8.0f;
    return juce::String(f, 2);
}
static double parseWavefolds(const juce::String& text) {
    double f = parseNumberSafe(text, 0.0);
    return std::clamp(f / 8.0, 0.0, 1.0);
}

static juce::String formatStages(double val) {
    int s = static_cast<int>(std::round(val * 32.0));
    return juce::String(s);
}
static double parseStages(const juce::String& text) {
    double s = parseNumberSafe(text, 4.0);
    return std::clamp(s / 32.0, 0.0, 1.0);
}

static juce::String formatClaps(double val) {
    int c = static_cast<int>(std::round(val * 32.0));
    return juce::String(c);
}
static double parseClaps(const juce::String& text) {
    double c = parseNumberSafe(text, 0.0);
    return std::clamp(c / 32.0, 0.0, 1.0);
}

static juce::String formatClapSpeed(double val) {
    float ms = 1.0f + static_cast<float>(val) * 14.0f;
    return juce::String(ms, 1) + " ms";
}
static double parseClapSpeed(const juce::String& text) {
    double ms = parseNumberSafe(text, 3.0);
    return std::clamp((ms - 1.0) / 14.0, 0.0, 1.0);
}

static juce::String formatLimiterGain(double val) {
    float db = -12.0f + static_cast<float>(val) * 36.0f;
    return (db > 0 ? "+" : "") + juce::String(db, 1) + " dB";
}
static double parseLimiterGain(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 12.0) / 36.0, 0.0, 1.0);
}

static juce::String formatLimiterThresh(double val) {
    float db = -24.0f + static_cast<float>(val) * 24.0f;
    return juce::String(db, 1) + " dB";
}
static double parseLimiterThresh(const juce::String& text) {
    double db = parseNumberSafe(text, 0.0);
    return std::clamp((db + 24.0) / 24.0, 0.0, 1.0);
}

static juce::String formatLimiterRelease(double val) {
    float ms = 1.0f * std::pow(500.0f / 1.0f, static_cast<float>(val));
    return juce::String(static_cast<int>(std::round(ms))) + " ms";
}
static double parseLimiterRelease(const juce::String& text) {
    double ms = parseNumberSafe(text, 50.0);
    ms = std::clamp(ms, 1.0, 500.0);
    return std::log(ms / 1.0) / std::log(500.0 / 1.0);
}

static juce::String formatSlop(double val) {
    return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
}
static double parseSlop(const juce::String& text) {
    return std::clamp(parseNumberSafe(text, 0.0) / 100.0, 0.0, 1.0);
}

static juce::String formatVelocitySlope(double val) {
    if (val < 0.45) return "EXP";
    if (val > 0.55) return "LOG";
    return "LIN";
}
static double parseVelocitySlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.0;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.5;
}

static juce::String formatSlope(double val) {
    if (val < 0.45) return "EXP";
    if (val > 0.55) return "LOG";
    return "LIN";
}
static double parseSlope(const juce::String& text) {
    if (text.containsIgnoreCase("exp")) return 0.0;
    if (text.containsIgnoreCase("log")) return 1.0;
    return 0.5;
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

    float trackHeight = 6.0f;
    float trackY = static_cast<float>(y) + (static_cast<float>(height) - trackHeight) * 0.5f;
    float trackX = static_cast<float>(x) + 2.0f;
    float trackW = static_cast<float>(width) - 4.0f;
    juce::Rectangle<float> trackRect(trackX, trackY, trackW, trackHeight);

    g.setColour(juce::Colour(0xff161922));
    g.fillRoundedRectangle(trackRect, 3.0f);
    g.setColour(juce::Colour(0xff2a3040));
    g.drawRoundedRectangle(trackRect, 3.0f, 1.0f);

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

    float thumbW = 7.0f;
    float thumbH = std::min(static_cast<float>(height) - 2.0f, 16.0f);
    float thumbY = static_cast<float>(y) + (static_cast<float>(height) - thumbH) * 0.5f;
    float thumbX = std::clamp(sliderPos - thumbW * 0.5f, static_cast<float>(x), static_cast<float>(x + width) - thumbW);

    juce::Rectangle<float> thumbRect(thumbX, thumbY, thumbW, thumbH);

    if (slider.isMouseOverOrDragging()) {
        g.setColour(fillColour.withAlpha(0.35f));
        g.drawRoundedRectangle(thumbRect.expanded(1.5f), 2.5f, 1.5f);
    }

    juce::ColourGradient thumbGrad(juce::Colour(0xffeff3fa), thumbX, thumbY,
                                  juce::Colour(0xffb0bac9), thumbX, thumbY + thumbH, false);
    g.setGradientFill(thumbGrad);
    g.fillRoundedRectangle(thumbRect, 2.0f);
    g.setColour(juce::Colour(0xff12151c));
    g.drawRoundedRectangle(thumbRect, 2.0f, 1.0f);
    g.setColour(fillColour);
    g.drawVerticalLine(static_cast<int>(thumbRect.getCentreX()), thumbY + 2.5f, thumbY + thumbH - 2.5f);
}

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jlimit(11.0f, 24.0f, juce::jmin(bounds.getWidth() * 0.46f, bounds.getHeight() * 0.46f));
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = juce::jmax(2.4f, radius * 0.18f);

    auto arcRadius = radius - lineW * 0.5f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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

        if (slider.isMouseOverOrDragging()) {
            auto fillCol = slider.findColour(juce::Slider::rotarySliderFillColourId);
            g.setColour(fillCol.withAlpha(0.30f));
            g.drawEllipse(knobBounds.expanded(1.5f), 1.0f);
        }

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

    g.setColour(juce::Colour(0xff0e1017));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff1d222e));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    if (plotMode == PlotMode::Oscilloscope) {
        float midY = bounds.getCentreY();
        g.setColour(juce::Colour(0xff1b202c));
        g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

        if (points.empty()) return;

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

        g.setColour(traceCol.withAlpha(0.25f));
        g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(traceCol);
        g.strokePath(p, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float left = bounds.getX();
    float top = bounds.getY();
    float right = bounds.getRight();
    float bottom = bounds.getBottom();

    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 24000.0f;
    float logRange = std::log(maxFreq / minFreq);

    auto freqToX = [&](float f) -> float {
        float norm = std::log(std::clamp(f, minFreq, maxFreq) / minFreq) / logRange;
        return left + norm * width;
    };

    g.setColour(juce::Colour(0xff181c26));
    float x100 = freqToX(100.0f);
    float x1k  = freqToX(1000.0f);
    float x10k = freqToX(10000.0f);
    g.drawVerticalLine(static_cast<int>(x100), top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x1k),  top + 2.0f, bottom - 2.0f);
    g.drawVerticalLine(static_cast<int>(x10k), top + 2.0f, bottom - 2.0f);

    g.setColour(juce::Colour(0xff333a4d));
    g.setFont(juce::FontOptions(8.5f));
    g.drawText("100", static_cast<int>(x100) - 12, static_cast<int>(bottom) - 12, 24, 10, juce::Justification::centred);
    g.drawText("1k",  static_cast<int>(x1k) - 10,   static_cast<int>(bottom) - 12, 20, 10, juce::Justification::centred);
    g.drawText("10k", static_cast<int>(x10k) - 12, static_cast<int>(bottom) - 12, 24, 10, juce::Justification::centred);

    juce::Path curvePath;
    float yZero = top + height * 0.5f;

    if (plotMode == PlotMode::FilterXY) {
        yZero = top + height * 0.35f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

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

        if (filterType != 0) {
            float xCutoff = freqToX(fc);
            g.setColour(traceCol.withAlpha(0.2f));
            g.drawVerticalLine(static_cast<int>(xCutoff), top + 2.0f, bottom - 2.0f);
        }
    } else if (plotMode == PlotMode::EqXY) {
        yZero = top + height * 0.5f;
        g.setColour(juce::Colour(0xff222838));
        g.drawHorizontalLine(static_cast<int>(yZero), left + 2.0f, right - 2.0f);

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

            float octDist = std::log2(f / f0);
            float term = (2.0f * octDist) / bw;
            float bellDb = eqGain * (1.0f / (1.0f + term * term));

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

        float xPeak = freqToX(f0);
        float yPeak = yZero - (eqGain / 24.0f) * (height * 0.42f);
        yPeak = std::clamp(yPeak, top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol.withAlpha(0.25f));
        g.drawVerticalLine(static_cast<int>(xPeak), top + 2.0f, bottom - 2.0f);
        g.setColour(traceCol);
        g.fillEllipse(xPeak - 2.5f, yPeak - 2.5f, 5.0f, 5.0f);
    }

    juce::Path fillPath(curvePath);
    fillPath.lineTo(right, yZero);
    fillPath.lineTo(left, yZero);
    fillPath.closeSubPath();
    g.setColour(traceCol.withAlpha(0.12f));
    g.fillPath(fillPath);

    g.setColour(traceCol.withAlpha(0.25f));
    g.strokePath(curvePath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
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

// --- ROTARY KNOB SLIDER ---

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

    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText(moduleTitle.toUpperCase(), 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void ModuleCardComponent::resized() {
    oscilloscope.setVisible(false); // Visualization is hosted in Slot 8

    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(22); // Title header
    area.removeFromTop(4);

    int count = 4;
    if (ledSelector != nullptr && secondLedSelector != nullptr) {
        int selH1 = (ledSelector->getNumItems() > 4) ? 28 : 22;
        int selH2 = (secondLedSelector->getNumItems() > 4) ? 28 : 22;
        ledSelector->setBounds(area.removeFromTop(selH1));
        area.removeFromTop(4);
        secondLedSelector->setBounds(area.removeFromTop(selH2));
        area.removeFromTop(6);
        count = 2;
        labels[2].setVisible(false);
        labels[3].setVisible(false);
        if (knobs[2]) knobs[2]->setVisible(false);
        if (knobs[3]) knobs[3]->setVisible(false);
    } else if (ledSelector != nullptr) {
        int selH = (ledSelector->getNumItems() > 4) ? 36 : 24;
        ledSelector->setBounds(area.removeFromTop(selH));
        area.removeFromTop(6);
        count = 3;
        labels[3].setVisible(false);
        if (knobs[3]) knobs[3]->setVisible(false);
    }

    int rowH = area.getHeight() / count;
    int labelW = 66;

    for (int i = 0; i < count; ++i) {
        auto row = area.removeFromTop(rowH).reduced(0, 2);
        labels[i].setVisible(true);
        labels[i].setBounds(row.removeFromLeft(labelW));
        row.removeFromLeft(4);
        if (knobs[i]) {
            knobs[i]->setVisible(true);
            knobs[i]->setBounds(row);
        }
    }
}

// --- BLANK PLATE COMPONENT ---

void BlankPlateComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient grad(juce::Colour(0xff161920), bounds.getX(), bounds.getY(),
                              juce::Colour(0xff0d0f14), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto drawScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff1f2430));
        g.fillEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xff384254));
        g.drawEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f, 1.0f);
        g.setColour(juce::Colour(0xff101318));
        g.drawLine(cx - 3.0f, cy, cx + 3.0f, cy, 1.2f);
    };

    drawScrew(bounds.getX() + 12.0f, bounds.getY() + 12.0f);
    drawScrew(bounds.getRight() - 12.0f, bounds.getY() + 12.0f);
    drawScrew(bounds.getX() + 12.0f, bounds.getBottom() - 12.0f);
    drawScrew(bounds.getRight() - 12.0f, bounds.getBottom() - 12.0f);

    g.setColour(juce::Colour(0xff1e222d));
    g.drawHorizontalLine(static_cast<int>(bounds.getCentreY()), bounds.getX() + 24.0f, bounds.getRight() - 24.0f);
}

// --- NAVIGATION CARD COMPONENT ---

NavigationCardComponent::NavigationCardComponent() {
    for (int i = 0; i < pageNames.size(); ++i) {
        auto btn = std::make_unique<juce::TextButton>(pageNames[i]);
        btn->setClickingTogglesState(false);
        int pageIdx = i;
        btn->onClick = [this, pageIdx]() {
            setSelectedPage(pageIdx);
            if (onPageSelected) onPageSelected(pageIdx);
        };
        addAndMakeVisible(btn.get());
        buttons.push_back(std::move(btn));
    }
    setSelectedPage(0);
}

void NavigationCardComponent::setSelectedPage(int pageIndex) {
    selectedPage = std::clamp(pageIndex, 0, (int)pageNames.size() - 1);
    for (int i = 0; i < (int)buttons.size(); ++i) {
        if (i == selectedPage) {
            buttons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
            buttons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
        } else {
            buttons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181c26));
            buttons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffb8c4d8));
        }
    }
    repaint();
}

void NavigationCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(juce::Colour(0xff00d2ff));
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff00d2ff));
    g.drawText("PAGES", 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void NavigationCardComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(22);
    area.removeFromTop(6);

    int n = static_cast<int>(buttons.size());
    if (n == 0) return;
    int gap = 6;
    int btnH = (area.getHeight() - (n - 1) * gap) / n;

    for (int i = 0; i < n; ++i) {
        buttons[i]->setBounds(area.removeFromTop(btnH));
        area.removeFromTop(gap);
    }
}

// --- FX PICKER CARD COMPONENT ---

FXPickerCardComponent::FXPickerCardComponent(const juce::String& titleText, juce::Colour accentCol)
    : title(titleText), accent(accentCol)
{
    const char* slotNames[4] = { "FX SLOT 1", "FX SLOT 2", "FX SLOT 3", "FX SLOT 4" };
    for (int i = 0; i < 4; ++i) {
        labels[i].setText(slotNames[i], juce::dontSendNotification);
        labels[i].setFont(juce::FontOptions(12.0f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        addAndMakeVisible(labels[i]);

        boxes[i].setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff14161d));
        boxes[i].setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2c3240));
        boxes[i].setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
        boxes[i].setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8b95a8));
        addAndMakeVisible(boxes[i]);
    }
}

void FXPickerCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText(title.toUpperCase(), 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void FXPickerCardComponent::resized() {
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(22);
    area.removeFromTop(8);

    int rowH = area.getHeight() / 4;
    for (int i = 0; i < 4; ++i) {
        auto row = area.removeFromTop(rowH).reduced(0, 4);
        labels[i].setBounds(row.removeFromTop(20));
        row.removeFromTop(4);
        boxes[i].setBounds(row.removeFromTop(28));
    }
}

// --- VISUALIZATION CARD COMPONENT ---

VisualizationCardComponent::VisualizationCardComponent(juce::Colour accentColour)
    : accent(accentColour), oscilloscope(accentColour)
{
    addAndMakeVisible(oscilloscope);
}

void VisualizationCardComponent::setAvailableTabs(const juce::StringArray& tabNames, const std::vector<int>& blockIndices) {
    currentTabs = tabNames;
    currentBlockIndices = blockIndices;

    tabButtons.clear();
    for (int i = 0; i < currentTabs.size(); ++i) {
        auto btn = std::make_unique<juce::TextButton>(currentTabs[i]);
        btn->setClickingTogglesState(false);
        int tabIdx = i;
        btn->onClick = [this, tabIdx]() {
            selectTab(tabIdx);
            if (onTabSelected && tabIdx < (int)currentBlockIndices.size()) {
                onTabSelected(currentBlockIndices[tabIdx]);
            }
        };
        addAndMakeVisible(btn.get());
        tabButtons.push_back(std::move(btn));
    }

    if (selectedTab >= (int)tabButtons.size()) {
        selectedTab = 0;
    }
    updateButtonStyles();
    resized();
}

void VisualizationCardComponent::selectTab(int tabIndex) {
    if (tabIndex >= 0 && tabIndex < (int)tabButtons.size()) {
        selectedTab = tabIndex;
        updateButtonStyles();
    }
}

void VisualizationCardComponent::selectBlock(int blockIndex) {
    for (int i = 0; i < (int)currentBlockIndices.size(); ++i) {
        if (currentBlockIndices[i] == blockIndex) {
            selectTab(i);
            break;
        }
    }
}

int VisualizationCardComponent::getCurrentBlockIndex() const {
    if (selectedTab >= 0 && selectedTab < (int)currentBlockIndices.size()) {
        return currentBlockIndices[selectedTab];
    }
    return -1;
}

juce::String VisualizationCardComponent::getCurrentTabName() const {
    if (selectedTab >= 0 && selectedTab < (int)currentTabs.size()) {
        return currentTabs[selectedTab];
    }
    return {};
}

void VisualizationCardComponent::updateButtonStyles() {
    for (int i = 0; i < (int)tabButtons.size(); ++i) {
        if (i == selectedTab) {
            tabButtons[i]->setColour(juce::TextButton::buttonColourId, accent);
            tabButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
        } else {
            tabButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1e28));
            tabButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
        }
    }
    repaint();
}

void VisualizationCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff151821));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff222736));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.setColour(accent);
    g.drawText("VISUALIZER", 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void VisualizationCardComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(22);
    area.removeFromTop(4);

    int numTabs = static_cast<int>(tabButtons.size());
    if (numTabs > 0) {
        int cols = (numTabs <= 4) ? 2 : 3;
        int rows = (numTabs + cols - 1) / cols;
        int tabGridH = rows * 24 + (rows - 1) * 3;
        auto tabArea = area.removeFromTop(tabGridH);
        area.removeFromTop(6);

        float colW = static_cast<float>(tabArea.getWidth() - (cols - 1) * 3) / static_cast<float>(cols);
        for (int i = 0; i < numTabs; ++i) {
            int c = i % cols;
            int r = i / cols;
            int x = tabArea.getX() + static_cast<int>(c * (colW + 3));
            int y = tabArea.getY() + r * 27;
            tabButtons[i]->setBounds(x, y, static_cast<int>(colW), 24);
        }
    }

    oscilloscope.setBounds(area);
}


// --- THE KLANG FARMER AUDIO PROCESSOR EDITOR CONSTRUCTOR ---

TheKlangFarmerAudioProcessorEditor::TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      carrier1TrackingSelector(juce::Colour(0xff00d2ff)),
      mod1TrackSelector(juce::Colour(0xffff7043)),
      mod1TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv1TargetSelector(juce::Colour(0xffffab00)),
      filter1TypeSelector(juce::Colour(0xff7c4dff)),
      filter1SlopeSelector(juce::Colour(0xff7c4dff)),
      carrier2TrackingSelector(juce::Colour(0xff00d2ff)),
      mod2TrackSelector(juce::Colour(0xffff7043)),
      mod2TypeSelector(juce::Colour(0xffff7043)),
      pitchEnv2TargetSelector(juce::Colour(0xffffab00)),
      filter2TypeSelector(juce::Colour(0xff7c4dff)),
      filter2SlopeSelector(juce::Colour(0xff7c4dff)),
      filter3TypeSelector(juce::Colour(0xff7c4dff)),
      filter3SlopeSelector(juce::Colour(0xff7c4dff)),
      driveLimiterSelector(juce::Colour(0xffff4081)),
      fxFilterTypeSelector(juce::Colour(0xff7c4dff)),
      fxFilterSlopeSelector(juce::Colour(0xff7c4dff)),
      waveFolderTypeSelector(juce::Colour(0xffff5252)),
      combTypeSelector(juce::Colour(0xff26a69a)),
      disperserTypeSelector(juce::Colour(0xffec407a)),
      ampLimiterSelector(juce::Colour(0xff00e5ff)),
      preLimiterEnableSelector(juce::Colour(0xffffab00)),
      postLimiterEnableSelector(juce::Colour(0xff00e5ff)),
      vizCard(juce::Colour(0xff00d2ff))
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

    // Permanent Slot 1 and Slot 8
    navCard.onPageSelected = [this](int pageIndex) {
        setPage(pageIndex);
    };
    addAndMakeVisible(navCard);

    vizCard.onTabSelected = [this](int /*blockIndex*/) {
        repaint();
    };
    addAndMakeVisible(vizCard);

    // Blank plates
    for (int i = 0; i < 6; ++i) {
        addChildComponent(&blankPlates[i]);
    }

    // FX Pickers
    preFXPickerCard = std::make_unique<FXPickerCardComponent>("PRE-AMP FX PICKER", juce::Colour(0xffff7043));
    addChildComponent(preFXPickerCard.get());

    postFXPickerCard = std::make_unique<FXPickerCardComponent>("POST-AMP FX PICKER", juce::Colour(0xff00e5ff));
    addChildComponent(postFXPickerCard.get());

    // --- CREATE MODULE CARDS & CONTROLS ---

    // 1. Carrier 1
    cardCarrier1 = std::make_unique<ModuleCardComponent>("Carrier 1", juce::Colour(0xff00d2ff));
    setupBox(carrier1TrackingBox);
    bindSelector(carrier1TrackingSelector, carrier1TrackingBox, "carrier1_tracking", { "Midi Track", "Fixed Freq", "Fine Semi" });
    cardCarrier1->setLedSelector(&carrier1TrackingSelector);

    setupKnob(carrier1PitchSlider, juce::Colour(0xff00d2ff), false, 36.0 / 127.0);
    carrier1PitchSlider.customFormatText = [this](double val) {
        if (carrier1TrackingBox.getSelectedItemIndex() == 2) return formatSemi(val);
        if (carrier1TrackingBox.getSelectedItemIndex() == 1) return formatFreqHz(val);
        return formatMidiNote(val);
    };
    carrier1PitchSlider.customParseText = [this](const juce::String& text) {
        if (carrier1TrackingBox.getSelectedItemIndex() == 2) return parseSemi(text);
        if (carrier1TrackingBox.getSelectedItemIndex() == 1) return parseFreqHz(text);
        return parseMidiNote(text);
    };

    setupKnob(carrier1ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier1DepthSlider, juce::Colour(0xff00d2ff), false, 0.5);
    carrier1DepthSlider.customFormatText = formatPercent;
    carrier1DepthSlider.customParseText  = parsePercent;

    cardCarrier1->setKnob(0, "Pitch", &carrier1PitchSlider);
    cardCarrier1->setKnob(1, "Shape", &carrier1ShapeSlider);
    cardCarrier1->setKnob(2, "Mod Depth", &carrier1DepthSlider);
    addChildComponent(cardCarrier1.get());

    // 2. Modulator 1
    cardMod1 = std::make_unique<ModuleCardComponent>("Modulator 1", juce::Colour(0xffff7043));
    setupBox(mod1TrackBox);
    bindSelector(mod1TrackSelector, mod1TrackBox, "mod1_track", { "Fixed", "Follow", "FM Op" });
    setupBox(mod1TypeBox);
    bindSelector(mod1TypeSelector, mod1TypeBox, "mod1_type", { "Osc", "Cyclic", "Noise" });
    cardMod1->setLedSelector(&mod1TrackSelector);
    cardMod1->setSecondLedSelector(&mod1TypeSelector);

    setupKnob(mod1ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod1ShapeSlider.customFormatText = [this](double val) {
        if (mod1TypeBox.getSelectedItemIndex() == 1) return formatBipolarPercent(val);
        if (mod1TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        return formatPercent(val);
    };
    mod1ShapeSlider.customParseText = [this](const juce::String& text) {
        if (mod1TypeBox.getSelectedItemIndex() == 1) return parseBipolarPercent(text);
        if (mod1TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        return parsePercent(text);
    };

    setupKnob(mod1SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);
    mod1SpeedSlider.customFormatText = [this](double val) {
        if (mod1TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod1SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod1TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod1->setKnob(0, "Shape", &mod1ShapeSlider);
    cardMod1->setKnob(1, "Speed", &mod1SpeedSlider);
    addChildComponent(cardMod1.get());

    // 3. Pitch Envelope 1
    cardPitchEnv1 = std::make_unique<ModuleCardComponent>("Pitch Env 1", juce::Colour(0xffffab00));
    setupBox(pitchEnv1TargetBox);
    bindSelector(pitchEnv1TargetSelector, pitchEnv1TargetBox, "pitchenv1_target", { "Off", "Carrier", "Mod", "Both" }, 2);
    cardPitchEnv1->setLedSelector(&pitchEnv1TargetSelector);

    setupKnob(pitchEnv1SlopeSlider, juce::Colour(0xffffab00), false, 0.0);
    pitchEnv1SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnv1SlopeSlider.customFormatText = formatSlope;
    pitchEnv1SlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnv1DepthSlider, juce::Colour(0xffffab00), true, 0.5);
    pitchEnv1DepthSlider.customFormatText = formatOctaves;
    pitchEnv1DepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnv1DecaySlider, juce::Colour(0xffffab00), false, 0.3806);
    pitchEnv1DecaySlider.customFormatText = formatTimeMs;
    pitchEnv1DecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv1->setKnob(0, "Slope", &pitchEnv1SlopeSlider);
    cardPitchEnv1->setKnob(1, "Depth", &pitchEnv1DepthSlider);
    cardPitchEnv1->setKnob(2, "Decay", &pitchEnv1DecaySlider);
    addChildComponent(cardPitchEnv1.get());

    // 4. Filter 1
    cardFilter1 = std::make_unique<ModuleCardComponent>("Filter 1", juce::Colour(0xff7c4dff));
    setupBox(filter1TypeBox);
    bindSelector(filter1TypeSelector, filter1TypeBox, "filter1_type", { "Off", "LPF", "BPF", "HPF", "BRF" }, 3);
    setupBox(filter1SlopeBox);
    bindSelector(filter1SlopeSelector, filter1SlopeBox, "filter1_slope", { "-6dB", "-12dB", "-18dB", "-24dB", "-36dB" }, 3);
    cardFilter1->setLedSelector(&filter1TypeSelector);
    cardFilter1->setSecondLedSelector(&filter1SlopeSelector);

    setupKnob(filter1CutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filter1CutoffSlider.customFormatText = formatFreqHz;
    filter1CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter1ResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filter1ResonanceSlider.customFormatText = formatPercent;
    filter1ResonanceSlider.customParseText  = parsePercent;

    cardFilter1->setKnob(0, "Cutoff", &filter1CutoffSlider);
    cardFilter1->setKnob(1, "Resonance", &filter1ResonanceSlider);
    addChildComponent(cardFilter1.get());

    // 5. Filter Envelope 1
    cardFilterEnv1 = std::make_unique<ModuleCardComponent>("Filter Env 1", juce::Colour(0xff7c4dff));
    setupKnob(filterEnv1SlopeSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filterEnv1SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv1SlopeSlider.customFormatText = formatSlope;
    filterEnv1SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv1DepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnv1DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv1DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv1DecaySlider, juce::Colour(0xff7c4dff), false, 0.3806);
    filterEnv1DecaySlider.customFormatText = formatTimeMs;
    filterEnv1DecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnv1PostDriveSlider, juce::Colour(0xff7c4dff), false, 0.5);
    filterEnv1PostDriveSlider.customFormatText = formatDb;
    filterEnv1PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv1->setKnob(0, "Slope", &filterEnv1SlopeSlider);
    cardFilterEnv1->setKnob(1, "Depth", &filterEnv1DepthSlider);
    cardFilterEnv1->setKnob(2, "Decay", &filterEnv1DecaySlider);
    cardFilterEnv1->setKnob(3, "Post-Drive", &filterEnv1PostDriveSlider);
    addChildComponent(cardFilterEnv1.get());

    // 6. Carrier 2
    cardCarrier2 = std::make_unique<ModuleCardComponent>("Carrier 2", juce::Colour(0xff00d2ff));
    setupBox(carrier2TrackingBox);
    bindSelector(carrier2TrackingSelector, carrier2TrackingBox, "carrier2_tracking", { "Midi Track", "Fixed Freq", "Fine Semi" });
    cardCarrier2->setLedSelector(&carrier2TrackingSelector);

    setupKnob(carrier2PitchSlider, juce::Colour(0xff00d2ff), false, 36.0 / 127.0);
    carrier2PitchSlider.customFormatText = [this](double val) {
        if (carrier2TrackingBox.getSelectedItemIndex() == 2) return formatSemi(val);
        if (carrier2TrackingBox.getSelectedItemIndex() == 1) return formatFreqHz(val);
        return formatMidiNote(val);
    };
    carrier2PitchSlider.customParseText = [this](const juce::String& text) {
        if (carrier2TrackingBox.getSelectedItemIndex() == 2) return parseSemi(text);
        if (carrier2TrackingBox.getSelectedItemIndex() == 1) return parseFreqHz(text);
        return parseMidiNote(text);
    };

    setupKnob(carrier2ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier2DepthSlider, juce::Colour(0xff00d2ff), false, 0.5);
    carrier2DepthSlider.customFormatText = formatPercent;
    carrier2DepthSlider.customParseText  = parsePercent;

    cardCarrier2->setKnob(0, "Pitch", &carrier2PitchSlider);
    cardCarrier2->setKnob(1, "Shape", &carrier2ShapeSlider);
    cardCarrier2->setKnob(2, "Mod Depth", &carrier2DepthSlider);
    addChildComponent(cardCarrier2.get());

    // 7. Modulator 2
    cardMod2 = std::make_unique<ModuleCardComponent>("Modulator 2", juce::Colour(0xffff7043));
    setupBox(mod2TrackBox);
    bindSelector(mod2TrackSelector, mod2TrackBox, "mod2_track", { "Fixed", "Follow", "FM Op" });
    setupBox(mod2TypeBox);
    bindSelector(mod2TypeSelector, mod2TypeBox, "mod2_type", { "Osc", "Cyclic", "Noise" });
    cardMod2->setLedSelector(&mod2TrackSelector);
    cardMod2->setSecondLedSelector(&mod2TypeSelector);

    setupKnob(mod2ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod2ShapeSlider.customFormatText = [this](double val) {
        if (mod2TypeBox.getSelectedItemIndex() == 1) return formatBipolarPercent(val);
        if (mod2TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        return formatPercent(val);
    };
    mod2ShapeSlider.customParseText = [this](const juce::String& text) {
        if (mod2TypeBox.getSelectedItemIndex() == 1) return parseBipolarPercent(text);
        if (mod2TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        return parsePercent(text);
    };

    setupKnob(mod2SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);
    mod2SpeedSlider.customFormatText = [this](double val) {
        if (mod2TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod2SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod2TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod2->setKnob(0, "Shape", &mod2ShapeSlider);
    cardMod2->setKnob(1, "Speed", &mod2SpeedSlider);
    addChildComponent(cardMod2.get());

    // 8. Pitch Envelope 2
    cardPitchEnv2 = std::make_unique<ModuleCardComponent>("Pitch Env 2", juce::Colour(0xffffab00));
    setupBox(pitchEnv2TargetBox);
    bindSelector(pitchEnv2TargetSelector, pitchEnv2TargetBox, "pitchenv2_target", { "Off", "Carrier", "Mod", "Both" }, 2);
    cardPitchEnv2->setLedSelector(&pitchEnv2TargetSelector);

    setupKnob(pitchEnv2SlopeSlider, juce::Colour(0xffffab00), false, 0.0);
    pitchEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnv2SlopeSlider.customFormatText = formatSlope;
    pitchEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnv2DepthSlider, juce::Colour(0xffffab00), true, 0.5);
    pitchEnv2DepthSlider.customFormatText = formatOctaves;
    pitchEnv2DepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnv2DecaySlider, juce::Colour(0xffffab00), false, 0.3806);
    pitchEnv2DecaySlider.customFormatText = formatTimeMs;
    pitchEnv2DecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv2->setKnob(0, "Slope", &pitchEnv2SlopeSlider);
    cardPitchEnv2->setKnob(1, "Depth", &pitchEnv2DepthSlider);
    cardPitchEnv2->setKnob(2, "Decay", &pitchEnv2DecaySlider);
    addChildComponent(cardPitchEnv2.get());

    // 9. Filter 2
    cardFilter2 = std::make_unique<ModuleCardComponent>("Filter 2", juce::Colour(0xff7c4dff));
    setupBox(filter2TypeBox);
    bindSelector(filter2TypeSelector, filter2TypeBox, "filter2_type", { "Off", "LPF", "BPF", "HPF", "BRF" }, 3);
    setupBox(filter2SlopeBox);
    bindSelector(filter2SlopeSelector, filter2SlopeBox, "filter2_slope", { "-6dB", "-12dB", "-18dB", "-24dB", "-36dB" }, 3);
    cardFilter2->setLedSelector(&filter2TypeSelector);
    cardFilter2->setSecondLedSelector(&filter2SlopeSelector);

    setupKnob(filter2CutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filter2CutoffSlider.customFormatText = formatFreqHz;
    filter2CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter2ResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filter2ResonanceSlider.customFormatText = formatPercent;
    filter2ResonanceSlider.customParseText  = parsePercent;

    cardFilter2->setKnob(0, "Cutoff", &filter2CutoffSlider);
    cardFilter2->setKnob(1, "Resonance", &filter2ResonanceSlider);
    addChildComponent(cardFilter2.get());

    // 10. Filter Envelope 2
    cardFilterEnv2 = std::make_unique<ModuleCardComponent>("Filter Env 2", juce::Colour(0xff7c4dff));
    setupKnob(filterEnv2SlopeSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filterEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv2SlopeSlider.customFormatText = formatSlope;
    filterEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv2DepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnv2DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv2DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv2DecaySlider, juce::Colour(0xff7c4dff), false, 0.3806);
    filterEnv2DecaySlider.customFormatText = formatTimeMs;
    filterEnv2DecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnv2PostDriveSlider, juce::Colour(0xff7c4dff), false, 0.5);
    filterEnv2PostDriveSlider.customFormatText = formatDb;
    filterEnv2PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv2->setKnob(0, "Slope", &filterEnv2SlopeSlider);
    cardFilterEnv2->setKnob(1, "Depth", &filterEnv2DepthSlider);
    cardFilterEnv2->setKnob(2, "Decay", &filterEnv2DecaySlider);
    cardFilterEnv2->setKnob(3, "Post-Drive", &filterEnv2PostDriveSlider);
    addChildComponent(cardFilterEnv2.get());

    // 11. Noise Transient
    cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", juce::Colour(0xff90a4ae));
    setupKnob(noiseShRateSlider, juce::Colour(0xff90a4ae), false, 1.0);
    noiseShRateSlider.customFormatText = formatFreqHz;
    noiseShRateSlider.customParseText  = parseFreqHz;

    setupKnob(noiseFilterSlider, juce::Colour(0xff90a4ae), true, 0.5);
    noiseFilterSlider.customFormatText = formatBipolarPercent;
    noiseFilterSlider.customParseText  = parseBipolarPercent;

    setupKnob(noiseDriveSlider, juce::Colour(0xff90a4ae), false, 0.5);
    noiseDriveSlider.customFormatText = formatDb;
    noiseDriveSlider.customParseText  = parseDb;

    setupKnob(noiseDecaySlider, juce::Colour(0xff90a4ae), false, 0.3078);
    noiseDecaySlider.customFormatText = formatNoiseTimeMs;
    noiseDecaySlider.customParseText  = parseNoiseTimeMs;

    cardNoise->setKnob(0, "S&H Rate", &noiseShRateSlider);
    cardNoise->setKnob(1, "Filter", &noiseFilterSlider);
    cardNoise->setKnob(2, "Drive", &noiseDriveSlider);
    cardNoise->setKnob(3, "Decay", &noiseDecaySlider);
    addChildComponent(cardNoise.get());

    // 12. Filter 3 (Transients Filter)
    cardFilter3 = std::make_unique<ModuleCardComponent>("Filter 3", juce::Colour(0xff7c4dff));
    setupBox(filter3TypeBox);
    bindSelector(filter3TypeSelector, filter3TypeBox, "filter3_type", { "Off", "LPF", "BPF", "HPF", "BRF" }, 3);
    setupBox(filter3SlopeBox);
    bindSelector(filter3SlopeSelector, filter3SlopeBox, "filter3_slope", { "-6dB", "-12dB", "-18dB", "-24dB", "-36dB" }, 3);
    cardFilter3->setLedSelector(&filter3TypeSelector);
    cardFilter3->setSecondLedSelector(&filter3SlopeSelector);

    setupKnob(filter3CutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filter3CutoffSlider.customFormatText = formatFreqHz;
    filter3CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter3ResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filter3ResonanceSlider.customFormatText = formatPercent;
    filter3ResonanceSlider.customParseText  = parsePercent;

    cardFilter3->setKnob(0, "Cutoff", &filter3CutoffSlider);
    cardFilter3->setKnob(1, "Resonance", &filter3ResonanceSlider);
    addChildComponent(cardFilter3.get());

    // 13. Filter Envelope 3 (Transients Filter Env)
    cardFilterEnv3 = std::make_unique<ModuleCardComponent>("Filter Env 3", juce::Colour(0xff7c4dff));
    setupKnob(filterEnv3SlopeSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filterEnv3SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv3SlopeSlider.customFormatText = formatSlope;
    filterEnv3SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv3DepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnv3DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv3DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv3DecaySlider, juce::Colour(0xff7c4dff), false, 0.3078);
    filterEnv3DecaySlider.customFormatText = formatNoiseTimeMs;
    filterEnv3DecaySlider.customParseText  = parseNoiseTimeMs;

    setupKnob(filterEnv3PostDriveSlider, juce::Colour(0xff7c4dff), false, 0.5);
    filterEnv3PostDriveSlider.customFormatText = formatDb;
    filterEnv3PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv3->setKnob(0, "Slope", &filterEnv3SlopeSlider);
    cardFilterEnv3->setKnob(1, "Depth", &filterEnv3DepthSlider);
    cardFilterEnv3->setKnob(2, "Decay", &filterEnv3DecaySlider);
    cardFilterEnv3->setKnob(3, "Post-Drive", &filterEnv3PostDriveSlider);
    addChildComponent(cardFilterEnv3.get());

    // 14. Mixer
    cardMixer = std::make_unique<ModuleCardComponent>("Mixer", juce::Colour(0xff26a69a));
    setupKnob(mixerCarrier1LevelSlider, juce::Colour(0xff26a69a), false, 0.5);
    mixerCarrier1LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier1LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerCarrier2LevelSlider, juce::Colour(0xff26a69a), false, 0.0);
    mixerCarrier2LevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerCarrier2LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier2LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerRingModSlider, juce::Colour(0xff26a69a), false, 0.0);
    mixerRingModSlider.setDoubleClickReturnValue(true, 0.5);
    mixerRingModSlider.customFormatText = formatMixerLevel;
    mixerRingModSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerNoiseLevelSlider, juce::Colour(0xff26a69a), false, 0.0);
    mixerNoiseLevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerNoiseLevelSlider.customFormatText = formatMixerLevel;
    mixerNoiseLevelSlider.customParseText  = parseMixerLevel;

    cardMixer->setKnob(0, "Carrier 1", &mixerCarrier1LevelSlider);
    cardMixer->setKnob(1, "Carrier 2", &mixerCarrier2LevelSlider);
    cardMixer->setKnob(2, "RingMod", &mixerRingModSlider);
    cardMixer->setKnob(3, "Noise", &mixerNoiseLevelSlider);
    addChildComponent(cardMixer.get());

    // 15. Drive
    cardDrive = std::make_unique<ModuleCardComponent>("Drive", juce::Colour(0xffff4081));
    setupBox(driveLimiterBox);
    bindSelector(driveLimiterSelector, driveLimiterBox, "drive_limiter", { "Off", "On" });
    cardDrive->setLedSelector(&driveLimiterSelector);

    setupKnob(driveAmountSlider, juce::Colour(0xffff4081), false, 0.5);
    driveAmountSlider.customFormatText = formatDb;
    driveAmountSlider.customParseText  = parseDb;

    setupKnob(driveBiasSlider, juce::Colour(0xffff4081), true, 0.5);
    driveBiasSlider.customFormatText = formatBipolarPercent;
    driveBiasSlider.customParseText  = parseBipolarPercent;

    setupKnob(driveFilterSlider, juce::Colour(0xffff4081), true, 0.5);
    driveFilterSlider.customFormatText = formatBipolarPercent;
    driveFilterSlider.customParseText  = parseBipolarPercent;

    cardDrive->setKnob(0, "Drive", &driveAmountSlider);
    cardDrive->setKnob(1, "Bias", &driveBiasSlider);
    cardDrive->setKnob(2, "Filter", &driveFilterSlider);
    addChildComponent(cardDrive.get());

    // 16. Standalone FX Filter
    cardFXFilter = std::make_unique<ModuleCardComponent>("FX Filter", juce::Colour(0xff7c4dff));
    setupBox(fxFilterTypeBox);
    bindSelector(fxFilterTypeSelector, fxFilterTypeBox, "fxfilter_type", { "Off", "LPF", "BPF", "HPF", "BRF" }, 3);
    setupBox(fxFilterSlopeBox);
    bindSelector(fxFilterSlopeSelector, fxFilterSlopeBox, "fxfilter_slope", { "-6dB", "-12dB", "-18dB", "-24dB", "-36dB" }, 3);
    cardFXFilter->setLedSelector(&fxFilterTypeSelector);
    cardFXFilter->setSecondLedSelector(&fxFilterSlopeSelector);

    setupKnob(fxFilterCutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    fxFilterCutoffSlider.customFormatText = formatFreqHz;
    fxFilterCutoffSlider.customParseText  = parseFreqHz;

    setupKnob(fxFilterResonanceSlider, juce::Colour(0xff7c4dff), false, 0.0);
    fxFilterResonanceSlider.customFormatText = formatPercent;
    fxFilterResonanceSlider.customParseText  = parsePercent;

    cardFXFilter->setKnob(0, "Cutoff", &fxFilterCutoffSlider);
    cardFXFilter->setKnob(1, "Resonance", &fxFilterResonanceSlider);
    addChildComponent(cardFXFilter.get());

    // 17. Wave Folder
    cardWaveFolder = std::make_unique<ModuleCardComponent>("Wave Folder", juce::Colour(0xffff5252));
    setupBox(waveFolderTypeBox);
    bindSelector(waveFolderTypeSelector, waveFolderTypeBox, "wavefolder_type", { "Off", "On" });
    cardWaveFolder->setLedSelector(&waveFolderTypeSelector);

    setupKnob(waveFolderFoldSlider, juce::Colour(0xffff5252), false, 0.0);
    waveFolderFoldSlider.customFormatText = formatWavefolds;
    waveFolderFoldSlider.customParseText  = parseWavefolds;

    setupKnob(waveFolderBiasSlider, juce::Colour(0xffff5252), true, 0.5);
    waveFolderBiasSlider.customFormatText = formatBipolarPercent;
    waveFolderBiasSlider.customParseText  = parseBipolarPercent;

    setupKnob(waveFolderFilterSlider, juce::Colour(0xffff5252), true, 0.5);
    waveFolderFilterSlider.customFormatText = formatBipolarPercent;
    waveFolderFilterSlider.customParseText  = parseBipolarPercent;

    cardWaveFolder->setKnob(0, "Fold", &waveFolderFoldSlider);
    cardWaveFolder->setKnob(1, "Bias", &waveFolderBiasSlider);
    cardWaveFolder->setKnob(2, "Filter", &waveFolderFilterSlider);
    addChildComponent(cardWaveFolder.get());

    // 18. RingMod
    cardRingMod = std::make_unique<ModuleCardComponent>("RingMod FX", juce::Colour(0xffff7043));
    setupKnob(ringModShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    ringModShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    ringModShapeSlider.customFormatText = formatPercent;
    ringModShapeSlider.customParseText  = parsePercent;

    setupKnob(ringModRateSlider, juce::Colour(0xffff7043), false, 0.50934);
    ringModRateSlider.customFormatText = formatFreqHz;
    ringModRateSlider.customParseText  = parseFreqHz;

    setupKnob(ringModAmountSlider, juce::Colour(0xffff7043), false, 0.0);
    ringModAmountSlider.customFormatText = formatPercent;
    ringModAmountSlider.customParseText  = parsePercent;

    setupKnob(ringModWidthSlider, juce::Colour(0xffff7043), true, 0.5);
    ringModWidthSlider.customFormatText = formatBipolarPercent;
    ringModWidthSlider.customParseText  = parseBipolarPercent;

    cardRingMod->setKnob(0, "Waveform", &ringModShapeSlider);
    cardRingMod->setKnob(1, "Rate", &ringModRateSlider);
    cardRingMod->setKnob(2, "Amount", &ringModAmountSlider);
    cardRingMod->setKnob(3, "Width", &ringModWidthSlider);
    addChildComponent(cardRingMod.get());

    // 19. Frequency Shifter
    cardFreqShift = std::make_unique<ModuleCardComponent>("Freq Shifter", juce::Colour(0xff00e676));
    setupKnob(freqShiftShiftSlider, juce::Colour(0xff00e676), true, 0.5);
    freqShiftShiftSlider.customFormatText = [this](double val) {
        float maxRange = TbdAudio::normToRangeHz(static_cast<float>(freqShiftRangeSlider.getValue()));
        float hz = static_cast<float>((val - 0.5) * 2.0 * maxRange);
        return (hz > 0 ? "+" : "") + juce::String(hz, 1) + " Hz";
    };
    freqShiftShiftSlider.customParseText = [this](const juce::String& text) {
        float maxRange = TbdAudio::normToRangeHz(static_cast<float>(freqShiftRangeSlider.getValue()));
        double hz = parseNumberSafe(text, 0.0);
        return std::clamp((hz / (2.0 * maxRange)) + 0.5, 0.0, 1.0);
    };

    setupKnob(freqShiftRangeSlider, juce::Colour(0xff00e676), false, TbdAudio::rangeHzToNorm(3.0f));
    freqShiftRangeSlider.customFormatText = [](double val) {
        float hz = TbdAudio::normToRangeHz(static_cast<float>(val));
        if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
        return juce::String(hz, 1) + " Hz";
    };
    freqShiftRangeSlider.customParseText = [](const juce::String& text) {
        double hz = parseNumberSafe(text, 3.0);
        return static_cast<double>(TbdAudio::rangeHzToNorm(static_cast<float>(hz)));
    };

    setupKnob(freqShiftBlendSlider, juce::Colour(0xff00e676), true, 0.5);
    freqShiftBlendSlider.customFormatText = [](double val) {
        int b = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (b == 0) return juce::String("Dry");
        return (b > 0 ? "+" : "") + juce::String(b) + "%";
    };
    freqShiftBlendSlider.customParseText = parseBipolarPercent;

    setupKnob(freqShiftWidthSlider, juce::Colour(0xff00e676), true, 0.5);
    freqShiftWidthSlider.customFormatText = formatBipolarPercent;
    freqShiftWidthSlider.customParseText  = parseBipolarPercent;

    cardFreqShift->setKnob(0, "Shift", &freqShiftShiftSlider);
    cardFreqShift->setKnob(1, "Range", &freqShiftRangeSlider);
    cardFreqShift->setKnob(2, "Blend", &freqShiftBlendSlider);
    cardFreqShift->setKnob(3, "Width", &freqShiftWidthSlider);
    addChildComponent(cardFreqShift.get());

    // 20. Grit FX
    cardGrit = std::make_unique<ModuleCardComponent>("Grit FX", juce::Colour(0xffff9100));
    setupKnob(gritBitsSlider, juce::Colour(0xffff9100), false, 1.0);
    gritBitsSlider.customFormatText = formatBits;
    gritBitsSlider.customParseText  = parseBits;

    setupKnob(gritRateSlider, juce::Colour(0xffff9100), false, 1.0);
    gritRateSlider.customFormatText = formatFreqHz;
    gritRateSlider.customParseText  = parseFreqHz;

    setupKnob(gritLowSlider, juce::Colour(0xffff9100), true, 0.5);
    gritLowSlider.customFormatText = formatBipolarDb;
    gritLowSlider.customParseText  = parseBipolarDb;

    setupKnob(gritHighSlider, juce::Colour(0xffff9100), true, 0.5);
    gritHighSlider.customFormatText = formatBipolarDb;
    gritHighSlider.customParseText  = parseBipolarDb;

    cardGrit->setKnob(0, "Bit Rate", &gritBitsSlider);
    cardGrit->setKnob(1, "Sample Rate", &gritRateSlider);
    cardGrit->setKnob(2, "Low", &gritLowSlider);
    cardGrit->setKnob(3, "High", &gritHighSlider);
    addChildComponent(cardGrit.get());

    // 21. Comb Filter
    cardComb = std::make_unique<ModuleCardComponent>("Comb Filter", juce::Colour(0xff26a69a));
    setupBox(combTypeBox);
    bindSelector(combTypeSelector, combTypeBox, "comb_type", { "Off", "On" });
    cardComb->setLedSelector(&combTypeSelector);

    setupKnob(combDampeningSlider, juce::Colour(0xff26a69a), false, 1.0);
    combDampeningSlider.customFormatText = formatFreqHz;
    combDampeningSlider.customParseText  = parseFreqHz;

    setupKnob(combCutoffSlider, juce::Colour(0xff26a69a), false, 1.0);
    combCutoffSlider.customFormatText = formatFreqHz;
    combCutoffSlider.customParseText  = parseFreqHz;

    setupKnob(combResonanceSlider, juce::Colour(0xff26a69a), true, 0.5);
    combResonanceSlider.customFormatText = formatBipolarPercent;
    combResonanceSlider.customParseText  = parseBipolarPercent;

    cardComb->setKnob(0, "Dampening", &combDampeningSlider);
    cardComb->setKnob(1, "Cutoff", &combCutoffSlider);
    cardComb->setKnob(2, "Resonance", &combResonanceSlider);
    addChildComponent(cardComb.get());

    // 22. Disperser
    cardDisperser = std::make_unique<ModuleCardComponent>("Disperser", juce::Colour(0xffec407a));
    setupBox(disperserTypeBox);
    bindSelector(disperserTypeSelector, disperserTypeBox, "disperser_type", { "Off", "On" });
    cardDisperser->setLedSelector(&disperserTypeSelector);

    setupKnob(disperserAmountSlider, juce::Colour(0xffec407a), false, 4.0 / 32.0);
    disperserAmountSlider.customFormatText = formatStages;
    disperserAmountSlider.customParseText  = parseStages;

    setupKnob(disperserCutoffSlider, juce::Colour(0xffec407a), false, 0.62124);
    disperserCutoffSlider.customFormatText = formatFreqHz;
    disperserCutoffSlider.customParseText  = parseFreqHz;

    setupKnob(disperserResonanceSlider, juce::Colour(0xffec407a), true, 0.5);
    disperserResonanceSlider.customFormatText = formatBipolarPercent;
    disperserResonanceSlider.customParseText  = parseBipolarPercent;

    cardDisperser->setKnob(0, "Amount", &disperserAmountSlider);
    cardDisperser->setKnob(1, "Cutoff", &disperserCutoffSlider);
    cardDisperser->setKnob(2, "Resonance", &disperserResonanceSlider);
    addChildComponent(cardDisperser.get());

    // 23. Bell EQ
    cardEQ = std::make_unique<ModuleCardComponent>("Bell EQ", juce::Colour(0xff29b6f6));
    setupKnob(eqFreqSlider, juce::Colour(0xff29b6f6), false, 1.0);
    eqFreqSlider.customFormatText = formatEqFreqHz;
    eqFreqSlider.customParseText  = parseEqFreqHz;

    setupKnob(eqWidthSlider, juce::Colour(0xff29b6f6), false, 0.0);
    eqWidthSlider.customFormatText = [](double val) {
        float oct = 0.1f * std::pow(100.0f, static_cast<float>(val));
        return juce::String(oct, 2) + " oct";
    };
    eqWidthSlider.customParseText = [](const juce::String& text) {
        double oct = parseNumberSafe(text, 0.1);
        oct = std::clamp(oct, 0.1, 10.0);
        return std::log(oct / 0.1) / std::log(100.0);
    };

    setupKnob(eqGainSlider, juce::Colour(0xff29b6f6), true, 0.5);
    eqGainSlider.customFormatText = formatBipolarDb;
    eqGainSlider.customParseText  = parseBipolarDb;

    setupKnob(eqFilterSlider, juce::Colour(0xff29b6f6), true, 0.5);
    eqFilterSlider.customFormatText = formatBipolarPercent;
    eqFilterSlider.customParseText  = parseBipolarPercent;

    cardEQ->setKnob(0, "Freq", &eqFreqSlider);
    cardEQ->setKnob(1, "Width", &eqWidthSlider);
    cardEQ->setKnob(2, "Gain", &eqGainSlider);
    cardEQ->setKnob(3, "DJ Filter", &eqFilterSlider);
    addChildComponent(cardEQ.get());

    // 24. Amp
    cardAmp = std::make_unique<ModuleCardComponent>("Amplifier", juce::Colour(0xff00e5ff));
    setupBox(ampLimiterBox);
    bindSelector(ampLimiterSelector, ampLimiterBox, "amp_limiter", { "Off", "On" });
    cardAmp->setLedSelector(&ampLimiterSelector);

    setupKnob(ampLevelSlider, juce::Colour(0xff00e5ff), false, 1.0);
    ampLevelSlider.customFormatText = formatPercent;
    ampLevelSlider.customParseText  = parsePercent;

    setupKnob(ampPanSlider, juce::Colour(0xff00e5ff), true, 0.5);
    ampPanSlider.customFormatText = [](double val) {
        int p = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (p == 0) return juce::String("Center");
        return (p < 0 ? juce::String(-p) + "% L" : juce::String(p) + "% R");
    };
    ampPanSlider.customParseText = parseBipolarPercent;

    setupKnob(ampDriveSlider, juce::Colour(0xff00e5ff), false, 0.5);
    ampDriveSlider.customFormatText = formatDb;
    ampDriveSlider.customParseText  = parseDb;

    cardAmp->setKnob(0, "Level", &ampLevelSlider);
    cardAmp->setKnob(1, "Pan", &ampPanSlider);
    cardAmp->setKnob(2, "Drive", &ampDriveSlider);
    addChildComponent(cardAmp.get());

    // 25. Amp Envelope
    cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Envelope", juce::Colour(0xff00e5ff));
    setupKnob(ampEnvClapsSlider, juce::Colour(0xff00e5ff), false, 0.0);
    ampEnvClapsSlider.customFormatText = formatClaps;
    ampEnvClapsSlider.customParseText  = parseClaps;

    setupKnob(ampEnvClapSpeedSlider, juce::Colour(0xff00e5ff), false, 2.0 / 14.0);
    ampEnvClapSpeedSlider.customFormatText = formatClapSpeed;
    ampEnvClapSpeedSlider.customParseText  = parseClapSpeed;

    setupKnob(ampEnvSlopeSlider, juce::Colour(0xff00e5ff), false, 0.0);
    ampEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    ampEnvSlopeSlider.customFormatText = formatSlope;
    ampEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(ampEnvDecaySlider, juce::Colour(0xff00e5ff), false, 0.3806);
    ampEnvDecaySlider.customFormatText = formatTimeMs;
    ampEnvDecaySlider.customParseText  = parseTimeMs;

    cardAmpEnv->setKnob(0, "Claps", &ampEnvClapsSlider);
    cardAmpEnv->setKnob(1, "Clap Speed", &ampEnvClapSpeedSlider);
    cardAmpEnv->setKnob(2, "Slope", &ampEnvSlopeSlider);
    cardAmpEnv->setKnob(3, "Decay", &ampEnvDecaySlider);
    addChildComponent(cardAmpEnv.get());

    // 26. Pre-Amp Limiter
    cardPreLimiter = std::make_unique<ModuleCardComponent>("Pre Limiter", juce::Colour(0xffffab00));
    setupBox(preLimiterEnableBox);
    bindSelector(preLimiterEnableSelector, preLimiterEnableBox, "pre_limiter_enable", { "Off", "On" });
    cardPreLimiter->setLedSelector(&preLimiterEnableSelector);

    setupKnob(preLimiterGainSlider, juce::Colour(0xffffab00), false, 12.0 / 36.0);
    preLimiterGainSlider.customFormatText = formatLimiterGain;
    preLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(preLimiterThreshSlider, juce::Colour(0xffffab00), false, 1.0);
    preLimiterThreshSlider.customFormatText = formatLimiterThresh;
    preLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(preLimiterReleaseSlider, juce::Colour(0xffffab00), false, 0.6296);
    preLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    preLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPreLimiter->setKnob(0, "In Gain", &preLimiterGainSlider);
    cardPreLimiter->setKnob(1, "Thresh", &preLimiterThreshSlider);
    cardPreLimiter->setKnob(2, "Release", &preLimiterReleaseSlider);
    addChildComponent(cardPreLimiter.get());

    // 27. Post-Amp Limiter
    cardPostLimiter = std::make_unique<ModuleCardComponent>("Post Limiter", juce::Colour(0xff00e5ff));
    setupBox(postLimiterEnableBox);
    bindSelector(postLimiterEnableSelector, postLimiterEnableBox, "post_limiter_enable", { "Off", "On" });
    cardPostLimiter->setLedSelector(&postLimiterEnableSelector);

    setupKnob(postLimiterGainSlider, juce::Colour(0xff00e5ff), false, 12.0 / 36.0);
    postLimiterGainSlider.customFormatText = formatLimiterGain;
    postLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(postLimiterThreshSlider, juce::Colour(0xff00e5ff), false, 1.0);
    postLimiterThreshSlider.customFormatText = formatLimiterThresh;
    postLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(postLimiterReleaseSlider, juce::Colour(0xff00e5ff), false, 0.6296);
    postLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    postLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPostLimiter->setKnob(0, "In Gain", &postLimiterGainSlider);
    cardPostLimiter->setKnob(1, "Thresh", &postLimiterThreshSlider);
    cardPostLimiter->setKnob(2, "Release", &postLimiterReleaseSlider);
    addChildComponent(cardPostLimiter.get());

    // 28. Velocity
    cardVelocity = std::make_unique<ModuleCardComponent>("Velocity", juce::Colour(0xff29b6f6));
    setupKnob(velSlopeSlider, juce::Colour(0xff29b6f6), false, 0.0);
    velSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    velSlopeSlider.customFormatText = formatVelocitySlope;
    velSlopeSlider.customParseText  = parseVelocitySlope;

    setupKnob(velDecaySlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDecaySlider.customFormatText = formatBipolarPercent;
    velDecaySlider.customParseText  = parseBipolarPercent;

    setupKnob(velDepthSlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDepthSlider.customFormatText = formatBipolarPercent;
    velDepthSlider.customParseText  = parseBipolarPercent;

    setupKnob(velVolumeSlider, juce::Colour(0xff29b6f6), false, 0.0);
    velVolumeSlider.customFormatText = [](double val) {
        int v = static_cast<int>(std::round(val * 100.0));
        return "-" + juce::String(v) + "%";
    };
    velVolumeSlider.customParseText = parsePercent;

    cardVelocity->setKnob(0, "Slope", &velSlopeSlider);
    cardVelocity->setKnob(1, "Decay", &velDecaySlider);
    cardVelocity->setKnob(2, "Depth", &velDepthSlider);
    cardVelocity->setKnob(3, "Volume", &velVolumeSlider);
    addChildComponent(cardVelocity.get());

    // 29. Slop
    cardSlop = std::make_unique<ModuleCardComponent>("Slop", juce::Colour(0xffab47bc));
    setupKnob(slopFreqSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopFreqSlider.customFormatText = formatSlop;
    slopFreqSlider.customParseText  = parseSlop;

    setupKnob(slopDepthSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopDepthSlider.customFormatText = formatSlop;
    slopDepthSlider.customParseText  = parseSlop;

    setupKnob(slopDecaySlider, juce::Colour(0xffab47bc), false, 0.0);
    slopDecaySlider.customFormatText = formatSlop;
    slopDecaySlider.customParseText  = parseSlop;

    setupKnob(slopPanSlider, juce::Colour(0xffab47bc), false, 0.0);
    slopPanSlider.customFormatText = formatSlop;
    slopPanSlider.customParseText  = parseSlop;

    cardSlop->setKnob(0, "Freq", &slopFreqSlider);
    cardSlop->setKnob(1, "Depth", &slopDepthSlider);
    cardSlop->setKnob(2, "Decay", &slopDecaySlider);
    cardSlop->setKnob(3, "Pan", &slopPanSlider);
    addChildComponent(cardSlop.get());

    // --- APVTS PARAMETER ATTACHMENTS ---

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

    // 4. Filter 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_type", filter1TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_slope", filter1SlopeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter1_cutoff", filter1CutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter1_resonance", filter1ResonanceSlider));

    // 5. Filter Envelope 1
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv1_slope", filterEnv1SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv1_depth", filterEnv1DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv1_decay", filterEnv1DecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv1_postdrive", filterEnv1PostDriveSlider));

    // 6. Carrier 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier2_tracking", carrier2TrackingBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_pitch", carrier2PitchSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_shape", carrier2ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier2_depth", carrier2DepthSlider));

    // 7. Modulator 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_track", mod2TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_type", mod2TypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod2_shape", mod2ShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod2_speed", mod2SpeedSlider));

    // 8. Pitch Envelope 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv2_target", pitchEnv2TargetBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_slope", pitchEnv2SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_depth", pitchEnv2DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv2_decay", pitchEnv2DecaySlider));

    // 9. Filter 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_type", filter2TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_slope", filter2SlopeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter2_cutoff", filter2CutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter2_resonance", filter2ResonanceSlider));

    // 10. Filter Envelope 2
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv2_slope", filterEnv2SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv2_depth", filterEnv2DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv2_decay", filterEnv2DecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv2_postdrive", filterEnv2PostDriveSlider));

    // 11. Noise Transient
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_sh_rate", noiseShRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_filter", noiseFilterSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_drive", noiseDriveSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_decay", noiseDecaySlider));

    // 12. Filter 3 (Transients Filter)
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_type", filter3TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_slope", filter3SlopeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter3_cutoff", filter3CutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter3_resonance", filter3ResonanceSlider));

    // 13. Filter Envelope 3 (Transients Filter Env)
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv3_slope", filterEnv3SlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv3_depth", filterEnv3DepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv3_decay", filterEnv3DecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv3_postdrive", filterEnv3PostDriveSlider));

    // 14. Mixer
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_carrier1_level", mixerCarrier1LevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_carrier2_level", mixerCarrier2LevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_ringmod", mixerRingModSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_noise_level", mixerNoiseLevelSlider));

    // 15. Drive
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_amount", driveAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_bias", driveBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_filter", driveFilterSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "drive_limiter", driveLimiterBox));

    // 16. Standalone FX Filter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "fxfilter_type", fxFilterTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "fxfilter_slope", fxFilterSlopeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "fxfilter_cutoff", fxFilterCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "fxfilter_resonance", fxFilterResonanceSlider));

    // 17. Wave Folder
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "wavefolder_type", waveFolderTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_fold", waveFolderFoldSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_bias", waveFolderBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "wavefolder_filter", waveFolderFilterSlider));

    // 18. RingMod
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_shape", ringModShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_rate", ringModRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_amount", ringModAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_width", ringModWidthSlider));

    // 19. Frequency Shifter
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_shift", freqShiftShiftSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_range", freqShiftRangeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_blend", freqShiftBlendSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_width", freqShiftWidthSlider));

    // 20. Grit FX
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_bits", gritBitsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_rate", gritRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_low", gritLowSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_high", gritHighSlider));

    // 21. Comb Filter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "comb_type", combTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_dampening", combDampeningSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_cutoff", combCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "comb_resonance", combResonanceSlider));

    // 22. Disperser
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "disperser_type", disperserTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_amount", disperserAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_cutoff", disperserCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "disperser_resonance", disperserResonanceSlider));

    // 23. EQ (Bell EQ)
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_freq", eqFreqSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_width", eqWidthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_gain", eqGainSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "eq_filter", eqFilterSlider));

    // 24. Amp
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_level", ampLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_pan", ampPanSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_drive", ampDriveSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_limiter", ampLimiterBox));

    // 25. Amp Envelope
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_claps", ampEnvClapsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_clapspeed", ampEnvClapSpeedSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_slope", ampEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_decay", ampEnvDecaySlider));

    // 26. Pre-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pre_limiter_enable", preLimiterEnableBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pre_limiter_gain", preLimiterGainSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pre_limiter_thresh", preLimiterThreshSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pre_limiter_release", preLimiterReleaseSlider));

    // 27. Post-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "post_limiter_enable", postLimiterEnableBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "post_limiter_gain", postLimiterGainSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "post_limiter_thresh", postLimiterThreshSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "post_limiter_release", postLimiterReleaseSlider));

    // 28. Velocity
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_slope", velSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_decay", velDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_depth", velDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "vel_volume", velVolumeSlider));

    // 29. Slop
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_freq", slopFreqSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_depth", slopDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_decay", slopDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "slop_pan", slopPanSlider));

    // --- FX PICKERS SETUP & ATTACHMENTS ---
    const juce::StringArray fxChoices { "None", "Drive", "Filter", "Wave Folder", "RingMod", "Frequency Shifter", "Grit FX", "Comb Filter", "Disperser", "Bell EQ" };

    for (int i = 0; i < 4; ++i) {
        preFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String paramId = "pre_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, paramId, preFXPickerCard->getBox(i)));
        preFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = preFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                audioProcessor.getEngine().setPreFXType(i, choice);
                updatePageLayout();
            }
        };

        postFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String postParamId = "post_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, postParamId, postFXPickerCard->getBox(i)));
        postFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = postFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                audioProcessor.getEngine().setPostFXType(i, choice);
                updatePageLayout();
            }
        };
    }

    scopeBuffer.resize(128, 0.0f);
    updateDynamicControls();

    setSize(1760, 440);
    setResizable(true, true);
    setResizeLimits(1200, 360, 2880, 1400);

    setPage(0);
    startTimerHz(30);
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
    updatePageLayout();
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

ModuleCardComponent* TheKlangFarmerAudioProcessorEditor::getFXCard(int fxType) {
    switch (fxType) {
        case 1: return cardDrive.get();
        case 2: return cardFXFilter.get();
        case 3: return cardWaveFolder.get();
        case 4: return cardRingMod.get();
        case 5: return cardFreqShift.get();
        case 6: return cardGrit.get();
        case 7: return cardComb.get();
        case 8: return cardDisperser.get();
        case 9: return cardEQ.get();
        default: return nullptr;
    }
}

void TheKlangFarmerAudioProcessorEditor::setPage(int pageIndex) {
    if (pageIndex < 0 || pageIndex > 6) return;
    currentPage = pageIndex;
    navCard.setSelectedPage(currentPage);
    updatePageLayout();
    repaint();
}

void TheKlangFarmerAudioProcessorEditor::updatePageLayout() {
    for (int i = 0; i < 6; ++i) blankPlates[i].setVisible(false);
    if (preFXPickerCard) preFXPickerCard->setVisible(false);
    if (postFXPickerCard) postFXPickerCard->setVisible(false);

    auto hideCard = [](std::unique_ptr<ModuleCardComponent>& c) {
        if (c) c->setVisible(false);
    };

    hideCard(cardCarrier1); hideCard(cardMod1); hideCard(cardPitchEnv1);
    hideCard(cardFilter1); hideCard(cardFilterEnv1);
    hideCard(cardCarrier2); hideCard(cardMod2); hideCard(cardPitchEnv2);
    hideCard(cardFilter2); hideCard(cardFilterEnv2);
    hideCard(cardNoise); hideCard(cardFilter3); hideCard(cardFilterEnv3);
    hideCard(cardMixer);
    hideCard(cardDrive); hideCard(cardFXFilter); hideCard(cardWaveFolder);
    hideCard(cardRingMod); hideCard(cardFreqShift); hideCard(cardGrit);
    hideCard(cardComb); hideCard(cardDisperser); hideCard(cardEQ);
    hideCard(cardAmp); hideCard(cardAmpEnv);
    hideCard(cardPreLimiter); hideCard(cardPostLimiter);
    hideCard(cardVelocity); hideCard(cardSlop);

    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;
    int slotW = (totalW - 7 * margin) / 8;
    int slotH = totalH;

    navCard.setBounds(margin, topOffset, slotW, slotH);
    navCard.setVisible(true);

    juce::Component* slotComponents[6] = { nullptr };
    juce::StringArray tabNames;
    std::vector<int> blockIndices;

    switch (currentPage) {
        case 0: // VOICE 1
            slotComponents[0] = cardCarrier1.get();
            slotComponents[1] = cardMod1.get();
            slotComponents[2] = cardPitchEnv1.get();
            slotComponents[3] = cardFilter1.get();
            slotComponents[4] = cardFilterEnv1.get();
            slotComponents[5] = cardMixer.get();

            tabNames = { "CARRIER 1", "MOD 1", "PITCH 1", "FILTER 1", "F-ENV 1", "MIXER" };
            blockIndices = { TbdAudio::ModularDrumEngine::BLK_CARRIER1,
                            TbdAudio::ModularDrumEngine::BLK_MODULATOR1,
                            TbdAudio::ModularDrumEngine::BLK_PITCHENV1,
                            TbdAudio::ModularDrumEngine::BLK_FILTER1,
                            TbdAudio::ModularDrumEngine::BLK_FILTERENV1,
                            TbdAudio::ModularDrumEngine::BLK_MIXER };
            break;

        case 1: // VOICE 2
            slotComponents[0] = cardCarrier2.get();
            slotComponents[1] = cardMod2.get();
            slotComponents[2] = cardPitchEnv2.get();
            slotComponents[3] = cardFilter2.get();
            slotComponents[4] = cardFilterEnv2.get();
            slotComponents[5] = cardMixer.get();

            tabNames = { "CARRIER 2", "MOD 2", "PITCH 2", "FILTER 2", "F-ENV 2", "MIXER" };
            blockIndices = { TbdAudio::ModularDrumEngine::BLK_CARRIER2,
                            TbdAudio::ModularDrumEngine::BLK_MODULATOR2,
                            TbdAudio::ModularDrumEngine::BLK_PITCHENV2,
                            TbdAudio::ModularDrumEngine::BLK_FILTER2,
                            TbdAudio::ModularDrumEngine::BLK_FILTERENV2,
                            TbdAudio::ModularDrumEngine::BLK_MIXER };
            break;

        case 2: // TRANSIENTS
            slotComponents[0] = cardNoise.get();
            slotComponents[1] = &blankPlates[0];
            slotComponents[2] = &blankPlates[1];
            slotComponents[3] = cardFilter3.get();
            slotComponents[4] = cardFilterEnv3.get();
            slotComponents[5] = cardMixer.get();

            tabNames = { "NOISE", "FILTER 3", "F-ENV 3", "MIXER" };
            blockIndices = { TbdAudio::ModularDrumEngine::BLK_NOISE,
                            TbdAudio::ModularDrumEngine::BLK_FILTER3,
                            TbdAudio::ModularDrumEngine::BLK_FILTERENV3,
                            TbdAudio::ModularDrumEngine::BLK_MIXER };
            break;

        case 3: // PRE-AMP FX
            slotComponents[0] = preFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPreFXType(s);
                auto* card = getFXCard(t);
                slotComponents[1 + s] = card ? static_cast<juce::Component*>(card) : static_cast<juce::Component*>(&blankPlates[s]);
                if (card) {
                    tabNames.add(card->getTitle().toUpperCase());
                    static const int fxBlockIds[] = {
                        -1,
                        TbdAudio::ModularDrumEngine::BLK_DRIVE,
                        TbdAudio::ModularDrumEngine::BLK_FXFILTER,
                        TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER,
                        TbdAudio::ModularDrumEngine::BLK_RINGMOD,
                        TbdAudio::ModularDrumEngine::BLK_FREQSHIFT,
                        TbdAudio::ModularDrumEngine::BLK_GRIT,
                        TbdAudio::ModularDrumEngine::BLK_COMB,
                        TbdAudio::ModularDrumEngine::BLK_DISPERSER,
                        TbdAudio::ModularDrumEngine::BLK_EQ
                    };
                    if (t >= 1 && t <= 9) blockIndices.push_back(fxBlockIds[t]);
                }
            }
            slotComponents[5] = cardPreLimiter.get();
            tabNames.add("LIMITER");
            blockIndices.push_back(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER);
            break;

        case 4: // AMPLIFIER
            slotComponents[0] = cardAmp.get();
            slotComponents[1] = cardAmpEnv.get();
            slotComponents[2] = &blankPlates[0];
            slotComponents[3] = &blankPlates[1];
            slotComponents[4] = cardPostLimiter.get();
            slotComponents[5] = cardMixer.get();

            tabNames = { "AMP", "AMP ENV", "LIMITER", "MIXER" };
            blockIndices = { TbdAudio::ModularDrumEngine::BLK_AMP,
                            TbdAudio::ModularDrumEngine::BLK_AMPENV,
                            TbdAudio::ModularDrumEngine::BLK_POST_LIMITER,
                            TbdAudio::ModularDrumEngine::BLK_MIXER };
            break;

        case 5: // POST-AMP FX
            slotComponents[0] = postFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPostFXType(s);
                auto* card = getFXCard(t);
                slotComponents[1 + s] = card ? static_cast<juce::Component*>(card) : static_cast<juce::Component*>(&blankPlates[s]);
                if (card) {
                    tabNames.add(card->getTitle().toUpperCase());
                    static const int fxBlockIds[] = {
                        -1,
                        TbdAudio::ModularDrumEngine::BLK_DRIVE,
                        TbdAudio::ModularDrumEngine::BLK_FXFILTER,
                        TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER,
                        TbdAudio::ModularDrumEngine::BLK_RINGMOD,
                        TbdAudio::ModularDrumEngine::BLK_FREQSHIFT,
                        TbdAudio::ModularDrumEngine::BLK_GRIT,
                        TbdAudio::ModularDrumEngine::BLK_COMB,
                        TbdAudio::ModularDrumEngine::BLK_DISPERSER,
                        TbdAudio::ModularDrumEngine::BLK_EQ
                    };
                    if (t >= 1 && t <= 9) blockIndices.push_back(fxBlockIds[t]);
                }
            }
            slotComponents[5] = cardPostLimiter.get();
            tabNames.add("LIMITER");
            blockIndices.push_back(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER);
            break;

        case 6: // MODULATIONS
            slotComponents[0] = cardVelocity.get();
            slotComponents[1] = cardSlop.get();
            slotComponents[2] = &blankPlates[0];
            slotComponents[3] = &blankPlates[1];
            slotComponents[4] = &blankPlates[2];
            slotComponents[5] = &blankPlates[3];

            tabNames = { "VELOCITY", "SLOP" };
            blockIndices = { TbdAudio::ModularDrumEngine::BLK_VELOCITY,
                            TbdAudio::ModularDrumEngine::BLK_SLOP };
            break;
    }

    for (int i = 0; i < 6; ++i) {
        if (slotComponents[i]) {
            int x = margin + (i + 1) * (slotW + margin);
            slotComponents[i]->setBounds(x, topOffset, slotW, slotH);
            slotComponents[i]->setVisible(true);
        }
    }

    vizCard.setBounds(margin + 7 * (slotW + margin), topOffset, slotW, slotH);
    vizCard.setVisible(true);
    vizCard.setAvailableTabs(tabNames, blockIndices);
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
            cardMod1->setKnobLabel(0, "Shape");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod1ShapeSlider.setBipolar(false);
        } else if (curMod1Type == 1) {
            cardMod1->setKnobLabel(0, "DJ Filter");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(true);
        } else {
            cardMod1->setKnobLabel(0, "S&H Rate");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(false);
        }
        mod1ShapeSlider.repaint();
        mod1ShapeSlider.updateText();
        mod1SpeedSlider.updateText();
    }

    syncSelector(pitchEnv1TargetBox, pitchEnv1TargetSelector, "pitchenv1_target", lastPitchEnv1Target);
    syncSelector(filter1TypeBox, filter1TypeSelector, "filter1_type", lastFilter1Type);
    syncSelector(filter1SlopeBox, filter1SlopeSelector, "filter1_slope", lastFilter1Slope);

    int curCarrier2Track = syncSelector(carrier2TrackingBox, carrier2TrackingSelector, "carrier2_tracking", lastCarrier2Track);
    if (curCarrier2Track >= 0) {
        carrier2PitchSlider.setBipolar(curCarrier2Track == 2);
        carrier2PitchSlider.updateText();
    }

    int curMod2Track = syncSelector(mod2TrackBox, mod2TrackSelector, "mod2_track", lastMod2Track);
    int curMod2Type  = syncSelector(mod2TypeBox, mod2TypeSelector, "mod2_type", lastMod2Type);
    if (curMod2Type >= 0 || curMod2Track >= 0) {
        if (curMod2Type == 0) {
            cardMod2->setKnobLabel(0, "Shape");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod2ShapeSlider.setBipolar(false);
        } else if (curMod2Type == 1) {
            cardMod2->setKnobLabel(0, "DJ Filter");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(true);
        } else {
            cardMod2->setKnobLabel(0, "S&H Rate");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(false);
        }
        mod2ShapeSlider.repaint();
        mod2ShapeSlider.updateText();
        mod2SpeedSlider.updateText();
    }

    syncSelector(pitchEnv2TargetBox, pitchEnv2TargetSelector, "pitchenv2_target", lastPitchEnv2Target);
    syncSelector(filter2TypeBox, filter2TypeSelector, "filter2_type", lastFilter2Type);
    syncSelector(filter2SlopeBox, filter2SlopeSelector, "filter2_slope", lastFilter2Slope);

    syncSelector(filter3TypeBox, filter3TypeSelector, "filter3_type", lastFilter3Type);
    syncSelector(filter3SlopeBox, filter3SlopeSelector, "filter3_slope", lastFilter3Slope);

    syncSelector(driveLimiterBox, driveLimiterSelector, "drive_limiter", lastDriveLimiter);
    syncSelector(fxFilterTypeBox, fxFilterTypeSelector, "fxfilter_type", lastFXFilterType);
    syncSelector(fxFilterSlopeBox, fxFilterSlopeSelector, "fxfilter_slope", lastFXFilterSlope);
    syncSelector(waveFolderTypeBox, waveFolderTypeSelector, "wavefolder_type", lastWaveFolderType);
    syncSelector(combTypeBox, combTypeSelector, "comb_type", lastCombType);
    syncSelector(disperserTypeBox, disperserTypeSelector, "disperser_type", lastDisperserType);
    syncSelector(ampLimiterBox, ampLimiterSelector, "amp_limiter", lastAmpLimiter);

    syncSelector(preLimiterEnableBox, preLimiterEnableSelector, "pre_limiter_enable", lastPreLimiterEnable);
    syncSelector(postLimiterEnableBox, postLimiterEnableSelector, "post_limiter_enable", lastPostLimiterEnable);
}

void TheKlangFarmerAudioProcessorEditor::timerCallback() {
    updateDynamicControls();

    int activeBlock = vizCard.getCurrentBlockIndex();
    if (activeBlock < 0) return;

    if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER1) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter1CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter1ResonanceSlider.getValue());
        int fType = filter1TypeBox.getSelectedItemIndex();
        int fSlope = filter1SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER2) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter2CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter2ResonanceSlider.getValue());
        int fType = filter2TypeBox.getSelectedItemIndex();
        int fSlope = filter2SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FILTER3) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(filter3CutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(filter3ResonanceSlider.getValue());
        int fType = filter3TypeBox.getSelectedItemIndex();
        int fSlope = filter3SlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_FXFILTER) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
        float fCutNorm = static_cast<float>(fxFilterCutoffSlider.getValue());
        float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
        float fRes = static_cast<float>(fxFilterResonanceSlider.getValue());
        int fType = fxFilterTypeBox.getSelectedItemIndex();
        int fSlope = fxFilterSlopeBox.getSelectedItemIndex();
        vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
    } else if (activeBlock == TbdAudio::ModularDrumEngine::BLK_EQ) {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
        float eqFNorm = static_cast<float>(eqFreqSlider.getValue());
        float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
        float eqWNorm = static_cast<float>(eqWidthSlider.getValue());
        float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
        float eqGNorm = static_cast<float>(eqGainSlider.getValue());
        float eqGDb = (eqGNorm - 0.5f) * 48.0f;
        float eqDjNorm = static_cast<float>(eqFilterSlider.getValue());
        vizCard.getOscilloscope().updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);
    } else {
        vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
        audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
    }
}

void TheKlangFarmerAudioProcessorEditor::paint(juce::Graphics& g) {
    juce::ColourGradient bgGrad(juce::Colour(0xff12141a), 0, 0,
                                juce::Colour(0xff0a0b0e), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    g.setColour(juce::Colour(0xff171a22));
    g.fillRect(0, 0, getWidth(), 36);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(36, 0.0f, static_cast<float>(getWidth()));

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER", 14, 0, 165, 36, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("PAGED MODULAR DUAL FM SYNTHESIS DRUM VOICE", 182, 0, 480, 36, juce::Justification::centredLeft);
}

void TheKlangFarmerAudioProcessorEditor::resized() {
    initButton.setBounds(getWidth() - 232, 5, 84, 26);
    triggerButton.setBounds(getWidth() - 138, 5, 124, 26);

    updatePageLayout();
}


