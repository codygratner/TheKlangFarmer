#include "PlanterEditor.h"

// --- PLANTER HEADER VISUALIZER ---

PlanterHeaderVisualizer::PlanterHeaderVisualizer() {
    points.resize(128, 0.0f);
}

void PlanterHeaderVisualizer::updateData(const float* scopeData, int numPoints, float peakL, float peakR) {
    if (scopeData && numPoints > 0) {
        int targetPoints = 128;
        if (static_cast<int>(points.size()) != targetPoints) points.resize(targetPoints);
        float step = static_cast<float>(numPoints) / static_cast<float>(targetPoints);
        for (int i = 0; i < targetPoints; ++i) {
            int srcIdx = std::clamp(static_cast<int>(i * step), 0, numPoints - 1);
            points[i] = scopeData[srcIdx];
        }
    }
    livePeakL = peakL;
    livePeakR = peakR;
    repaint();
}

void PlanterHeaderVisualizer::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff12151d));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff252b3b));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Left area: Oscilloscope (width - 34px)
    auto scopeArea = bounds.removeFromLeft(bounds.getWidth() - 34.0f).reduced(2.0f);
    g.setColour(juce::Colour(0x2200d2ff));
    g.drawHorizontalLine(static_cast<int>(scopeArea.getCentreY()), scopeArea.getX(), scopeArea.getRight());

    if (!points.empty()) {
        juce::Path p;
        float midY = scopeArea.getCentreY();
        float halfH = scopeArea.getHeight() * 0.45f;
        float stepX = scopeArea.getWidth() / static_cast<float>(points.size() - 1);

        for (size_t i = 0; i < points.size(); ++i) {
            float x = scopeArea.getX() + static_cast<float>(i) * stepX;
            float y = midY - std::clamp(points[i], -1.0f, 1.0f) * halfH;
            if (i == 0) p.startNewSubPath(x, y);
            else        p.lineTo(x, y);
        }

        g.setColour(juce::Colour(0xff00e5ff));
        g.strokePath(p, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved));
    }

    // Right area: Stereo Peak Meters (L & R)
    auto meterArea = bounds.reduced(3.0f, 3.0f);
    float barW = (meterArea.getWidth() - 2.0f) * 0.5f;

    auto drawMeterBar = [&](float x, float peakVal) {
        juce::Rectangle<float> barBg(x, meterArea.getY(), barW, meterArea.getHeight());
        g.setColour(juce::Colour(0xff1a1d26));
        g.fillRect(barBg);

        float fillH = std::clamp(peakVal, 0.0f, 1.0f) * meterArea.getHeight();
        if (fillH > 0.5f) {
            juce::Rectangle<float> fillRect(x, meterArea.getBottom() - fillH, barW, fillH);
            juce::Colour col = (peakVal > 0.95f) ? juce::Colour(0xffe53935) :
                               ((peakVal > 0.75f) ? juce::Colour(0xffffb300) : juce::Colour(0xff00e676));
            g.setColour(col);
            g.fillRect(fillRect);
        }
    };

    drawMeterBar(meterArea.getX(), livePeakL);
    drawMeterBar(meterArea.getX() + barW + 2.0f, livePeakR);
}

// --- THE KLANG PLANTER AUDIO PROCESSOR EDITOR ---

