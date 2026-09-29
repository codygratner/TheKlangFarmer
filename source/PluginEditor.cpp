#include "PluginEditor.h"

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

void RotaryKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = juce::jmax(2.5f, radius * 0.16f);
    auto arcRadius = radius - lineW * 0.5f;

    // Track background
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value Arc
    if (slider.isEnabled() && sliderPosProportional > 0.001f) {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Inner dial disc
    auto innerRadius = arcRadius - lineW * 0.9f;
    if (innerRadius > 4.0f) {
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
        auto pointerThickness = 2.0f;
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
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 16);
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
    editor->setSize(88, 24);
    editor->setFont(juce::FontOptions(12.0f, juce::Font::bold));
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
            g.setColour(accent.withAlpha(0.40f));
            g.drawRoundedRectangle(r, 3.0f, 1.0f);
        } else if (isHov) {
            g.setColour(juce::Colour(0x14ffffff));
            g.fillRoundedRectangle(r, 3.0f);
        }

        float ledX = r.getX() + 8.0f;
        float ledY = r.getCentreY();
        float ledR = 3.2f;

        if (isSel) {
            // Radiant halo
            g.setColour(accent.withAlpha(0.35f));
            g.fillEllipse(ledX - 6.0f, ledY - 6.0f, 12.0f, 12.0f);

            // Lit Core
            g.setColour(accent.brighter(0.35f));
            g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

            // Specular glass shine
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            g.fillEllipse(ledX - 1.2f, ledY - 1.8f, 1.8f, 1.8f);

            // Text
            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        } else {
            // Unlit socket
            g.setColour(juce::Colour(0xff2a3040));
            g.drawEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f, 1.0f);

            g.setColour(juce::Colour(0xff12151c));
            g.fillEllipse(ledX - (ledR - 0.5f), ledY - (ledR - 0.5f), (ledR - 0.5f) * 2.0f, (ledR - 0.5f) * 2.0f);

            // Text
            g.setColour(isHov ? juce::Colour(0xffb0bac9) : juce::Colour(0xff758195));
            g.setFont(juce::FontOptions(9.0f, juce::Font::plain));
        }

        auto textRect = juce::Rectangle<float>(ledX + 8.0f, r.getY(), r.getWidth() - 17.0f, r.getHeight());
        g.drawText(items[i], textRect, juce::Justification::centredLeft, true);
    }
}

void LedSelectorComponent::mouseMove(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx != hoveredIndex) {
        hoveredIndex = idx;
        setMouseCursor(hoveredIndex >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void LedSelectorComponent::mouseExit(const juce::MouseEvent&) {
    if (hoveredIndex != -1) {
        hoveredIndex = -1;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void LedSelectorComponent::mouseDown(const juce::MouseEvent& e) {
    int idx = getItemIndexAt(e.getPosition());
    if (idx >= 0 && idx < items.size()) {
        setSelectedIndex(idx, juce::sendNotificationSync);
    }
}

// --- MODULE CARD COMPONENT ---

ModuleCardComponent::ModuleCardComponent(const juce::String& title, juce::Colour accentColour)
    : moduleTitle(title), accent(accentColour), oscilloscope(accentColour)
{
    addAndMakeVisible(oscilloscope);

    for (int i = 0; i < 4; ++i) {
        labels[i].setFont(juce::FontOptions(10.0f, juce::Font::bold));
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xff8a96ab));
        addChildComponent(labels[i]);
    }
}

void ModuleCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Module background
    g.setColour(juce::Colour(0xff161820));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Subtle outline
    g.setColour(juce::Colour(0xff242936));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Top Accent line
    g.setColour(accent);
    g.fillRoundedRectangle(bounds.getX() + 6.0f, bounds.getY() + 2.0f, bounds.getWidth() - 12.0f, 2.5f, 1.0f);

    // Title pill
    g.setColour(juce::Colour(0xff1e222d));
    g.fillRoundedRectangle(bounds.getX() + 6.0f, bounds.getY() + 6.0f, bounds.getWidth() - 12.0f, 20.0f, 3.0f);

    // Title text
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(moduleTitle, juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getY() + 6.0f, bounds.getWidth() - 20.0f, 20.0f),
               juce::Justification::centredLeft, true);
}

void ModuleCardComponent::updateScope(const float* data, int numSamples) {
    oscilloscope.updateData(data, numSamples);
}

void ModuleCardComponent::setLedSelector(LedSelectorComponent* selector) {
    ledSelector = selector;
    if (selector) addAndMakeVisible(selector);
}

void ModuleCardComponent::setSelector(juce::ComboBox* box) {
    selectorBox = box;
    if (box) addAndMakeVisible(box);
}

void ModuleCardComponent::setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider) {
    if (slotIndex < 0 || slotIndex >= 4) return;
    labels[slotIndex].setText(label, juce::dontSendNotification);
    labels[slotIndex].setVisible(true);
    knobs[slotIndex] = slider;
    if (slider) addAndMakeVisible(slider);
}

void ModuleCardComponent::setKnobLabel(int slotIndex, const juce::String& label) {
    if (slotIndex >= 0 && slotIndex < 4) {
        labels[slotIndex].setText(label, juce::dontSendNotification);
    }
}

void ModuleCardComponent::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(27); // header

    // Oscilloscope area
    auto scopeArea = area.removeFromTop(44).reduced(6, 2);
    oscilloscope.setBounds(scopeArea);

    area.reduce(6, 4);

    if (ledSelector != nullptr) {
        int selectorH = 38;
        if (ledSelector->getNumItems() > 8) {
            selectorH = 80;
        } else if (ledSelector->getNumItems() > 4) {
            selectorH = 66;
        } else if (ledSelector->getNumItems() <= 2) {
            selectorH = 22;
        }
        ledSelector->setBounds(area.removeFromTop(selectorH).reduced(2, 0));
        area.removeFromTop(4);

        // 3 knobs side-by-side
        int knobW = area.getWidth() / 3;
        for (int i = 0; i < 3; ++i) {
            auto slot = juce::Rectangle<int>(area.getX() + i * knobW, area.getY(), knobW, area.getHeight()).reduced(2);
            labels[i].setBounds(slot.removeFromTop(15));
            if (knobs[i]) knobs[i]->setBounds(slot);
        }
    } else if (selectorBox != nullptr) {
        // Selector row
        selectorBox->setBounds(area.removeFromTop(24).reduced(2, 0));
        area.removeFromTop(4);

        // 3 knobs side-by-side
        int knobW = area.getWidth() / 3;
        for (int i = 0; i < 3; ++i) {
            auto slot = juce::Rectangle<int>(area.getX() + i * knobW, area.getY(), knobW, area.getHeight()).reduced(2);
            labels[i].setBounds(slot.removeFromTop(15));
            if (knobs[i]) knobs[i]->setBounds(slot);
        }
    } else if (numActiveKnobs == 2) {
        // Grit FX: 2 knobs side-by-side, centered
        int knobW = area.getWidth() / 2;
        int topPad = (area.getHeight() - 140) / 2;
        if (topPad > 0) area.removeFromTop(topPad);

        for (int i = 0; i < 2; ++i) {
            auto slot = juce::Rectangle<int>(area.getX() + i * knobW, area.getY(), knobW, 140).reduced(4);
            labels[i].setBounds(slot.removeFromTop(16));
            if (knobs[i]) knobs[i]->setBounds(slot);
        }
    } else {
        // 4 knobs: 2x2 grid
        int colW = area.getWidth() / 2;
        int rowH = area.getHeight() / 2;
        for (int i = 0; i < 4; ++i) {
            int row = i / 2;
            int col = i % 2;
            auto slot = juce::Rectangle<int>(area.getX() + col * colW,
                                             area.getY() + row * rowH,
                                             colW, rowH).reduced(2);
            labels[i].setBounds(slot.removeFromTop(15));
            if (knobs[i]) knobs[i]->setBounds(slot);
        }
    }
}

