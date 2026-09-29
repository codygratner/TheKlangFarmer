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
    auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::FontOptions(12.5f, juce::Font::bold));
    l->setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xee11141a));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0x44303848));
    return l;
}

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    // Compact dial radius so larger text and labels have plenty of breathing room
    auto radius = juce::jmin(bounds.getWidth() * 0.46f, bounds.getHeight() * 0.44f) - 3.0f;
    radius = juce::jmax(9.0f, radius);
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = juce::jmax(2.2f, radius * 0.16f);
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
    auto innerRadius = arcRadius - lineW * 0.9f;
    if (innerRadius > 3.0f) {
        auto knobBounds = juce::Rectangle<float>(centre.x - innerRadius, centre.y - innerRadius,
                                                 innerRadius * 2.0f, innerRadius * 2.0f);
        juce::ColourGradient grad(juce::Colour(0xff2a2f3d), centre.x, centre.y - innerRadius,
                                  juce::Colour(0xff14161c), centre.x, centre.y + innerRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colour(0xff3b4354));
        g.drawEllipse(knobBounds, 1.0f);

        // Pointer indicator
        juce::Path p;
        auto pointerLength = innerRadius * 0.65f;
        auto pointerThickness = juce::jmax(1.6f, innerRadius * 0.14f);
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

void MiniOscilloscopeComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Dark background display
    g.setColour(juce::Colour(0xff0e1017));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Subtle outline
    g.setColour(juce::Colour(0xff1d222e));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

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
}

// --- ROTARY KNOB SLIDER WITH RIGHT CLICK EDITING ---

RotaryKnobSlider::RotaryKnobSlider() {
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
}

void RotaryKnobSlider::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isRightButtonDown() || e.mods.isPopupMenu()) {
        openHoveringEditor();
        return;
    }
    juce::Slider::mouseDown(e);
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
        labels[i].setFont(juce::FontOptions(12.5f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
        labels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(labels[i]);
    }
}

void ModuleCardComponent::setLedSelector(LedSelectorComponent* selector) {
    ledSelector = selector;
    if (ledSelector) addAndMakeVisible(ledSelector);
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

    oscilloscope.setBounds(area.removeFromTop(26).reduced(2, 0));
    area.removeFromTop(4);

    if (ledSelector != nullptr) {
        int selH = (ledSelector->getNumItems() > 4) ? 36 : 22;
        ledSelector->setBounds(area.removeFromTop(selH));
        area.removeFromTop(3);

        int knobCount = 3;
        int knobW = area.getWidth() / knobCount;
        for (int i = 0; i < knobCount; ++i) {
            auto kArea = area.removeFromLeft(knobW);
            labels[i].setBounds(kArea.removeFromTop(15));
            if (knobs[i]) knobs[i]->setBounds(kArea);
        }
    } else {
        // 4 knobs in 2x2 grid
        int rowH = area.getHeight() / 2;
        auto topRow = area.removeFromTop(rowH);
        auto botRow = area;

        int wTop = topRow.getWidth() / 2;
        int wBot = botRow.getWidth() / 2;

        auto k0Area = topRow.removeFromLeft(wTop);
        labels[0].setBounds(k0Area.removeFromTop(15));
        if (knobs[0]) knobs[0]->setBounds(k0Area);

        auto k1Area = topRow;
        labels[1].setBounds(k1Area.removeFromTop(15));
        if (knobs[1]) knobs[1]->setBounds(k1Area);

        auto k2Area = botRow.removeFromLeft(wBot);
        labels[2].setBounds(k2Area.removeFromTop(15));
        if (knobs[2]) knobs[2]->setBounds(k2Area);

        auto k3Area = botRow;
        labels[3].setBounds(k3Area.removeFromTop(15));
        if (knobs[3]) knobs[3]->setBounds(k3Area);
    }
}

// --- BIA ER-1 EDITOR ---

