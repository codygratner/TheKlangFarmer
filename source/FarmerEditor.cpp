#include "FarmerEditor.h"
#include "UIComponents.h"


// --- FX SLOT CARD COMPONENT ---

FXSlotCardComponent::FXSlotCardComponent(int slot, bool isPostRack)
    : slotIndex(slot), isPost(isPostRack),
      selector1(juce::Colour(0xff00d2ff)),
      selector2(juce::Colour(0xff00d2ff))
{
    addChildComponent(&selector1);
    addChildComponent(&selector2);

    for (int i = 0; i < 4; ++i) {
        labels[i].setFont(juce::FontOptions(13.0f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        labels[i].setJustificationType(juce::Justification::centredLeft);
        labels[i].setVisible(false);

        knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knobs[i].setScrollWheelEnabled(true);
        knobs[i].setRange(0.0, 1.0, 0.0005);
        addAndMakeVisible(knobs[i]);
    }
}

void FXSlotCardComponent::mouseDown(const juce::MouseEvent& /*e*/) {
    if (onCardClicked) onCardClicked();
}

void FXSlotCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    auto compColour = accent.withRotatedHue(0.5f);
    auto panelBg = juce::Colour(0xff151821).interpolatedWith(compColour, 0.12f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.15f);

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.setColour(accent);
    juce::String headerText = (isPost ? "POST " : "PRE ") + juce::String(slotIndex + 1) + ": " + title.toUpperCase();
    g.drawText(headerText, 10, 4, getWidth() - 20, 18, juce::Justification::left, true);
}

void FXSlotCardComponent::resized() {
    for (int i = 0; i < 4; ++i) {
        labels[i].setVisible(false);
    }

    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24); // Title header
    area.removeFromTop(6);

    std::vector<int> visibleKnobs;
    for (int i = 0; i < 4; ++i) {
        if (knobs[i].isVisible()) visibleKnobs.push_back(i);
    }

    if (selector1.isVisible() && selector2.isVisible()) {
        int selH1 = 28;
        int selH2 = 28;
        selector1.setBounds(area.removeFromTop(selH1));
        area.removeFromTop(5);
        selector2.setBounds(area.removeFromTop(selH2));
        area.removeFromTop(8);
    } else if (selector1.isVisible()) {
        int selH = 30;
        selector1.setBounds(area.removeFromTop(selH));
        area.removeFromTop(8);
    }

    int count = static_cast<int>(visibleKnobs.size());
    if (count > 0) {
        int rowH = area.getHeight() / count;
        int sliderH = (count == 4) ? 36 : (count == 3 ? 38 : 40);

        for (int kIdx : visibleKnobs) {
            auto row = area.removeFromTop(rowH);
            knobs[kIdx].setBounds(row.withSizeKeepingCentre(row.getWidth(), std::min(row.getHeight() - 4, sliderH)));
        }
    }
}

