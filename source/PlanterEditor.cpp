#include "PlanterEditor.h"
#include "ParameterManager.h"

// --- PLANTER HEADER VISUALIZER ---

PlanterHeaderVisualizer::PlanterHeaderVisualizer()
{
    setOpaque(true);
    points.resize(64, 0.0f);
    limitText = RlyehSound::ParameterManager::getInstance().getGlobalString("badge_limit", "LIMIT");
    setTooltip("MASTER MONITOR: Real-time stereo output oscilloscope, peak level meters, and brickwall limiter activity indicator.");
}

void PlanterHeaderVisualizer::updateData(const float* scopeData, int numPoints, float peakL, float peakR, float limiterActivity)
{
    bool isSilent = (peakL < 0.001f && peakR < 0.001f && limiterActivity < 0.001f);
    if (isSilent && livePeakL < 0.001f && livePeakR < 0.001f && liveLimiterAct < 0.001f) {
        bool hasSignal = false;
        for (float pt : points) {
            if (std::abs(pt) > 0.001f) {
                hasSignal = true;
                break;
            }
        }
        if (!hasSignal) return;
    }

    if (scopeData && numPoints > 0) {
        int targetPoints = 64;
        if (static_cast<int>(points.size()) != targetPoints) points.resize(targetPoints);
        float step = static_cast<float>(numPoints) / static_cast<float>(targetPoints);
        for (int i = 0; i < targetPoints; ++i) {
            int srcIdx = std::clamp(static_cast<int>(i * step), 0, numPoints - 1);
            points[i] = scopeData[srcIdx];
        }
    }
    livePeakL = peakL;
    livePeakR = peakR;
    liveLimiterAct = limiterActivity;
    repaint();
}

juce::Rectangle<float> PlanterHeaderVisualizer::getScopeArea() const {
    auto b = getLocalBounds().toFloat();
    return b.removeFromLeft(b.getWidth() - 76.0f).reduced(2.0f);
}

juce::Rectangle<float> PlanterHeaderVisualizer::getLimitArea() const {
    auto b = getLocalBounds().toFloat();
    b.removeFromLeft(b.getWidth() - 76.0f);
    return b.removeFromLeft(38.0f).reduced(3.0f, 4.0f);
}

juce::Rectangle<float> PlanterHeaderVisualizer::getMeterArea() const {
    auto b = getLocalBounds().toFloat();
    return b.removeFromRight(38.0f).reduced(3.0f, 3.0f);
}

void PlanterHeaderVisualizer::triggerFlash() {
    isFlashing = true;
    repaint();
    juce::Timer::callAfterDelay(150, [safe = juce::Component::SafePointer<PlanterHeaderVisualizer>(this)] {
        if (safe) {
            safe->isFlashing = false;
            safe->repaint();
        }
    });
}

void PlanterHeaderVisualizer::mouseDown(const juce::MouseEvent& e) {
    auto pos = e.position;
    if (getMeterArea().contains(pos)) {
        triggerFlash();
        if (onPanic) onPanic();
    } else if (getLimitArea().expanded(2.0f, 5.0f).contains(pos)) {
        if (onLimiterCalloutRequested) {
            onLimiterCalloutRequested(getLimitArea().toNearestInt());
        }
    }
}

void PlanterHeaderVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Chassis background fill for opaque component
    g.fillAll(juce::Colour(0xff151821));

    // Background card
    g.setColour(juce::Colour(0xff12151d));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff252b3b));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // 1. Left area: Oscilloscope
    auto scopeArea = getScopeArea();
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
        g.strokePath(p, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 2. Center area: LIMIT warning indicator badge
    auto limitArea = getLimitArea();
    bool isLimiting = (liveLimiterAct > 0.01f);
    if (isLimiting) {
        float alpha = std::clamp(liveLimiterAct * 2.0f, 0.4f, 1.0f);
        g.setColour(juce::Colour(0xffff1744).withAlpha(alpha * 0.35f));
        g.fillRoundedRectangle(limitArea, 3.0f);
        g.setColour(juce::Colour(0xffff1744).withAlpha(alpha));
        g.drawRoundedRectangle(limitArea, 3.0f, 1.2f);
        g.setColour(juce::Colours::white);
    } else {
        g.setColour(juce::Colour(0xff181b24));
        g.fillRoundedRectangle(limitArea, 3.0f);
        g.setColour(juce::Colour(0xff2a3040));
        g.drawRoundedRectangle(limitArea, 3.0f, 1.0f);
        g.setColour(juce::Colour(0x558899aa));
    }
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText(limitText, limitArea, juce::Justification::centred, false);

    // 3. Right area: Stereo Peak Meters (L & R)
    auto meterArea = getMeterArea();
    if (isFlashing) {
        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle(meterArea, 2.0f);
    } else {
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
}

// --- PLANTER LIMITER CALLOUT COMPONENT ---

PlanterLimiterCalloutComponent::PlanterLimiterCalloutComponent(TheKlangPlanterAudioProcessor& p)
    : audioProcessor(p)
{
    setSize(300, 130);

    // 1. Enable Selector & ComboBox
    enableBox.clear(juce::dontSendNotification);
    enableBox.addItem("Off", 1);
    enableBox.addItem("On", 2);
    enableBox.setVisible(false);
    addChildComponent(enableBox);

    enableSelector.setItems({ "Off", "On" }, 1);
    enableSelector.setParamId("planter_limiter_enable");
    enableSelector.setTooltip("ENABLE LIMITER: Toggle master brickwall safety limiter.");
    enableSelector.setItemTooltips(TooltipHelper::getLedSelectorItemTooltips("toggle"));
    enableSelector.onChange = [this](int idx) {
        enableBox.setSelectedId(idx + 1, juce::sendNotificationSync);
        if (auto* param = audioProcessor.apvts.getParameter("planter_limiter_enable")) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
        }
    };
    enableBox.onChange = [this]() {
        enableSelector.setSelectedIndex(enableBox.getSelectedItemIndex(), juce::dontSendNotification);
    };
    addAndMakeVisible(enableSelector);

    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "planter_limiter_enable", enableBox);
    enableSelector.setSelectedIndex(enableBox.getSelectedItemIndex(), juce::dontSendNotification);

    // 2. Gain Slider
    gainSlider.setAccentColour(juce::Colour(0xffe53935));
    gainSlider.setLabel("");
    gainSlider.setParamId("planter_limiter_gain");
    gainSlider.setDoubleClickReturnValue(true, 12.0 / 36.0);
    gainSlider.getDefaultValue = []() { return 12.0 / 36.0; };
    gainSlider.customFormatText = formatLimiterGain;
    gainSlider.customParseText  = parseLimiterGain;
    gainSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_gain", "Master limiter makeup and input boost gain", false));
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "planter_limiter_gain", gainSlider);

    // 3. Ceiling (Threshold) Slider
    threshSlider.setAccentColour(juce::Colour(0xffe53935));
    threshSlider.setLabel("");
    threshSlider.setParamId("planter_limiter_thresh");
    threshSlider.setDoubleClickReturnValue(true, 1.0);
    threshSlider.getDefaultValue = []() { return 1.0; };
    threshSlider.customFormatText = formatLimiterThresh;
    threshSlider.customParseText  = parseLimiterThresh;
    threshSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_thresh", "Master limiter brickwall ceiling peak threshold", false));
    addAndMakeVisible(threshSlider);
    threshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "planter_limiter_thresh", threshSlider);

    // 4. Release Slider
    releaseSlider.setAccentColour(juce::Colour(0xffe53935));
    releaseSlider.setLabel("");
    releaseSlider.setParamId("planter_limiter_release");
    releaseSlider.setDoubleClickReturnValue(true, 0.6296);
    releaseSlider.getDefaultValue = []() { return 0.6296; };
    releaseSlider.customFormatText = formatLimiterRelease;
    releaseSlider.customParseText  = parseLimiterRelease;
    releaseSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_release", "Master limiter gain reduction recovery release time", false));
    addAndMakeVisible(releaseSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "planter_limiter_release", releaseSlider);
}

void PlanterLimiterCalloutComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Chassis background & border
    g.setColour(juce::Colour(0xff141720));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(0xffe53935).withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.2f);

    // Header bar
    auto headerArea = bounds.removeFromTop(24.0f);
    g.setColour(juce::Colour(0xff1d222e));
    g.fillRect(headerArea);
    g.setColour(juce::Colour(0xffe53935));
    g.drawHorizontalLine(24, 0.0f, bounds.getWidth());

    // Red status indicator dot
    g.setColour(juce::Colour(0xffe53935));
    g.fillEllipse(10.0f, 8.0f, 8.0f, 8.0f);

    // Title
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xfff1f5f9));
    g.drawText("MASTER LIMITER", 24, 0, getWidth() - 30, 24, juce::Justification::centredLeft, true);

    // Column header labels
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffcfd8dc));

    auto drawColLabel = [&](const juce::String& text, const juce::Component& comp) {
        g.drawText(text, comp.getX(), 27, comp.getWidth(), 16, juce::Justification::centred, true);
    };

    drawColLabel("LIMIT", enableSelector);
    drawColLabel("GAIN", gainSlider);
    drawColLabel("CEIL", threshSlider);
    drawColLabel("REL", releaseSlider);
}

void PlanterLimiterCalloutComponent::resized() {
    int topY = 46;
    int h = getHeight() - topY - 10;

    enableSelector.setBounds(10, topY, 46, h);

    int knobW = 72;
    int startX = 64;
    int gap = (getWidth() - startX - knobW * 3) / 3;

    gainSlider.setBounds(startX, topY, knobW, h);
    threshSlider.setBounds(startX + knobW + gap, topY, knobW, h);
    releaseSlider.setBounds(startX + (knobW + gap) * 2, topY, knobW, h);
}

// --- THE KLANG PLANTER AUDIO PROCESSOR EDITOR ---