// --- VALUE FORMATTING & PARSING HELPERS ---

static juce::String formatHz(float hz) {
    if (!std::isfinite(hz)) return "0 Hz";
    if (hz >= 1000.0f)
        return juce::String(hz * 0.001f, 2) + " kHz";
    return juce::String(hz, (hz < 10.0f ? 2 : 1)) + " Hz";
}

static double parseHz(const juce::String& str, float minHz, float maxHz) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return 0.5;

    double multiplier = 1.0;
    if (text.endsWith("khz") || text.endsWith("k")) {
        multiplier = 1000.0;
        text = text.upToFirstOccurrenceOf("k", false, false).trim();
    } else if (text.endsWith("hz")) {
        text = text.upToFirstOccurrenceOf("h", false, false).trim();
    }
    double hz = text.getDoubleValue() * multiplier;

    if (minHz <= 0.0f) {
        // Linear frequency range (e.g. 0 Hz to 5000 Hz)
        if (maxHz <= minHz) return 0.0;
        double norm = (hz - minHz) / (maxHz - minHz);
        if (!std::isfinite(norm)) return 0.0;
        return std::clamp(norm, 0.0, 1.0);
    } else {
        // Logarithmic frequency range (e.g. 20 Hz to 20 kHz)
        hz = std::clamp(hz, static_cast<double>(minHz), static_cast<double>(maxHz));
        double denom = std::log(maxHz / minHz);
        if (denom <= 0.0) return 0.0;
        double norm = std::log(hz / minHz) / denom;
        if (!std::isfinite(norm)) return 0.0;
        return std::clamp(norm, 0.0, 1.0);
    }
}

static juce::String formatMsOrS(float sec) {
    if (!std::isfinite(sec)) return "0 ms";
    if (sec >= 1.0f)
        return juce::String(sec, 2) + " s";
    return juce::String(static_cast<int>(std::round(sec * 1000.0f))) + " ms";
}

static double parseMsOrS(const juce::String& str, float minSec, float midSec, float maxSec) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return 0.5;

    double sec = 0.0;
    if (text.endsWith("ms")) {
        sec = text.upToFirstOccurrenceOf("ms", false, false).trim().getDoubleValue() * 0.001;
    } else if (text.endsWith("s")) {
        sec = text.upToFirstOccurrenceOf("s", false, false).trim().getDoubleValue();
    } else {
        double val = text.getDoubleValue();
        sec = (val >= 10.0 && maxSec <= 10.0) ? (val * 0.001) : val;
    }
    sec = std::clamp(sec, static_cast<double>(minSec), static_cast<double>(maxSec));
    double norm = 0.0;
    if (sec <= midSec) {
        double denom = std::log(midSec / minSec);
        norm = (denom > 0.0) ? (std::log(sec / minSec) / denom * 0.5) : 0.0;
    } else {
        double denom = std::log(maxSec / midSec);
        norm = (denom > 0.0) ? (0.5 + std::log(sec / midSec) / denom * 0.5) : 0.5;
    }
    if (!std::isfinite(norm)) return 0.5;
    return std::clamp(norm, 0.0, 1.0);
}

static juce::String formatPercent(float norm, bool signedPrefix = false) {
    if (!std::isfinite(norm)) norm = 0.0f;
    float pct = std::round((signedPrefix ? ((norm - 0.5f) * 200.0f) : (norm * 100.0f)));
    if (signedPrefix && pct > 0.0f)
        return "+" + juce::String(static_cast<int>(pct)) + " %";
    return juce::String(static_cast<int>(pct)) + " %";
}

static double parsePercent(const juce::String& str, bool isSigned) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return (isSigned ? 0.5 : 0.0);
    if (text.endsWith("%")) text = text.dropLastCharacters(1).trim();
    double val = text.getDoubleValue();
    double norm = 0.0;
    if (isSigned) {
        norm = (val + 100.0) / 200.0;
    } else {
        if (val > 1.0 && val <= 100.0) val *= 0.01;
        norm = val;
    }
    if (!std::isfinite(norm)) return (isSigned ? 0.5 : 0.0);
    return std::clamp(norm, 0.0, 1.0);
}

static juce::String formatLevel(float norm) {
    if (!std::isfinite(norm)) norm = 0.5f;
    float pct = (norm <= 0.5f) ? (norm * 200.0f) : (100.0f + (norm - 0.5f) * 600.0f);
    return juce::String(static_cast<int>(std::round(pct))) + " %";
}

static double parseLevel(const juce::String& str) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return 0.5;
    if (text.endsWith("%")) text = text.dropLastCharacters(1).trim();
    double pct = text.getDoubleValue();
    if (pct > 0.0 && pct <= 4.0 && !str.contains("%")) {
        pct *= 100.0;
    }
    double norm = 0.0;
    if (pct <= 100.0) {
        norm = pct / 200.0;
    } else {
        norm = 0.5 + ((pct - 100.0) / 600.0) * 0.5;
    }
    if (!std::isfinite(norm)) return 0.5;
    return std::clamp(norm, 0.0, 1.0);
}

static juce::String formatDb(float db) {
    if (!std::isfinite(db)) return "0 dB";
    if (db > 0.0f)
        return "+" + juce::String(db, 1) + " dB";
    return juce::String(db, 1) + " dB";
}

static double parseDb(const juce::String& str, float minDb, float midDb, float maxDb) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return 0.5;
    if (text.endsWith("db")) text = text.upToFirstOccurrenceOf("db", false, false).trim();
    double db = text.getDoubleValue();
    db = std::clamp(db, static_cast<double>(minDb), static_cast<double>(maxDb));
    double norm = 0.0;
    if (db <= midDb) {
        norm = ((db - minDb) / (midDb - minDb)) * 0.5;
    } else {
        norm = 0.5 + ((db - midDb) / (maxDb - midDb)) * 0.5;
    }
    if (!std::isfinite(norm)) return 0.5;
    return std::clamp(norm, 0.0, 1.0);
}

static juce::String midiNoteName(int note) {
    static const char* const names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (note / 12) - 1;
    int n = note % 12;
    if (n < 0) n += 12;
    return juce::String(names[n]) + juce::String(octave) + " [" + juce::String(note) + "]";
}