void FXSlotCardComponent::configureForType(int fxType) {
    currentType = fxType;
    lastSel1 = -1;
    lastSel2 = -1;

    for (int i = 0; i < 4; ++i) {
        knobs[i].diagramType = RotaryKnobSlider::DiagramType::None;
        knobs[i].customFormatText = nullptr;
        knobs[i].customParseText = nullptr;
        knobs[i].setVisible(false);
        labels[i].setVisible(false);
    }
    selector1.setVisible(false);
    selector2.setVisible(false);

    auto setupK = [this, fxType](int kIdx, const juce::String& name, bool bipolar,
                         std::function<juce::String(double)> fmt,
                         std::function<double(const juce::String&)> prs) {
        knobs[kIdx].setVisible(true);
        labels[kIdx].setText(name, juce::dontSendNotification);
        labels[kIdx].setVisible(false);
        knobs[kIdx].setLabel(name);
        knobs[kIdx].setAccentColour(accent);
        knobs[kIdx].setBipolar(bipolar);
        knobs[kIdx].setColour(juce::Slider::rotarySliderFillColourId, accent);
        knobs[kIdx].setColour(juce::Slider::trackColourId, accent);
        knobs[kIdx].customFormatText = fmt;
        knobs[kIdx].customParseText = prs;
        if (bipolar) {
            knobs[kIdx].setDoubleClickReturnValue(true, 0.5);
            knobs[kIdx].getDefaultValue = []() { return 0.5; };
        } else {
            knobs[kIdx].setDoubleClickReturnValue(true, 0.0);
            knobs[kIdx].getDefaultValue = []() { return 0.0; };
        }
        knobs[kIdx].setTooltip(TooltipHelper::getFxKnobTooltip(fxType, kIdx));
        knobs[kIdx].repaint();
        knobs[kIdx].updateText();
    };

    switch (fxType) {
        case 1: { // Bell EQ
            title = "Bell EQ";
            accent = juce::Colour(0xff29b6f6);
            setupK(0, "Freq", false, formatEqFreqHz, parseEqFreqHz);
            setupK(1, "Width", false, formatEqWidthOct, parseEqWidthOct);
            setupK(2, "Gain", true, formatBipolarDb, parseBipolarDb);
            setupK(3, "DJ Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 2: { // Chorus
            title = "Chorus";
            accent = juce::Colour(0xff38bdf8);
            setupK(0, "Rate", false, formatChorusRate, parseChorusRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 3: { // Comb Filter
            title = "Comb Filter";
            accent = juce::Colour(0xff26a69a);
            setupK(0, "Dampening", false, formatFreqHz, parseFreqHz);
            setupK(1, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(2, "Resonance", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", true, formatWetDry, parseWetDry);
            knobs[3].setDoubleClickReturnValue(true, 0.5); // 0%:100% (Dry)
            knobs[3].getDefaultValue = []() { return 0.5; };
            break;
        }
        case 4: { // Drive
            title = "Drive";
            accent = juce::Colour(0xffff4081);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "Off", "On" }, 2);
            selector1.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("toggle"));
            selector1.setTooltip("DRIVE LIMITER: Output brickwall safety limiter toggle.");
            selector1.onChange = [this](int idx) {
                knobs[3].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(0, "Drive", false, formatDb, parseDb);
            knobs[0].setDoubleClickReturnValue(true, 0.2); // 0 dB
            knobs[0].getDefaultValue = []() { return 0.2; }; // 0 dB
            setupK(1, "Bias", true, formatBipolarPercent, parseBipolarPercent);
            setupK(2, "Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 5: { // Filter
            title = "FX Filter";
            accent = juce::Colour(0xff7c4dff);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "LPF", "BPF", "HPF", "BRF" }, 4);
            selector1.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("filter_type"));
            selector1.setTooltip("FILTER TYPE: Low-Pass, Band-Pass, High-Pass, or Notch.");
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx / 3.0, juce::sendNotification);
            };
            selector2.setVisible(true);
            selector2.setAccent(accent);
            selector2.setItems({ "6", "12", "18", "24", "36" }, 5);
            selector2.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("filter_slope_5"));
            selector2.setTooltip("FILTER SLOPE: Attenuation slope from 6 to 36 dB/oct.");
            selector2.onChange = [this](int idx) {
                knobs[1].setValue(idx * 0.25, juce::sendNotification);
            };
            setupK(2, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(3, "Resonance", false, formatPercent, parsePercent);
            break;
        }
        case 6: { // Flanger
            title = "Flanger";
            accent = juce::Colour(0xffec4899);
            setupK(0, "Rate", false, formatFlangerRate, parseFlangerRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 7: { // Frequency Shifter
            title = "Freq Shifter";
            accent = juce::Colour(0xff00e676);
            auto fmtShift = [this](double val) {
                float maxRange = TbdAudio::normToRangeHz(static_cast<float>(knobs[1].getValue()));
                float hz = static_cast<float>((val - 0.5) * 2.0 * maxRange);
                return (hz > 0 ? "+" : "") + juce::String(hz, 1) + " Hz";
            };
            auto prsShift = [this](const juce::String& text) {
                float maxRange = TbdAudio::normToRangeHz(static_cast<float>(knobs[1].getValue()));
                double hz = parseNumberSafe(text, 0.0);
                return std::clamp((hz / (2.0 * maxRange)) + 0.5, 0.0, 1.0);
            };
            setupK(0, "Shift", true, fmtShift, prsShift);
            setupK(1, "Range", false, formatRangeHz, parseRangeHz);
            setupK(2, "Blend", true, formatWetDry, parseWetDry);
            knobs[2].setDoubleClickReturnValue(true, 0.5); // 0%:100% (Dry)
            knobs[2].getDefaultValue = []() { return 0.5; };
            setupK(3, "Width", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 8: { // Grit FX
            title = "Grit FX";
            accent = juce::Colour(0xffff9100);
            setupK(0, "Bit Rate", false, formatBits, parseBits);
            setupK(1, "Sample Rate", false, formatFreqHz, parseFreqHz);
            setupK(2, "Low", true, formatBipolarDb, parseBipolarDb);
            setupK(3, "High", true, formatBipolarDb, parseBipolarDb);
            break;
        }
        case 9: { // Phase Smear
            title = "Phase Smear";
            accent = juce::Colour(0xffec407a);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "2nd", "4th" }, 2);
            selector1.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("phase_smear"));
            selector1.setTooltip("ALL-PASS ORDER: 2nd-order or 4th-order dispersion networks.");
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(1, "Amount", false, formatStages, parseStages);
            setupK(2, "Cutoff", false, formatFreqHz, parseFreqHz);
            setupK(3, "Resonance", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 10: { // Phaser
            title = "Phaser";
            accent = juce::Colour(0xffa855f7);
            setupK(0, "Rate", false, formatPhaserRate, parsePhaserRate);
            setupK(1, "Depth", false, formatPercent, parsePercent);
            setupK(2, "Feedback", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 11: { // RingMod
            title = "RingMod FX";
            accent = juce::Colour(0xffff7043);
            setupK(0, "Waveform", false, formatPercent, parsePercent);
            knobs[0].diagramType = RotaryKnobSlider::DiagramType::Waveform;
            setupK(1, "Rate", false, formatFreqHz, parseFreqHz);
            setupK(2, "Amount", false, formatPercent, parsePercent);
            setupK(3, "Width", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        case 12: { // Tempo Delay
            title = "Tempo Delay";
            accent = juce::Colour(0xff10b981);
            setupK(0, "Division", false, formatDelayDiv, parseDelayDiv);
            setupK(1, "Feedback", false, formatPercent, parsePercent);
            setupK(2, "Tone", false, formatDelayTone, parseDelayTone);
            setupK(3, "Mix", false, formatPercent, parsePercent);
            break;
        }
        case 13: { // Wave Folder
            title = "Wave Folder";
            accent = juce::Colour(0xffff5252);
            selector1.setVisible(true);
            selector1.setAccent(accent);
            selector1.setItems({ "Off", "On" }, 2);
            selector1.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("toggle"));
            selector1.setTooltip("WAVEFOLDER LIMITER: Output brickwall safety limiter toggle.");
            selector1.onChange = [this](int idx) {
                knobs[0].setValue(idx == 0 ? 0.0 : 1.0, juce::sendNotification);
            };
            setupK(1, "Fold", false, formatWavefolds, parseWavefolds);
            setupK(2, "Bias", true, formatBipolarPercent, parseBipolarPercent);
            setupK(3, "Filter", true, formatBipolarPercent, parseBipolarPercent);
            break;
        }
        default: { // None
            title = "Empty Slot";
            accent = juce::Colour(0xff75849b);
            break;
        }
    }

    setTooltip(TooltipHelper::getFxAlgorithmTooltip(fxType));
    resized();
    repaint();
}

void FXSlotCardComponent::updateDynamicControls() {
    if (currentType == 4) { // Drive: knob 3 is Limiter Off/On
        int sel = (knobs[3].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
        }
    } else if (currentType == 5) { // Filter: knob 0 is Type, knob 1 is Slope
        int sel1 = std::clamp(static_cast<int>(std::round(knobs[0].getValue() * 3.0)), 0, 3);
        if (sel1 != lastSel1) {
            selector1.setSelectedIndex(sel1, juce::dontSendNotification);
            lastSel1 = sel1;
        }
        int sel2 = std::clamp(static_cast<int>(std::round(knobs[1].getValue() * 4.0)), 0, 4);
        if (sel2 != lastSel2) {
            selector2.setSelectedIndex(sel2, juce::dontSendNotification);
            lastSel2 = sel2;
        }
    } else if (currentType == 9) { // Phase Smear: knob 0 is Order (0: 2nd, 1: 4th)
        int sel = (knobs[0].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
        }
    } else if (currentType == 13) { // WaveFolder: knob 0 is Off/On
        int sel = (knobs[0].getValue() >= 0.5) ? 1 : 0;
        if (sel != lastSel1) {
            selector1.setSelectedIndex(sel, juce::dontSendNotification);
            lastSel1 = sel;
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
    const juce::String pageTips[7] = {
        "PAGE 1: VOICE 1 — Carrier 1, Modulator 1, Pitch Envelope 1, Filter 1, and Filter Envelope 1.",
        "PAGE 2: VOICE 2 — Carrier 2, Modulator 2, Pitch Envelope 2, Filter 2, and Filter Envelope 2.",
        "PAGE 3: TRANSIENTS — Sample-and-Hold Noise Generator, Filter 3, and Transient Envelopes.",
        "PAGE 4: PRE-AMP FX — Modular FX Slots 1–4 inserted before the Master Amplifier.",
        "PAGE 5: AMPLIFIER — Master Saturation Drive, Stereo Pan, Master Level, Voice Mixer, and Clap Generator.",
        "PAGE 6: POST-AMP FX — Modular FX Slots 1–4 inserted after the Master Amplifier.",
        "PAGE 7: MODULATIONS — Modulation Matrix, Analog Slop / Drift, and Multi-Wave LFO."
    };
    for (int i = 0; i < pageNames.size(); ++i) {
        auto btn = std::make_unique<juce::TextButton>(pageNames[i]);
        btn->setClickingTogglesState(false);
        if (i < 7) btn->setTooltip(pageTips[i]);
        int pageIdx = i;
        btn->onClick = [this, pageIdx]() {
            setSelectedPage(pageIdx);
            if (onPageSelected) onPageSelected(pageIdx);
        };
        addAndMakeVisible(btn.get());
        buttons.push_back(std::move(btn));
    }
    setTooltip("NAVIGATION: Select active rack page (Pages 1 to 7) to access modules.");
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

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
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
        labels[i].setFont(juce::FontOptions(13.5f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc5d0e0));
        addAndMakeVisible(labels[i]);

        boxes[i].setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff14161d));
        boxes[i].setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2c3240));
        boxes[i].setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
        boxes[i].setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff8b95a8));
        boxes[i].setTooltip("FX ALGORITHM SELECTOR: Choose an effect module (0 to 13) for Slot " + juce::String(i + 1) + ".");
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

    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
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

juce::Rectangle<int> VisualizationCardComponent::getLockBounds() const {
    return juce::Rectangle<int>(getWidth() - 34, 4, 26, 20);
}

juce::Rectangle<int> VisualizationCardComponent::getOffBounds() const {
    return juce::Rectangle<int>(getWidth() - 70, 4, 32, 20);
}

void VisualizationCardComponent::setIsOff(bool off) {
    if (isOff != off) {
        isOff = off;
        if (isOff) {
            oscilloscope.setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
            static const float zeros[128] = { 0.0f };
            oscilloscope.updateData(zeros, 128);
        }
        repaint();
    }
}

void VisualizationCardComponent::setVisualizedBlock(int blockIndex, const juce::String& blockName) {
    if (isLocked) return;
    currentBlockIndex = blockIndex;
    currentBlockName = blockName.toUpperCase();
    repaint();
}

juce::String VisualizationCardComponent::getTooltip() {
    if (isOffHovered) {
        return isOff ? "DISPLAY OFF: Visualizer rendering suspended to conserve CPU. Click to resume."
                     : "DISPLAY ON: Visualizer actively rendering real-time waveform / Bode plot. Click to suspend.";
    }
    if (isLockHovered) {
        return isLocked ? "VISUALIZER LOCKED: Pinned to current module. Click to unlock auto-tracking."
                        : "VISUALIZER UNLOCKED: Follows currently selected module. Click to pin.";
    }
    return "VISUALIZER: High-speed real-time waveform oscilloscope and XY filter frequency response plot.";
}

void VisualizationCardComponent::mouseDown(const juce::MouseEvent& e) {
    if (getOffBounds().contains(e.getPosition())) {
        setIsOff(!isOff);
        return;
    }
    if (getLockBounds().contains(e.getPosition())) {
        isLocked = !isLocked;
        repaint();
        return;
    }
}

void VisualizationCardComponent::mouseMove(const juce::MouseEvent& e) {
    bool lockHov = getLockBounds().contains(e.getPosition());
    bool offHov  = getOffBounds().contains(e.getPosition());
    bool needRepaint = false;
    if (lockHov != isLockHovered) {
        isLockHovered = lockHov;
        needRepaint = true;
    }
    if (offHov != isOffHovered) {
        isOffHovered = offHov;
        needRepaint = true;
    }
    if (needRepaint) {
        setMouseCursor((lockHov || offHov) ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void VisualizationCardComponent::mouseExit(const juce::MouseEvent& /*e*/) {
    if (isLockHovered || isOffHovered) {
        isLockHovered = false;
        isOffHovered = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void VisualizationCardComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto compColour = accent.withRotatedHue(0.5f);
    auto panelBg = juce::Colour(0xff151821).interpolatedWith(compColour, 0.12f);
    auto panelBorder = juce::Colour(0xff222736).interpolatedWith(compColour, 0.15f);

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(panelBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    auto headerStrip = bounds.removeFromTop(3.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(headerStrip, 2.0f);

    // Module name on top left (bounded before OFF button)
    g.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    g.setColour(accent);
    juce::String headerText = "VISUALIZER: " + currentBlockName;
    g.drawText(headerText, 10, 4, getWidth() - 78, 20, juce::Justification::centredLeft, true);

    // OFF button: dim greyed out when visualizer is running, lit up glowing red when OFF
    auto offR = getOffBounds().toFloat();
    juce::Colour offBg = isOff ? juce::Colour(0x35ff2d55) : juce::Colour(0x14ffffff);
    juce::Colour offBorder = isOff ? juce::Colour(0xeeff2d55) : juce::Colour(0x338892a4);
    juce::Colour offText = isOff ? juce::Colour(0xffff3b5c) : juce::Colour(0xff687488);

    if (isOffHovered) {
        offBg = offBg.brighter(0.25f);
        offBorder = offBorder.brighter(0.25f);
        offText = offText.brighter(0.25f);
    }

    g.setColour(offBg);
    g.fillRoundedRectangle(offR, 3.0f);
    g.setColour(offBorder);
    g.drawRoundedRectangle(offR, 3.0f, isOff ? 1.4f : 1.0f);

    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(offText);
    g.drawText("OFF", offR, juce::Justification::centred, false);

    // Lock icon on top right: grey (0xff8892a4) when unlocked, bright yellow (0xffffd600) when locked
    auto lockR = getLockBounds().toFloat();
    juce::Colour lockColour = isLocked ? juce::Colour(0xffffd600) : juce::Colour(0xff8892a4);
    if (isLockHovered) lockColour = lockColour.brighter(0.25f);

    float cx = lockR.getCentreX();
    float cy = lockR.getCentreY();

    // Padlock body
    float bw = 12.0f;
    float bh = 9.0f;
    float bx = cx - bw * 0.5f;
    float by = cy - bh * 0.5f + 3.0f;
    juce::Rectangle<float> bodyRect(bx, by, bw, bh);
    g.setColour(lockColour);
    g.fillRoundedRectangle(bodyRect, 2.0f);

    // Padlock shackle
    juce::Path shackle;
    float sw = 8.0f;
    float sx = cx - sw * 0.5f;
    float archTop = by - 6.0f;

    if (isLocked) {
        shackle.startNewSubPath(sx + 1.0f, by);
        shackle.lineTo(sx + 1.0f, archTop + 3.0f);
        shackle.addCentredArc(cx, archTop + 3.0f, sw * 0.5f - 1.0f, sw * 0.5f - 1.0f, 0.0f, -juce::MathConstants<float>::pi, 0.0f, false);
        shackle.lineTo(sx + sw - 1.0f, by);
    } else {
        float openArchTop = archTop - 2.0f;
        shackle.startNewSubPath(sx + 1.0f, by);
        shackle.lineTo(sx + 1.0f, openArchTop + 3.0f);
        shackle.addCentredArc(cx, openArchTop + 3.0f, sw * 0.5f - 1.0f, sw * 0.5f - 1.0f, 0.0f, -juce::MathConstants<float>::pi, 0.0f, false);
    }
    g.strokePath(shackle, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Padlock keyhole
    g.setColour(juce::Colour(0xff151821));
    g.fillEllipse(cx - 1.2f, by + 2.5f, 2.4f, 2.4f);
    g.fillRect(cx - 0.7f, by + 4.0f, 1.4f, 2.5f);
}

void VisualizationCardComponent::resized() {
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(24);
    oscilloscope.setBounds(area);
}

// --- QUICKSTART GUIDE MODAL COMPONENT ---

QuickstartGuideModalComponent::QuickstartGuideModalComponent() {
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff232938));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));
    closeButton.onClick = [this]() { setVisible(false); };
    addAndMakeVisible(closeButton);
    setWantsKeyboardFocus(true);
}

juce::Rectangle<int> QuickstartGuideModalComponent::getCardBounds() const {
    int cardW = std::min(920, getWidth() - 40);
    int cardH = std::min(640, getHeight() - 40);
    int cardX = (getWidth() - cardW) / 2;
    int cardY = (getHeight() - cardH) / 2;
    return juce::Rectangle<int>(cardX, cardY, cardW, cardH);
}

void QuickstartGuideModalComponent::resized() {
    auto card = getCardBounds();
    closeButton.setBounds(card.getRight() - 36, card.getY() + 10, 26, 26);
}

void QuickstartGuideModalComponent::mouseDown(const juce::MouseEvent& e) {
    if (!getCardBounds().contains(e.getPosition())) {
        setVisible(false);
    }
}

bool QuickstartGuideModalComponent::keyPressed(const juce::KeyPress& key) {
    if (key.isKeyCode(juce::KeyPress::escapeKey)) {
        setVisible(false);
        return true;
    }
    return false;
}

void QuickstartGuideModalComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xd00a0d14));

    auto card = getCardBounds().toFloat();

    g.setColour(juce::Colour(0xff12151c));
    g.fillRoundedRectangle(card, 8.0f);
    g.setColour(juce::Colour(0xff2a3245));
    g.drawRoundedRectangle(card.reduced(0.5f), 8.0f, 1.5f);

    auto topStrip = card.removeFromTop(4.0f);
    g.setColour(juce::Colour(0xff00d2ff));
    g.fillRoundedRectangle(topStrip, 2.0f);

    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER — QUICKSTART GUIDE", static_cast<int>(card.getX()) + 20, static_cast<int>(card.getY()) + 10, 500, 24, juce::Justification::left, true);

    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8892a4));
    g.drawText("Paged Modular Dual FM Drum Voice with 13 Multi-Instance Effects", static_cast<int>(card.getX()) + 20, static_cast<int>(card.getY()) + 34, 600, 18, juce::Justification::left, true);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(static_cast<int>(card.getY()) + 56, card.getX() + 16.0f, card.getRight() - 16.0f);

    auto contentArea = card;
    contentArea.removeFromTop(62.0f);
    contentArea.reduce(14.0f, 12.0f);

    float gap = 12.0f;
    float colW = (contentArea.getWidth() - gap) * 0.5f;
    float rowH = (contentArea.getHeight() - gap) * 0.5f;

    auto drawPanel = [&g](const juce::Rectangle<float>& r, const juce::String& title, juce::Colour accentCol, const juce::StringArray& bullets) {
        g.setColour(juce::Colour(0xff161a24));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colour(0xff242c3d));
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 1.0f);

        auto panelHeader = r;
        auto titleArea = panelHeader.removeFromTop(28.0f);

        g.setColour(accentCol);
        g.fillRoundedRectangle(titleArea.getX() + 10.0f, titleArea.getY() + 8.0f, 4.0f, 12.0f, 2.0f);

        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText(title, static_cast<int>(titleArea.getX()) + 20, static_cast<int>(titleArea.getY()) + 4, static_cast<int>(titleArea.getWidth()) - 30, 20, juce::Justification::left, true);

        float y = r.getY() + 32.0f;
        float x = r.getX() + 12.0f;
        float w = r.getWidth() - 24.0f;

        for (const auto& bullet : bullets) {
            g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
            g.setColour(juce::Colour(0xffc5d1e8));
            g.drawFittedText(bullet, static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), 32, juce::Justification::topLeft, 2);
            y += 35.0f;
        }
    };

    // Panel 1: Architecture & Signal Flow
    juce::Rectangle<float> p1(contentArea.getX(), contentArea.getY(), colW, rowH);
    juce::StringArray b1 = {
        "• DUAL FM VOICES: Two parallel voices each with Carrier (MIDI/Freq/Note), FM Modulator (Fixed/Follow/FM), dedicated Pitch Env, and Multimode Filter (LPF/BPF/HPF/BRF with 6-36dB slopes).",
        "• TRANSIENT NOISE: Analog-modeled White/Pink/Metallic noise source with dedicated Filter 3 and Filter Env for snappy clicks, snaps, and snare rattle.",
        "• 4-CHANNEL MIXER: Balance Carrier 1, Carrier 2, RingMod, and Noise Transients into the processing chain.",
        "• SIGNAL FLOW: Mixer -> Pre-Amp FX Rack (4 Slots) -> Pre-Limiter -> Amplifier + Amp Env -> Post-Amp FX Rack (4 Slots) -> Master Limiter -> Audio Output."
    };
    drawPanel(p1, "1. ARCHITECTURE & SIGNAL FLOW", juce::Colour(0xff00d2ff), b1);

    // Panel 2: Navigation & Smart Visualizer
    juce::Rectangle<float> p2(contentArea.getX() + colW + gap, contentArea.getY(), colW, rowH);
    juce::StringArray b2 = {
        "• 7-PAGE NAVIGATION (Slot 1): Instant 1-click access to Voice 1, Voice 2, Transients, Pre-Amp FX, Amplifier, Post-Amp FX, and Modulations.",
        "• AUTO-TRACKING VISUALIZER (Slot 5): Automatically switches to display real-time analysis for whichever module card or knob you click or edit.",
        "• BODE & OSCILLOSCOPE: Filters and EQ show interactive X-Y frequency response curves; Oscillators and FX display real-time triggered waveforms.",
        "• PADLOCK ICON (Top Right): Grey = auto-tracking active. Yellow = LOCKED! Lock visualizer to an FX (e.g. Wavefolder), then switch pages to sculpt sound while watching the locked waveform!"
    };
    drawPanel(p2, "2. NAVIGATION & AUTO-VISUALIZER", juce::Colour(0xffffd600), b2);

    // Panel 3: 13-Effects Engine & Multi-Instance
    juce::Rectangle<float> p3(contentArea.getX(), contentArea.getY() + rowH + gap, colW, rowH);
    juce::StringArray b3 = {
        "• 8 FX SLOTS: 4 Pre-Amp slots (pre-saturation) and 4 Post-Amp slots (post-saturation / spatial).",
        "• MULTI-INSTANCE: Assign ANY of the 13 effects to ANY slot. Stack up to 8 of the same effect in series if desired (e.g. multiple Wavefolders or cascading Filters)!",
        "• 13 DSP PROCESSORS: Bell EQ, Chorus, Comb Filter, Drive, Filter, Flanger, Frequency Shifter, Grit FX, Phase Smear, Phaser, RingMod, Tempo Delay, Wave Folder.",
        "• HARDWARE CONTROL: Standardized 4-knob tactile interface with illuminated LED button switches for quick, intuitive sound design."
    };
    drawPanel(p3, "3. MULTI-INSTANCE FX (13 EFFECTS)", juce::Colour(0xffff7043), b3);

    // Panel 4: Sound Design Recipes & Tips
    juce::Rectangle<float> p4(contentArea.getX() + colW + gap, contentArea.getY() + rowH + gap, colW, rowH);
    juce::StringArray b4 = {
        "• PUNCHY KICK: Voice 1 Carrier in MIDI mode (pitch ~36), Sine shape; Pitch Env fast decay (25ms), Depth +36st; Pre-Amp Wavefolder (Fold 2-4) + Drive (30%); Post-Limiter ON.",
        "• METALLIC SNARE: Voice 1 snappy body; Transients metallic noise with Filter 3 set to BPF (2kHz); Post-Amp Comb Filter or Chorus for stereo width.",
        "• MODULATIONS: 3 freely assignable Mod Envelopes on top row targetable to any parameter; Map Velocity, Key Tracking, and analog Slop on bottom row.",
        "• AUDITION HIT: Click the AUDITION HIT button in the top-right header at any time to audition the sound at full velocity."
    };
    drawPanel(p4, "4. SOUND DESIGN RECIPES & TIPS", juce::Colour(0xff00e5ff), b4);
}