BiaEr1AudioProcessorEditor::BiaEr1AudioProcessorEditor(BiaEr1AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      carrierTrackingSelector(juce::Colour(0xff00d2ff)),
      modTypeSelector(juce::Colour(0xffff7043)),
      pitchEnvTargetSelector(juce::Colour(0xffffab00)),
      driveTypeSelector(juce::Colour(0xffff4081)),
      mixerLimiterSelector(juce::Colour(0xff40c4ff)),
      filterTypeSelector(juce::Colour(0xff7c4dff)),
      ampLimiterSelector(juce::Colour(0xff00e5ff))
{
    setLookAndFeel(&knobLookAndFeel);

    // Setup Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    // 1. CARRIER CARD
    auto cardCarrier = std::make_unique<ModuleCardComponent>("Carrier", juce::Colour(0xff00d2ff));
    bindSelector(carrierTrackingSelector, carrierTrackingBox, "carrier_tracking", { "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 3);
    setupKnob(carrierPitchSlider, juce::Colour(0xff00d2ff));
    setupKnob(carrierShapeSlider, juce::Colour(0xff00d2ff));
    setupKnob(carrierDriveSlider, juce::Colour(0xff00d2ff));

    carrierPitchSlider.customFormatText = [this](double val) -> juce::String {
        int track = carrierTrackingSelector.getSelectedIndex();
        if (track == 0) {
            float hz = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        } else if (track == 1) {
            int note = static_cast<int>(std::round(val * 127.0));
            return getMidiNoteName(note);
        } else {
            int offset = static_cast<int>(std::round((val - 0.5) * 120.0));
            return (offset >= 0 ? "+" : "") + juce::String(offset) + " st";
        }
    };
    carrierPitchSlider.customParseText = [this](const juce::String& text) -> double {
        int track = carrierTrackingSelector.getSelectedIndex();
        double parsed = parseNumberSafe(text, carrierPitchSlider.getValue());
        if (track == 0) {
            double hz = std::clamp(parsed, 20.0, 20000.0);
            return std::log(hz / 20.0) / std::log(20000.0 / 20.0);
        } else if (track == 1) {
            return std::clamp(parsed / 127.0, 0.0, 1.0);
        } else {
            return std::clamp(0.5 + parsed / 120.0, 0.0, 1.0);
        }
    };

    carrierShapeSlider.customFormatText = [](double val) {
        float f = static_cast<float>(val);
        if (f <= 0.20f) return "Sine (" + juce::String(static_cast<int>(f * 500.0f)) + "%)";
        if (f <= 0.40f) return "Tri (" + juce::String(static_cast<int>((f - 0.20f) * 500.0f)) + "%)";
        if (f <= 0.60f) return "Saw (" + juce::String(static_cast<int>((f - 0.40f) * 500.0f)) + "%)";
        return "PWM (" + juce::String(static_cast<int>((f - 0.60f) * 250.0f)) + "%)";
    };
    carrierShapeSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(p / 100.0, 0.0, 1.0);
    };

    carrierDriveSlider.customFormatText = [](double val) {
        float db = TbdAudio::normToDriveDb(static_cast<float>(val));
        return (db >= 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
    };
    carrierDriveSlider.customParseText = [](const juce::String& text) {
        double db = parseNumberSafe(text, 0.0);
        return TbdAudio::driveDbToNorm(static_cast<float>(db));
    };

    cardCarrier->setLedSelector(&carrierTrackingSelector);
    cardCarrier->setKnob(0, "Pitch", &carrierPitchSlider);
    cardCarrier->setKnob(1, "Shape", &carrierShapeSlider);
    cardCarrier->setKnob(2, "Drive", &carrierDriveSlider);
    cards.push_back(std::move(cardCarrier));

    // 2. MODULATOR CARD
    auto cardMod = std::make_unique<ModuleCardComponent>("Modulator", juce::Colour(0xffff7043));
    bindSelector(modTypeSelector, modTypeBox, "mod_type",
                 { "Fixed", "Follow", "FM Op", "Fix Sin*Nz", "Fol Sin*Nz", "FM Sin*Nz", "S&H Nz" }, 4);
    setupKnob(modShapeSlider, juce::Colour(0xffff7043));
    setupKnob(modDepthSlider, juce::Colour(0xffff7043), true); // Bipolar
    setupKnob(modSpeedSlider, juce::Colour(0xffff7043));

    modDepthSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 400.0));
        return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
    };
    modDepthSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 400.0, 0.0, 1.0);
    };

    modSpeedSlider.customFormatText = [this](double val) -> juce::String {
        int t = modTypeSelector.getSelectedIndex();
        if (t == 0 || t == 3) {
            float hz = 0.1f * std::pow(15000.0f / 0.1f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        } else if (t == 1 || t == 4) {
            int st = static_cast<int>(std::round((val - 0.5) * 128.0));
            return (st >= 0 ? "+" : "") + juce::String(st) + " st";
        } else if (t == 2 || t == 5) {
            if (val <= 0.5) {
                float denom = 32.0f - static_cast<float>(val * 2.0 * 31.0);
                return "1:" + juce::String(denom, 1);
            } else {
                float num = 1.0f + static_cast<float>((val - 0.5) * 2.0 * 31.0);
                return juce::String(num, 1) + ":1";
            }
        } else {
            float hz = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
        }
    };
    modSpeedSlider.customParseText = [this](const juce::String& text) -> double {
        int t = modTypeSelector.getSelectedIndex();
        double p = parseNumberSafe(text, 55.0);
        if (t == 0 || t == 3) {
            double hz = std::clamp(p, 0.1, 15000.0);
            return std::log(hz / 0.1) / std::log(15000.0 / 0.1);
        } else if (t == 1 || t == 4) {
            return std::clamp(0.5 + p / 128.0, 0.0, 1.0);
        } else {
            double hz = std::clamp(p, 0.1, 20000.0);
            return std::log(hz / 0.1) / std::log(20000.0 / 0.1);
        }
    };

    cardMod->setLedSelector(&modTypeSelector);
    cardMod->setKnob(0, "Shape", &modShapeSlider);
    cardMod->setKnob(1, "Depth", &modDepthSlider);
    cardMod->setKnob(2, "Speed", &modSpeedSlider);
    cards.push_back(std::move(cardMod));

    // 3. PITCH ENVELOPE CARD
    auto cardPitchEnv = std::make_unique<ModuleCardComponent>("Pitch Env", juce::Colour(0xffffab00));
    bindSelector(pitchEnvTargetSelector, pitchEnvTargetBox, "pitchenv_target",
                 { "Off", "Carrier", "Mod", "Both" }, 4);
    setupKnob(pitchEnvSlopeSlider, juce::Colour(0xffffab00));
    setupKnob(pitchEnvDepthSlider, juce::Colour(0xffffab00), true); // Bipolar
    setupKnob(pitchEnvDecaySlider, juce::Colour(0xffffab00));

    pitchEnvSlopeSlider.customFormatText = [](double val) {
        if (val < 0.33) return "Exp (" + juce::String(static_cast<int>(val * 300.0)) + "%)";
        if (val < 0.67) return "Lin (" + juce::String(static_cast<int>((val - 0.33) * 300.0)) + "%)";
        return "Log (" + juce::String(static_cast<int>((val - 0.67) * 300.0)) + "%)";
    };
    pitchEnvDepthSlider.customFormatText = [](double val) {
        int pct = static_cast<int>(std::round((val - 0.5) * 200.0));
        return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
    };
    pitchEnvDepthSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
    };
    pitchEnvDecaySlider.customFormatText = [](double val) {
        float sec = TbdAudio::warp5PointTime(static_cast<float>(val));
        return (sec < 1.0f) ? juce::String(static_cast<int>(std::round(sec * 1000.0f))) + " ms" : juce::String(sec, 2) + " s";
    };
    pitchEnvDecaySlider.customParseText = [](const juce::String& text) {
        double v = parseNumberSafe(text, 333.0);
        double sec = (text.containsIgnoreCase("ms")) ? (v / 1000.0) : ((v > 60.0) ? (v / 1000.0) : v);
        return TbdAudio::unwarp5PointTime(static_cast<float>(sec));
    };

    cardPitchEnv->setLedSelector(&pitchEnvTargetSelector);
    cardPitchEnv->setKnob(0, "Slope", &pitchEnvSlopeSlider);
    cardPitchEnv->setKnob(1, "Depth", &pitchEnvDepthSlider);
    cardPitchEnv->setKnob(2, "Decay", &pitchEnvDecaySlider);
    cards.push_back(std::move(cardPitchEnv));

    // 4. DRIVE CARD
    auto cardDrive = std::make_unique<ModuleCardComponent>("Drive", juce::Colour(0xffff4081));
    bindSelector(driveTypeSelector, driveTypeBox, "drive_type",
                 { "Off", "Saturation", "Wave Folder" }, 3);
    setupKnob(driveAmountSlider, juce::Colour(0xffff4081));
    setupKnob(driveBiasSlider, juce::Colour(0xffff4081), true); // Bipolar
    setupKnob(driveFilterSlider, juce::Colour(0xffff4081), true); // Bipolar

    driveAmountSlider.customFormatText = [this](double val) -> juce::String {
        int t = driveTypeSelector.getSelectedIndex();
        if (t == 2) {
            return juce::String(val * 8.0, 1) + " folds";
        }
        float db = (val <= 0.5) ? static_cast<float>(-6.0 + val * 12.0) : static_cast<float>((val - 0.5) * 48.0);
        return (db >= 0 ? "+" : "") + juce::String(db, 1) + " dB";
    };
    driveAmountSlider.customParseText = [this](const juce::String& text) -> double {
        int t = driveTypeSelector.getSelectedIndex();
        double p = parseNumberSafe(text, 0.0);
        if (t == 2) return std::clamp(p / 8.0, 0.0, 1.0);
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
    driveFilterSlider.customFormatText = [](double val) {
        if (val >= 0.49 && val <= 0.51) return juce::String("Flat (50%)");
        if (val < 0.49) {
            float hz = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(val / 0.49));
            return "LP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
        }
        float hz = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>((val - 0.51) / 0.49));
        return "HP " + ((hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k" : juce::String(hz, 0));
    };

    cardDrive->setLedSelector(&driveTypeSelector);
    cardDrive->setKnob(0, "Drive", &driveAmountSlider);
    cardDrive->setKnob(1, "Bias", &driveBiasSlider);
    cardDrive->setKnob(2, "Post-Filter", &driveFilterSlider);
    cards.push_back(std::move(cardDrive));

    // 5. NOISE TRANSIENT CARD (4 Knobs)
    auto cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", juce::Colour(0xff00e676));
    setupKnob(noiseShRateSlider, juce::Colour(0xff00e676));
    setupKnob(noiseFilterSlider, juce::Colour(0xff00e676), true); // Bipolar
    setupKnob(noiseDriveSlider, juce::Colour(0xff00e676));
    setupKnob(noiseDecaySlider, juce::Colour(0xff00e676));

    noiseShRateSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    noiseShRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 20000.0), 0.1, 20000.0);
        return std::log(hz / 0.1) / std::log(20000.0 / 0.1);
    };
    noiseFilterSlider.customFormatText = driveFilterSlider.customFormatText;
    noiseDriveSlider.customFormatText = [](double val) {
        float db = TbdAudio::normToDriveDb(static_cast<float>(val));
        return (db >= 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
    };
    noiseDriveSlider.customParseText = [](const juce::String& text) {
        double db = parseNumberSafe(text, 0.0);
        return TbdAudio::driveDbToNorm(static_cast<float>(db));
    };
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

    // 6. MIXER CARD
    auto cardMixer = std::make_unique<ModuleCardComponent>("Mixer", juce::Colour(0xff40c4ff));
    bindSelector(mixerLimiterSelector, mixerLimiterBox, "mixer_limiter", { "Off", "On" }, 2);
    setupKnob(mixerCarrierLevelSlider, juce::Colour(0xff40c4ff));
    setupKnob(mixerNoiseLevelSlider, juce::Colour(0xff40c4ff));
    setupKnob(mixerDriveSlider, juce::Colour(0xff40c4ff));

    auto format0to400Pct = [](double val) {
        float pct = (val <= 0.5) ? static_cast<float>(val * 200.0) : static_cast<float>(100.0 + (val - 0.5) * 600.0);
        return juce::String(static_cast<int>(std::round(pct))) + "%";
    };
    auto parse0to400Pct = [](const juce::String& text) {
        double p = parseNumberSafe(text, 100.0);
        return (p <= 100.0) ? std::clamp(p / 200.0, 0.0, 0.5) : std::clamp(0.5 + (p - 100.0) / 600.0, 0.5, 1.0);
    };

    mixerCarrierLevelSlider.customFormatText = format0to400Pct;
    mixerCarrierLevelSlider.customParseText = parse0to400Pct;
    mixerNoiseLevelSlider.customFormatText = format0to400Pct;
    mixerNoiseLevelSlider.customParseText = parse0to400Pct;
    mixerDriveSlider.customFormatText = [](double val) {
        float db = (val <= 0.5) ? static_cast<float>(-6.0 + val * 12.0) : static_cast<float>((val - 0.5) * 48.0);
        return (db >= 0 ? "+" : "") + juce::String(db, 1) + " dB";
    };
    mixerDriveSlider.customParseText = [](const juce::String& text) {
        double p = parseNumberSafe(text, 0.0);
        return (p <= 0.0) ? std::clamp((p + 6.0) / 12.0, 0.0, 0.5) : std::clamp(0.5 + p / 48.0, 0.5, 1.0);
    };

    cardMixer->setLedSelector(&mixerLimiterSelector);
    cardMixer->setKnob(0, "Carrier Lvl", &mixerCarrierLevelSlider);
    cardMixer->setKnob(1, "Noise Lvl", &mixerNoiseLevelSlider);
    cardMixer->setKnob(2, "Drive", &mixerDriveSlider);
    cards.push_back(std::move(cardMixer));

    // 7. FILTER CARD
    auto cardFilter = std::make_unique<ModuleCardComponent>("Filter", juce::Colour(0xff7c4dff));
    bindSelector(filterTypeSelector, filterTypeBox, "filter_type",
                 { "Off", "LPF", "BPF", "HPF", "Notch", "Comb", "Disperser" }, 4);
    setupKnob(filterStyleSlider, juce::Colour(0xff7c4dff));
    setupKnob(filterCutoffSlider, juce::Colour(0xff7c4dff));
    setupKnob(filterResonanceSlider, juce::Colour(0xff7c4dff));

    filterStyleSlider.customFormatText = [this](double val) -> juce::String {
        int t = filterTypeSelector.getSelectedIndex();
        if (t <= 4) {
            float db = (val <= 0.5) ? static_cast<float>(-6.0 - val * 36.0) : static_cast<float>(-24.0 - (val - 0.5) * 144.0);
            return juce::String(static_cast<int>(std::round(db))) + " dB/oct";
        } else if (t == 5) {
            float hz = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(val));
            return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 1) + "k Damp" : juce::String(hz, 0) + " Hz";
        } else {
            int stages = static_cast<int>(std::round(val * 32.0));
            return juce::String(stages) + " APFs";
        }
    };
    filterCutoffSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    filterCutoffSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 20000.0), 0.1, 20000.0);
        return std::log(hz / 0.1) / std::log(20000.0 / 0.1);
    };
    filterResonanceSlider.customFormatText = [this](double val) -> juce::String {
        int t = filterTypeSelector.getSelectedIndex();
        if (t == 5 || t == 6) {
            int pct = static_cast<int>(std::round((val - 0.5) * 200.0));
            return (pct >= 0 ? "+" : "") + juce::String(pct) + "%";
        }
        return juce::String(static_cast<int>(std::round(val * 100.0))) + "%";
    };
    filterResonanceSlider.customParseText = [this](const juce::String& text) -> double {
        int t = filterTypeSelector.getSelectedIndex();
        double p = parseNumberSafe(text, 0.0);
        if (t == 5 || t == 6) return std::clamp(0.5 + p / 200.0, 0.0, 1.0);
        return std::clamp(p / 100.0, 0.0, 1.0);
    };

    cardFilter->setLedSelector(&filterTypeSelector);
    cardFilter->setKnob(0, "Style", &filterStyleSlider);
    cardFilter->setKnob(1, "Cutoff", &filterCutoffSlider);
    cardFilter->setKnob(2, "Resonance", &filterResonanceSlider);
    cards.push_back(std::move(cardFilter));

    // 8. FILTER ENVELOPE CARD (4 Knobs)
    auto cardFilterEnv = std::make_unique<ModuleCardComponent>("Filter Env", juce::Colour(0xffb388ff));
    setupKnob(filterEnvSlopeSlider, juce::Colour(0xffb388ff));
    setupKnob(filterEnvDepthSlider, juce::Colour(0xffb388ff), true); // Bipolar
    setupKnob(filterEnvDecaySlider, juce::Colour(0xffb388ff));
    setupKnob(filterEnvPreDriveSlider, juce::Colour(0xffb388ff));

    filterEnvSlopeSlider.customFormatText = pitchEnvSlopeSlider.customFormatText;
    filterEnvDepthSlider.customFormatText = pitchEnvDepthSlider.customFormatText;
    filterEnvDepthSlider.customParseText = pitchEnvDepthSlider.customParseText;
    filterEnvDecaySlider.customFormatText = pitchEnvDecaySlider.customFormatText;
    filterEnvDecaySlider.customParseText = pitchEnvDecaySlider.customParseText;
    filterEnvPreDriveSlider.customFormatText = mixerDriveSlider.customFormatText;
    filterEnvPreDriveSlider.customParseText = mixerDriveSlider.customParseText;

    cardFilterEnv->setKnob(0, "Slope", &filterEnvSlopeSlider);
    cardFilterEnv->setKnob(1, "Depth", &filterEnvDepthSlider);
    cardFilterEnv->setKnob(2, "Decay", &filterEnvDecaySlider);
    cardFilterEnv->setKnob(3, "Pre-Drive", &filterEnvPreDriveSlider);
    cards.push_back(std::move(cardFilterEnv));

    // 9. RINGMOD CARD (4 Knobs)
    auto cardRingMod = std::make_unique<ModuleCardComponent>("RingMod", juce::Colour(0xffff5252));
    setupKnob(ringModShapeSlider, juce::Colour(0xffff5252));
    setupKnob(ringModRateSlider, juce::Colour(0xffff5252));
    setupKnob(ringModAmountSlider, juce::Colour(0xffff5252));
    setupKnob(ringModWidthSlider, juce::Colour(0xffff5252), true); // Bipolar

    ringModShapeSlider.customFormatText = carrierShapeSlider.customFormatText;
    ringModRateSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(15000.0f / 0.1f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 1) + " Hz";
    };
    ringModRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 55.0), 0.1, 15000.0);
        return std::log(hz / 0.1) / std::log(15000.0 / 0.1);
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

    // 10. FREQUENCY SHIFTER CARD (4 Knobs)
    auto cardFreqShift = std::make_unique<ModuleCardComponent>("Freq Shifter", juce::Colour(0xff69f0ae));
    setupKnob(freqShiftShiftSlider, juce::Colour(0xff69f0ae), true); // Bipolar
    setupKnob(freqShiftRangeSlider, juce::Colour(0xff69f0ae));
    setupKnob(freqShiftBlendSlider, juce::Colour(0xff69f0ae), true); // Bipolar
    setupKnob(freqShiftWidthSlider, juce::Colour(0xff69f0ae), true); // Bipolar

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

    // 11. GRIT FX CARD (4 Knobs)
    auto cardGrit = std::make_unique<ModuleCardComponent>("Grit FX", juce::Colour(0xffffd740));
    setupKnob(gritBitsSlider, juce::Colour(0xffffd740));
    setupKnob(gritRateSlider, juce::Colour(0xffffd740));
    setupKnob(gritLowBoostSlider, juce::Colour(0xffffd740));
    setupKnob(gritHighBoostSlider, juce::Colour(0xffffd740));

    gritBitsSlider.customFormatText = [](double val) {
        return juce::String(1.0 + val * 15.0, 1) + " Bits";
    };
    gritBitsSlider.customParseText = [](const juce::String& text) {
        double b = std::clamp(parseNumberSafe(text, 16.0), 1.0, 16.0);
        return (b - 1.0) / 15.0;
    };
    gritRateSlider.customFormatText = [](double val) {
        float hz = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(val));
        return (hz >= 1000.0f) ? juce::String(hz / 1000.0f, 2) + " kHz" : juce::String(hz, 0) + " Hz";
    };
    gritRateSlider.customParseText = [](const juce::String& text) {
        double hz = std::clamp(parseNumberSafe(text, 20000.0), 20.0, 20000.0);
        return std::log(hz / 20.0) / std::log(20000.0 / 20.0);
    };
    gritLowBoostSlider.customFormatText = [](double val) {
        return "+" + juce::String(val * 24.0, 1) + " dB";
    };
    gritLowBoostSlider.customParseText = [](const juce::String& text) {
        return std::clamp(parseNumberSafe(text, 0.0) / 24.0, 0.0, 1.0);
    };
    gritHighBoostSlider.customFormatText = gritLowBoostSlider.customFormatText;
    gritHighBoostSlider.customParseText = gritLowBoostSlider.customParseText;

    cardGrit->setKnob(0, "Bit Crush", &gritBitsSlider);
    cardGrit->setKnob(1, "Downsample", &gritRateSlider);
    cardGrit->setKnob(2, "Low Boost", &gritLowBoostSlider);
    cardGrit->setKnob(3, "High Boost", &gritHighBoostSlider);
    cards.push_back(std::move(cardGrit));

    // 12. AMP CARD
    auto cardAmp = std::make_unique<ModuleCardComponent>("Amp", juce::Colour(0xff00e5ff));
    bindSelector(ampLimiterSelector, ampLimiterBox, "amp_limiter", { "Off", "On" }, 2);
    setupKnob(ampPanSlider, juce::Colour(0xff00e5ff), true); // Bipolar
    setupKnob(ampLevelSlider, juce::Colour(0xff00e5ff));
    setupKnob(ampDriveSlider, juce::Colour(0xff00e5ff));

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
    ampLevelSlider.customFormatText = format0to400Pct;
    ampLevelSlider.customParseText = parse0to400Pct;
    ampDriveSlider.customFormatText = mixerDriveSlider.customFormatText;
    ampDriveSlider.customParseText = mixerDriveSlider.customParseText;

    cardAmp->setLedSelector(&ampLimiterSelector);
    cardAmp->setKnob(0, "Pan", &ampPanSlider);
    cardAmp->setKnob(1, "Master Lvl", &ampLevelSlider);
    cardAmp->setKnob(2, "Drive", &ampDriveSlider);
    cards.push_back(std::move(cardAmp));

    // 13. AMP ENVELOPE CARD (4 Knobs)
    auto cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Env", juce::Colour(0xff64ffda));
    setupKnob(ampEnvClapsSlider, juce::Colour(0xff64ffda));
    setupKnob(ampEnvClapSpeedSlider, juce::Colour(0xff64ffda));
    setupKnob(ampEnvSlopeSlider, juce::Colour(0xff64ffda));
    setupKnob(ampEnvDecaySlider, juce::Colour(0xff64ffda));

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
    ampEnvSlopeSlider.customFormatText = pitchEnvSlopeSlider.customFormatText;
    ampEnvDecaySlider.customFormatText = pitchEnvDecaySlider.customFormatText;
    ampEnvDecaySlider.customParseText = pitchEnvDecaySlider.customParseText;

    cardAmpEnv->setKnob(0, "Claps", &ampEnvClapsSlider);
    cardAmpEnv->setKnob(1, "Clap Speed", &ampEnvClapSpeedSlider);
    cardAmpEnv->setKnob(2, "Slope", &ampEnvSlopeSlider);
    cardAmpEnv->setKnob(3, "Decay", &ampEnvDecaySlider);
    cards.push_back(std::move(cardAmpEnv));

    // Add all 13 cards to editor
    for (auto& c : cards) {
        addAndMakeVisible(c.get());
    }

    // Attach all APVTS parameters
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier_tracking", carrierTrackingBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_pitch", carrierPitchSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_shape", carrierShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_drive", carrierDriveSlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod_type", modTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_shape", modShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_depth", modDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_speed", modSpeedSlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv_target", pitchEnvTargetBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv_slope", pitchEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv_depth", pitchEnvDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "pitchenv_decay", pitchEnvDecaySlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "drive_type", driveTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_amount", driveAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_bias", driveBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_filter", driveFilterSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_sh_rate", noiseShRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_filter", noiseFilterSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_drive", noiseDriveSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_decay", noiseDecaySlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_carrier_level", mixerCarrierLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_noise_level", mixerNoiseLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mixer_drive", mixerDriveSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mixer_limiter", mixerLimiterBox));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter_type", filterTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_style", filterStyleSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_cutoff", filterCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_resonance", filterResonanceSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_slope", filterEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_depth", filterEnvDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_decay", filterEnvDecaySlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filterenv_predrive", filterEnvPreDriveSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_shape", ringModShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_rate", ringModRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_amount", ringModAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_width", ringModWidthSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_shift", freqShiftShiftSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_range", freqShiftRangeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_blend", freqShiftBlendSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_width", freqShiftWidthSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_bits", gritBitsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_rate", gritRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_low_boost", gritLowBoostSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_high_boost", gritHighBoostSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_pan", ampPanSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_level", ampLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_drive", ampDriveSlider));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_limiter", ampLimiterBox));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_claps", ampEnvClapsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_clapspeed", ampEnvClapSpeedSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_slope", ampEnvSlopeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ampenv_decay", ampEnvDecaySlider));

    updateDynamicControls();
    setSize(1100, 620);
    setResizable(true, true);
    setResizeLimits(900, 500, 1920, 1200);
    startTimerHz(30); // 30 FPS oscilloscope & GUI update
}