TheKlangPlanterAudioProcessorEditor::TheKlangPlanterAudioProcessorEditor(TheKlangPlanterAudioProcessor& p)
    : KlangCoreEditor(p), audioProcessor(p),
      carrierTrackingSelector(juce::Colour(0xffff3b30)),
      modTrackSelector(juce::Colour(0xff00d2ff)),
      modTypeSelector(juce::Colour(0xff00d2ff)),
      pitchEnvTargetSelector(juce::Colour(0xffcfd8dc)),
      filterTypeSelector(juce::Colour(0xff2979ff)),
      filterSlopeSelector(juce::Colour(0xff2979ff)),
      limiterEnableSelector(juce::Colour(0xffe53935))
{
    setLookAndFeel(&knobLookAndFeel);
    scopeBuffer.resize(512, 0.0f);

    // Header Tooltips Toggle Button
    tooltipsButton.setTooltip("TOOLTIPS — Toggle hover parameter and control tooltips on or off.");
    tooltipsButton.onClick = [this]() {
        setTooltipsEnabled(!tooltipsEnabled);
    };
    addAndMakeVisible(tooltipsButton);
    setTooltipsEnabled(false);

    // Header Visualizer
    addAndMakeVisible(headerViz);

    // Header Initialize Button
    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222736));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc5d1e8));
    initButton.setTooltip("INIT — Reset all parameters to a clean default drum patch.");
    initButton.onClick = [this]() {
        audioProcessor.panic();
        audioProcessor.getEngine().setDefaultParameters();
        for (auto* param : audioProcessor.getParameters()) {
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(param)) {
                rp->setValueNotifyingHost(rp->getDefaultValue());
            }
        }
    };
    addAndMakeVisible(initButton);

    // Header Audition Trigger Button
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00e5ff));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0f1115));
    triggerButton.setTooltip("TRIGGER — Fire a manual audition drum hit at full velocity.");
    triggerButton.onClick = [this]() {
        audioProcessor.getEngine().trigger(1.0f);
    };
    addAndMakeVisible(triggerButton);

    // --- PAIR 1: CARRIER (Red) & MODULATOR (Cyan) ---
    // Carrier: Red accent with Cyan panel tint
    const juce::Colour colRed = RlyehSound::ParameterManager::getInstance().getModuleColor("carrier_accent", juce::Colour(0xffff3b30));
    const juce::Colour colCyan = RlyehSound::ParameterManager::getInstance().getModuleColor("modulator_accent", juce::Colour(0xff00d2ff));

    cardCarrier = std::make_unique<ModuleCardComponent>("Carrier", colRed);
    cardCarrier->setPanelTintBaseColour(colCyan);
    cardCarrier->setTooltip("CARRIER: Main pitched tonal oscillator with shape morphing and tracking modes.");
    setupBox(carrierTrackingBox);
    bindSelector(carrierTrackingSelector, carrierTrackingBox, "planter_carrier_tracking", { "MIDI", "Freq", "Note" }, 3);
    carrierTrackingBox.onChange = [this]() {
        carrierTrackingSelector.setSelectedIndex(carrierTrackingBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateCarrierControls();
    };
    cardCarrier->setLedSelector(&carrierTrackingSelector);

    setupKnob(carrierPitchSlider, colRed, true, 0.5);
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

    setupKnob(carrierShapeSlider, colRed, false, 0.0);
    carrierShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    carrierShapeSlider.customFormatText = formatWaveshape;
    carrierShapeSlider.customParseText  = parseWaveshape;

    setupKnob(carrierDepthSlider, colCyan, true, 0.5);
    carrierDepthSlider.customFormatText = formatBipolarPercent;
    carrierDepthSlider.customParseText  = parseBipolarPercent;

    cardCarrier->setKnob(0, "Offset", &carrierPitchSlider);
    cardCarrier->setKnob(1, "Shape", &carrierShapeSlider);
    cardCarrier->setKnob(2, "Mod Depth", &carrierDepthSlider, colCyan);
    addAndMakeVisible(cardCarrier.get());

    // Modulator: Cyan accent with Red panel tint
    cardMod = std::make_unique<ModuleCardComponent>("Modulator", colCyan);
    cardMod->setPanelTintBaseColour(colRed);
    cardMod->setTooltip("MODULATOR: FM modulation oscillator providing frequency and timbre modulation.");
    setupBox(modTrackBox);
    bindSelector(modTrackSelector, modTrackBox, "planter_mod_track", { "Fixed", "Follow", "FM" }, 3);
    modTrackBox.onChange = [this]() {
        modTrackSelector.setSelectedIndex(modTrackBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateModControls();
    };
    setupBox(modTypeBox);
    bindSelector(modTypeSelector, modTypeBox, "planter_mod_type", { "Osc", "Cyclic", "Noise" }, 3);
    modTypeBox.onChange = [this]() {
        modTypeSelector.setSelectedIndex(modTypeBox.getSelectedItemIndex(), juce::dontSendNotification);
        updateModControls();
    };
    cardMod->setLedSelector(&modTrackSelector);
    cardMod->setSecondLedSelector(&modTypeSelector);

    setupKnob(modShapeSlider, colCyan, false, 0.0);
    modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
    modShapeSlider.customFormatText = [this](double val) {
        int t = modTypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return formatBipolarPercent(val);
        return formatPercent(val);
    };
    modShapeSlider.customParseText = [this](const juce::String& text) {
        int t = modTypeBox.getSelectedItemIndex();
        if (t == 1 || t == 2) return parseBipolarPercent(text);
        return parsePercent(text);
    };

    setupKnob(modSpeedSlider, colCyan, false, 0.5);
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

    // --- PAIR 2: PITCH ENV (Silver) & NOISE TRANSIENT (Dark/Silver) ---
    // Pitch Env: Silver accent with Dark Grey panel tint
    const juce::Colour colSilver = RlyehSound::ParameterManager::getInstance().getModuleColor("pitch_env_accent", juce::Colour(0xffcfd8dc));
    const juce::Colour colDarkGrey = RlyehSound::ParameterManager::getInstance().getModuleColor("transients_accent", juce::Colour(0xff263238));

    cardPitchEnv = std::make_unique<ModuleCardComponent>("Pitch Env", colSilver);
    cardPitchEnv->setPanelTintBaseColour(colDarkGrey);
    cardPitchEnv->setTooltip("PITCH ENV: Dedicated pitch modulation envelope with routing destinations.");
    setupBox(pitchEnvTargetBox);
    bindSelector(pitchEnvTargetSelector, pitchEnvTargetBox, "planter_pitchenv_target", { "Car", "Mod", "Both", "Opp" }, 4);
    pitchEnvTargetSelector.setItemStyle(0, { colRed,  std::nullopt, std::nullopt }); // Car:  Carrier Red accent
    pitchEnvTargetSelector.setItemStyle(1, { colCyan, std::nullopt, std::nullopt }); // Mod:  Modulator Cyan accent
    pitchEnvTargetSelector.setItemStyle(2, { colCyan, colRed,       colRed       }); // Both: Red text, Cyan outline, dual halo
    pitchEnvTargetSelector.setItemStyle(3, { colRed,  colCyan,      colCyan      }); // Opp:  Cyan text, Red outline, dual halo
    cardPitchEnv->setSelectorAtBottom(true);
    cardPitchEnv->setLedSelector(&pitchEnvTargetSelector);

    setupKnob(pitchEnvSlopeSlider, colSilver, false, 0.5886);
    pitchEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    pitchEnvSlopeSlider.customFormatText = formatSlope;
    pitchEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(pitchEnvDepthSlider, colSilver, true, 0.5);
    pitchEnvDepthSlider.customFormatText = formatOctaves;
    pitchEnvDepthSlider.customParseText  = parseOctaves;

    setupKnob(pitchEnvDecaySlider, colSilver, false, 0.3806);
    pitchEnvDecaySlider.customFormatText = formatTimeMs;
    pitchEnvDecaySlider.customParseText  = parseTimeMs;

    cardPitchEnv->setKnob(0, "Slope", &pitchEnvSlopeSlider);
    cardPitchEnv->setKnob(1, "Depth", &pitchEnvDepthSlider);
    cardPitchEnv->setKnob(2, "Decay", &pitchEnvDecaySlider);
    addAndMakeVisible(cardPitchEnv.get());

    // Noise Transient: Silver panel (DoepferSilver), Dark Grey accent, with Silver highlight Crossfader at Knob 4
    cardNoise = std::make_unique<ModuleCardComponent>("Noise Transient", colDarkGrey, ModuleCardComponent::PanelStyle::DoepferSilver);
    cardNoise->setKnobsLightTrough(true);
    cardNoise->setTooltip("TRANSIENTS: Tuned sample-and-hold noise generator with transient envelope.");

    setupKnob(noiseShRateSlider, colDarkGrey, false, 1.0);
    noiseShRateSlider.setLightTrough(true);
    noiseShRateSlider.customFormatText = formatFreqHz;
    noiseShRateSlider.customParseText  = parseFreqHz;

    setupKnob(noiseFilterSlider, colDarkGrey, true, 0.5);
    noiseFilterSlider.setLightTrough(true);
    noiseFilterSlider.customFormatText = formatBipolarPercent;
    noiseFilterSlider.customParseText  = parseBipolarPercent;

    setupKnob(noiseDecaySlider, colDarkGrey, false, 0.3078);
    noiseDecaySlider.setLightTrough(true);
    noiseDecaySlider.customFormatText = formatNoiseTimeMs;
    noiseDecaySlider.customParseText  = parseNoiseTimeMs;

    // Knob 4: FM / NOISE Crossfader (Inverse colors like Pitch Env: Silver accent, dark trough)
    setupKnob(noiseCrossfadeSlider, colSilver, true, 1.0);
    noiseCrossfadeSlider.setLightTrough(false);
    noiseCrossfadeSlider.getDefaultValue = []() { return 0.5; }; // Double-click resets to 0% Both (0.5)
    noiseCrossfadeSlider.customFormatText = formatCrossfade;
    noiseCrossfadeSlider.customParseText  = parseCrossfade;

    cardNoise->setKnob(0, "S&H Rate", &noiseShRateSlider);
    cardNoise->setKnob(1, "DJ Filter", &noiseFilterSlider);
    cardNoise->setKnob(2, "Decay", &noiseDecaySlider);
    cardNoise->setKnob(3, "FM / NOISE", &noiseCrossfadeSlider, colSilver, false);
    noiseCrossfadeSlider.setAccentColour(colSilver);
    noiseCrossfadeSlider.setLightTrough(false);
    addAndMakeVisible(cardNoise.get());

    // --- PAIR 3: FILTER (Blue) & FILTER ENV (Amber) ---
    const juce::Colour colBlue = RlyehSound::ParameterManager::getInstance().getModuleColor("filter_accent", juce::Colour(0xff2979ff));
    const juce::Colour colAmber = RlyehSound::ParameterManager::getInstance().getModuleColor("filter_env_accent", juce::Colour(0xffffa000));

    // Filter: Blue accent with Amber panel tint
    cardFilter = std::make_unique<ModuleCardComponent>("Filter", colBlue);
    cardFilter->setPanelTintBaseColour(colAmber);
    cardFilter->setTooltip("FILTER: State-variable resonant filter with multiple types and slopes.");
    setupBox(filterTypeBox);
    bindSelector(filterTypeSelector, filterTypeBox, "planter_filter_type", { "LPF", "BPF", "HPF", "BRF" }, 4);
    setupBox(filterSlopeBox);
    bindSelector(filterSlopeSelector, filterSlopeBox, "planter_filter_slope", { "6", "12", "18", "24", "36" }, 5);
    cardFilter->setLedSelector(&filterTypeSelector);
    cardFilter->setSecondLedSelector(&filterSlopeSelector);

    setupKnob(filterCutoffSlider, colBlue, false, 1.0);
    filterCutoffSlider.customFormatText = formatFreqHz;
    filterCutoffSlider.customParseText  = parseFreqHz;

    setupKnob(filterResoSlider, colBlue, false, 0.0);
    filterResoSlider.customFormatText = formatPercent;
    filterResoSlider.customParseText  = parsePercent;

    cardFilter->setKnob(0, "Cutoff", &filterCutoffSlider);
    cardFilter->setKnob(1, "Resonance", &filterResoSlider);
    addAndMakeVisible(cardFilter.get());

    // Filter Env: Amber accent with Blue panel tint (Knob 4 is Pre-Filter Drive)
    cardFilterEnv = std::make_unique<ModuleCardComponent>("Filter Env", colAmber);
    cardFilterEnv->setPanelTintBaseColour(colBlue);
    cardFilterEnv->setTooltip("FILTER ENV: Cutoff sweep envelope and pre-filter overdrive.");
    setupKnob(filterEnvSlopeSlider, colAmber, false, 0.5886);
    filterEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    filterEnvSlopeSlider.customFormatText = formatSlope;
    filterEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(filterEnvDepthSlider, colAmber, true, 0.5);
    filterEnvDepthSlider.customFormatText = formatFilterOctaves;
    filterEnvDepthSlider.customParseText  = parseFilterOctaves;

    setupKnob(filterEnvDecaySlider, colAmber, false, 0.3806);
    filterEnvDecaySlider.customFormatText = formatTimeMs;
    filterEnvDecaySlider.customParseText  = parseTimeMs;

    setupKnob(filterEnvDriveSlider, colBlue, false, 0.5);
    filterEnvDriveSlider.customFormatText = [](double val) {
        float db = (val <= 0.5) ? static_cast<float>(-6.0 + val * 12.0)
                                : static_cast<float>((val - 0.5) * 48.0);
        if (std::abs(db) < 0.05f) return juce::String("0.0 dB");
        return (db > 0.0f ? "+" : "") + juce::String(db, 1) + " dB";
    };
    filterEnvDriveSlider.customParseText  = [](const juce::String& text) {
        double db = parseNumberSafe(text, 0.0);
        if (db <= 0.0) {
            db = std::clamp(db, -6.0, 0.0);
            return (db + 6.0) / 12.0;
        } else {
            db = std::clamp(db, 0.0, 24.0);
            return 0.5 + (db / 48.0);
        }
    };

    cardFilterEnv->setKnob(0, "Slope", &filterEnvSlopeSlider);
    cardFilterEnv->setKnob(1, "Depth", &filterEnvDepthSlider);
    cardFilterEnv->setKnob(2, "Decay", &filterEnvDecaySlider);
    cardFilterEnv->setKnob(3, "Pre-Filter Drive", &filterEnvDriveSlider, colBlue);
    addAndMakeVisible(cardFilterEnv.get());

    // --- PAIR 4: AMPLIFIER (Green) & AMP ENVELOPE (Magenta) ---
    const juce::Colour colGreen = RlyehSound::ParameterManager::getInstance().getModuleColor("amplifier_accent", juce::Colour(0xff00e676));
    const juce::Colour colMagenta = RlyehSound::ParameterManager::getInstance().getModuleColor("amp_env_accent", juce::Colour(0xffe040fb));

    // Amplifier: Green accent with Magenta panel tint
    cardAmp = std::make_unique<ModuleCardComponent>("Amplifier", colGreen);
    cardAmp->setPanelTintBaseColour(colMagenta);
    cardAmp->setTooltip("AMPLIFIER: Master saturation drive, stereo panning, and velocity scaling.");

    // Knob 0: Amp Drive (-inf..0..+24dB, default 0dB = 0.5)
    setupKnob(ampDriveSlider, colGreen, false, 0.5);
    ampDriveSlider.customFormatText = formatAmpDriveDb;
    ampDriveSlider.customParseText  = parseAmpDriveDb;

    // Knob 1: Pan (Center = 0.5)
    setupKnob(ampPanSlider, colGreen, true, 0.5);
    ampPanSlider.customFormatText = [](double val) {
        int p = static_cast<int>(std::round((val - 0.5) * 200.0));
        if (p == 0) return juce::String("Center");
        return (p < 0 ? juce::String(-p) + "% L" : juce::String(p) + "% R");
    };
    ampPanSlider.customParseText = parseBipolarPercent;

    // Knob 2: Vel Slope (default LIN 0.75, double-click EXP 0.5886)
    setupKnob(ampVelSlopeSlider, colGreen, false, 0.75);
    ampVelSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    ampVelSlopeSlider.getDefaultValue = []() { return 0.5886; }; // double-click sets EXP
    ampVelSlopeSlider.customFormatText = formatSlope;
    ampVelSlopeSlider.customParseText  = parseSlope;

    // Knob 3: Velocity Floor (1%..100%, default 50% = 0.5, dbl-click 50%)
    setupKnob(ampVelFloorSlider, colGreen, false, 0.5);
    ampVelFloorSlider.customFormatText = formatVelocityFloor;
    ampVelFloorSlider.customParseText  = parseVelocityFloor;

    cardAmp->setKnob(0, "Pre-Limiter Drive", &ampDriveSlider);
    cardAmp->setKnob(1, "Pan", &ampPanSlider);
    cardAmp->setKnob(2, "Vel Slope", &ampVelSlopeSlider);
    cardAmp->setKnob(3, "Vel Min Level", &ampVelFloorSlider);
    cardAmp->onCardMouseDown = [this](const juce::MouseEvent& e) {
        if (e.mods.isPopupMenu() || e.mods.isRightButtonDown()) {
            auto callout = std::make_unique<PlanterLimiterCalloutComponent>(audioProcessor);
            auto area = cardAmp->getBounds();
            juce::CallOutBox::launchAsynchronously(std::move(callout), area, this);
        }
    };
    addAndMakeVisible(cardAmp.get());

    // Amp Envelope: Magenta accent with Green panel tint
    cardAmpEnv = std::make_unique<ModuleCardComponent>("Amp Envelope", colMagenta);
    cardAmpEnv->setPanelTintBaseColour(colGreen);
    cardAmpEnv->setTooltip("AMP ENV: Master percussive amplitude decay envelope and clap burst generator.");
    setupKnob(ampEnvClapsSlider, colMagenta, false, 0.0);
    ampEnvClapsSlider.customFormatText = formatClaps;
    ampEnvClapsSlider.customParseText  = parseClaps;

    setupKnob(ampEnvClapSpeedSlider, colMagenta, false, 2.0 / 14.0);
    ampEnvClapSpeedSlider.customFormatText = formatClapSpeed;
    ampEnvClapSpeedSlider.customParseText  = parseClapSpeed;

    setupKnob(ampEnvSlopeSlider, colMagenta, false, 0.5886);
    ampEnvSlopeSlider.diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
    ampEnvSlopeSlider.customFormatText = formatSlope;
    ampEnvSlopeSlider.customParseText  = parseSlope;

    setupKnob(ampEnvDecaySlider, colMagenta, false, 0.3806);
    ampEnvDecaySlider.customFormatText = formatTimeMs;
    ampEnvDecaySlider.customParseText  = parseTimeMs;

    cardAmpEnv->setKnob(0, "Claps", &ampEnvClapsSlider);
    cardAmpEnv->setKnob(1, "Clap Speed", &ampEnvClapSpeedSlider);
    cardAmpEnv->setKnob(2, "Slope", &ampEnvSlopeSlider);
    cardAmpEnv->setKnob(3, "Decay", &ampEnvDecaySlider);
    addAndMakeVisible(cardAmpEnv.get());

    // 6. Limiter Card
    cardLimiter = std::make_unique<ModuleCardComponent>("Limiter", juce::Colour(0xffe53935), ModuleCardComponent::PanelStyle::DoepferSilver);
    cardLimiter->setTooltip("LIMITER \u2014 Final mastering soft-clipper to prevent digital clipping.");
    setupKnob(limiterGainSlider, juce::Colour(0xffe53935), false, 12.0 / 36.0);
    setupKnob(limiterThreshSlider, juce::Colour(0xffe53935), false, 1.0);
    setupKnob(limiterReleaseSlider, juce::Colour(0xffe53935), false, 0.6296);
    cardLimiter->setKnob(1, "Gain", &limiterGainSlider);
    cardLimiter->setKnob(2, "Threshold", &limiterThreshSlider);
    cardLimiter->setKnob(3, "Release", &limiterReleaseSlider);
    bindSelector(limiterEnableSelector, limiterEnableBox, "planter_limiter_enable", { "Off", "On" }, 2);
    limiterEnableSelector.setAccent(juce::Colour(0xffe53935));
    limiterEnableSelector.setTooltip("ENABLE LIMITER");
    cardLimiter->setLedSelector(&limiterEnableSelector);
    cardLimiter->onCardMouseDown = [this](const juce::MouseEvent& e) {
        if (e.mods.isPopupMenu() || e.mods.isRightButtonDown()) {
            auto callout = std::make_unique<PlanterLimiterCalloutComponent>(audioProcessor);
            auto area = cardLimiter->getBounds();
            juce::CallOutBox::launchAsynchronously(std::move(callout), area, this);
        }
    };
    addAndMakeVisible(cardLimiter.get());

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
    bindSlider("planter_noise_decay", noiseDecaySlider);
    bindSlider("planter_noise_crossfade", noiseCrossfadeSlider);

    bindSlider("planter_filter_cutoff", filterCutoffSlider);
    bindSlider("planter_filter_reso", filterResoSlider);

    bindSlider("planter_filterenv_slope", filterEnvSlopeSlider);
    bindSlider("planter_filterenv_depth", filterEnvDepthSlider);
    bindSlider("planter_filterenv_decay", filterEnvDecaySlider);
    bindSlider("planter_filterenv_drive", filterEnvDriveSlider);

    bindSlider("planter_amp_drive", ampDriveSlider);
    bindSlider("planter_amp_pan", ampPanSlider);
    bindSlider("planter_amp_vel_slope", ampVelSlopeSlider);
    bindSlider("planter_amp_vel_floor", ampVelFloorSlider);

    bindSlider("planter_ampenv_claps", ampEnvClapsSlider);
    bindSlider("planter_ampenv_clapspeed", ampEnvClapSpeedSlider);
    bindSlider("planter_ampenv_slope", ampEnvSlopeSlider);
    bindSlider("planter_ampenv_decay", ampEnvDecaySlider);

    bindSlider("planter_limiter_gain", limiterGainSlider);
    limiterGainSlider.customFormatText = formatLimiterGain;
    limiterGainSlider.customParseText  = parseLimiterGain;

    bindSlider("planter_limiter_thresh", limiterThreshSlider);
    limiterThreshSlider.customFormatText = formatLimiterThresh;
    limiterThreshSlider.customParseText  = parseLimiterThresh;

    bindSlider("planter_limiter_release", limiterReleaseSlider);
    limiterReleaseSlider.customFormatText = formatLimiterRelease;
    limiterReleaseSlider.customParseText  = parseLimiterRelease;

    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_carrier_tracking", carrierTrackingBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_mod_track", modTrackBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_mod_type", modTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_pitchenv_target", pitchEnvTargetBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_filter_type", filterTypeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_filter_slope", filterSlopeBox));
    boxAttachments.push_back(std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "planter_limiter_enable", limiterEnableBox));

    // Wire parameter tooltips
    carrierPitchSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_carrier_pitch", "Carrier oscillator pitch or frequency depending on tracking mode", true));
    carrierShapeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_carrier_shape", "Morph carrier waveform from sine through triangle and saw to pulse/square", false));
    carrierDepthSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_carrier_depth", "Bipolar FM modulation index/depth from Modulator into Carrier", true));

    modShapeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_mod_shape", "Modulator oscillator waveform shape morphing", false));
    modSpeedSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_mod_speed", "Modulator rate, harmonic ratio, or note pitch offset", false));

    pitchEnvSlopeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_pitchenv_slope", "Pitch decay curve contour (exponential drop to linear glide)", false));
    pitchEnvDepthSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_pitchenv_depth", "Bipolar pitch envelope modulation depth", true));
    pitchEnvDecaySlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_pitchenv_decay", "Pitch envelope decay duration", false));

    noiseShRateSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_noise_sh_rate", "Sample-and-hold clock rate / granular transient pitch", false));
    noiseFilterSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_noise_filter", "Noise center frequency and tonal color tilt", true));
    noiseDecaySlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_noise_decay", "Transient noise burst decay time", false));
    noiseCrossfadeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_noise_crossfade", "Blend balance between FM tonal voice and noise burst", true));

    filterCutoffSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filter_cutoff", "Master filter corner or center frequency", false));
    filterResoSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filter_reso", "Resonance / Q factor emphasis at cutoff frequency", false));

    filterEnvSlopeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filterenv_slope", "Filter envelope decay curve contour", false));
    filterEnvDepthSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filterenv_depth", "Bipolar cutoff modulation sweep depth in octaves", true));
    filterEnvDecaySlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filterenv_decay", "Filter envelope decay duration", false));
    filterEnvDriveSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_filterenv_drive", "Pre-filter saturation drive and harmonic distortion", false));

    ampDriveSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_amp_drive", "Master output saturation drive and soft-clipping gain", false));
    ampPanSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_amp_pan", "Stereo output pan position", true));
    ampVelSlopeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_amp_vel_slope", "MIDI velocity response curve (exponential, linear, logarithmic)", false));
    ampVelFloorSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_amp_vel_floor", "Minimum volume floor at lowest MIDI velocity (1% to 100%)", false));

    ampEnvClapsSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_ampenv_claps", "Number of pre-decay transient handclap bursts (0 = disabled)", false));
    ampEnvClapSpeedSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_ampenv_clapspeed", "Time interval spacing between handclap bursts", false));
    ampEnvSlopeSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_ampenv_slope", "Master amplitude decay curve contour", false));
    ampEnvDecaySlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_ampenv_decay", "Master amplitude decay duration", false));
    limiterGainSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_gain", "Master limiter makeup and input boost gain", false));
    limiterThreshSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_thresh", "Master limiter brickwall ceiling peak threshold", false));
    limiterReleaseSlider.setTooltip(TooltipHelper::makeKnobTooltipFromParam(audioProcessor.apvts, "planter_limiter_release", "Master limiter gain reduction recovery release time", false));

    titleText = RlyehSound::ParameterManager::getInstance().getGlobalString("tkp_title", "THE KLANG PLANTER");
    subtitleText = RlyehSound::ParameterManager::getInstance().getGlobalString("tkp_subtitle", "COMPACT FM PERCUSSION SYNTHESIZER");