// --- THE KLANG FARMER AUDIO PROCESSOR EDITOR CONSTRUCTOR ---

TheKlangFarmerAudioProcessorEditor::TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor& p)
    : KlangCoreEditor(p), audioProcessor(p),
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
      ampLimiterSelector(juce::Colour(0xffe53935)),
      preLimiterEnableSelector(juce::Colour(0xffe53935)),
      postLimiterEnableSelector(juce::Colour(0xffe53935)),
      vizCard(juce::Colour(0xff00d2ff))
{
    setLookAndFeel(&knobLookAndFeel);

    // Setup Header Tooltips Toggle Button
    tooltipsButton.setTooltip("TOOLTIPS — Toggle hover parameter and control tooltips on or off.");
    tooltipsButton.onClick = [this]() {
        setTooltipsEnabled(!tooltipsEnabled);
    };
    addAndMakeVisible(tooltipsButton);
    setTooltipsEnabled(false);

    // Setup Header Quickstart Guide Button
    guideButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1f2430));
    guideButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00d2ff));
    guideButton.setTooltip("QUICKSTART GUIDE — Display synthesized drum architecture overview and recipes.");
    guideButton.onClick = [this]() {
        quickstartGuide.setVisible(true);
        quickstartGuide.toFront(true);
        quickstartGuide.grabKeyboardFocus();
    };
    initButton.onClick = [this] { resetToDefaults(); };
    triggerButton.onClick = [this] { audioProcessor.getEngine().trigger(); };
    addAndMakeVisible(guideButton);

    addChildComponent(quickstartGuide);

    // Setup Header Initialize Button
    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222736));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.setTooltip("INITIALIZE — Reset parameters to factory default or clean FX state.");
    initButton.onClick = [this]() {
        auto* alert = new juce::AlertWindow("Initialize Preset",
                                           "Select initialization preset mode:\n\nDefault: Restores factory synthesis and default FX rack.\nClean: Restores factory synthesis with empty FX slots.",
                                           juce::AlertWindow::QuestionIcon, this);
        alert->addButton("Default", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("Clean", 2);
        alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        alert->setColour(juce::AlertWindow::backgroundColourId, juce::Colour(0xff151821));
        alert->setColour(juce::AlertWindow::textColourId, juce::Colour(0xffffffff));
        alert->setColour(juce::AlertWindow::outlineColourId, juce::Colour(0xff00d2ff));
        alert->enterModalState(true, juce::ModalCallbackFunction::create([this](int result) {
            if (result == 1) {
                resetToDefaults(false);
            } else if (result == 2) {
                resetToDefaults(true);
            }
        }), true);
    };
    addAndMakeVisible(initButton);

    // Setup Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.setTooltip("AUDITION HIT — Fire a manual audition drum hit at full velocity.");
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    // Permanent Slot 1 and Slot 8
    navCard.setTooltip("NAVIGATION — Select rack page 1 through 7 to view and edit synthesizer modules.");
    navCard.onPageSelected = [this](int pageIndex) {
        setPage(pageIndex);
    };
    addAndMakeVisible(navCard);

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
    cardCarrier1->setTooltip("CARRIER 1: Primary tonal FM oscillator with morphable sine/tri/saw/pulse waveforms.");
    setupBox(carrier1TrackingBox);
    bindSelector(carrier1TrackingSelector, carrier1TrackingBox, "carrier1_tracking", { "MIDI", "Freq", "Note" }, 3);
    cardCarrier1->setLedSelector(&carrier1TrackingSelector);

    setupKnob(carrier1PitchSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrier1PitchSlider.customFormatText = [this](double val) {
        int mode = carrier1TrackingBox.getSelectedItemIndex();
        if (mode == 0) return formatSemi24(val);
        if (mode == 1) return formatCarrierFreqHz(val);
        return formatNoteDetail(val);
    };
    carrier1PitchSlider.customParseText = [this](const juce::String& text) {
        int mode = carrier1TrackingBox.getSelectedItemIndex();
        if (mode == 0) return parseSemi24(text);
        if (mode == 1) return parseCarrierFreqHz(text);
        return parseNoteDetail(text);
    };
    carrier1TrackingBox.onChange = [this]() {
        updateCarrier1Controls();
    };

    setupKnob(carrier1ShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier1DepthSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrier1DepthSlider.customFormatText = formatBipolarPercent;
    carrier1DepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier1->setKnob(0, "Offset", &carrier1PitchSlider);
    cardCarrier1->setKnob(1, "Shape", &carrier1ShapeSlider);
    cardCarrier1->setKnob(2, "Mod Depth", &carrier1DepthSlider);
    addChildComponent(cardCarrier1.get());
    updateCarrier1Controls();

    // 2. Modulator 1
    cardMod1 = std::make_unique<ModuleCardComponent>("Modulator 1", juce::Colour(0xffff7043));
    cardMod1->setTooltip("MODULATOR 1: Frequency modulation oscillator in Fixed Hz, Pitch Follow, or Harmonic Ratio modes.");
    setupBox(mod1TrackBox);
    bindSelector(mod1TrackSelector, mod1TrackBox, "mod1_track", { "Fixed", "Follow", "FM" }, 3);
    setupBox(mod1TypeBox);
    bindSelector(mod1TypeSelector, mod1TypeBox, "mod1_type", { "Osc", "Cyclic", "Noise" }, 3);
    mod1TypeBox.onChange = [this]() {
        int t = mod1TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) {
            if (mod1ShapeSlider.getValue() == 0.0) {
                mod1ShapeSlider.setValue(0.5, juce::sendNotification);
            }
            if (t == 2 && mod1SpeedSlider.getValue() < 0.9) {
                mod1SpeedSlider.setValue(1.0, juce::sendNotification);
            }
        }
        updateDynamicControls();
    };
    cardMod1->setLedSelector(&mod1TrackSelector);
    cardMod1->setSecondLedSelector(&mod1TypeSelector);

    setupKnob(mod1ShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod1ShapeSlider.customFormatText = [this](double val) {
        int t = mod1TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return formatBipolarPercent(val);
        return formatPercent(val);
    };
    mod1ShapeSlider.customParseText = [this](const juce::String& text) {
        int t = mod1TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return parseBipolarPercent(text);
        return parsePercent(text);
    };

    setupKnob(mod1SpeedSlider, juce::Colour(0xffff7043), false, 0.50934);
    mod1SpeedSlider.customFormatText = [this](double val) {
        if (mod1TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        if (mod1TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod1SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod1TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        if (mod1TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod1TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod1->setKnob(0, "Shape", &mod1ShapeSlider);
    cardMod1->setKnob(1, "Speed", &mod1SpeedSlider);
    addChildComponent(cardMod1.get());

    // 3. Pitch Envelope 1
    cardPitchEnv1 = std::make_unique<ModuleCardComponent>("Pitch Env 1", juce::Colour(0xffffab00));
    cardPitchEnv1->setTooltip("PITCH ENV 1: High-speed exponential pitch envelope routable to Carrier, Modulator, or Both.");
    setupBox(pitchEnv1TargetBox);
    bindSelector(pitchEnv1TargetSelector, pitchEnv1TargetBox, "pitchenv1_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardPitchEnv1->setSelectorAtBottom(true);
    cardPitchEnv1->setLedSelector(&pitchEnv1TargetSelector);

    setupKnob(pitchEnv1SlopeSlider, juce::Colour(0xffffab00), false, 0.5886);
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
    cardFilter1->setTooltip("FILTER 1: Voice 1 multi-mode resonant filter with selectable 6-36 dB/oct slope.");
    setupBox(filter1TypeBox);
    bindSelector(filter1TypeSelector, filter1TypeBox, "filter1_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter1SlopeBox);
    bindSelector(filter1SlopeSelector, filter1SlopeBox, "filter1_slope", { "6", "12", "18", "24", "36" }, 5);
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
    cardFilterEnv1->setTooltip("FILTER ENV 1: Voice 1 filter cutoff modulation envelope and pre-filter overdrive.");
    setupKnob(filterEnv1SlopeSlider, juce::Colour(0xff7c4dff), false, 0.5886);
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
    cardFilterEnv1->setKnob(3, "Pre-Filter Drive", &filterEnv1PostDriveSlider);
    addChildComponent(cardFilterEnv1.get());

    // Voice 2 colors (swapped accent and complementary colors from Voice 1)
    const auto carrier2Colour = juce::Colour(0xff00d2ff).withRotatedHue(0.5f);
    const auto mod2Colour     = juce::Colour(0xffff7043).withRotatedHue(0.5f);
    const auto pitchEnv2Colour= juce::Colour(0xffffab00).withRotatedHue(0.5f);
    const auto filter2Colour  = juce::Colour(0xff7c4dff).withRotatedHue(0.5f);

    // 6. Carrier 2
    cardCarrier2 = std::make_unique<ModuleCardComponent>("Carrier 2", carrier2Colour);
    cardCarrier2->setTooltip("CARRIER 2: Secondary tonal FM oscillator for layered drum bodies and sub harmonics.");
    setupBox(carrier2TrackingBox);
    bindSelector(carrier2TrackingSelector, carrier2TrackingBox, "carrier2_tracking", { "MIDI", "Freq", "Note" }, 3);
    cardCarrier2->setLedSelector(&carrier2TrackingSelector);

    setupKnob(carrier2PitchSlider, carrier2Colour, true, 0.5);
    carrier2PitchSlider.customFormatText = [this](double val) {
        int mode = carrier2TrackingBox.getSelectedItemIndex();
        if (mode == 0) return formatSemi24(val);
        if (mode == 1) return formatCarrierFreqHz(val);
        return formatNoteDetail(val);
    };
    carrier2PitchSlider.customParseText = [this](const juce::String& text) {
        int mode = carrier2TrackingBox.getSelectedItemIndex();
        if (mode == 0) return parseSemi24(text);
        if (mode == 1) return parseCarrierFreqHz(text);
        return parseNoteDetail(text);
    };
    carrier2TrackingBox.onChange = [this]() {
        updateCarrier2Controls();
    };

    setupKnob(carrier2ShapeSlider, carrier2Colour, false, 0.0);
    carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;

    setupKnob(carrier2DepthSlider, carrier2Colour, true, 0.5);
    carrier2DepthSlider.customFormatText = formatBipolarPercent;
    carrier2DepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier2->setKnob(0, "Offset", &carrier2PitchSlider);
    cardCarrier2->setKnob(1, "Shape", &carrier2ShapeSlider);
    cardCarrier2->setKnob(2, "Mod Depth", &carrier2DepthSlider);
    addChildComponent(cardCarrier2.get());
    updateCarrier2Controls();

    // 7. Modulator 2
    cardMod2 = std::make_unique<ModuleCardComponent>("Modulator 2", mod2Colour);
    cardMod2->setTooltip("MODULATOR 2: Secondary modulation source (Oscillator, Cyclic LFO, or Noise).");
    setupBox(mod2TrackBox);
    bindSelector(mod2TrackSelector, mod2TrackBox, "mod2_track", { "Fixed", "Follow", "FM" }, 3);
    setupBox(mod2TypeBox);
    bindSelector(mod2TypeSelector, mod2TypeBox, "mod2_type", { "Osc", "Cyclic", "Noise" }, 3);
    mod2TypeBox.onChange = [this]() {
        int t = mod2TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) {
            if (mod2ShapeSlider.getValue() == 0.0) {
                mod2ShapeSlider.setValue(0.5, juce::sendNotification);
            }
            if (t == 2 && mod2SpeedSlider.getValue() < 0.9) {
                mod2SpeedSlider.setValue(1.0, juce::sendNotification);
            }
        }
        updateDynamicControls();
    };
    cardMod2->setLedSelector(&mod2TrackSelector);
    cardMod2->setSecondLedSelector(&mod2TypeSelector);

    setupKnob(mod2ShapeSlider, mod2Colour, false, 0.0);
    mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    mod2ShapeSlider.customFormatText = [this](double val) {
        int t = mod2TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return formatBipolarPercent(val);
        return formatPercent(val);
    };
    mod2ShapeSlider.customParseText = [this](const juce::String& text) {
        int t = mod2TypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return parseBipolarPercent(text);
        return parsePercent(text);
    };

    setupKnob(mod2SpeedSlider, mod2Colour, false, 0.50934);
    mod2SpeedSlider.customFormatText = [this](double val) {
        if (mod2TypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        if (mod2TrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    mod2SpeedSlider.customParseText = [this](const juce::String& text) {
        if (mod2TypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        if (mod2TrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (mod2TrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod2->setKnob(0, "Shape", &mod2ShapeSlider);
    cardMod2->setKnob(1, "Speed", &mod2SpeedSlider);
    addChildComponent(cardMod2.get());

    // 8. Pitch Envelope 2
    cardPitchEnv2 = std::make_unique<ModuleCardComponent>("Pitch Env 2", pitchEnv2Colour);
    cardPitchEnv2->setTooltip("PITCH ENV 2: Voice 2 pitch envelope for transient attack sweeps.");
    setupBox(pitchEnv2TargetBox);
    bindSelector(pitchEnv2TargetSelector, pitchEnv2TargetBox, "pitchenv2_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardPitchEnv2->setSelectorAtBottom(true);
    cardPitchEnv2->setLedSelector(&pitchEnv2TargetSelector);

    setupKnob(pitchEnv2SlopeSlider, pitchEnv2Colour, false, 0.5886);
    pitchEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnv2SlopeSlider.customFormatText = formatSlope;
    pitchEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnv2DepthSlider, pitchEnv2Colour, true, 0.5);
    pitchEnv2DepthSlider.customFormatText = formatOctaves;
    pitchEnv2DepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnv2DecaySlider, pitchEnv2Colour, false, 0.3806);
    pitchEnv2DecaySlider.customFormatText = formatTimeMs;
    pitchEnv2DecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv2->setKnob(0, "Slope", &pitchEnv2SlopeSlider);
    cardPitchEnv2->setKnob(1, "Depth", &pitchEnv2DepthSlider);
    cardPitchEnv2->setKnob(2, "Decay", &pitchEnv2DecaySlider);
    addChildComponent(cardPitchEnv2.get());

    // 9. Filter 2
    cardFilter2 = std::make_unique<ModuleCardComponent>("Filter 2", filter2Colour);
    cardFilter2->setTooltip("FILTER 2: Voice 2 multi-mode resonant filter.");
    setupBox(filter2TypeBox);
    bindSelector(filter2TypeSelector, filter2TypeBox, "filter2_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter2SlopeBox);
    bindSelector(filter2SlopeSelector, filter2SlopeBox, "filter2_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter2->setLedSelector(&filter2TypeSelector);
    cardFilter2->setSecondLedSelector(&filter2SlopeSelector);

    setupKnob(filter2CutoffSlider, filter2Colour, false, 1.0);
    filter2CutoffSlider.customFormatText = formatFreqHz;
    filter2CutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filter2ResonanceSlider, filter2Colour, false, 0.0);
    filter2ResonanceSlider.customFormatText = formatPercent;
    filter2ResonanceSlider.customParseText  = parsePercent;

    cardFilter2->setKnob(0, "Cutoff", &filter2CutoffSlider);
    cardFilter2->setKnob(1, "Resonance", &filter2ResonanceSlider);
    addChildComponent(cardFilter2.get());

    // 10. Filter Envelope 2
    cardFilterEnv2 = std::make_unique<ModuleCardComponent>("Filter Env 2", filter2Colour);
    cardFilterEnv2->setTooltip("FILTER ENV 2: Voice 2 cutoff modulation envelope and pre-filter overdrive.");
    setupKnob(filterEnv2SlopeSlider, filter2Colour, false, 0.5886);
    filterEnv2SlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnv2SlopeSlider.customFormatText = formatSlope;
    filterEnv2SlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnv2DepthSlider, filter2Colour, true, 0.5);
    filterEnv2DepthSlider.customFormatText = formatFilterOctaves;
    filterEnv2DepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnv2DecaySlider, filter2Colour, false, 0.3806);
    filterEnv2DecaySlider.customFormatText = formatTimeMs;
    filterEnv2DecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnv2PostDriveSlider, filter2Colour, false, 0.5);
    filterEnv2PostDriveSlider.customFormatText = formatDb;
    filterEnv2PostDriveSlider.customParseText  = parseDb;

    cardFilterEnv2->setKnob(0, "Slope", &filterEnv2SlopeSlider);
    cardFilterEnv2->setKnob(1, "Depth", &filterEnv2DepthSlider);
    cardFilterEnv2->setKnob(2, "Decay", &filterEnv2DecaySlider);
    cardFilterEnv2->setKnob(3, "Pre-Filter Drive", &filterEnv2PostDriveSlider);
    addChildComponent(cardFilterEnv2.get());

    // 11. Noise Transient
    cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", juce::Colour(0xff90a4ae));
    cardNoise->setTooltip("NOISE TRANSIENT: Metallic transient generator with sample-and-hold clock and center frequency filter.");
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
    cardFilter3->setTooltip("FILTER 3: Dedicated multi-mode filter processing the Noise transient burst.");
    setupBox(filter3TypeBox);
    bindSelector(filter3TypeSelector, filter3TypeBox, "filter3_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filter3SlopeBox);
    bindSelector(filter3SlopeSelector, filter3SlopeBox, "filter3_slope", { "6", "12", "18", "24", "36" }, 5);
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
    cardFilterEnv3->setTooltip("FILTER ENV 3: Envelope shaping Noise filter cutoff modulation and pre-filter drive.");
    setupKnob(filterEnv3SlopeSlider, juce::Colour(0xff7c4dff), false, 0.5886);
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
    cardFilterEnv3->setKnob(3, "Pre-Filter Drive", &filterEnv3PostDriveSlider);
    addChildComponent(cardFilterEnv3.get());

    // 14. Mixer
    cardMixer = std::make_unique<ModuleCardComponent>("Mixer", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardMixer->setTooltip("MIXER: 4-channel analog summing mixer blending Carrier 1, Carrier 2, Ring Mod, and Noise.");
    setupKnob(mixerCarrier1LevelSlider, juce::Colour(0xffe53935), false, 0.5);
    mixerCarrier1LevelSlider.setLightTrough(true);
    mixerCarrier1LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier1LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerCarrier2LevelSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerCarrier2LevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerCarrier2LevelSlider.setLightTrough(true);
    mixerCarrier2LevelSlider.customFormatText = formatMixerLevel;
    mixerCarrier2LevelSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerRingModSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerRingModSlider.setDoubleClickReturnValue(true, 0.5);
    mixerRingModSlider.setLightTrough(true);
    mixerRingModSlider.customFormatText = formatMixerLevel;
    mixerRingModSlider.customParseText  = parseMixerLevel;

    setupKnob(mixerNoiseLevelSlider, juce::Colour(0xffe53935), false, 0.0);
    mixerNoiseLevelSlider.setDoubleClickReturnValue(true, 0.5);
    mixerNoiseLevelSlider.setLightTrough(true);
    mixerNoiseLevelSlider.customFormatText = formatMixerLevel;
    mixerNoiseLevelSlider.customParseText  = parseMixerLevel;

    cardMixer->setKnob(0, "Carrier 1", &mixerCarrier1LevelSlider);
    cardMixer->setKnob(1, "Carrier 2", &mixerCarrier2LevelSlider);
    cardMixer->setKnob(2, "RingMod", &mixerRingModSlider);
    cardMixer->setKnob(3, "Noise", &mixerNoiseLevelSlider);
    addChildComponent(cardMixer.get());

    // Multi-Instance FX Slot Cards (4 Pre-Amp FX, 4 Post-Amp FX)
    for (int s = 0; s < 4; ++s) {
        preFXCards[s] = std::make_unique<FXSlotCardComponent>(s, false);
        addChildComponent(preFXCards[s].get());

        postFXCards[s] = std::make_unique<FXSlotCardComponent>(s, true);
        addChildComponent(postFXCards[s].get());
    }

    // 24. Amp
    cardAmp = std::make_unique<ModuleCardComponent>("Amplifier", juce::Colour(0xff00e5ff));
    cardAmp->setTooltip("AMPLIFIER: Master output gain stage with stereo panning and pre-limiter saturation drive.");
    setupBox(ampLimiterBox);
    bindSelector(ampLimiterSelector, ampLimiterBox, "amp_limiter", { "Off", "On" }, 2);
    ampLimiterSelector.setAccent(juce::Colour(0xffe53935));
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
    cardAmp->setKnob(2, "Pre-Limiter Drive", &ampDriveSlider);
    addChildComponent(cardAmp.get());

    // 25. Amp Envelope
    cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Envelope", juce::Colour(0xff00e5ff));
    cardAmpEnv->setTooltip("AMP ENVELOPE: Master amplitude decay envelope with integrated multi-burst clap generator.");
    setupKnob(ampEnvClapsSlider, juce::Colour(0xff00e5ff), false, 0.0);
    ampEnvClapsSlider.customFormatText = formatClaps;
    ampEnvClapsSlider.customParseText  = parseClaps;

    setupKnob(ampEnvClapSpeedSlider, juce::Colour(0xff00e5ff), false, 2.0 / 14.0);
    ampEnvClapSpeedSlider.customFormatText = formatClapSpeed;
    ampEnvClapSpeedSlider.customParseText  = parseClapSpeed;

    setupKnob(ampEnvSlopeSlider, juce::Colour(0xff00e5ff), false, 0.5886);
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
    cardPreLimiter = std::make_unique<ModuleCardComponent>("Pre Limiter", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardPreLimiter->setTooltip("PRE LIMITER: Pre-saturation brickwall safety limiter and gain booster.");
    cardPreLimiter->setKnobsLightTrough(false);
    setupBox(preLimiterEnableBox);
    bindSelector(preLimiterEnableSelector, preLimiterEnableBox, "pre_limiter_enable", { "Off", "On" }, 2);
    preLimiterEnableSelector.setAccent(juce::Colour(0xffe53935));
    cardPreLimiter->setLedSelector(&preLimiterEnableSelector);

    setupKnob(preLimiterGainSlider, juce::Colour(0xffe53935), false, 12.0 / 36.0);
    preLimiterGainSlider.customFormatText = formatLimiterGain;
    preLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(preLimiterThreshSlider, juce::Colour(0xffe53935), false, 1.0);
    preLimiterThreshSlider.customFormatText = formatLimiterThresh;
    preLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(preLimiterReleaseSlider, juce::Colour(0xffe53935), false, 0.6296);
    preLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    preLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPreLimiter->setKnob(0, "In Gain", &preLimiterGainSlider);
    cardPreLimiter->setKnob(1, "Thresh", &preLimiterThreshSlider);
    cardPreLimiter->setKnob(2, "Release", &preLimiterReleaseSlider);
    addChildComponent(cardPreLimiter.get());

    // 27. Post-Amp Limiter
    cardPostLimiter = std::make_unique<ModuleCardComponent>("Post Limiter", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardPostLimiter->setTooltip("POST LIMITER: Master output brickwall peak limiter with auto soft-knee release.");
    cardPostLimiter->setKnobsLightTrough(false);
    setupBox(postLimiterEnableBox);
    bindSelector(postLimiterEnableSelector, postLimiterEnableBox, "post_limiter_enable", { "Off", "On" }, 2);
    postLimiterEnableSelector.setAccent(juce::Colour(0xffe53935));
    cardPostLimiter->setLedSelector(&postLimiterEnableSelector);

    setupKnob(postLimiterGainSlider, juce::Colour(0xffe53935), false, 12.0 / 36.0);
    postLimiterGainSlider.customFormatText = formatLimiterGain;
    postLimiterGainSlider.customParseText  = parseLimiterGain;

    setupKnob(postLimiterThreshSlider, juce::Colour(0xffe53935), false, 1.0);
    postLimiterThreshSlider.customFormatText = formatLimiterThresh;
    postLimiterThreshSlider.customParseText  = parseLimiterThresh;

    setupKnob(postLimiterReleaseSlider, juce::Colour(0xffe53935), false, 0.6296);
    postLimiterReleaseSlider.customFormatText = formatLimiterRelease;
    postLimiterReleaseSlider.customParseText  = parseLimiterRelease;

    cardPostLimiter->setKnob(0, "In Gain", &postLimiterGainSlider);
    cardPostLimiter->setKnob(1, "Thresh", &postLimiterThreshSlider);
    cardPostLimiter->setKnob(2, "Release", &postLimiterReleaseSlider);
    addChildComponent(cardPostLimiter.get());

    // 28. Velocity
    cardVelocity = std::make_unique<ModuleCardComponent>("Velocity", juce::Colour(0xff29b6f6));
    cardVelocity->setTooltip("VELOCITY: Global MIDI velocity modulation matrix mapping strike force to synth parameters.");
    setupKnob(velSlopeSlider, juce::Colour(0xff29b6f6), false, 0.5886);
    velSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    velSlopeSlider.customFormatText = formatVelocitySlope;
    velSlopeSlider.customParseText  = parseVelocitySlope;

    setupKnob(velDepthSlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDepthSlider.customFormatText = formatBipolarPercent;
    velDepthSlider.customParseText  = parseBipolarPercent;

    setupKnob(velDecaySlider, juce::Colour(0xff29b6f6), true, 0.5);
    velDecaySlider.customFormatText = formatBipolarPercent;
    velDecaySlider.customParseText  = parseBipolarPercent;

    setupKnob(velVolumeSlider, juce::Colour(0xff29b6f6), false, 0.0);
    velVolumeSlider.customFormatText = [](double val) {
        int v = static_cast<int>(std::round(val * 100.0));
        return "-" + juce::String(v) + "%";
    };
    velVolumeSlider.customParseText = parsePercent;

    cardVelocity->setKnob(0, "Slope", &velSlopeSlider);
    cardVelocity->setKnob(1, "Depth", &velDepthSlider);
    cardVelocity->setKnob(2, "Decay", &velDecaySlider);
    cardVelocity->setKnob(3, "Volume", &velVolumeSlider);
    addChildComponent(cardVelocity.get());

    // 29. Key Tracking
    cardKeyTrack = std::make_unique<ModuleCardComponent>("Key Track", juce::Colour(0xff26a69a));
    cardKeyTrack->setTooltip("KEY TRACK: MIDI pitch-tracking matrix scaling cutoff, decay, and level across keyboard.");
    setupKnob(keySlopeSlider, juce::Colour(0xff26a69a), false, 0.5886);
    keySlopeSlider.diagramType = RotaryKnobSlider::DiagramType::VelocitySlope;
    keySlopeSlider.customFormatText = formatVelocitySlope;
    keySlopeSlider.customParseText  = parseVelocitySlope;

    setupKnob(keyDepthSlider, juce::Colour(0xff26a69a), true, 0.5);
    keyDepthSlider.customFormatText = formatBipolarPercent;
    keyDepthSlider.customParseText  = parseBipolarPercent;

    setupKnob(keyDecaySlider, juce::Colour(0xff26a69a), true, 0.5);
    keyDecaySlider.customFormatText = formatBipolarPercent;
    keyDecaySlider.customParseText  = parseBipolarPercent;

    setupKnob(keyVolumeSlider, juce::Colour(0xff26a69a), false, 0.0);
    keyVolumeSlider.customFormatText = [](double val) {
        int v = static_cast<int>(std::round(val * 100.0));
        return "-" + juce::String(v) + "%";
    };
    keyVolumeSlider.customParseText = parsePercent;

    cardKeyTrack->setKnob(0, "Slope", &keySlopeSlider);
    cardKeyTrack->setKnob(1, "Depth", &keyDepthSlider);
    cardKeyTrack->setKnob(2, "Decay", &keyDecaySlider);
    cardKeyTrack->setKnob(3, "Volume", &keyVolumeSlider);
    addChildComponent(cardKeyTrack.get());

    // 30. Slop
    cardSlop = std::make_unique<ModuleCardComponent>("Slop", juce::Colour(0xffab47bc));
    cardSlop->setTooltip("SLOP: Analog pitch, decay, and pan micro-drift simulation for organic acoustic variation.");
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

    // 31. Mod Envelopes 1..3
    const auto modChoices = TheKlangFarmerAudioProcessor::getModDestinationChoices();

    auto setupModEnvCard = [this, &modChoices](std::unique_ptr<ModuleCardComponent>& card,
                                                const juce::String& title,
                                                RotaryKnobSlider& slope,
                                                RotaryKnobSlider& depth,
                                                RotaryKnobSlider& decay,
                                                juce::ComboBox& targetBox) {
        card = std::make_unique<ModuleCardComponent>(title, juce::Colour(0xffffb300));
        card->setTooltip(title.toUpperCase() + ": Freely assignable modulation envelope routable to any synthesizer parameter.");
        targetBox.clear();
        targetBox.addItemList(modChoices, 1);
        targetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161922));
        targetBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
        targetBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2d3342));
        targetBox.setJustificationType(juce::Justification::centredLeft);
        targetBox.setTooltip("MOD DESTINATION: Select the synthesizer parameter modulated by this envelope.");
        card->setSelector(&targetBox);
        card->setSelectorAtBottom(true);

        setupKnob(slope, juce::Colour(0xffffb300), false, 0.5886);
        slope.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
        slope.customFormatText = formatSlope;
        slope.customParseText  = parseSlope;

        setupKnob(depth, juce::Colour(0xffffb300), true, 0.5);
        depth.customFormatText = formatBipolarPercent;
        depth.customParseText  = parseBipolarPercent;

        setupKnob(decay, juce::Colour(0xffffb300), false, 0.3806);
        decay.customFormatText = formatTimeMs;
        decay.customParseText  = parseTimeMs;

        card->setKnob(0, "Slope", &slope);
        card->setKnob(1, "Depth", &depth);
        card->setKnob(2, "Decay", &decay);
        addChildComponent(card.get());
    };

    setupModEnvCard(cardModEnv1, "Mod Env 1", modEnv1SlopeSlider, modEnv1DepthSlider, modEnv1DecaySlider, modEnv1TargetBox);
    setupModEnvCard(cardModEnv2, "Mod Env 2", modEnv2SlopeSlider, modEnv2DepthSlider, modEnv2DecaySlider, modEnv2TargetBox);
    setupModEnvCard(cardModEnv3, "Mod Env 3", modEnv3SlopeSlider, modEnv3DepthSlider, modEnv3DecaySlider, modEnv3TargetBox);
    updateModTargetBoxItems();

    // --- APVTS PARAMETER ATTACHMENTS ---

    // 1. Carrier 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier1_tracking", carrier1TrackingBox));
    bindSlider("carrier1_pitch", carrier1PitchSlider);
    bindSlider("carrier1_shape", carrier1ShapeSlider);
    bindSlider("carrier1_depth", carrier1DepthSlider);

    // 2. Modulator 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_track", mod1TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod1_type", mod1TypeBox));
    bindSlider("mod1_shape", mod1ShapeSlider);
    bindSlider("mod1_speed", mod1SpeedSlider);

    // 3. Pitch Envelope 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv1_target", pitchEnv1TargetBox));
    bindSlider("pitchenv1_slope", pitchEnv1SlopeSlider);
    bindSlider("pitchenv1_depth", pitchEnv1DepthSlider);
    bindSlider("pitchenv1_decay", pitchEnv1DecaySlider);

    // 4. Filter 1
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_type", filter1TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter1_slope", filter1SlopeBox));
    bindSlider("filter1_cutoff", filter1CutoffSlider);
    bindSlider("filter1_resonance", filter1ResonanceSlider);

    // 5. Filter Envelope 1
    bindSlider("filterenv1_slope", filterEnv1SlopeSlider);
    bindSlider("filterenv1_depth", filterEnv1DepthSlider);
    bindSlider("filterenv1_decay", filterEnv1DecaySlider);
    bindSlider("filterenv1_postdrive", filterEnv1PostDriveSlider);

    // 6. Carrier 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "carrier2_tracking", carrier2TrackingBox));
    bindSlider("carrier2_pitch", carrier2PitchSlider);
    bindSlider("carrier2_shape", carrier2ShapeSlider);
    bindSlider("carrier2_depth", carrier2DepthSlider);

    // 7. Modulator 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_track", mod2TrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "mod2_type", mod2TypeBox));
    bindSlider("mod2_shape", mod2ShapeSlider);
    bindSlider("mod2_speed", mod2SpeedSlider);

    // 8. Pitch Envelope 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pitchenv2_target", pitchEnv2TargetBox));
    bindSlider("pitchenv2_slope", pitchEnv2SlopeSlider);
    bindSlider("pitchenv2_depth", pitchEnv2DepthSlider);
    bindSlider("pitchenv2_decay", pitchEnv2DecaySlider);

    // 9. Filter 2
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_type", filter2TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter2_slope", filter2SlopeBox));
    bindSlider("filter2_cutoff", filter2CutoffSlider);
    bindSlider("filter2_resonance", filter2ResonanceSlider);

    // 10. Filter Envelope 2
    bindSlider("filterenv2_slope", filterEnv2SlopeSlider);
    bindSlider("filterenv2_depth", filterEnv2DepthSlider);
    bindSlider("filterenv2_decay", filterEnv2DecaySlider);
    bindSlider("filterenv2_postdrive", filterEnv2PostDriveSlider);

    // 11. Noise Transient
    bindSlider("noise_sh_rate", noiseShRateSlider);
    bindSlider("noise_filter", noiseFilterSlider);
    bindSlider("noise_drive", noiseDriveSlider);
    bindSlider("noise_decay", noiseDecaySlider);

    // 12. Filter 3 (Transients Filter)
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_type", filter3TypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "filter3_slope", filter3SlopeBox));
    bindSlider("filter3_cutoff", filter3CutoffSlider);
    bindSlider("filter3_resonance", filter3ResonanceSlider);

    // 13. Filter Envelope 3 (Transients Filter Env)
    bindSlider("filterenv3_slope", filterEnv3SlopeSlider);
    bindSlider("filterenv3_depth", filterEnv3DepthSlider);
    bindSlider("filterenv3_decay", filterEnv3DecaySlider);
    bindSlider("filterenv3_postdrive", filterEnv3PostDriveSlider);

    // 14. Mixer
    bindSlider("mixer_carrier1_level", mixerCarrier1LevelSlider);
    bindSlider("mixer_carrier2_level", mixerCarrier2LevelSlider);
    bindSlider("mixer_ringmod", mixerRingModSlider);
    bindSlider("mixer_noise_level", mixerNoiseLevelSlider);

    // Multi-Instance FX Slot Knobs Attachments
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            juce::String preParamId = "pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            bindSlider(preParamId, preFXCards[s]->getKnob(p));

            juce::String postParamId = "post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            bindSlider(postParamId, postFXCards[s]->getKnob(p));
        }
    }

    // 24. Amp
    bindSlider("amp_level", ampLevelSlider);
    bindSlider("amp_pan", ampPanSlider);
    bindSlider("amp_drive", ampDriveSlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "amp_limiter", ampLimiterBox));

    // 25. Amp Envelope
    bindSlider("ampenv_claps", ampEnvClapsSlider);
    bindSlider("ampenv_clapspeed", ampEnvClapSpeedSlider);
    bindSlider("ampenv_slope", ampEnvSlopeSlider);
    bindSlider("ampenv_decay", ampEnvDecaySlider);

    // 26. Pre-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "pre_limiter_enable", preLimiterEnableBox));
    bindSlider("pre_limiter_gain", preLimiterGainSlider);
    bindSlider("pre_limiter_thresh", preLimiterThreshSlider);
    bindSlider("pre_limiter_release", preLimiterReleaseSlider);

    // 27. Post-Amp Limiter
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "post_limiter_enable", postLimiterEnableBox));
    bindSlider("post_limiter_gain", postLimiterGainSlider);
    bindSlider("post_limiter_thresh", postLimiterThreshSlider);
    bindSlider("post_limiter_release", postLimiterReleaseSlider);

    // 28. Velocity
    bindSlider("vel_slope", velSlopeSlider);
    bindSlider("vel_depth", velDepthSlider);
    bindSlider("vel_decay", velDecaySlider);
    bindSlider("vel_volume", velVolumeSlider);

    // 29. Key Tracking
    bindSlider("key_slope", keySlopeSlider);
    bindSlider("key_depth", keyDepthSlider);
    bindSlider("key_decay", keyDecaySlider);
    bindSlider("key_volume", keyVolumeSlider);

    // 30. Slop
    bindSlider("slop_freq", slopFreqSlider);
    bindSlider("slop_depth", slopDepthSlider);
    bindSlider("slop_decay", slopDecaySlider);
    bindSlider("slop_pan", slopPanSlider);

    // 31. Mod Envelopes 1..3
    bindSlider("modenv1_slope", modEnv1SlopeSlider);
    bindSlider("modenv1_depth", modEnv1DepthSlider);
    bindSlider("modenv1_decay", modEnv1DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv1_target", modEnv1TargetBox));

    bindSlider("modenv2_slope", modEnv2SlopeSlider);
    bindSlider("modenv2_depth", modEnv2DepthSlider);
    bindSlider("modenv2_decay", modEnv2DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv2_target", modEnv2TargetBox));

    bindSlider("modenv3_slope", modEnv3SlopeSlider);
    bindSlider("modenv3_depth", modEnv3DepthSlider);
    bindSlider("modenv3_decay", modEnv3DecaySlider);
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "modenv3_target", modEnv3TargetBox));

    // --- FX PICKERS SETUP & ATTACHMENTS ---
    const juce::StringArray fxChoices {
        "None",
        "Bell EQ",
        "Chorus",
        "Comb Filter",
        "Drive",
        "Filter",
        "Flanger",
        "Frequency Shifter",
        "Grit FX",
        "Phase Smear",
        "Phaser",
        "RingMod",
        "Tempo Delay",
        "Wave Folder"
    };

    for (int i = 0; i < 4; ++i) {
        preFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String paramId = "pre_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, paramId, preFXPickerCard->getBox(i)));
        preFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = preFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                setFXSlotDefaults(i, false, choice);
                audioProcessor.getEngine().setPreFXType(i, choice);
                preFXCards[i]->configureForType(choice);
                updatePageLayout();
                updateModTargetBoxItems();
            }
        };

        postFXPickerCard->getBox(i).addItemList(fxChoices, 1);
        juce::String postParamId = "post_fx_" + juce::String(i + 1) + "_type";
        boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, postParamId, postFXPickerCard->getBox(i)));
        postFXPickerCard->getBox(i).onChange = [this, i]() {
            int choice = postFXPickerCard->getBox(i).getSelectedItemIndex();
            if (choice >= 0) {
                setFXSlotDefaults(i, true, choice);
                audioProcessor.getEngine().setPostFXType(i, choice);
                postFXCards[i]->configureForType(choice);
                updatePageLayout();
                updateModTargetBoxItems();
            }
        };

        preFXCards[i]->configureForType(audioProcessor.getEngine().getPreFXType(i));
        postFXCards[i]->configureForType(audioProcessor.getEngine().getPostFXType(i));
    }

    auto setupCardListeners = [this](juce::Component* card, int blockId, const juce::String& name) {
        if (!card) return;
        card->addMouseListener(this, true);
        if (auto* modCard = dynamic_cast<ModuleCardComponent*>(card)) {
            modCard->onCardClicked = [this, blockId, name]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(blockId, name);
                }
            };
        }
    };

    setupCardListeners(cardCarrier1.get(), TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1");
    setupCardListeners(cardMod1.get(), TbdAudio::ModularDrumEngine::BLK_MODULATOR1, "MODULATOR 1");
    setupCardListeners(cardPitchEnv1.get(), TbdAudio::ModularDrumEngine::BLK_PITCHENV1, "PITCH ENV 1");
    setupCardListeners(cardFilter1.get(), TbdAudio::ModularDrumEngine::BLK_FILTER1, "FILTER 1");
    setupCardListeners(cardFilterEnv1.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV1, "FILTER ENV 1");
    setupCardListeners(cardCarrier2.get(), TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2");
    setupCardListeners(cardMod2.get(), TbdAudio::ModularDrumEngine::BLK_MODULATOR2, "MODULATOR 2");
    setupCardListeners(cardPitchEnv2.get(), TbdAudio::ModularDrumEngine::BLK_PITCHENV2, "PITCH ENV 2");
    setupCardListeners(cardFilter2.get(), TbdAudio::ModularDrumEngine::BLK_FILTER2, "FILTER 2");
    setupCardListeners(cardFilterEnv2.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV2, "FILTER ENV 2");
    setupCardListeners(cardNoise.get(), TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE");
    setupCardListeners(cardFilter3.get(), TbdAudio::ModularDrumEngine::BLK_FILTER3, "FILTER 3");
    setupCardListeners(cardFilterEnv3.get(), TbdAudio::ModularDrumEngine::BLK_FILTERENV3, "FILTER ENV 3");
    setupCardListeners(cardMixer.get(), TbdAudio::ModularDrumEngine::BLK_MIXER, "MIXER");
    setupCardListeners(cardAmp.get(), TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER");
    setupCardListeners(cardAmpEnv.get(), TbdAudio::ModularDrumEngine::BLK_AMPENV, "AMP ENV");
    setupCardListeners(cardPreLimiter.get(), TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, "PRE LIMITER");
    setupCardListeners(cardPostLimiter.get(), TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, "MASTER LIMITER");
    setupCardListeners(cardVelocity.get(), TbdAudio::ModularDrumEngine::BLK_VELOCITY, "VELOCITY");
    setupCardListeners(cardKeyTrack.get(), TbdAudio::ModularDrumEngine::BLK_KEYTRACK, "KEY TRACK");
    setupCardListeners(cardSlop.get(), TbdAudio::ModularDrumEngine::BLK_SLOP, "SLOP");
    setupCardListeners(cardModEnv1.get(), TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1");
    setupCardListeners(cardModEnv2.get(), TbdAudio::ModularDrumEngine::BLK_MODENV2, "MOD ENV 2");
    setupCardListeners(cardModEnv3.get(), TbdAudio::ModularDrumEngine::BLK_MODENV3, "MOD ENV 3");

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) {
            preFXCards[s]->addMouseListener(this, true);
            preFXCards[s]->onCardClicked = [this, s]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s,
                                               "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle());
                }
            };
        }
        if (postFXCards[s]) {
            postFXCards[s]->addMouseListener(this, true);
            postFXCards[s]->onCardClicked = [this, s]() {
                if (!vizCard.getIsLocked()) {
                    vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s,
                                               "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle());
                }
            };
        }
    }

    scopeBuffer.resize(128, 0.0f);
    updateDynamicControls();

    setSize(1040, 740);
    setResizable(true, true);
    setResizeLimits(800, 560, 2400, 1600);

    setPage(0);
    startTimerHz(30);
}