TheKlangPlanterAudioProcessorEditor::TheKlangPlanterAudioProcessorEditor(TheKlangPlanterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      carrierTrackingSelector(juce::Colour(0xff00d2ff)),
      modTrackSelector(juce::Colour(0xffff7043)),
      modTypeSelector(juce::Colour(0xffff7043)),
      pitchEnvTargetSelector(juce::Colour(0xffffab00)),
      filterTypeSelector(juce::Colour(0xff7c4dff)),
      filterSlopeSelector(juce::Colour(0xff7c4dff)),
      ampLimiterSelector(juce::Colour(0xffe53935))
{
    setLookAndFeel(&knobLookAndFeel);
    scopeBuffer.resize(512, 0.0f);

    // Header Visualizer
    addAndMakeVisible(headerViz);

    // Header Initialize Button
    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222736));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.onClick = [this]() {
        audioProcessor.getEngine().setDefaultParameters();
        // Reset APVTS parameters to defaults
        auto& state = audioProcessor.apvts;
        for (auto* param : audioProcessor.getParameters()) {
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param)) {
                p->setValueNotifyingHost(p->getDefaultValue());
            }
        }
    };
    addAndMakeVisible(initButton);

    // Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00d2ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    // --- 1. CARRIER ---
    cardCarrier = std::make_unique<ModuleCardComponent>("Carrier", juce::Colour(0xff00d2ff));
    setupBox(carrierTrackingBox);
    bindSelector(carrierTrackingSelector, carrierTrackingBox, "planter_carrier_tracking", { "MIDI", "Freq", "Note" }, 3);
    cardCarrier->setLedSelector(&carrierTrackingSelector);

    setupKnob(carrierPitchSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrierPitchSlider.customFormatText = [this](double val) {
        int mode = carrierTrackingBox.getSelectedItemIndex();
        if (mode == 0) return formatSemi24(val);
        if (mode == 1) return formatCarrierFreqHz(val);
        return formatNoteDetail(val);
    };
    carrierPitchSlider.customParseText = [this](const juce::String& text) {
        int mode = carrierTrackingBox.getSelectedItemIndex();
        if (mode == 0) return parseSemi24(text);
        if (mode == 1) return parseCarrierFreqHz(text);
        return parseNoteDetail(text);
    };

    setupKnob(carrierShapeSlider, juce::Colour(0xff00d2ff), false, 0.0);
    carrierShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    carrierShapeSlider.customFormatText = formatPercent;
    carrierShapeSlider.customParseText  = parsePercent;

    setupKnob(carrierDepthSlider, juce::Colour(0xff00d2ff), true, 0.5);
    carrierDepthSlider.customFormatText = formatBipolarPercent;
    carrierDepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier->setKnob(0, "Offset", &carrierPitchSlider);
    cardCarrier->setKnob(1, "Shape", &carrierShapeSlider);
    cardCarrier->setKnob(2, "Mod Depth", &carrierDepthSlider);
    addAndMakeVisible(cardCarrier.get());

    // --- 2. MODULATOR ---
    cardMod = std::make_unique<ModuleCardComponent>("Modulator", juce::Colour(0xffff7043));
    setupBox(modTrackBox);
    bindSelector(modTrackSelector, modTrackBox, "planter_mod_track", { "Fixed", "Follow", "FM" }, 3);
    setupBox(modTypeBox);
    bindSelector(modTypeSelector, modTypeBox, "planter_mod_type", { "Osc", "Cyclic", "Noise" }, 3);
    cardMod->setLedSelector(&modTrackSelector);
    cardMod->setSecondLedSelector(&modTypeSelector);

    setupKnob(modShapeSlider, juce::Colour(0xffff7043), false, 0.0);
    modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    modShapeSlider.customFormatText = formatPercent;
    modShapeSlider.customParseText  = parsePercent;

    setupKnob(modSpeedSlider, juce::Colour(0xffff7043), false, 0.50934);
    modSpeedSlider.customFormatText = [this](double val) {
        if (modTypeBox.getSelectedItemIndex() == 2) return formatFreqHz(val);
        if (modTrackBox.getSelectedItemIndex() == 2) return formatRatio(val);
        if (modTrackBox.getSelectedItemIndex() == 1) return formatSemi(val);
        return formatFreqHz(val);
    };
    modSpeedSlider.customParseText = [this](const juce::String& text) {
        if (modTypeBox.getSelectedItemIndex() == 2) return parseFreqHz(text);
        if (modTrackBox.getSelectedItemIndex() == 2) return parseRatio(text);
        if (modTrackBox.getSelectedItemIndex() == 1) return parseSemi(text);
        return parseFreqHz(text);
    };

    cardMod->setKnob(0, "Shape", &modShapeSlider);
    cardMod->setKnob(1, "Speed", &modSpeedSlider);
    addAndMakeVisible(cardMod.get());

    // --- 3. PITCH ENVELOPE ---
    cardPitchEnv = std::make_unique<ModuleCardComponent>("Pitch Env", juce::Colour(0xffffab00));
    setupBox(pitchEnvTargetBox);
    bindSelector(pitchEnvTargetSelector, pitchEnvTargetBox, "planter_pitchenv_target", { "Car", "Mod", "Both", "Opp" }, 4);
    cardPitchEnv->setSelectorAtBottom(true);
    cardPitchEnv->setLedSelector(&pitchEnvTargetSelector);

    setupKnob(pitchEnvSlopeSlider, juce::Colour(0xffffab00), false, 0.5886);
    pitchEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnvSlopeSlider.customFormatText = formatSlope;
    pitchEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnvDepthSlider, juce::Colour(0xffffab00), true, 0.5);
    pitchEnvDepthSlider.customFormatText = formatOctaves;
    pitchEnvDepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnvDecaySlider, juce::Colour(0xffffab00), false, 0.3806);
    pitchEnvDecaySlider.customFormatText = formatTimeMs;
    pitchEnvDecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv->setKnob(0, "Slope", &pitchEnvSlopeSlider);
    cardPitchEnv->setKnob(1, "Depth", &pitchEnvDepthSlider);
    cardPitchEnv->setKnob(2, "Decay", &pitchEnvDecaySlider);
    addAndMakeVisible(cardPitchEnv.get());

    // --- 4. NOISE TRANSIENT ---
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
    cardNoise->setKnob(1, "DJ Filter", &noiseFilterSlider);
    cardNoise->setKnob(2, "Drive", &noiseDriveSlider);
    cardNoise->setKnob(3, "Decay", &noiseDecaySlider);
    addAndMakeVisible(cardNoise.get());

    // --- 5. FILTER ---
    cardFilter = std::make_unique<ModuleCardComponent>("Filter", juce::Colour(0xff7c4dff));
    setupBox(filterTypeBox);
    bindSelector(filterTypeSelector, filterTypeBox, "planter_filter_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filterSlopeBox);
    bindSelector(filterSlopeSelector, filterSlopeBox, "planter_filter_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter->setLedSelector(&filterTypeSelector);
    cardFilter->setSecondLedSelector(&filterSlopeSelector);

    setupKnob(filterCutoffSlider, juce::Colour(0xff7c4dff), false, 1.0);
    filterCutoffSlider.customFormatText = formatFreqHz;
    filterCutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filterResoSlider, juce::Colour(0xff7c4dff), false, 0.0);
    filterResoSlider.customFormatText = formatPercent;
    filterResoSlider.customParseText  = parsePercent;

    cardFilter->setKnob(0, "Cutoff", &filterCutoffSlider);
    cardFilter->setKnob(1, "Resonance", &filterResoSlider);
    addAndMakeVisible(cardFilter.get());

    // --- 6. FILTER ENVELOPE (with Crossfader at Knob 4) ---
    cardFilterEnv = std::make_unique<ModuleCardComponent>("Filter Env", juce::Colour(0xff7c4dff));
    setupKnob(filterEnvSlopeSlider, juce::Colour(0xff7c4dff), false, 0.5886);
    filterEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnvSlopeSlider.customFormatText = formatSlope;
    filterEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnvDepthSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnvDepthSlider.customFormatText = formatFilterOctaves;
    filterEnvDepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnvDecaySlider, juce::Colour(0xff7c4dff), false, 0.3806);
    filterEnvDecaySlider.customFormatText = formatTimeMs;
    filterEnvDecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnvCrossfadeSlider, juce::Colour(0xff7c4dff), true, 0.5);
    filterEnvCrossfadeSlider.customFormatText = formatCrossfade;
    filterEnvCrossfadeSlider.customParseText  = parseCrossfade;

    cardFilterEnv->setKnob(0, "Slope", &filterEnvSlopeSlider);
    cardFilterEnv->setKnob(1, "Depth", &filterEnvDepthSlider);
    cardFilterEnv->setKnob(2, "Decay", &filterEnvDecaySlider);
    cardFilterEnv->setKnob(3, "FM / Noise", &filterEnvCrossfadeSlider);
    addAndMakeVisible(cardFilterEnv.get());

    // --- 7. AMPLIFIER (Level 0..200%, Limiter bypass/limit) ---
    cardAmp = std::make_unique<ModuleCardComponent>("Amplifier", juce::Colour(0xff00e5ff));
    setupBox(ampLimiterBox);
    bindSelector(ampLimiterSelector, ampLimiterBox, "planter_amp_limiter", { "bypass", "limit" }, 2);
    ampLimiterSelector.setAccent(juce::Colour(0xffe53935));
    cardAmp->setLedSelector(&ampLimiterSelector);

    setupKnob(ampLevelSlider, juce::Colour(0xff00e5ff), false, 0.5);
    ampLevelSlider.customFormatText = formatPercent200;
    ampLevelSlider.customParseText  = parsePercent200;

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
    addAndMakeVisible(cardAmp.get());

    // --- 8. AMP ENVELOPE ---
    cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Envelope", juce::Colour(0xff00e5ff));
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
    addAndMakeVisible(cardAmpEnv.get());

    // Connect slider attachments
    bindSlider("planter_carrier_pitch", carrierPitchSlider);
    bindSlider("planter_carrier_shape", carrierShapeSlider);
    bindSlider("planter_carrier_depth", carrierDepthSlider);

    bindSlider("planter_mod_shape", modShapeSlider);
    bindSlider("planter_mod_speed", modSpeedSlider);

    bindSlider("planter_pitchenv_slope", pitchEnvSlopeSlider);
    bindSlider("planter_pitchenv_depth", pitchEnvDepthSlider);
    bindSlider("planter_pitchenv_decay", pitchEnvDecaySlider);

    bindSlider("planter_noise_sh_rate", noiseShRateSlider);
    bindSlider("planter_noise_filter", noiseFilterSlider);
    bindSlider("planter_noise_drive", noiseDriveSlider);
    bindSlider("planter_noise_decay", noiseDecaySlider);

    bindSlider("planter_filter_cutoff", filterCutoffSlider);
    bindSlider("planter_filter_reso", filterResoSlider);

    bindSlider("planter_filterenv_slope", filterEnvSlopeSlider);
    bindSlider("planter_filterenv_depth", filterEnvDepthSlider);
    bindSlider("planter_filterenv_decay", filterEnvDecaySlider);
    bindSlider("planter_filterenv_crossfade", filterEnvCrossfadeSlider);

    bindSlider("planter_amp_level", ampLevelSlider);
    bindSlider("planter_amp_pan", ampPanSlider);
    bindSlider("planter_amp_drive", ampDriveSlider);

    bindSlider("planter_ampenv_claps", ampEnvClapsSlider);
    bindSlider("planter_ampenv_clapspeed", ampEnvClapSpeedSlider);
    bindSlider("planter_ampenv_slope", ampEnvSlopeSlider);
    bindSlider("planter_ampenv_decay", ampEnvDecaySlider);

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_carrier_tracking", carrierTrackingBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_mod_track", modTrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_mod_type", modTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_pitchenv_target", pitchEnvTargetBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_filter_type", filterTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_filter_slope", filterSlopeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_amp_limiter", ampLimiterBox));

    setSize(1040, 740);
    setResizable(true, true);
    setResizeLimits(800, 560, 2400, 1600);

    startTimerHz(60);
}