static double parseMidiNote(const juce::String& str) {
    auto text = str.trim().toUpperCase();
    if (text.isEmpty()) return 0.5;

    // Support entering raw frequency e.g. "440Hz", "440 Hz"
    if (text.endsWith("HZ")) {
        double hz = text.upToFirstOccurrenceOf("H", false, false).trim().getDoubleValue();
        if (hz > 0.0) {
            double note = 69.0 + 12.0 * std::log2(hz / 440.0);
            return std::clamp(note / 127.0, 0.0, 1.0);
        }
    }

    if (text.containsOnly("-0123456789")) {
        int note = text.getIntValue();
        return std::clamp(static_cast<double>(note) / 127.0, 0.0, 1.0);
    }

    static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int semi = -1;
    int charIdx = 0;

    if (text.length() >= 2) {
        juce::String two = text.substring(0, 2);
        for (int i = 0; i < 12; ++i) {
            if (two == noteNames[i]) { semi = i; charIdx = 2; break; }
        }
        if (semi == -1) {
            if (two == "DB") { semi = 1; charIdx = 2; }
            else if (two == "EB") { semi = 3; charIdx = 2; }
            else if (two == "GB") { semi = 6; charIdx = 2; }
            else if (two == "AB") { semi = 8; charIdx = 2; }
            else if (two == "BB") { semi = 10; charIdx = 2; }
        }
    }
    if (semi == -1 && text.length() >= 1) {
        juce::String one = text.substring(0, 1);
        for (int i = 0; i < 12; ++i) {
            if (one == noteNames[i]) { semi = i; charIdx = 1; break; }
        }
    }

    if (semi >= 0) {
        int octave = text.substring(charIdx).trim().getIntValue();
        int note = (octave + 1) * 12 + semi;
        return std::clamp(static_cast<double>(note) / 127.0, 0.0, 1.0);
    }

    return std::clamp(text.getDoubleValue() / 127.0, 0.0, 1.0);
}

static juce::String formatSemitones(float st) {
    if (!std::isfinite(st)) return "0 st";
    int s = static_cast<int>(std::round(st));
    if (s > 0) return "+" + juce::String(s) + " st";
    return juce::String(s) + " st";
}

static double parseSemitones(const juce::String& str, float minSt, float maxSt) {
    auto text = str.trim().toLowerCase();
    if (text.isEmpty()) return 0.5;
    if (text.endsWith("st") || text.endsWith("semitones"))
        text = text.upToFirstOccurrenceOf("s", false, false).trim();
    double st = text.getDoubleValue();
    st = std::clamp(st, static_cast<double>(minSt), static_cast<double>(maxSt));
    double norm = (st - minSt) / (maxSt - minSt);
    if (!std::isfinite(norm)) return 0.5;
    return std::clamp(norm, 0.0, 1.0);
}

static juce::String formatWaveform(float norm) {
    if (norm <= 0.15f) return "Sine";
    if (norm <= 0.32f) return "Tri";
    if (norm <= 0.45f) return "Saw";
    if (norm <= 0.55f) return "Square";
    return "PWM " + juce::String(static_cast<int>(std::round(100.0f - (norm - 0.5f) * 200.0f))) + "%";
}

static juce::String formatSlope(float norm) {
    if (norm < 0.48f)
        return "Exp (" + juce::String(1.0f + (0.49f - norm) * 6.0f, 1) + ")";
    if (norm <= 0.52f)
        return "Linear";
    return "Log (" + juce::String(1.0f + (norm - 0.51f) * 6.0f, 1) + ")";
}

static juce::String formatFmRatio(float norm) {
    if (norm <= 0.5f) {
        float r = 32.0f - (norm * 2.0f) * 31.0f;
        return "1:" + juce::String(r, 1);
    } else {
        float r = 1.0f + ((norm - 0.5f) * 2.0f) * 31.0f;
        return juce::String(r, 1) + ":1";
    }
}

// --- MAIN PROCESSOR EDITOR ---