TheKlangFarmerAudioProcessorEditor::~TheKlangFarmerAudioProcessorEditor() {
    stopTimer();
    if (tooltipWindow) {
        tooltipWindow->setLookAndFeel(nullptr);
        tooltipWindow.reset();
    }
    setLookAndFeel(nullptr);
}

void TheKlangFarmerAudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar, double defaultVal) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setScrollWheelEnabled(true);
    slider.setBipolar(isBipolar);
    slider.setAccentColour(trackColour);
    slider.setColour(juce::Slider::rotarySliderFillColourId, trackColour);
    slider.setColour(juce::Slider::trackColourId, trackColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff232733));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff161922));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe8edf5));
    slider.setRange(0.0, 1.0, 0.0005);
    slider.setDoubleClickReturnValue(true, defaultVal);
    slider.getDefaultValue = [defaultVal]() { return defaultVal; };
}

static juce::String getFarmerParamDescription(const juce::String& paramId, bool& isBipolar) {
    isBipolar = false;
    // Carrier 1
    if (paramId == "carrier1_pitch") return "Carrier 1 pitch transpose / frequency offset";
    if (paramId == "carrier1_shape") return "Carrier 1 waveform shape morphing (Sine -> Triangle -> Saw -> Pulse)";
    if (paramId == "carrier1_depth") { isBipolar = true; return "Frequency modulation depth from Modulator 1"; }

    // Modulator 1
    if (paramId == "mod1_shape") return "Modulator 1 waveform shape / symmetry";
    if (paramId == "mod1_speed") return "Modulator 1 frequency / semitone offset / FM ratio";

    // Pitch Env 1
    if (paramId == "pitchenv1_slope") return "Pitch envelope decay curve tension from punchy exponential to linear";
    if (paramId == "pitchenv1_depth") { isBipolar = true; return "Bipolar pitch envelope modulation depth in semitones"; }
    if (paramId == "pitchenv1_decay") return "Pitch envelope decay duration";

    // Filter 1
    if (paramId == "filter1_cutoff") return "Filter 1 cutoff corner / center frequency (20 Hz - 24 kHz)";
    if (paramId == "filter1_resonance") return "Filter 1 resonance / Q sharpness boost at cutoff";

    // Filter Env 1
    if (paramId == "filterenv1_slope") return "Filter envelope decay curve tension";
    if (paramId == "filterenv1_depth") { isBipolar = true; return "Cutoff modulation depth in octaves (-10 to +10 oct)"; }
    if (paramId == "filterenv1_decay") return "Filter envelope decay duration";
    if (paramId == "filterenv1_postdrive") return "Pre-filter analog saturation drive gain (-6 to +24 dB)";

    // Carrier 2
    if (paramId == "carrier2_pitch") return "Carrier 2 pitch transpose / frequency offset";
    if (paramId == "carrier2_shape") return "Carrier 2 waveform shape morphing";
    if (paramId == "carrier2_depth") { isBipolar = true; return "Frequency modulation depth from Modulator 2"; }

    // Modulator 2
    if (paramId == "mod2_shape") return "Modulator 2 waveform shape / symmetry";
    if (paramId == "mod2_speed") return "Modulator 2 frequency / semitone offset / FM ratio";

    // Pitch Env 2
    if (paramId == "pitchenv2_slope") return "Voice 2 pitch envelope decay curve tension";
    if (paramId == "pitchenv2_depth") { isBipolar = true; return "Voice 2 pitch envelope modulation depth in semitones"; }
    if (paramId == "pitchenv2_decay") return "Voice 2 pitch envelope decay duration";

    // Filter 2
    if (paramId == "filter2_cutoff") return "Filter 2 cutoff corner / center frequency";
    if (paramId == "filter2_resonance") return "Filter 2 resonance / Q sharpness boost at cutoff";

    // Filter Env 2
    if (paramId == "filterenv2_slope") return "Filter 2 envelope decay curve tension";
    if (paramId == "filterenv2_depth") { isBipolar = true; return "Filter 2 cutoff modulation depth in octaves"; }
    if (paramId == "filterenv2_decay") return "Filter 2 envelope decay duration";
    if (paramId == "filterenv2_postdrive") return "Filter 2 pre-filter saturation drive gain";

    // Noise Transient
    if (paramId == "noise_sh_rate") return "Sample-and-hold downsampling clock rate for metallic textures";
    if (paramId == "noise_filter") return "Transient noise band-pass / center frequency color";
    if (paramId == "noise_drive") return "Transient noise saturation drive gain";
    if (paramId == "noise_decay") return "Transient noise burst decay time";

    // Filter 3 (Transients Filter)
    if (paramId == "filter3_cutoff") return "Filter 3 cutoff corner / center frequency";
    if (paramId == "filter3_resonance") return "Filter 3 resonance / Q sharpness boost at cutoff";

    // Filter Env 3
    if (paramId == "filterenv3_slope") return "Filter 3 envelope decay curve tension";
    if (paramId == "filterenv3_depth") { isBipolar = true; return "Filter 3 cutoff modulation depth in octaves"; }
    if (paramId == "filterenv3_decay") return "Filter 3 envelope decay duration";
    if (paramId == "filterenv3_postdrive") return "Filter 3 pre-filter saturation drive gain";

    // Mixer
    if (paramId == "mixer_carrier1_level") return "Carrier 1 voice output volume level";
    if (paramId == "mixer_carrier2_level") return "Carrier 2 voice output volume level";
    if (paramId == "mixer_ringmod") return "Ring modulator (Carrier 1 x Carrier 2) mix level";
    if (paramId == "mixer_noise_level") return "Transients noise generator mix level";

    // Amp
    if (paramId == "amp_level") return "Master amplifier output volume level";
    if (paramId == "amp_pan") { isBipolar = true; return "Stereo panorama position (Left <-> Right)"; }
    if (paramId == "amp_drive") return "Pre-limiter analog saturation drive gain";

    // Amp Env
    if (paramId == "ampenv_claps") return "Pre-decay transient hand-clap burst count (0 to 32 bursts)";
    if (paramId == "ampenv_clapspeed") return "Time spacing interval between clap bursts (1 to 15 ms)";
    if (paramId == "ampenv_slope") return "Master amplitude envelope decay curve tension";
    if (paramId == "ampenv_decay") return "Master amplitude envelope decay duration";

    // Pre-Limiter
    if (paramId == "pre_limiter_gain") return "Input boost gain into the pre-limiter";
    if (paramId == "pre_limiter_thresh") return "Ceiling threshold for pre-limiter peak reduction";
    if (paramId == "pre_limiter_release") return "Release recovery time for the pre-limiter";

    // Post-Limiter
    if (paramId == "post_limiter_gain") return "Input boost gain into the master post-limiter";
    if (paramId == "post_limiter_thresh") return "Master ceiling threshold for peak limiting";
    if (paramId == "post_limiter_release") return "Release recovery time for the master limiter";

    // Velocity
    if (paramId == "vel_slope") return "MIDI velocity dynamic response curve";
    if (paramId == "vel_depth") { isBipolar = true; return "Velocity scaling of modulation envelope depth"; }
    if (paramId == "vel_decay") { isBipolar = true; return "Velocity scaling of envelope decay durations"; }
    if (paramId == "vel_volume") return "Minimum volume floor attenuation at zero velocity";

    // Key Track
    if (paramId == "key_slope") return "MIDI note keyboard tracking response curve";
    if (paramId == "key_depth") { isBipolar = true; return "Key tracking scaling of modulation envelope depth"; }
    if (paramId == "key_decay") { isBipolar = true; return "Key tracking scaling of envelope decay durations"; }
    if (paramId == "key_volume") return "Key tracking scaling of voice output volume";

    // Slop
    if (paramId == "slop_freq") return "Analog frequency drift / random pitch fluctuation amount";
    if (paramId == "slop_depth") return "Analog envelope depth fluctuation amount";
    if (paramId == "slop_decay") return "Analog envelope decay time fluctuation amount";
    if (paramId == "slop_pan") return "Subtle stereo position wander per note strike";

    // Mod Envelopes
    if (paramId.startsWith("modenv") && paramId.endsWith("_slope")) return "Modulation envelope decay curve tension";
    if (paramId.startsWith("modenv") && paramId.endsWith("_depth")) { isBipolar = true; return "Bipolar modulation envelope depth to assigned target"; }
    if (paramId.startsWith("modenv") && paramId.endsWith("_decay")) return "Modulation envelope decay duration";

    return "";
}