TheKlangPlanterAudioProcessorEditor::~TheKlangPlanterAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void TheKlangPlanterAudioProcessorEditor::setupKnob(RotaryKnobSlider& slider, juce::Colour accent, bool bipolar, double defaultValue) {
    slider.setAccentColour(accent);
    slider.setBipolar(bipolar);
    slider.setDoubleClickReturnValue(true, defaultValue);
    slider.getDefaultValue = [defaultValue]() { return defaultValue; };
    slider.getModInfoFunc = [this](const juce::String& paramId) {
        return audioProcessor.getParamModulationInfo(paramId);
    };
}

void TheKlangPlanterAudioProcessorEditor::setupBox(juce::ComboBox& box) {
    box.setVisible(false);
    addChildComponent(&box);
}

void TheKlangPlanterAudioProcessorEditor::bindSlider(const juce::String& paramId, RotaryKnobSlider& slider) {
    slider.setParamId(paramId);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, slider));
}

void TheKlangPlanterAudioProcessorEditor::bindSelector(LedSelectorComponent& sel, juce::ComboBox& box,
                                                       const juce::String& /*paramId*/, const juce::StringArray& items, int columns) {
    box.clear(juce::dontSendNotification);
    for (int i = 0; i < items.size(); ++i) {
        box.addItem(items[i], i + 1);
    }
    sel.setItems(items, columns);
    sel.onChange = [&box](int index) {
        box.setSelectedItemIndex(index, juce::sendNotification);
    };
    box.onChange = [&box, &sel]() {
        sel.setSelectedIndex(box.getSelectedItemIndex(), juce::dontSendNotification);
    };
}