BiaEr1AudioProcessorEditor::BiaEr1AudioProcessorEditor(BiaEr1AudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      carrierTrackingSelector(juce::Colour(0xff00d2ff)),
      modTypeSelector(juce::Colour(0xffff7b00)),
      driveTypeSelector(juce::Colour(0xffff3366)),
      filterTypeSelector(juce::Colour(0xff00e676)),
      ampEnvTypeSelector(juce::Colour(0xffab47bc))
{
    setLookAndFeel(&knobLookAndFeel);

    // 1. Accent Colours for all 10 Blocks
    const juce::Colour colCarrier   (0xff00d2ff); // Cyan
    const juce::Colour colModulator (0xffff7b00); // Orange
    const juce::Colour colDrive     (0xffff3366); // Crimson
    const juce::Colour colNoise     (0xffb388ff); // Lavender
    const juce::Colour colFilter    (0xff00e676); // Green
    const juce::Colour colRingMod   (0xffffd600); // Yellow
    const juce::Colour colGrit      (0xffff4081); // Pink
    const juce::Colour colFreqShift (0xff00e5ff); // Teal
    const juce::Colour colAmp       (0xffffab00); // Amber
    const juce::Colour colAmpEnv    (0xffab47bc); // Purple

    // 2. Setup All 10 Cards
    cards.push_back(std::make_unique<ModuleCardComponent>("01  CARRIER", colCarrier));
    cards.push_back(std::make_unique<ModuleCardComponent>("02  MODULATOR", colModulator));
    cards.push_back(std::make_unique<ModuleCardComponent>("03  DRIVE", colDrive));
    cards.push_back(std::make_unique<ModuleCardComponent>("04  NOISE TRANSIENT", colNoise));
    cards.push_back(std::make_unique<ModuleCardComponent>("05  FILTER", colFilter));
    cards.push_back(std::make_unique<ModuleCardComponent>("06  RING MOD", colRingMod));
    cards.push_back(std::make_unique<ModuleCardComponent>("07  GRIT FX", colGrit));
    cards.push_back(std::make_unique<ModuleCardComponent>("08  FREQ SHIFTER", colFreqShift));
    cards.push_back(std::make_unique<ModuleCardComponent>("09  AMP", colAmp));
    cards.push_back(std::make_unique<ModuleCardComponent>("10  AMP ENVELOPE", colAmpEnv));

    for (auto& card : cards) addAndMakeVisible(card.get());

    // 3. Setup Selectors and Items
    carrierTrackingBox.addItemList({ "Fixed Freq", "Fixed Pitch", "MIDI Pitch", "Fixed Noise" }, 1);
    modTypeBox.addItemList({ "Fixed Osc", "Follow Osc", "FM Ratio", "Sine x Noise",
                             "Follow x Noise", "S&H Noise", "Fast Decay", "Slow Decay" }, 1);
    driveTypeBox.addItemList({ "Off", "Saturation", "Clipper", "Wave Folder" }, 1);
    filterTypeBox.addItemList({ "NoRez LPF", "NoRez BPF", "NoRez HPF", "NoRez Notch",
                                "Rezzy LPF", "Rezzy BPF", "Rezzy HPF", "Rezzy Notch", "Comb", "APF Disperser" }, 1);
    ampEnvTypeBox.addItemList({ "Fast Decay", "Slow Decay" }, 1);

    setupBox(carrierTrackingBox);
    setupBox(modTypeBox);
    setupBox(driveTypeBox);
    setupBox(filterTypeBox);
    setupBox(ampEnvTypeBox);

    carrierTrackingSelector.setItems({ "Fixed Freq", "Fixed Pitch", "MIDI Pitch", "Fixed Noise" }, 2);
    modTypeSelector.setItems({ "Fixed Osc", "Follow Osc", "FM Ratio", "Sine x Noise",
                               "Follow x Noise", "S&H Noise", "Fast Decay", "Slow Decay" }, 2);
    driveTypeSelector.setItems({ "Off", "Saturation", "Clipper", "Wave Folder" }, 2);
    filterTypeSelector.setItems({ "NoRez LPF", "NoRez BPF", "NoRez HPF", "NoRez Notch",
                                  "Rezzy LPF", "Rezzy BPF", "Rezzy HPF", "Rezzy Notch", "Comb", "APF Disperser" }, 2);
    ampEnvTypeSelector.setItems({ "Fast Decay", "Slow Decay" }, 2);

    carrierTrackingSelector.onChange = [this](int idx) {
        carrierTrackingBox.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };
    modTypeSelector.onChange = [this](int idx) {
        modTypeBox.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };
    driveTypeSelector.onChange = [this](int idx) {
        driveTypeBox.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };
    filterTypeSelector.onChange = [this](int idx) {
        filterTypeBox.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };
    ampEnvTypeSelector.onChange = [this](int idx) {
        ampEnvTypeBox.setSelectedItemIndex(idx, juce::sendNotificationSync);
    };

    // 4. Setup Sliders
    setupKnob(carrierPitchSlider, colCarrier);
    setupKnob(carrierShapeSlider, colCarrier);
    setupKnob(carrierLevelSlider, colCarrier);

    setupKnob(modShapeSlider, colModulator);
    setupKnob(modDepthSlider, colModulator);
    setupKnob(modSpeedSlider, colModulator);

    setupKnob(driveAmountSlider, colDrive);
    setupKnob(driveBiasSlider, colDrive);
    setupKnob(driveFilterSlider, colDrive);

    setupKnob(noiseShRateSlider, colNoise);
    setupKnob(noiseFilterSlider, colNoise);
    setupKnob(noiseLevelSlider, colNoise);
    setupKnob(noiseDecaySlider, colNoise);

    setupKnob(filterCutoffSlider, colFilter);
    setupKnob(filterDepthSlider, colFilter);
    setupKnob(filterDecaySlider, colFilter);

    setupKnob(ringModShapeSlider, colRingMod);
    setupKnob(ringModRateSlider, colRingMod);
    setupKnob(ringModAmountSlider, colRingMod);
    setupKnob(ringModWidthSlider, colRingMod);

    setupKnob(gritBitsSlider, colGrit);
    setupKnob(gritRateSlider, colGrit);

    setupKnob(freqShiftShiftSlider, colFreqShift);
    setupKnob(freqShiftRangeSlider, colFreqShift);
    setupKnob(freqShiftBlendSlider, colFreqShift);
    setupKnob(freqShiftWidthSlider, colFreqShift);

    setupKnob(ampPanSlider, colAmp);
    setupKnob(ampLevelSlider, colAmp);
    setupKnob(ampDriveSlider, colAmp);
    setupKnob(ampLowBoostSlider, colAmp);

    setupKnob(ampEnvClapsSlider, colAmpEnv);
    setupKnob(ampEnvShapeSlider, colAmpEnv);
    setupKnob(ampEnvDecaySlider, colAmpEnv);

    // 5. APVTS Attachments
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier_tracking", carrierTrackingBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_pitch", carrierPitchSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_shape", carrierShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "carrier_level", carrierLevelSlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod_type", modTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_shape", modShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_depth", modDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "mod_speed", modSpeedSlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "drive_type", driveTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_amount", driveAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_bias", driveBiasSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "drive_filter", driveFilterSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_sh_rate", noiseShRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_filter", noiseFilterSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_level", noiseLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "noise_decay", noiseDecaySlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter_type", filterTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_cutoff", filterCutoffSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_depth", filterDepthSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_decay", filterDecaySlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_shape", ringModShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_rate", ringModRateSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_amount", ringModAmountSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "ringmod_width", ringModWidthSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_bits", gritBitsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "grit_rate", gritRateSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_shift", freqShiftShiftSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_range", freqShiftRangeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_blend", freqShiftBlendSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "freqshift_width", freqShiftWidthSlider));

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_pan", ampPanSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_level", ampLevelSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_drive", ampDriveSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_low_boost", ampLowBoostSlider));

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_env_type", ampEnvTypeBox));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_env_claps", ampEnvClapsSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_env_shape", ampEnvShapeSlider));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, "amp_env_decay", ampEnvDecaySlider));

    // 6. Assign Controls to Cards
    // Card 0: Carrier
    cards[0]->setLedSelector(&carrierTrackingSelector);
    cards[0]->setKnob(0, "FREQUENCY", &carrierPitchSlider);
    cards[0]->setKnob(1, "WAVEFORM", &carrierShapeSlider);
    cards[0]->setKnob(2, "LEVEL", &carrierLevelSlider);

    // Card 1: Modulator
    cards[1]->setLedSelector(&modTypeSelector);
    cards[1]->setKnob(0, "WAVEFORM", &modShapeSlider);
    cards[1]->setKnob(1, "DEPTH", &modDepthSlider);
    cards[1]->setKnob(2, "SPEED", &modSpeedSlider);

    // Card 2: Drive
    cards[2]->setLedSelector(&driveTypeSelector);
    cards[2]->setKnob(0, "DRIVE", &driveAmountSlider);
    cards[2]->setKnob(1, "BIAS", &driveBiasSlider);
    cards[2]->setKnob(2, "DJ FILTER", &driveFilterSlider);

    // Card 3: Noise Transient
    cards[3]->setKnob(0, "S&H RATE", &noiseShRateSlider);
    cards[3]->setKnob(1, "DJ FILTER", &noiseFilterSlider);
    cards[3]->setKnob(2, "LEVEL", &noiseLevelSlider);
    cards[3]->setKnob(3, "DECAY", &noiseDecaySlider);

    // Card 4: Filter
    cards[4]->setLedSelector(&filterTypeSelector);
    cards[4]->setKnob(0, "CUTOFF", &filterCutoffSlider);
    cards[4]->setKnob(1, "ENV DEPTH", &filterDepthSlider);
    cards[4]->setKnob(2, "DECAY", &filterDecaySlider);

    // Card 5: Ring Mod
    cards[5]->setKnob(0, "WAVEFORM", &ringModShapeSlider);
    cards[5]->setKnob(1, "RATE", &ringModRateSlider);
    cards[5]->setKnob(2, "AMOUNT", &ringModAmountSlider);
    cards[5]->setKnob(3, "WIDTH", &ringModWidthSlider);

    // Card 6: Grit FX (2 active knobs)
    cards[6]->setNumActiveKnobs(2);
    cards[6]->setKnob(0, "BITS", &gritBitsSlider);
    cards[6]->setKnob(1, "SAMPLE RATE", &gritRateSlider);

    // Card 7: Freq Shifter
    cards[7]->setKnob(0, "SHIFT", &freqShiftShiftSlider);
    cards[7]->setKnob(1, "RANGE", &freqShiftRangeSlider);
    cards[7]->setKnob(2, "BLEND", &freqShiftBlendSlider);
    cards[7]->setKnob(3, "WIDTH", &freqShiftWidthSlider);

    // Card 8: Amp
    cards[8]->setKnob(0, "PAN", &ampPanSlider);
    cards[8]->setKnob(1, "LEVEL", &ampLevelSlider);
    cards[8]->setKnob(2, "DRIVE", &ampDriveSlider);
    cards[8]->setKnob(3, "LOW BOOST", &ampLowBoostSlider);

    // Card 9: Amp Env
    cards[9]->setLedSelector(&ampEnvTypeSelector);
    cards[9]->setKnob(0, "CLAPS", &ampEnvClapsSlider);
    cards[9]->setKnob(1, "SLOPE", &ampEnvShapeSlider);
    cards[9]->setKnob(2, "DECAY", &ampEnvDecaySlider);

    // Header Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222733));
    triggerButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00d2ff));
    triggerButton.onClick = [this] {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    carrierTrackingBox.onChange = [this] { updateDynamicControls(); };
    modTypeBox.onChange         = [this] { updateDynamicControls(); };
    driveTypeBox.onChange       = [this] { updateDynamicControls(); };
    filterTypeBox.onChange      = [this] { updateDynamicControls(); };
    ampEnvTypeBox.onChange      = [this] { updateDynamicControls(); };

    // Initial setup
    updateDynamicControls();

    // 30 Hz timer for smooth scopes and state check
    startTimerHz(30);

    // 1280 x 720 single screen with no scrolling
    setSize(1280, 720);
    setResizable(true, true);
    setResizeLimits(1000, 600, 2560, 1440);
}

