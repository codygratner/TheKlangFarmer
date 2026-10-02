#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PlanterProcessor.h"
#include "UIComponents.h"

// Compact Header Oscilloscope, Limiter Warning Badge, and Peak Meters for The Klang Planter
class PlanterHeaderVisualizer : public juce::Component {
public:
    PlanterHeaderVisualizer();
    void updateData(const float* scopeData, int numPoints, float peakL, float peakR, float limiterActivity);
    void paint(juce::Graphics& g) override;

private:
    std::vector<float> points;
    float livePeakL = 0.0f;
    float livePeakR = 0.0f;
    float liveLimiterAct = 0.0f;
};

class TheKlangPlanterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
    explicit TheKlangPlanterAudioProcessorEditor(TheKlangPlanterAudioProcessor&);
    ~TheKlangPlanterAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TheKlangPlanterAudioProcessor& audioProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;

    // Header Components
    PlanterHeaderVisualizer headerViz;
    juce::TextButton initButton{ "INIT" };
    juce::TextButton triggerButton{ "TRIGGER" };

    // 8 Module Cards (4 Columns x 2 Rows)
    std::unique_ptr<ModuleCardComponent> cardCarrier;
    std::unique_ptr<ModuleCardComponent> cardMod;
    std::unique_ptr<ModuleCardComponent> cardPitchEnv;
    std::unique_ptr<ModuleCardComponent> cardNoise;
    std::unique_ptr<ModuleCardComponent> cardFilter;
    std::unique_ptr<ModuleCardComponent> cardFilterEnv;
    std::unique_ptr<ModuleCardComponent> cardAmp;
    std::unique_ptr<ModuleCardComponent> cardAmpEnv;

    // Carrier Controls
    juce::ComboBox carrierTrackingBox;
    LedSelectorComponent carrierTrackingSelector;
    RotaryKnobSlider carrierPitchSlider;
    RotaryKnobSlider carrierShapeSlider;
    RotaryKnobSlider carrierDepthSlider;

    // Modulator Controls
    juce::ComboBox modTrackBox;
    LedSelectorComponent modTrackSelector;
    juce::ComboBox modTypeBox;
    LedSelectorComponent modTypeSelector;
    RotaryKnobSlider modShapeSlider;
    RotaryKnobSlider modSpeedSlider;

    // Pitch Envelope Controls
    juce::ComboBox pitchEnvTargetBox;
    LedSelectorComponent pitchEnvTargetSelector;
    RotaryKnobSlider pitchEnvSlopeSlider;
    RotaryKnobSlider pitchEnvDepthSlider;
    RotaryKnobSlider pitchEnvDecaySlider;

    // Noise Transient Controls (S&H Rate, DJ Filter, Decay, Crossfade)
    RotaryKnobSlider noiseShRateSlider;
    RotaryKnobSlider noiseFilterSlider;
    RotaryKnobSlider noiseDecaySlider;
    RotaryKnobSlider noiseCrossfadeSlider;

    // Filter Controls
    juce::ComboBox filterTypeBox;
    LedSelectorComponent filterTypeSelector;
    juce::ComboBox filterSlopeBox;
    LedSelectorComponent filterSlopeSelector;
    RotaryKnobSlider filterCutoffSlider;
    RotaryKnobSlider filterResoSlider;

    // Filter Envelope Controls (Slope, Depth, Decay, Pre-Filter Drive)
    RotaryKnobSlider filterEnvSlopeSlider;
    RotaryKnobSlider filterEnvDepthSlider;
    RotaryKnobSlider filterEnvDecaySlider;
    RotaryKnobSlider filterEnvDriveSlider;

    // Amplifier Controls (Drive, Pan, Vel Slope, Velocity)
    RotaryKnobSlider ampDriveSlider;
    RotaryKnobSlider ampPanSlider;
    RotaryKnobSlider ampVelSlopeSlider;
    RotaryKnobSlider ampVelFloorSlider;

    // Amp Envelope Controls
    RotaryKnobSlider ampEnvClapsSlider;
    RotaryKnobSlider ampEnvClapSpeedSlider;
    RotaryKnobSlider ampEnvSlopeSlider;
    RotaryKnobSlider ampEnvDecaySlider;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    std::vector<float> scopeBuffer;

    void setupKnob(RotaryKnobSlider& slider, juce::Colour accent, bool bipolar, double defaultValue);
    void setupBox(juce::ComboBox& box);
    void bindSlider(const juce::String& paramId, RotaryKnobSlider& slider);
    void bindSelector(LedSelectorComponent& sel, juce::ComboBox& box, const juce::String& paramId,
                      const juce::StringArray& items, int columns = 2);
    int syncSelector(juce::ComboBox& box, LedSelectorComponent& sel, const juce::String& paramId, int& lastVal);
    void updateCarrierControls();
    void updateModControls();

    int lastCarrierTracking = -1;
    int lastModTrack = -1;
    int lastModType = -1;
    int lastPitchEnvTarget = -1;
    int lastFilterType = -1;
    int lastFilterSlope = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangPlanterAudioProcessorEditor)
};