#ifdef JucePlugin_VersionString
    versionText = "v" JucePlugin_VersionString;
#else
    versionText = "v0.3.0";
#endif
#ifdef TKF_FEATURE_TAG
    if (juce::String(TKF_FEATURE_TAG).isNotEmpty()) {
        versionText += juce::String(TKF_FEATURE_TAG);
    }
#endif
    juce::Font verFont(juce::FontOptions(11.0f, juce::Font::bold));
    versionWidth = juce::GlyphArrangement::getStringWidthInt(verFont, versionText) + 8;

    setSize(1040, 740);
    setResizable(true, true);
    setResizeLimits(800, 560, 2400, 1600);

    headerViz.onPanic = [this] {
        audioProcessor.panic();
    };

    headerViz.onLimiterCalloutRequested = [this](const juce::Rectangle<int>& area) {
        auto callout = std::make_unique<PlanterLimiterCalloutComponent>(audioProcessor);
        auto limitEditorArea = area + headerViz.getPosition();
        juce::CallOutBox::launchAsynchronously(std::move(callout), limitEditorArea, this);
    };

    wireHeaderHover(headerViz, "HEADER DISPLAY", "Scope & Meters",
                    "Click meters to panic flush. Right-click LIMIT badge to summon master limiter.",
                    "L-Click: Panic", "R-Click: Limiter");

    updateCarrierControls();
    updateModControls();
    startTimerHz(30);
}