BiaEr1AudioProcessorEditor::~BiaEr1AudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void BiaEr1AudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar) {
    slider.setBipolar(isBipolar);
    slider.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    slider.setRange(0.0, 1.0, 0.0005);
}

void BiaEr1AudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setVisible(false);
}

void BiaEr1AudioProcessorEditor::bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
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

void BiaEr1AudioProcessorEditor::updateDynamicControls() {
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

    int curCarrierTrack = syncSelector(carrierTrackingBox, carrierTrackingSelector, "carrier_tracking", lastCarrierTrack);
    if (curCarrierTrack >= 0) {
        carrierPitchSlider.setBipolar(curCarrierTrack == 2);
        carrierPitchSlider.updateText();
    }

    int curMod = syncSelector(modTypeBox, modTypeSelector, "mod_type", lastModType);
    if (curMod >= 0) {
        modSpeedSlider.updateText();
    }

    syncSelector(pitchEnvTargetBox, pitchEnvTargetSelector, "pitchenv_target", lastPitchEnvTarget);

    int curDrive = syncSelector(driveTypeBox, driveTypeSelector, "drive_type", lastDriveType);
    if (curDrive >= 0) {
        driveAmountSlider.updateText();
    }

    syncSelector(mixerLimiterBox, mixerLimiterSelector, "mixer_limiter", lastMixerLimiter);

    int curFilter = syncSelector(filterTypeBox, filterTypeSelector, "filter_type", lastFilterType);
    if (curFilter >= 0) {
        filterResonanceSlider.setBipolar(curFilter == 5 || curFilter == 6);
        filterStyleSlider.updateText();
        filterResonanceSlider.updateText();
    }

    syncSelector(ampLimiterBox, ampLimiterSelector, "amp_limiter", lastAmpLimiter);
}