BiaEr1AudioProcessorEditor::~BiaEr1AudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void BiaEr1AudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour) {
    slider.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
    slider.setRange(0.0, 1.0, 0.0005);
}

void BiaEr1AudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setJustificationType(juce::Justification::centred);
}

void BiaEr1AudioProcessorEditor::timerCallback() {
    // 1. Pull real-time oscilloscope data for all 10 cards
    float scopeBuffer[128];
    for (int i = 0; i < 10; ++i) {
        audioProcessor.getEngine().getScopeData(i, scopeBuffer, 128);
        cards[i]->updateScope(scopeBuffer, 128);
    }

    // 2. Check for selector changes
    int curCarrierTrack = carrierTrackingBox.getSelectedItemIndex();
    int curModType = modTypeBox.getSelectedItemIndex();
    int curDriveType = driveTypeBox.getSelectedItemIndex();
    int curFilterType = filterTypeBox.getSelectedItemIndex();
    int curAmpEnvType = ampEnvTypeBox.getSelectedItemIndex();

    if (curCarrierTrack != lastCarrierTrack ||
        curModType != lastModType ||
        curDriveType != lastDriveType ||
        curFilterType != lastFilterType ||
        curAmpEnvType != lastAmpEnvType)
    {
        updateDynamicControls();
    }
}