TheKlangPlanterAudioProcessorEditor::~TheKlangPlanterAudioProcessorEditor() {
    stopTimer();
    unwireHeaderHover(headerViz);
    if (tooltipWindow) {
        tooltipWindow->setLookAndFeel(nullptr);
        tooltipWindow.reset();
    }
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
    if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
        float defVal = param->getDefaultValue();
        const auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
        
        if (def != nullptr) {
            slider.setDoubleClickReturnValue(true, def->doubleClickValue);
            slider.getDefaultValue = [v = def->doubleClickValue]() { return v; };
            for (auto& pt : def->snapPoints) slider.snapValues.push_back(pt.value);
        } else {
            slider.setDoubleClickReturnValue(true, defVal);
            slider.getDefaultValue = [defVal]() { return defVal; };
        }
    }
    slider.setParamId(paramId);

    slider.onMouseEnter = [this, paramId](RotaryKnobSlider* s) {
        if (!s) return;
        auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
        juce::String name = def ? def->name : (s->getLabel().isNotEmpty() ? s->getLabel() : paramId);
        juce::String val = s->getTextFromValue(s->getValue());
        juce::String desc = def ? def->description : "";
        juce::String rc = (def && !def->snapPoints.empty()) ? "Right-Click: Snap Points" : "Right-Click: Details";
        juce::String dc = def ? ("2x-Click: Reset (" + juce::String(def->doubleClickValue, 2) + ")") : "2x-Click: Default";
        statusBar.setHoveredControl(name, val, desc, rc, dc);
    };
    slider.onMouseExit = [this](RotaryKnobSlider*) {
        statusBar.clearHoveredControl();
    };

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, slider));
}