void TheKlangPlanterAudioProcessorEditor::syncSelector(juce::ComboBox& box, LedSelectorComponent& sel,
                                                       const juce::String& paramId, int& lastVal) {
    if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
        int curVal = static_cast<int>(std::round(param->getValue() * static_cast<float>(sel.getNumItems() - 1)));
        if (curVal != lastVal) {
            lastVal = curVal;
            sel.setSelectedIndex(curVal, juce::dontSendNotification);
            box.setSelectedItemIndex(curVal, juce::dontSendNotification);
        }
    }
}

void TheKlangPlanterAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0d0f14));

    // Header bar
    g.setColour(juce::Colour(0xff151821));
    g.fillRect(0, 0, getWidth(), 36);

    g.setColour(juce::Colour(0xff222736));
    g.drawHorizontalLine(36, 0.0f, static_cast<float>(getWidth()));

    // Title
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("THE KLANG PLANTER", 14, 0, 190, 36, juce::Justification::centredLeft);

    // Version
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff4a9eff));
    g.drawText("v0.1.6", 206, 0, 48, 36, juce::Justification::centredLeft);

    // Subtitle
    int subtitleWidth = juce::jmax(0, getWidth() - 480 - 260);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText("COMPACT FM PERCUSSION SYNTHESIZER", 260, 0, subtitleWidth, 36, juce::Justification::centredLeft);
}