void BiaEr1AudioProcessorEditor::updateDynamicControls() {
    lastCarrierTrack = carrierTrackingBox.getSelectedItemIndex();
    lastModType      = modTypeBox.getSelectedItemIndex();
    lastDriveType    = driveTypeBox.getSelectedItemIndex();
    lastFilterType   = filterTypeBox.getSelectedItemIndex();
    lastAmpEnvType   = ampEnvTypeBox.getSelectedItemIndex();

    carrierTrackingSelector.setSelectedIndex(lastCarrierTrack, juce::dontSendNotification);
    modTypeSelector.setSelectedIndex(lastModType, juce::dontSendNotification);
    driveTypeSelector.setSelectedIndex(lastDriveType, juce::dontSendNotification);
    filterTypeSelector.setSelectedIndex(lastFilterType, juce::dontSendNotification);
    ampEnvTypeSelector.setSelectedIndex(lastAmpEnvType, juce::dontSendNotification);

    // ==========================================
    // 1. CARRIER
    // ==========================================
    if (lastCarrierTrack == 0) { // Fixed Freq: 20 Hz to 20 kHz
        cards[0]->setKnobLabel(0, "FREQUENCY");
        cards[0]->setKnobLabel(1, "WAVEFORM");
        carrierPitchSlider.customFormatText = [](double v) {
            float f = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(v));
            return formatHz(f);
        };
        carrierPitchSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 20.0f, 20000.0f);
        };
        carrierShapeSlider.customFormatText = [](double v) {
            return formatWaveform(static_cast<float>(v));
        };
        carrierShapeSlider.customParseText = [](const juce::String& s) {
            auto t = s.trim().toLowerCase();
            if (t.contains("sine")) return 0.0;
            if (t.contains("tri")) return 0.25;
            if (t.contains("saw")) return 0.40;
            if (t.contains("sq")) return 0.50;
            if (t.contains("pwm")) return 0.75;
            return parsePercent(s, false);
        };
    } else if (lastCarrierTrack == 1) { // Fixed Pitch: MIDI note 0 to 127
        cards[0]->setKnobLabel(0, "PITCH");
        cards[0]->setKnobLabel(1, "WAVEFORM");
        carrierPitchSlider.customFormatText = [](double v) {
            int note = std::clamp(static_cast<int>(std::round(v * 127.0)), 0, 127);
            return midiNoteName(note);
        };
        carrierPitchSlider.customParseText = [](const juce::String& s) {
            return parseMidiNote(s);
        };
        carrierShapeSlider.customFormatText = [](double v) {
            return formatWaveform(static_cast<float>(v));
        };
        carrierShapeSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, false);
        };
    } else if (lastCarrierTrack == 2) { // MIDI Pitch: offset -60 to +60
        cards[0]->setKnobLabel(0, "OFFSET");
        cards[0]->setKnobLabel(1, "WAVEFORM");
        carrierPitchSlider.customFormatText = [](double v) {
            float st = std::round((static_cast<float>(v) - 0.5f) * 120.0f);
            return formatSemitones(st);
        };
        carrierPitchSlider.customParseText = [](const juce::String& s) {
            return parseSemitones(s, -60.0f, 60.0f);
        };
        carrierShapeSlider.customFormatText = [](double v) {
            return formatWaveform(static_cast<float>(v));
        };
        carrierShapeSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, false);
        };
    } else { // Fixed Noise: S&H Rate 0.1 Hz to 5 kHz & LPF 20 Hz to 20 kHz
        cards[0]->setKnobLabel(0, "S&H RATE");
        cards[0]->setKnobLabel(1, "NOISE LPF");
        carrierPitchSlider.customFormatText = [](double v) {
            float f = 0.1f * std::pow(5000.0f / 0.1f, static_cast<float>(v));
            return formatHz(f);
        };
        carrierPitchSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 0.1f, 5000.0f);
        };
        carrierShapeSlider.customFormatText = [](double v) {
            float f = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(v));
            return formatHz(f);
        };
        carrierShapeSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 20.0f, 20000.0f);
        };
    }

    carrierLevelSlider.customFormatText = [](double v) {
        return formatLevel(static_cast<float>(v));
    };
    carrierLevelSlider.customParseText = [](const juce::String& s) {
        return parseLevel(s);
    };

    // ==========================================
    // 2. MODULATOR
    // ==========================================
    modDepthSlider.customFormatText = [](double v) {
        return formatPercent(static_cast<float>(v), true);
    };
    modDepthSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    if (lastModType <= 2) { // 0=Fixed, 1=Follow, 2=FM Ratio
        cards[1]->setKnobLabel(0, "WAVEFORM");
        modShapeSlider.customFormatText = [](double v) {
            return formatWaveform(static_cast<float>(v));
        };
        modShapeSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, false);
        };

        if (lastModType == 0) {
            cards[1]->setKnobLabel(2, "SPEED");
            modSpeedSlider.customFormatText = [](double v) {
                float f = 0.1f * std::pow(5000.0f / 0.1f, static_cast<float>(v));
                return formatHz(f);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseHz(s, 0.1f, 5000.0f);
            };
        } else if (lastModType == 1) {
            cards[1]->setKnobLabel(2, "OFFSET");
            modSpeedSlider.customFormatText = [](double v) {
                float st = -64.0f + static_cast<float>(v) * 128.0f;
                return formatSemitones(st);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseSemitones(s, -64.0f, 64.0f);
            };
        } else {
            cards[1]->setKnobLabel(2, "FM RATIO");
            modSpeedSlider.customFormatText = [](double v) {
                return formatFmRatio(static_cast<float>(v));
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parsePercent(s, false);
            };
        }
    } else if (lastModType == 3 || lastModType == 4) { // Sine x Noise
        cards[1]->setKnobLabel(0, "NOISE RATE");
        modShapeSlider.customFormatText = [](double v) {
            float f = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(v));
            return formatHz(f);
        };
        modShapeSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 0.1f, 20000.0f);
        };

        if (lastModType == 3) {
            cards[1]->setKnobLabel(2, "SINE SPEED");
            modSpeedSlider.customFormatText = [](double v) {
                float f = 0.1f * std::pow(5000.0f / 0.1f, static_cast<float>(v));
                return formatHz(f);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseHz(s, 0.1f, 5000.0f);
            };
        } else {
            cards[1]->setKnobLabel(2, "SINE OFFSET");
            modSpeedSlider.customFormatText = [](double v) {
                float st = -64.0f + static_cast<float>(v) * 128.0f;
                return formatSemitones(st);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseSemitones(s, -64.0f, 64.0f);
            };
        }
    } else if (lastModType == 5) { // S&H Noise
        cards[1]->setKnobLabel(0, "S&H RATE");
        cards[1]->setKnobLabel(2, "CLOCK SPEED");
        modShapeSlider.customFormatText = [](double v) {
            float f = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(v));
            return formatHz(f);
        };
        modShapeSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 0.1f, 20000.0f);
        };
        modSpeedSlider.customFormatText = [](double v) {
            float f = 0.1f * std::pow(5000.0f / 0.1f, static_cast<float>(v));
            return formatHz(f);
        };
        modSpeedSlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 0.1f, 5000.0f);
        };
    } else { // 6=Fast decay, 7=Slow decay
        cards[1]->setKnobLabel(0, "SLOPE");
        cards[1]->setKnobLabel(2, "DECAY TIME");
        modShapeSlider.customFormatText = [](double v) {
            return formatSlope(static_cast<float>(v));
        };
        modShapeSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, false);
        };

        if (lastModType == 6) {
            modSpeedSlider.customFormatText = [](double v) {
                float sec = (v <= 0.5) ? (0.010f * std::pow(0.333f / 0.010f, static_cast<float>(v) * 2.0f))
                                       : (0.333f * std::pow(5.0f / 0.333f, (static_cast<float>(v) - 0.5f) * 2.0f));
                return formatMsOrS(sec);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseMsOrS(s, 0.010f, 0.333f, 5.0f);
            };
        } else {
            modSpeedSlider.customFormatText = [](double v) {
                float sec = (v <= 0.5) ? (0.100f * std::pow(5.0f / 0.100f, static_cast<float>(v) * 2.0f))
                                       : (5.0f * std::pow(60.0f / 5.0f, (static_cast<float>(v) - 0.5f) * 2.0f));
                return formatMsOrS(sec);
            };
            modSpeedSlider.customParseText = [](const juce::String& s) {
                return parseMsOrS(s, 0.100f, 5.0f, 60.0f);
            };
        }
    }

    // ==========================================
    // 3. DRIVE
    // ==========================================
    if (lastDriveType == 0) { // Off
        driveAmountSlider.setEnabled(false);
        driveBiasSlider.setEnabled(false);
        driveFilterSlider.setEnabled(false);
    } else {
        driveAmountSlider.setEnabled(true);
        driveBiasSlider.setEnabled(true);
        driveFilterSlider.setEnabled(true);
    }

    if (lastDriveType == 1) { // Saturation: -6dB to 0dB to +24dB
        cards[2]->setKnobLabel(0, "DRIVE");
        driveAmountSlider.customFormatText = [](double v) {
            float db = (v <= 0.5) ? (-6.0f + static_cast<float>(v) * 12.0f) : ((static_cast<float>(v) - 0.5f) * 48.0f);
            return formatDb(db);
        };
        driveAmountSlider.customParseText = [](const juce::String& s) {
            return parseDb(s, -6.0f, 0.0f, 24.0f);
        };
    } else if (lastDriveType == 2) { // Clipper: -96dB to 0dB
        cards[2]->setKnobLabel(0, "THRESHOLD");
        driveAmountSlider.customFormatText = [](double v) {
            float db = -96.0f + static_cast<float>(v) * 96.0f;
            return formatDb(db);
        };
        driveAmountSlider.customParseText = [](const juce::String& s) {
            return parseDb(s, -96.0f, -48.0f, 0.0f);
        };
    } else { // Wavefolder: -96dB to 0dB
        cards[2]->setKnobLabel(0, "FOLD GAIN");
        driveAmountSlider.customFormatText = [](double v) {
            float db = -96.0f + static_cast<float>(v) * 96.0f;
            return formatDb(db);
        };
        driveAmountSlider.customParseText = [](const juce::String& s) {
            return parseDb(s, -96.0f, -48.0f, 0.0f);
        };
    }

    driveBiasSlider.customFormatText = [](double v) {
        float b = (static_cast<float>(v) - 0.5f) * 2.0f;
        return (b > 0.0f ? "+" : "") + juce::String(b, 2);
    };
    driveBiasSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    driveFilterSlider.customFormatText = [](double v) {
        if (v >= 0.49 && v <= 0.51) return juce::String("Flat");
        if (v < 0.49) {
            float norm = static_cast<float>(v) / 0.49f;
            float f = 20.0f * std::pow(20000.0f / 20.0f, norm);
            return "LP " + formatHz(f);
        } else {
            float norm = (static_cast<float>(v) - 0.51f) / 0.49f;
            float f = 20.0f * std::pow(20000.0f / 20.0f, norm);
            return "HP " + formatHz(f);
        }
    };
    driveFilterSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    // ==========================================
    // 4. NOISE TRANSIENT
    // ==========================================
    noiseShRateSlider.customFormatText = [](double v) {
        float f = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(v));
        return formatHz(f);
    };
    noiseShRateSlider.customParseText = [](const juce::String& s) {
        return parseHz(s, 0.1f, 20000.0f);
    };

    noiseFilterSlider.customFormatText = [](double v) {
        if (v >= 0.49 && v <= 0.51) return juce::String("Flat");
        if (v < 0.49) {
            float norm = static_cast<float>(v) / 0.49f;
            float f = 20.0f * std::pow(20000.0f / 20.0f, norm);
            return "LP " + formatHz(f);
        } else {
            float norm = (static_cast<float>(v) - 0.51f) / 0.49f;
            float f = 20.0f * std::pow(20000.0f / 20.0f, norm);
            return "HP " + formatHz(f);
        }
    };
    noiseFilterSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    noiseLevelSlider.customFormatText = [](double v) {
        return formatPercent(static_cast<float>(v));
    };
    noiseLevelSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    noiseDecaySlider.customFormatText = [](double v) {
        float sec = (v <= 0.5) ? (0.010f * std::pow(0.333f / 0.010f, static_cast<float>(v) * 2.0f))
                               : (0.333f * std::pow(5.0f / 0.333f, (static_cast<float>(v) - 0.5f) * 2.0f));
        return formatMsOrS(sec);
    };
    noiseDecaySlider.customParseText = [](const juce::String& s) {
        return parseMsOrS(s, 0.010f, 0.333f, 5.0f);
    };

    // ==========================================
    // 5. FILTER
    // ==========================================
    filterCutoffSlider.customFormatText = [](double v) {
        float f = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(v));
        return formatHz(f);
    };
    filterCutoffSlider.customParseText = [](const juce::String& s) {
        return parseHz(s, 20.0f, 20000.0f);
    };

    if (lastFilterType <= 7) { // NoRez or Rezzy
        cards[4]->setKnobLabel(1, "ENV DEPTH");
        cards[4]->setKnobLabel(2, "DECAY");
        filterDepthSlider.customFormatText = [](double v) {
            return formatPercent(static_cast<float>(v), true);
        };
        filterDepthSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, true);
        };

        if (lastFilterType <= 3) { // NoRez: 100ms to 5s to 60s
            filterDecaySlider.customFormatText = [](double v) {
                float sec = (v <= 0.5) ? (0.100f * std::pow(5.0f / 0.100f, static_cast<float>(v) * 2.0f))
                                       : (5.0f * std::pow(60.0f / 5.0f, (static_cast<float>(v) - 0.5f) * 2.0f));
                return formatMsOrS(sec);
            };
            filterDecaySlider.customParseText = [](const juce::String& s) {
                return parseMsOrS(s, 0.100f, 5.0f, 60.0f);
            };
        } else { // Rezzy: 10ms to 333ms to 5s
            filterDecaySlider.customFormatText = [](double v) {
                float sec = (v <= 0.5) ? (0.010f * std::pow(0.333f / 0.010f, static_cast<float>(v) * 2.0f))
                                       : (0.333f * std::pow(5.0f / 0.333f, (static_cast<float>(v) - 0.5f) * 2.0f));
                return formatMsOrS(sec);
            };
            filterDecaySlider.customParseText = [](const juce::String& s) {
                return parseMsOrS(s, 0.010f, 0.333f, 5.0f);
            };
        }
    } else if (lastFilterType == 8) { // Comb
        cards[4]->setKnobLabel(1, "RESONANCE");
        cards[4]->setKnobLabel(2, "DAMPEN");
        filterDepthSlider.customFormatText = [](double v) {
            return formatPercent(static_cast<float>(v), true);
        };
        filterDepthSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, true);
        };
        filterDecaySlider.customFormatText = [](double v) {
            float f = 0.1f * std::pow(20000.0f / 0.1f, static_cast<float>(v));
            return formatHz(f);
        };
        filterDecaySlider.customParseText = [](const juce::String& s) {
            return parseHz(s, 0.1f, 20000.0f);
        };
    } else { // APF Disperser
        cards[4]->setKnobLabel(1, "RESONANCE");
        cards[4]->setKnobLabel(2, "STAGES");
        filterDepthSlider.customFormatText = [](double v) {
            return formatPercent(static_cast<float>(v), true);
        };
        filterDepthSlider.customParseText = [](const juce::String& s) {
            return parsePercent(s, true);
        };
        filterDecaySlider.customFormatText = [](double v) {
            int stages = static_cast<int>(std::round(v * 32.0));
            return juce::String(stages) + (stages == 1 ? " stage" : " stages");
        };
        filterDecaySlider.customParseText = [](const juce::String& s) {
            int val = s.trim().getIntValue();
            return std::clamp(static_cast<double>(val) / 32.0, 0.0, 1.0);
        };
    }

    // ==========================================
    // 6. RING MOD
    // ==========================================
    ringModShapeSlider.customFormatText = [](double v) {
        return formatWaveform(static_cast<float>(v));
    };
    ringModShapeSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    ringModRateSlider.customFormatText = [](double v) {
        float f = 0.1f * std::pow(5000.0f / 0.1f, static_cast<float>(v));
        return formatHz(f);
    };
    ringModRateSlider.customParseText = [](const juce::String& s) {
        return parseHz(s, 0.1f, 5000.0f);
    };

    ringModAmountSlider.customFormatText = [](double v) {
        return formatPercent(static_cast<float>(v));
    };
    ringModAmountSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    ringModWidthSlider.customFormatText = [](double v) {
        return formatPercent(static_cast<float>(v), true);
    };
    ringModWidthSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    // ==========================================
    // 7. GRIT FX
    // ==========================================
    gritBitsSlider.customFormatText = [](double v) {
        float b = 1.0f + static_cast<float>(v) * 15.0f;
        return juce::String(b, 1) + " bit";
    };
    gritBitsSlider.customParseText = [](const juce::String& s) {
        auto t = s.trim().toLowerCase();
        if (t.endsWith("bit")) t = t.upToFirstOccurrenceOf("bit", false, false);
        double b = t.getDoubleValue();
        return std::clamp((b - 1.0) / 15.0, 0.0, 1.0);
    };

    gritRateSlider.customFormatText = [](double v) {
        float f = 20.0f * std::pow(20000.0f / 20.0f, static_cast<float>(v));
        return formatHz(f);
    };
    gritRateSlider.customParseText = [](const juce::String& s) {
        return parseHz(s, 20.0f, 20000.0f);
    };

    // ==========================================
    // 8. FREQUENCY SHIFTER
    // ==========================================
    freqShiftRangeSlider.customFormatText = [](double v) {
        if (!std::isfinite(v)) v = 0.0;
        float f = static_cast<float>(v) * 5000.0f;
        return formatHz(f);
    };
    freqShiftRangeSlider.customParseText = [](const juce::String& s) {
        return parseHz(s, 0.0f, 5000.0f);
    };

    freqShiftShiftSlider.customFormatText = [this](double v) {
        if (!std::isfinite(v)) v = 0.5;
        float curRange = static_cast<float>(freqShiftRangeSlider.getValue()) * 5000.0f;
        float shiftHz = (static_cast<float>(v) - 0.5f) * 2.0f * curRange;
        return (shiftHz > 0.0f ? "+" : "") + formatHz(shiftHz);
    };
    freqShiftShiftSlider.customParseText = [this](const juce::String& s) {
        float curRange = static_cast<float>(freqShiftRangeSlider.getValue()) * 5000.0f;
        if (curRange < 0.1f) return 0.5;
        auto t = s.trim().toLowerCase();
        double mult = 1.0;
        if (t.endsWith("khz") || t.endsWith("k")) {
            mult = 1000.0;
            t = t.upToFirstOccurrenceOf("k", false, false).trim();
        } else if (t.endsWith("hz")) {
            t = t.upToFirstOccurrenceOf("h", false, false).trim();
        }
        double val = t.getDoubleValue() * mult;
        double norm = 0.5 + (val / (2.0 * curRange));
        if (!std::isfinite(norm)) return 0.5;
        return std::clamp(norm, 0.0, 1.0);
    };

    freqShiftBlendSlider.customFormatText = [](double v) {
        if (v < 0.05) return juce::String("LSB 100%");
        if (v > 0.95) return juce::String("USB 100%");
        if (v >= 0.48 && v <= 0.52) return juce::String("Dry");
        if (v < 0.48) {
            float p = static_cast<float>(v) / 0.5f;
            return juce::String(static_cast<int>(std::round((1.0f - p) * 100.0f))) + "% LSB";
        } else {
            float p = (static_cast<float>(v) - 0.5f) / 0.5f;
            return juce::String(static_cast<int>(std::round(p * 100.0f))) + "% USB";
        }
    };
    freqShiftBlendSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    freqShiftWidthSlider.customFormatText = [](double v) {
        return formatPercent(static_cast<float>(v), true);
    };
    freqShiftWidthSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    // ==========================================
    // 9. AMP
    // ==========================================
    ampPanSlider.customFormatText = [](double v) {
        if (v >= 0.48 && v <= 0.52) return juce::String("Center");
        if (v < 0.48) return juce::String(static_cast<int>(std::round((0.5 - v) * 200.0))) + "% L";
        return juce::String(static_cast<int>(std::round((v - 0.5) * 200.0))) + "% R";
    };
    ampPanSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, true);
    };

    ampLevelSlider.customFormatText = [](double v) {
        return formatLevel(static_cast<float>(v));
    };
    ampLevelSlider.customParseText = [](const juce::String& s) {
        return parseLevel(s);
    };

    ampDriveSlider.customFormatText = [](double v) {
        float db = (v <= 0.5) ? (-6.0f + static_cast<float>(v) * 12.0f) : ((static_cast<float>(v) - 0.5f) * 48.0f);
        return formatDb(db);
    };
    ampDriveSlider.customParseText = [](const juce::String& s) {
        return parseDb(s, -6.0f, 0.0f, 24.0f);
    };

    ampLowBoostSlider.customFormatText = [](double v) {
        float db = static_cast<float>(v) * 24.0f;
        return formatDb(db);
    };
    ampLowBoostSlider.customParseText = [](const juce::String& s) {
        auto t = s.trim().toLowerCase();
        if (t.endsWith("db")) t = t.upToFirstOccurrenceOf("db", false, false);
        double val = t.getDoubleValue();
        return std::clamp(val / 24.0, 0.0, 1.0);
    };

    // ==========================================
    // 10. AMP ENVELOPE
    // ==========================================
    ampEnvClapsSlider.customFormatText = [](double v) {
        int claps = 1 + static_cast<int>(std::round(v * 15.0));
        return juce::String(claps) + (claps == 1 ? " clap" : " claps");
    };
    ampEnvClapsSlider.customParseText = [](const juce::String& s) {
        int val = s.trim().getIntValue();
        return std::clamp(static_cast<double>(val - 1) / 15.0, 0.0, 1.0);
    };

    ampEnvShapeSlider.customFormatText = [](double v) {
        return formatSlope(static_cast<float>(v));
    };
    ampEnvShapeSlider.customParseText = [](const juce::String& s) {
        return parsePercent(s, false);
    };

    if (lastAmpEnvType == 0) { // Fast time: 10 ms to 333 ms to 5 s
        ampEnvDecaySlider.customFormatText = [](double v) {
            float sec = (v <= 0.5) ? (0.010f * std::pow(0.333f / 0.010f, static_cast<float>(v) * 2.0f))
                                   : (0.333f * std::pow(5.0f / 0.333f, (static_cast<float>(v) - 0.5f) * 2.0f));
            return formatMsOrS(sec);
        };
        ampEnvDecaySlider.customParseText = [](const juce::String& s) {
            return parseMsOrS(s, 0.010f, 0.333f, 5.0f);
        };
    } else { // Slow time: 100 ms to 5 s to 60 s
        ampEnvDecaySlider.customFormatText = [](double v) {
            float sec = (v <= 0.5) ? (0.100f * std::pow(5.0f / 0.100f, static_cast<float>(v) * 2.0f))
                                   : (5.0f * std::pow(60.0f / 5.0f, (static_cast<float>(v) - 0.5f) * 2.0f));
            return formatMsOrS(sec);
        };
        ampEnvDecaySlider.customParseText = [](const juce::String& s) {
            return parseMsOrS(s, 0.100f, 5.0f, 60.0f);
        };
    }

    // Force repaint of text boxes with newly updated formatters
    repaint();
}