void TheKlangPlanterAudioProcessorEditor::bindSelector(LedSelectorComponent& sel, juce::ComboBox& box,
                                                       const juce::String& paramId, const juce::StringArray& items, int columns) {
    box.clear(juce::dontSendNotification);
    for (int i = 0; i < items.size(); ++i) {
        box.addItem(items[i], i + 1);
    }
    sel.setItems(items, columns);
    sel.setParamId(paramId);
    if (paramId.isNotEmpty()) {
        auto itemTips = TooltipHelper::getLedSelectorItemTooltips(paramId);
        sel.setItemTooltips(itemTips);
    }
    sel.onChange = [this, &box, paramId](int index) {
        box.setSelectedId(index + 1, juce::sendNotification);
        if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
        }
    };
    box.onChange = [&box, &sel]() {
        sel.setSelectedIndex(box.getSelectedItemIndex(), juce::dontSendNotification);
    };

    sel.onMouseEnter = [this, paramId](LedSelectorComponent* s) {
        if (!s) return;
        auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
        juce::String name = def ? def->name : paramId;
        juce::String desc = s->getTooltip();
        juce::String val = juce::String(s->getSelectedIndex() + 1) + "/" + juce::String(s->getNumItems());
        statusBar.setHoveredControl(name, val, desc, "", "Click: Select Mode");
    };
    sel.onMouseExit = [this](LedSelectorComponent*) {
        statusBar.clearHoveredControl();
    };
}