void TheKlangFarmerAudioProcessorEditor::bindSlider(const juce::String& paramId, RotaryKnobSlider& slider) {
    if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
        float defVal = param->getDefaultValue();
        slider.setDoubleClickReturnValue(true, defVal);
        slider.getDefaultValue = [defVal]() { return defVal; };
    }
    slider.setParamId(paramId);
    slider.getModInfoFunc = [this](const juce::String& pid) {
        return audioProcessor.getParamModulationInfo(pid);
    };

    if (!paramId.startsWith("pre_fx_") && !paramId.startsWith("post_fx_")) {
        bool isBipolar = false;
        juce::String desc = getFarmerParamDescription(paramId, isBipolar);
        slider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, paramId, desc, isBipolar));
    }

    registeredSliders.push_back(&slider);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, slider));
}

void TheKlangFarmerAudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setVisible(false);
}

void TheKlangFarmerAudioProcessorEditor::updateCarrier1Controls() {
    int mode = carrier1TrackingBox.getSelectedItemIndex();
    if (mode == 0) { // MIDI
        cardCarrier1->setKnobLabel(0, "Offset");
        carrier1PitchSlider.setBipolar(true);
        carrier1PitchSlider.getDefaultValue = []() { return 0.5; };
    } else if (mode == 1) { // Freq
        cardCarrier1->setKnobLabel(0, "Frequency");
        carrier1PitchSlider.setBipolar(false);
        carrier1PitchSlider.getDefaultValue = []() {
            return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
        };
    } else { // Note
        cardCarrier1->setKnobLabel(0, "Note");
        carrier1PitchSlider.setBipolar(false);
        carrier1PitchSlider.getDefaultValue = []() { return 33.0 / 127.0; };
    }
    carrier1PitchSlider.setDoubleClickReturnValue(true, carrier1PitchSlider.getDefaultValue());
    juce::String desc = (mode == 0 ? "Pitch transpose offset in semitones" : (mode == 1 ? "Carrier base frequency in Hertz" : "Base musical note pitch"));
    carrier1PitchSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "carrier1_pitch", desc, mode == 0));
    carrier1PitchSlider.repaint();
    carrier1PitchSlider.updateText();
}