void BiaEr1AudioProcessorEditor::paint(juce::Graphics& g) {
    // Window background
    g.fillAll(juce::Colour(0xff0f1116));

    // Top Header Banner
    auto headerBounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(getWidth()), 44.0f);
    juce::ColourGradient grad(juce::Colour(0xff181a24), 0.0f, 0.0f,
                              juce::Colour(0xff11131a), 0.0f, 44.0f, false);
    g.setGradientFill(grad);
    g.fillRect(headerBounds);

    g.setColour(juce::Colour(0xff252936));
    g.drawHorizontalLine(44, 0.0f, static_cast<float>(getWidth()));

    // Title & Subtitle
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("BIA ER-1 DRUM VOICE", 16, 6, 260, 20, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff758296));
    g.setFont(juce::FontOptions(10.5f));
    g.drawText("10-Block Modular Drum Synthesizer | CTAG TBD Native Engine", 16, 24, 400, 16, juce::Justification::centredLeft);
}

void BiaEr1AudioProcessorEditor::resized() {
    auto area = getLocalBounds();

    // Top header
    auto headerArea = area.removeFromTop(44).reduced(12, 6);
    triggerButton.setBounds(headerArea.removeFromRight(120).reduced(0, 2));

    area.reduce(8, 4);

    // 10 cards in 5 columns x 2 rows
    int numCols = 5;
    int numRows = 2;
    int cardW = area.getWidth() / numCols;
    int cardH = area.getHeight() / numRows;

    for (int i = 0; i < 10; ++i) {
        int col = i % numCols;
        int row = i / numCols;
        if (i < static_cast<int>(cards.size())) {
            cards[i]->setBounds(juce::Rectangle<int>(
                area.getX() + col * cardW,
                area.getY() + row * cardH,
                cardW, cardH).reduced(4));
        }
    }
}