void BiaEr1AudioProcessorEditor::timerCallback() {
    updateDynamicControls();

    // Fetch and display synchronized oscilloscope buffers across all 13 modules
    float scopeBuffer[128];
    for (int b = 0; b < static_cast<int>(cards.size()); ++b) {
        audioProcessor.getEngine().getScopeData(b, scopeBuffer, 128);
        cards[b]->updateScope(scopeBuffer, 128);
    }
}

void BiaEr1AudioProcessorEditor::paint(juce::Graphics& g) {
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
    g.drawText("BIA ER-1", 14, 0, 90, 36, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("13-MODULE HARDWARE SYNTHESIS DRUM VOICE", 94, 0, 380, 36, juce::Justification::centredLeft);
}

void BiaEr1AudioProcessorEditor::resized() {
    triggerButton.setBounds(getWidth() - 138, 5, 124, 26);

    int margin = 6;
    int topOffset = 38;
    int totalH = getHeight() - topOffset - margin;
    int rowH = (totalH - 2 * margin) / 3;

    // Row 1: 5 Cards (Carrier, Modulator, Pitch Env, Drive, Noise Transient)
    int row1Y = topOffset;
    int row1CardW = (getWidth() - 6 * margin) / 5;
    for (int i = 0; i < 5 && i < static_cast<int>(cards.size()); ++i) {
        int x = margin + i * (row1CardW + margin);
        cards[i]->setBounds(x, row1Y, row1CardW, rowH);
    }

    // Row 2: 4 Cards (Mixer, Filter, Filter Env, RingMod)
    int row2Y = row1Y + rowH + margin;
    int row2CardW = (getWidth() - 5 * margin) / 4;
    for (int i = 0; i < 4 && (i + 5) < static_cast<int>(cards.size()); ++i) {
        int x = margin + i * (row2CardW + margin);
        cards[i + 5]->setBounds(x, row2Y, row2CardW, rowH);
    }

    // Row 3: 4 Cards (Freq Shifter, Grit FX, Amp, Amp Env)
    int row3Y = row2Y + rowH + margin;
    int row3CardW = (getWidth() - 5 * margin) / 4;
    for (int i = 0; i < 4 && (i + 9) < static_cast<int>(cards.size()); ++i) {
        int x = margin + i * (row3CardW + margin);
        cards[i + 9]->setBounds(x, row3Y, row3CardW, rowH);
    }
}