void TheKlangFarmerAudioProcessorEditor::updateCarrier2Controls() {
    int mode = carrier2TrackingBox.getSelectedItemIndex();
    if (mode == 0) { // MIDI
        cardCarrier2->setKnobLabel(0, "Offset");
        carrier2PitchSlider.setBipolar(true);
        carrier2PitchSlider.getDefaultValue = []() { return 0.5; };
    } else if (mode == 1) { // Freq
        cardCarrier2->setKnobLabel(0, "Frequency");
        carrier2PitchSlider.setBipolar(false);
        carrier2PitchSlider.getDefaultValue = []() {
            return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
        };
    } else { // Note
        cardCarrier2->setKnobLabel(0, "Note");
        carrier2PitchSlider.setBipolar(false);
        carrier2PitchSlider.getDefaultValue = []() { return 33.0 / 127.0; };
    }
    carrier2PitchSlider.setDoubleClickReturnValue(true, carrier2PitchSlider.getDefaultValue());
    juce::String desc = (mode == 0 ? "Pitch transpose offset in semitones" : (mode == 1 ? "Carrier base frequency in Hertz" : "Base musical note pitch"));
    carrier2PitchSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "carrier2_pitch", desc, mode == 0));
    carrier2PitchSlider.repaint();
    carrier2PitchSlider.updateText();
}

void TheKlangFarmerAudioProcessorEditor::resetToDefaults(bool cleanFX) {
    for (auto* param : audioProcessor.getParameters()) {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param)) {
            rangedParam->setValueNotifyingHost(rangedParam->getDefaultValue());
        }
    }

    if (cleanFX) {
        for (int i = 0; i < 4; ++i) {
            juce::String preId = "pre_fx_" + juce::String(i + 1) + "_type";
            if (auto* p = audioProcessor.apvts.getParameter(preId)) {
                p->setValueNotifyingHost(0.0f);
            }
            audioProcessor.getEngine().setPreFXType(i, 0);
            preFXPickerCard->getBox(i).setSelectedId(1, juce::dontSendNotification);
            if (preFXCards[i]) preFXCards[i]->configureForType(0);

            juce::String postId = "post_fx_" + juce::String(i + 1) + "_type";
            if (auto* p = audioProcessor.apvts.getParameter(postId)) {
                p->setValueNotifyingHost(0.0f);
            }
            audioProcessor.getEngine().setPostFXType(i, 0);
            postFXPickerCard->getBox(i).setSelectedId(1, juce::dontSendNotification);
            if (postFXCards[i]) postFXCards[i]->configureForType(0);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            int preType = audioProcessor.getEngine().getPreFXType(i);
            preFXPickerCard->getBox(i).setSelectedId(preType + 1, juce::dontSendNotification);
            if (preFXCards[i]) preFXCards[i]->configureForType(preType);

            int postType = audioProcessor.getEngine().getPostFXType(i);
            postFXPickerCard->getBox(i).setSelectedId(postType + 1, juce::dontSendNotification);
            if (postFXCards[i]) postFXCards[i]->configureForType(postType);
        }
    }

    updateDynamicControls();
    updatePageLayout();
    updateModTargetBoxItems();
    repaint();
}