int TheKlangPlanterAudioProcessorEditor::syncSelector(juce::ComboBox& box, LedSelectorComponent& sel,
                                                       const juce::String& paramId, int& lastVal) {
    if (auto* param = audioProcessor.apvts.getParameter(paramId)) {
        int curVal = static_cast<int>(std::round(param->getValue() * static_cast<float>(sel.getNumItems() - 1)));
        if (curVal != lastVal) {
            lastVal = curVal;
            sel.setSelectedIndex(curVal, juce::dontSendNotification);
            box.setSelectedItemIndex(curVal, juce::dontSendNotification);
            return curVal;
        }
    }
    return -1;
}

void TheKlangPlanterAudioProcessorEditor::updateCarrierControls() {
    int mode = carrierTrackingBox.getSelectedItemIndex();
    if (mode == 0) { // MIDI
        cardCarrier->setKnobLabel(0, "Offset");
        carrierPitchSlider.setBipolar(true);
        carrierPitchSlider.getDefaultValue = []() { return 0.5; };
    } else if (mode == 1) { // Freq
        cardCarrier->setKnobLabel(0, "Frequency");
        carrierPitchSlider.setBipolar(false);
        carrierPitchSlider.getDefaultValue = []() {
            return static_cast<double>(std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f));
        };
    } else { // Note
        cardCarrier->setKnobLabel(0, "Note");
        carrierPitchSlider.setBipolar(false);
        carrierPitchSlider.getDefaultValue = []() { return 33.0 / 127.0; };
    }
    carrierPitchSlider.setDoubleClickReturnValue(true, carrierPitchSlider.getDefaultValue());
    carrierPitchSlider.repaint();
    carrierPitchSlider.updateText();
}