void TheKlangPlanterAudioProcessorEditor::resized() {
    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin;

    int numCols = 4;
    int numRows = 2;
    int slotW = (totalW - (numCols - 1) * margin) / numCols;
    int slotH = (totalH - (numRows - 1) * margin) / numRows;

    auto getSlotBounds = [margin, topOffset, slotW, slotH](int col, int row) {
        int x = margin + col * (slotW + margin);
        int y = topOffset + row * (slotH + margin);
        return juce::Rectangle<int>(x, y, slotW, slotH);
    };

    // Header controls
    headerViz.setBounds(getWidth() - 436, 5, 180, 26);
    initButton.setBounds(getWidth() - 246, 5, 90, 26);
    triggerButton.setBounds(getWidth() - 146, 5, 136, 26);

    // Top Row: [0] Carrier, [1] Modulator, [2] Pitch Env, [3] Noise Transient
    if (cardCarrier)  cardCarrier->setBounds(getSlotBounds(0, 0));
    if (cardMod)      cardMod->setBounds(getSlotBounds(1, 0));
    if (cardPitchEnv) cardPitchEnv->setBounds(getSlotBounds(2, 0));
    if (cardNoise)    cardNoise->setBounds(getSlotBounds(3, 0));

    // Bottom Row: [0] Filter, [1] Filter Env, [2] Amplifier, [3] Amp Envelope
    if (cardFilter)    cardFilter->setBounds(getSlotBounds(0, 1));
    if (cardFilterEnv) cardFilterEnv->setBounds(getSlotBounds(1, 1));
    if (cardAmp)       cardAmp->setBounds(getSlotBounds(2, 1));
    if (cardAmpEnv)    cardAmpEnv->setBounds(getSlotBounds(3, 1));
}

void TheKlangPlanterAudioProcessorEditor::timerCallback() {
    // 1. Sync selectors
    syncSelector(carrierTrackingBox, carrierTrackingSelector, "planter_carrier_tracking", lastCarrierTracking);
    syncSelector(modTrackBox, modTrackSelector, "planter_mod_track", lastModTrack);
    syncSelector(modTypeBox, modTypeSelector, "planter_mod_type", lastModType);
    syncSelector(pitchEnvTargetBox, pitchEnvTargetSelector, "planter_pitchenv_target", lastPitchEnvTarget);
    syncSelector(filterTypeBox, filterTypeSelector, "planter_filter_type", lastFilterType);
    syncSelector(filterSlopeBox, filterSlopeSelector, "planter_filter_slope", lastFilterSlope);
    syncSelector(ampLimiterBox, ampLimiterSelector, "planter_amp_limiter", lastAmpLimiter);

    // 2. Fetch live oscilloscope data & peak levels
    audioProcessor.getEngine().getScopeData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
    float peakL = audioProcessor.getEngine().getPeakL();
    float peakR = audioProcessor.getEngine().getPeakR();

    headerViz.updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()), peakL, peakR);
}