void TheKlangFarmerAudioProcessorEditor::bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                                              const juce::String& paramId, const juce::StringArray& items, int numColumns) {
    box.clear();
    box.addItemList(items, 1);
    box.setVisible(false);
    selector.setItems(items, numColumns);
    selector.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips(paramId));
    if (paramId.contains("carrier") && paramId.contains("tracking")) selector.setTooltip("CARRIER TRACKING: Select pitch tracking mode (MIDI note, Fixed Hz, or Semitone Note).");
    else if (paramId.contains("mod") && paramId.contains("track")) selector.setTooltip("MODULATOR TRACKING: Select tracking mode (Fixed Hz, Follow semitones, or FM Ratio).");
    else if (paramId.contains("mod") && paramId.contains("type")) selector.setTooltip("MODULATOR TYPE: Select modulator waveform type (Osc, Cyclic, or Noise).");
    else if (paramId.contains("target")) selector.setTooltip("PITCH ENV ROUTING: Select modulation routing destination (Carrier, Modulator, Both, or Opposite).");
    else if (paramId.contains("filter") && paramId.contains("type")) selector.setTooltip("FILTER TYPE: Select filter characteristic (Low-Pass, Band-Pass, High-Pass, Notch).");
    else if (paramId.contains("filter") && paramId.contains("slope")) selector.setTooltip("FILTER SLOPE: Select filter attenuation roll-off from 6 to 36 dB/oct.");
    else if (paramId.contains("limiter")) selector.setTooltip("LIMITER TOGGLE: Enable or bypass brickwall safety limiter.");

    selector.onChange = [this, &box, paramId](int idx) {
        box.setSelectedId(idx + 1, juce::sendNotification);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        }
    };
}

void TheKlangFarmerAudioProcessorEditor::setFXSlotDefaults(int slot, bool isPost, int fxType) {
    float defs[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    switch (fxType) {
        case 1:  defs[0] = 1.0f; defs[1] = 0.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // Bell EQ: 1kHz, 0.1 oct, 0dB, flat
        case 2:  defs[0] = 0.5398f; defs[1] = 0.60f; defs[2] = 0.60f; defs[3] = 0.50f; break; // Chorus: 1.2Hz, 60% depth, +20% fb, 50% mix
        case 3:  defs[0] = 1.0f; defs[1] = 1.0f; defs[2] = 0.5f; defs[3] = 0.75f; break; // Comb: Damp 24kHz, Cut 24kHz, Res 0%, Mix +50%:50%
        case 4:  defs[0] = 0.4f; defs[1] = 0.5f; defs[2] = 0.5f; defs[3] = 1.0f; break; // Drive: +6dB, 0 bias, 50% flat, Limiter On
        case 5:  defs[0] = 0.0f; defs[1] = 0.25f; defs[2] = 1.0f; defs[3] = 0.0f; break; // Filter: LPF, -12dB, 24kHz, 0% res
        case 6:  defs[0] = 0.3500f; defs[1] = 0.70f; defs[2] = 0.868f; defs[3] = 0.50f; break; // Flanger: 0.25Hz, 70% depth, +70% fb, 50% mix
        case 7:  defs[0] = 0.5f; defs[1] = TbdAudio::rangeHzToNorm(3.0f); defs[2] = 0.75f; defs[3] = 0.5f; break; // FreqShift: 0 shift, 3Hz, Blend +50%:50%, center
        case 8:  defs[0] = 1.0f; defs[1] = 1.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // Grit: 16 bit, 24kHz, 0dB, 0dB
        case 9:  defs[0] = 0.0f; defs[1] = 4.0f / 32.0f; defs[2] = 0.62124f; defs[3] = 0.5f; break; // Phase Smear: 2nd Order, 4 stages, 1kHz, 0%
        case 10: defs[0] = 0.4530f; defs[1] = 0.70f; defs[2] = 0.763f; defs[3] = 0.50f; break; // Phaser: 0.5Hz, 70% depth, +50% fb, 50% mix
        case 11: defs[0] = 0.0f; defs[1] = 0.50934f; defs[2] = 0.0f; defs[3] = 0.5f; break; // RingMod: sine, 1kHz, 0% amt, center
        case 12: defs[0] = 5.0f / 9.0f; defs[1] = 0.40f; defs[2] = 0.70f; defs[3] = 0.35f; break; // Tempo Delay: 1/8, 40% fb, 8kHz tone, 35% mix
        case 13: defs[0] = 1.0f; defs[1] = 0.0f; defs[2] = 0.5f; defs[3] = 0.5f; break; // WaveFolder: On, 0 fold, 0 bias, 50% flat
        default: break;
    }
    juce::String prefix = isPost ? "post_fx_" : "pre_fx_";
    for (int p = 0; p < 4; ++p) {
        juce::String paramId = prefix + juce::String(slot + 1) + "_p" + juce::String(p + 1);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(defs[p]);
        }
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
    hideCard(cardAmp); hideCard(cardAmpEnv);
    hideCard(cardPreLimiter); hideCard(cardPostLimiter);
    hideCard(cardVelocity); hideCard(cardKeyTrack); hideCard(cardSlop);
    hideCard(cardModEnv1); hideCard(cardModEnv2); hideCard(cardModEnv3);

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) preFXCards[s]->setVisible(false);
        if (postFXCards[s]) postFXCards[s]->setVisible(false);
    }

    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;
    int numCols = 4;
    int numRows = 2;
    int slotW = (totalW - (numCols - 1) * margin) / numCols;
    int slotH = (totalH - (numRows - 1) * margin) / numRows;

    auto getSlotBounds = [margin, topOffset, slotW, slotH](int slotIndex) {
        int r = slotIndex / 4;
        int c = slotIndex % 4;
        int x = margin + c * (slotW + margin);
        int y = topOffset + r * (slotH + margin);
        return juce::Rectangle<int>(x, y, slotW, slotH);
    };

    navCard.setBounds(getSlotBounds(0));
    navCard.setVisible(true);

    juce::Component* slotComponents[6] = { nullptr };

    switch (currentPage) {
        case 0: // VOICE 1
            slotComponents[0] = cardCarrier1.get();
            slotComponents[1] = cardMod1.get();
            slotComponents[2] = cardPitchEnv1.get();
            slotComponents[3] = cardFilter1.get();
            slotComponents[4] = cardFilterEnv1.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 1: // VOICE 2
            slotComponents[0] = cardCarrier2.get();
            slotComponents[1] = cardMod2.get();
            slotComponents[2] = cardPitchEnv2.get();
            slotComponents[3] = cardFilter2.get();
            slotComponents[4] = cardFilterEnv2.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 2: // TRANSIENTS
            slotComponents[0] = cardNoise.get();
            slotComponents[1] = &blankPlates[0];
            slotComponents[2] = &blankPlates[1];
            slotComponents[3] = cardFilter3.get();
            slotComponents[4] = cardFilterEnv3.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 3: // PRE-AMP FX
            slotComponents[0] = preFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPreFXType(s);
                if (t > 0) {
                    slotComponents[1 + s] = preFXCards[s].get();
                } else {
                    slotComponents[1 + s] = &blankPlates[s];
                }
            }
            slotComponents[5] = cardPreLimiter.get();
            break;

        case 4: // AMPLIFIER
            slotComponents[0] = cardAmp.get();
            slotComponents[1] = cardAmpEnv.get();
            slotComponents[2] = &blankPlates[0];
            slotComponents[3] = &blankPlates[1];
            slotComponents[4] = cardPostLimiter.get();
            slotComponents[5] = cardMixer.get();
            break;

        case 5: // POST-AMP FX
            slotComponents[0] = postFXPickerCard.get();
            for (int s = 0; s < 4; ++s) {
                int t = audioProcessor.getEngine().getPostFXType(s);
                if (t > 0) {
                    slotComponents[1 + s] = postFXCards[s].get();
                } else {
                    slotComponents[1 + s] = &blankPlates[s];
                }
            }
            slotComponents[5] = cardPostLimiter.get();
            break;

        case 6: // MODULATIONS
            slotComponents[0] = cardModEnv1.get();
            slotComponents[1] = cardModEnv2.get();
            slotComponents[2] = cardModEnv3.get();
            slotComponents[3] = cardVelocity.get();
            slotComponents[4] = cardKeyTrack.get();
            slotComponents[5] = cardSlop.get();
            break;
    }

    vizCard.setBounds(getSlotBounds(4));
    vizCard.setVisible(true);

    for (int i = 0; i < 3; ++i) {
        if (slotComponents[i]) {
            slotComponents[i]->setBounds(getSlotBounds(i + 1));
            slotComponents[i]->setVisible(true);
        }
    }

    for (int i = 3; i < 6; ++i) {
        if (slotComponents[i]) {
            slotComponents[i]->setBounds(getSlotBounds(i + 2));
            slotComponents[i]->setVisible(true);
        }
    }

    if (!vizCard.getIsLocked()) {
        switch (currentPage) {
            case 0: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1"); break;
            case 1: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2"); break;
            case 2: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE"); break;
            case 3: {
                int blk = TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER;
                juce::String name = "PRE LIMITER";
                for (int s = 0; s < 4; ++s) {
                    if (audioProcessor.getEngine().getPreFXType(s) > 0) {
                        blk = TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s;
                        name = "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle();
                        break;
                    }
                }
                vizCard.setVisualizedBlock(blk, name);
                break;
            }
            case 4: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER"); break;
            case 5: {
                int blk = TbdAudio::ModularDrumEngine::BLK_POST_LIMITER;
                juce::String name = "MASTER LIMITER";
                for (int s = 0; s < 4; ++s) {
                    if (audioProcessor.getEngine().getPostFXType(s) > 0) {
                        blk = TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s;
                        name = "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle();
                        break;
                    }
                }
                vizCard.setVisualizedBlock(blk, name);
                break;
            }
            case 6: vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1"); break;
        }
    }
}