void TheKlangPlanterAudioProcessorEditor::updateModControls() {
    int curType = modTypeBox.getSelectedItemIndex();
    int curTrack = modTrackBox.getSelectedItemIndex();

    if (curType == 0) {
        // Oscillator: Shape is Waveform, Speed is Frequency or Ratio
        cardMod->setKnobLabel(0, "Shape");
        modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;
        modShapeSlider.customFormatText = formatWaveshape;
        modShapeSlider.customParseText = parseWaveshape;
        modShapeSlider.setBipolar(false);
        modShapeSlider.getDefaultValue = []() { return 0.0; };
        modShapeSlider.setDoubleClickReturnValue(true, 0.0);

        cardMod->setKnobLabel(1, "Speed");
        if (curTrack == 0) {
            modSpeedSlider.getDefaultValue = []() { return 0.50934; }; // 55 Hz
            modSpeedSlider.setDoubleClickReturnValue(true, 0.50934);
        } else {
            modSpeedSlider.getDefaultValue = []() { return 0.5; };
            modSpeedSlider.setDoubleClickReturnValue(true, 0.5);
        }
    } else if (curType == 1) {
        // Cyclic: Shape is DJ Filter (bipolar), Speed is Sine Pitch (Hz / semitones / ratio)
        cardMod->setKnobLabel(0, "DJ Filter");
        modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
        modShapeSlider.setBipolar(true);
        modShapeSlider.getDefaultValue = []() { return 0.5; };
        modShapeSlider.setDoubleClickReturnValue(true, 0.5);

        cardMod->setKnobLabel(1, "Speed");
        if (curTrack == 0) {
            modSpeedSlider.getDefaultValue = []() { return 0.50934; };
            modSpeedSlider.setDoubleClickReturnValue(true, 0.50934);
        } else {
            modSpeedSlider.getDefaultValue = []() { return 0.5; };
            modSpeedSlider.setDoubleClickReturnValue(true, 0.5);
        }
    } else {
        // Noise (curType == 2): Shape is DJ Filter (bipolar), Speed is S&H Rate (Hz)
        cardMod->setKnobLabel(0, "DJ Filter");
        modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::None;
        modShapeSlider.setBipolar(true);
        modShapeSlider.getDefaultValue = []() { return 0.5; };
        modShapeSlider.setDoubleClickReturnValue(true, 0.5);

        cardMod->setKnobLabel(1, "S&H Rate");
        modSpeedSlider.getDefaultValue = []() { return 1.0; }; // 24 kHz
        modSpeedSlider.setDoubleClickReturnValue(true, 1.0);
    }
    modShapeSlider.repaint();
    modShapeSlider.updateText();
    modSpeedSlider.repaint();
    modSpeedSlider.updateText();
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
    g.drawText(titleText, 14, 0, titleWidth, 36, juce::Justification::centredLeft);

    // Version
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff4a9eff));
    g.drawText(versionText, 206, 0, versionWidth, 36, juce::Justification::centredLeft);

    int badgeOffset = 0;
    if (updateBadgeButton.isVisible()) {
        badgeOffset = updateBadgeButton.getWidth() + 8;
    }

    // Subtitle
    int subX = 206 + versionWidth + 8 + badgeOffset;
    int subtitleWidth = juce::jmax(0, getWidth() - 586 - subX);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff75849b));
    g.drawText(subtitleText, subX, 0, subtitleWidth, 36, juce::Justification::centredLeft);
}

void TheKlangPlanterAudioProcessorEditor::resized() {
    int margin = 6;
    int topOffset = 38;
    int totalW = getWidth() - 2 * margin;
    int totalH = getHeight() - topOffset - margin - 36;

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
    headerViz.setBounds(getWidth() - 580, 5, 214, 26);
    settingsButton.setBounds(getWidth() - 356, 5, 26, 26);
    tooltipsButton.setBounds(getWidth() - 324, 5, 72, 26);
    initButton.setBounds(getWidth() - 246, 5, 90, 26);
    triggerButton.setBounds(getWidth() - 146, 5, 136, 26);

    updateBadgeButton.setBounds(206 + versionWidth + 8, 7, 136, 22);

    if (settingsModal)
        settingsModal->setBounds(getLocalBounds());

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

    statusBar.setBounds(0, getHeight() - 36, getWidth(), 36);
    statusBar.toFront(false);
}

void TheKlangPlanterAudioProcessorEditor::timerCallback() {
    // 1. Sync selectors
    int curCarrierTrack = syncSelector(carrierTrackingBox, carrierTrackingSelector, "planter_carrier_tracking", lastCarrierTracking);
    if (curCarrierTrack >= 0) {
        updateCarrierControls();
    }

    int curModTrack = syncSelector(modTrackBox, modTrackSelector, "planter_mod_track", lastModTrack);
    int curModType  = syncSelector(modTypeBox, modTypeSelector, "planter_mod_type", lastModType);
    if (curModType >= 0 || curModTrack >= 0) {
        updateModControls();
    }

    syncSelector(pitchEnvTargetBox, pitchEnvTargetSelector, "planter_pitchenv_target", lastPitchEnvTarget);
    syncSelector(filterTypeBox, filterTypeSelector, "planter_filter_type", lastFilterType);
    syncSelector(filterSlopeBox, filterSlopeSelector, "planter_filter_slope", lastFilterSlope);
    syncSelector(limiterEnableBox, limiterEnableSelector, "planter_limiter_enable", lastLimiterEnable);

    // 2. Fetch live oscilloscope data & peak levels
    audioProcessor.getEngine().getScopeData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()));
    float peakL = audioProcessor.getEngine().getPeakL();
    float peakR = audioProcessor.getEngine().getPeakR();
    float limAct = audioProcessor.getLimiterActivity();

    headerViz.updateData(scopeBuffer.data(), static_cast<int>(scopeBuffer.size()), peakL, peakR, limAct);
}