void TheKlangFarmerAudioProcessorEditor::updateModTargetBoxItems() {
    const auto& destinations = TheKlangFarmerAudioProcessor::getModDestinations();
    auto updateBox = [&](juce::ComboBox& box) {
        for (size_t i = 0; i < destinations.size(); ++i) {
            const auto& d = destinations[i];
            int itemId = static_cast<int>(i + 2); // 1 is "None", so i=0 is itemId 2
            if (d.type == TheKlangFarmerAudioProcessor::ModTargetType::PreFX) {
                int fxType = audioProcessor.getEngine().getPreFXType(d.blockOrSlot);
                juce::String name = TheKlangFarmerAudioProcessor::getFXParamDisplayName(false, d.blockOrSlot, fxType, d.paramIndex);
                box.changeItemText(itemId, name);
            } else if (d.type == TheKlangFarmerAudioProcessor::ModTargetType::PostFX) {
                int fxType = audioProcessor.getEngine().getPostFXType(d.blockOrSlot);
                juce::String name = TheKlangFarmerAudioProcessor::getFXParamDisplayName(true, d.blockOrSlot, fxType, d.paramIndex);
                box.changeItemText(itemId, name);
            }
        }
    };

    updateBox(modEnv1TargetBox);
    updateBox(modEnv2TargetBox);
    updateBox(modEnv3TargetBox);
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
        updateCarrier1Controls();
    }

    int curMod1Track = syncSelector(mod1TrackBox, mod1TrackSelector, "mod1_track", lastMod1Track);
    int curMod1Type  = syncSelector(mod1TypeBox, mod1TypeSelector, "mod1_type", lastMod1Type);
    if (curMod1Type >= 0 || curMod1Track >= 0) {
        if (curMod1Type == 0) {
            cardMod1->setKnobLabel(0, "Shape");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod1ShapeSlider.setBipolar(false);
            mod1ShapeSlider.getDefaultValue = []() { return 0.0; };
            mod1ShapeSlider.setDoubleClickReturnValue(true, 0.0);
            cardMod1->setKnobLabel(1, "Speed");
            if (curMod1Track == 0) {
                mod1SpeedSlider.getDefaultValue = []() { return 0.50934; }; // 55 Hz
                mod1SpeedSlider.setDoubleClickReturnValue(true, 0.50934);
            } else {
                mod1SpeedSlider.getDefaultValue = []() { return 0.5; };
                mod1SpeedSlider.setDoubleClickReturnValue(true, 0.5);
            }
        } else if (curMod1Type == 1) {
            cardMod1->setKnobLabel(0, "DJ Filter");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(true);
            mod1ShapeSlider.getDefaultValue = []() { return 0.5; };
            mod1ShapeSlider.setDoubleClickReturnValue(true, 0.5);
            cardMod1->setKnobLabel(1, "Speed");
            if (curMod1Track == 0) {
                mod1SpeedSlider.getDefaultValue = []() { return 0.50934; };
                mod1SpeedSlider.setDoubleClickReturnValue(true, 0.50934);
            } else {
                mod1SpeedSlider.getDefaultValue = []() { return 0.5; };
                mod1SpeedSlider.setDoubleClickReturnValue(true, 0.5);
            }
        } else {
            // curMod1Type == 2 (S&H Noise): Knob 0 is DJ Filter, Knob 1 is Speed (S&H rate def 24 kHz)
            cardMod1->setKnobLabel(0, "DJ Filter");
            mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod1ShapeSlider.setBipolar(true);
            mod1ShapeSlider.getDefaultValue = []() { return 0.5; };
            mod1ShapeSlider.setDoubleClickReturnValue(true, 0.5);
            cardMod1->setKnobLabel(1, "Speed");
            mod1SpeedSlider.getDefaultValue = []() { return 1.0; }; // 24 kHz
            mod1SpeedSlider.setDoubleClickReturnValue(true, 1.0);
        }
        mod1ShapeSlider.repaint();
        mod1ShapeSlider.updateText();
        mod1SpeedSlider.repaint();
        mod1SpeedSlider.updateText();
    }

    syncSelector(pitchEnv1TargetBox, pitchEnv1TargetSelector, "pitchenv1_target", lastPitchEnv1Target);
    syncSelector(filter1TypeBox, filter1TypeSelector, "filter1_type", lastFilter1Type);
    syncSelector(filter1SlopeBox, filter1SlopeSelector, "filter1_slope", lastFilter1Slope);

    int curCarrier2Track = syncSelector(carrier2TrackingBox, carrier2TrackingSelector, "carrier2_tracking", lastCarrier2Track);
    if (curCarrier2Track >= 0) {
        updateCarrier2Controls();
    }

    int curMod2Track = syncSelector(mod2TrackBox, mod2TrackSelector, "mod2_track", lastMod2Track);
    int curMod2Type  = syncSelector(mod2TypeBox, mod2TypeSelector, "mod2_type", lastMod2Type);
    if (curMod2Type >= 0 || curMod2Track >= 0) {
        if (curMod2Type == 0) {
            cardMod2->setKnobLabel(0, "Shape");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
            mod2ShapeSlider.setBipolar(false);
            mod2ShapeSlider.getDefaultValue = []() { return 0.0; };
            mod2ShapeSlider.setDoubleClickReturnValue(true, 0.0);
            cardMod2->setKnobLabel(1, "Speed");
            if (curMod2Track == 0) {
                mod2SpeedSlider.getDefaultValue = []() { return 0.50934; }; // 55 Hz
                mod2SpeedSlider.setDoubleClickReturnValue(true, 0.50934);
            } else {
                mod2SpeedSlider.getDefaultValue = []() { return 0.5; };
                mod2SpeedSlider.setDoubleClickReturnValue(true, 0.5);
            }
        } else if (curMod2Type == 1) {
            cardMod2->setKnobLabel(0, "DJ Filter");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(true);
            mod2ShapeSlider.getDefaultValue = []() { return 0.5; };
            mod2ShapeSlider.setDoubleClickReturnValue(true, 0.5);
            cardMod2->setKnobLabel(1, "Speed");
            if (curMod2Track == 0) {
                mod2SpeedSlider.getDefaultValue = []() { return 0.50934; };
                mod2SpeedSlider.setDoubleClickReturnValue(true, 0.50934);
            } else {
                mod2SpeedSlider.getDefaultValue = []() { return 0.5; };
                mod2SpeedSlider.setDoubleClickReturnValue(true, 0.5);
            }
        } else {
            // curMod2Type == 2 (S&H Noise): Knob 0 is DJ Filter, Knob 1 is Speed (S&H rate def 24 kHz)
            cardMod2->setKnobLabel(0, "DJ Filter");
            mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
            mod2ShapeSlider.setBipolar(true);
            mod2ShapeSlider.getDefaultValue = []() { return 0.5; };
            mod2ShapeSlider.setDoubleClickReturnValue(true, 0.5);
            cardMod2->setKnobLabel(1, "Speed");
            mod2SpeedSlider.getDefaultValue = []() { return 1.0; }; // 24 kHz
            mod2SpeedSlider.setDoubleClickReturnValue(true, 1.0);
        }
        mod2ShapeSlider.repaint();
        mod2ShapeSlider.updateText();
        mod2SpeedSlider.repaint();
        mod2SpeedSlider.updateText();
    }

    syncSelector(pitchEnv2TargetBox, pitchEnv2TargetSelector, "pitchenv2_target", lastPitchEnv2Target);
    syncSelector(filter2TypeBox, filter2TypeSelector, "filter2_type", lastFilter2Type);
    syncSelector(filter2SlopeBox, filter2SlopeSelector, "filter2_slope", lastFilter2Slope);

    syncSelector(filter3TypeBox, filter3TypeSelector, "filter3_type", lastFilter3Type);
    syncSelector(filter3SlopeBox, filter3SlopeSelector, "filter3_slope", lastFilter3Slope);

    syncSelector(ampLimiterBox, ampLimiterSelector, "amp_limiter", lastAmpLimiter);

    syncSelector(preLimiterEnableBox, preLimiterEnableSelector, "pre_limiter_enable", lastPreLimiterEnable);
    syncSelector(postLimiterEnableBox, postLimiterEnableSelector, "post_limiter_enable", lastPostLimiterEnable);

    for (int s = 0; s < 4; ++s) {
        if (preFXCards[s]) preFXCards[s]->updateDynamicControls();
        if (postFXCards[s]) postFXCards[s]->updateDynamicControls();
    }

    bool fxTypesChanged = false;
    for (int s = 0; s < 4; ++s) {
        int curPre = audioProcessor.getEngine().getPreFXType(s);
        if (curPre != lastPreFXTypes[s]) {
            lastPreFXTypes[s] = curPre;
            fxTypesChanged = true;
        }
        int curPost = audioProcessor.getEngine().getPostFXType(s);
        if (curPost != lastPostFXTypes[s]) {
            lastPostFXTypes[s] = curPost;
            fxTypesChanged = true;
        }
    }
    if (fxTypesChanged) {
        updateModTargetBoxItems();
    }
}

void TheKlangFarmerAudioProcessorEditor::timerCallback() {
    updateDynamicControls();

    for (auto* s : registeredSliders) {
        if (!s || s->getParamId().isEmpty()) continue;
        if (!s->isVisible()) continue;

        auto info = audioProcessor.getParamModulationInfo(s->getParamId());
        RotaryKnobSlider::ModulationVisual mv;
        mv.isModulated = info.isModulated;
        mv.rangeMinNorm = info.rangeMinNorm;
        mv.rangeMaxNorm = info.rangeMaxNorm;
        mv.currentNorm = info.currentNorm;
        mv.showNeedle = info.showNeedle;
        s->setModulation(mv);
    }

    if (vizCard.getIsOff()) return;

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
    } else if (activeBlock >= TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 && activeBlock <= TbdAudio::ModularDrumEngine::BLK_PRE_FX_4) {
        int s = activeBlock - TbdAudio::ModularDrumEngine::BLK_PRE_FX_1;
        int fxType = audioProcessor.getEngine().getPreFXType(s);
        if (fxType == 5 && preFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
            float fCutNorm = static_cast<float>(preFXCards[s]->getKnob(2).getValue());
            float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
            float fRes = static_cast<float>(preFXCards[s]->getKnob(3).getValue());
            int fType = static_cast<int>(std::round(preFXCards[s]->getKnob(0).getValue() * 3.0f));
            int fSlope = static_cast<int>(std::round(preFXCards[s]->getKnob(1).getValue() * 4.0f));
            vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
        } else if (fxType == 1 && preFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
            float eqFNorm = static_cast<float>(preFXCards[s]->getKnob(0).getValue());
            float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
            float eqWNorm = static_cast<float>(preFXCards[s]->getKnob(1).getValue());
            float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
            float eqGNorm = static_cast<float>(preFXCards[s]->getKnob(2).getValue());
            float eqGDb = (eqGNorm - 0.5f) * 48.0f;
            float eqDjNorm = static_cast<float>(preFXCards[s]->getKnob(3).getValue());
            vizCard.getOscilloscope().updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);
        } else {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
            audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
            vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        }
    } else if (activeBlock >= TbdAudio::ModularDrumEngine::BLK_POST_FX_1 && activeBlock <= TbdAudio::ModularDrumEngine::BLK_POST_FX_4) {
        int s = activeBlock - TbdAudio::ModularDrumEngine::BLK_POST_FX_1;
        int fxType = audioProcessor.getEngine().getPostFXType(s);
        if (fxType == 5 && postFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::FilterXY);
            float fCutNorm = static_cast<float>(postFXCards[s]->getKnob(2).getValue());
            float fCutHz = 0.1f * std::pow(24000.0f / 0.1f, fCutNorm);
            float fRes = static_cast<float>(postFXCards[s]->getKnob(3).getValue());
            int fType = static_cast<int>(std::round(postFXCards[s]->getKnob(0).getValue() * 3.0f));
            int fSlope = static_cast<int>(std::round(postFXCards[s]->getKnob(1).getValue() * 4.0f));
            vizCard.getOscilloscope().updateFilterParams(fType, fSlope, fCutHz, fRes);
        } else if (fxType == 1 && postFXCards[s]) {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::EqXY);
            float eqFNorm = static_cast<float>(postFXCards[s]->getKnob(0).getValue());
            float eqFHz = 20.0f * std::pow(24000.0f / 20.0f, eqFNorm);
            float eqWNorm = static_cast<float>(postFXCards[s]->getKnob(1).getValue());
            float eqWOct = 0.1f * std::pow(100.0f, eqWNorm);
            float eqGNorm = static_cast<float>(postFXCards[s]->getKnob(2).getValue());
            float eqGDb = (eqGNorm - 0.5f) * 48.0f;
            float eqDjNorm = static_cast<float>(postFXCards[s]->getKnob(3).getValue());
            vizCard.getOscilloscope().updateEqParams(eqFHz, eqWOct, eqGDb, eqDjNorm);
        } else {
            vizCard.getOscilloscope().setPlotMode(MiniOscilloscopeComponent::PlotMode::Oscilloscope);
            audioProcessor.getEngine().getScopeData(activeBlock, scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
            vizCard.getOscilloscope().updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
        }
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

    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG FARMER", 14, 0, 180, 36, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff4a9eff));
#ifdef JucePlugin_VersionString
    juce::String verStr = "v" JucePlugin_VersionString;
#else
    juce::String verStr = "v0.2.0";
#endif
#ifdef TKF_FEATURE_TAG
    if (juce::String(TKF_FEATURE_TAG).isNotEmpty()) {
        verStr += juce::String(TKF_FEATURE_TAG);
    }
#endif
    int verWidth = juce::GlyphArrangement::getStringWidthInt(g.getCurrentFont(), verStr) + 8;
    g.drawText(verStr, 196, 0, verWidth, 36, juce::Justification::centredLeft);

    int subX = 196 + verWidth + 8;
    int subtitleWidth = juce::jmax(0, getWidth() - 434 - subX);
    g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("PAGED MODULAR DUAL FM SYNTHESIS DRUM VOICE", subX, 0, subtitleWidth, 36, juce::Justification::centredLeft);
}

void TheKlangFarmerAudioProcessorEditor::resized() {
    tooltipsButton.setBounds(getWidth() - 424, 5, 72, 26);
    guideButton.setBounds(getWidth() - 346, 5, 90, 26);
    initButton.setBounds(getWidth() - 246, 5, 90, 26);
    triggerButton.setBounds(getWidth() - 146, 5, 136, 26);
    quickstartGuide.setBounds(getLocalBounds());

    updatePageLayout();
}

void TheKlangFarmerAudioProcessorEditor::handleCardInteraction(juce::Component* comp) {
    if (vizCard.getIsLocked()) return;
    if (comp == nullptr) return;

    auto isInside = [](juce::Component* c, juce::Component* target) {
        if (!target) return false;
        while (c != nullptr) {
            if (c == target) return true;
            c = c->getParentComponent();
        }
        return false;
    };

    if (isInside(comp, cardCarrier1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER1, "CARRIER 1");
    } else if (isInside(comp, cardMod1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, "MODULATOR 1");
    } else if (isInside(comp, cardPitchEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, "PITCH ENV 1");
    } else if (isInside(comp, cardFilter1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER1, "FILTER 1");
    } else if (isInside(comp, cardFilterEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, "FILTER ENV 1");
    } else if (isInside(comp, cardCarrier2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER2, "CARRIER 2");
    } else if (isInside(comp, cardMod2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, "MODULATOR 2");
    } else if (isInside(comp, cardPitchEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, "PITCH ENV 2");
    } else if (isInside(comp, cardFilter2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER2, "FILTER 2");
    } else if (isInside(comp, cardFilterEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, "FILTER ENV 2");
    } else if (isInside(comp, cardNoise.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_NOISE, "NOISE");
    } else if (isInside(comp, cardFilter3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTER3, "FILTER 3");
    } else if (isInside(comp, cardFilterEnv3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, "FILTER ENV 3");
    } else if (isInside(comp, cardMixer.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MIXER, "MIXER");
    } else if (isInside(comp, cardAmp.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMP, "AMPLIFIER");
    } else if (isInside(comp, cardAmpEnv.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_AMPENV, "AMP ENV");
    } else if (isInside(comp, cardPreLimiter.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, "PRE LIMITER");
    } else if (isInside(comp, cardPostLimiter.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, "MASTER LIMITER");
    } else if (isInside(comp, cardVelocity.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_VELOCITY, "VELOCITY");
    } else if (isInside(comp, cardKeyTrack.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, "KEY TRACK");
    } else if (isInside(comp, cardSlop.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_SLOP, "SLOP");
    } else if (isInside(comp, cardModEnv1.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV1, "MOD ENV 1");
    } else if (isInside(comp, cardModEnv2.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV2, "MOD ENV 2");
    } else if (isInside(comp, cardModEnv3.get())) {
        vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_MODENV3, "MOD ENV 3");
    } else {
        for (int s = 0; s < 4; ++s) {
            if (isInside(comp, preFXCards[s].get())) {
                vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_PRE_FX_1 + s,
                                           "PRE " + juce::String(s + 1) + ": " + preFXCards[s]->getTitle());
                return;
            }
            if (isInside(comp, postFXCards[s].get())) {
                vizCard.setVisualizedBlock(TbdAudio::ModularDrumEngine::BLK_POST_FX_1 + s,
                                           "POST " + juce::String(s + 1) + ": " + postFXCards[s]->getTitle());
                return;
            }
        }
    }
}

void TheKlangFarmerAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    handleCardInteraction(e.eventComponent);
}

void TheKlangFarmerAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    handleCardInteraction(e.eventComponent);
}

void TheKlangFarmerAudioProcessorEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& /*d*/) {
    handleCardInteraction(e.eventComponent);
}









