#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include <vector>
#include <memory>
#include <functional>

// Sleek LookAndFeel for rotary knobs
class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    RotaryKnobLookAndFeel();
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
};

// Mini Oscilloscope or Frequency-Gain Plot widget inside each card
class MiniOscilloscopeComponent : public juce::Component {
public:
    enum class PlotMode {
        Oscilloscope,
        FilterXY,
        EqXY
    };

    MiniOscilloscopeComponent(juce::Colour traceColour);
    void updateData(const float* data, int numPoints);
    void setPlotMode(PlotMode mode);
    void updateFilterParams(int type, int slope, float cutoffHz, float resonance);
    void updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter);
    void paint(juce::Graphics& g) override;

private:
    juce::Colour traceCol;
    PlotMode plotMode = PlotMode::Oscilloscope;
    std::vector<float> points;

    // Filter plot params
    int filterType = 0;
    int filterSlope = 1;
    float filterCutoff = 24000.0f;
    float filterResonance = 0.0f;

    // EQ plot params
    float eqFreq = 24000.0f;
    float eqWidth = 0.1f;
    float eqGain = 0.0f;
    float eqDJ = 0.5f;
};

// Rotary knob / horizontal slider with right-click hovering text box editor, bipolar arc/bar, and waveform/slope diagram support
class RotaryKnobSlider : public juce::Slider {
public:
    enum class DiagramType {
        None,
        Waveform,
        EnvelopeSlope,
        VelocitySlope,
        FilterSlope
    };

    RotaryKnobSlider();
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void openHoveringEditor();

    bool isBipolar = false;
    void setBipolar(bool bipolar) { isBipolar = bipolar; repaint(); }

    DiagramType diagramType = DiagramType::None;

    std::function<double()> getDefaultValue;
    std::function<juce::String(double)> customFormatText;
    std::function<double(const juce::String&)> customParseText;

    juce::String getTextFromValue(double val) override;
    double getValueFromText(const juce::String& text) override;
};

// Custom diagram-rendering label used as slider text box
class DiagramSliderLabel : public juce::Label, public juce::Slider::Listener {
public:
    explicit DiagramSliderLabel(RotaryKnobSlider& s);
    ~DiagramSliderLabel() override;
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void sliderValueChanged(juce::Slider*) override { repaint(); }

private:
    RotaryKnobSlider& slider;
};

// Sleek hardware-style LED list selector
class LedSelectorComponent : public juce::Component {
public:
    explicit LedSelectorComponent(juce::Colour activeAccent);

    void setItems(const juce::StringArray& newItems, int numColumns = 2);
    void setSelectedIndex(int newIndex, juce::NotificationType notification = juce::sendNotificationAsync);
    int getSelectedIndex() const { return selectedIndex; }
    int getNumItems() const { return items.size(); }

    std::function<void(int)> onChange;

    void paint(juce::Graphics& g) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::StringArray items;
    int selectedIndex = 0;
    int hoveredIndex = -1;
    int columns = 2;
    juce::Colour accent;

    int getItemIndexAt(juce::Point<int> pos) const;
    juce::Rectangle<int> getItemBounds(int index) const;
};

// Card component representing one of the 15 blocks
class ModuleCardComponent : public juce::Component {
public:
    ModuleCardComponent(const juce::String& title, juce::Colour accentColour);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateScope(const float* data, int numSamples);
    void setPlotMode(MiniOscilloscopeComponent::PlotMode mode);
    void updateFilterParams(int type, int slope, float cutoffHz, float resonance);
    void updateEqParams(float freqHz, float widthOct, float gainDb, float djFilter);
    void setLedSelector(LedSelectorComponent* selector);
    void setSecondLedSelector(LedSelectorComponent* selector);
    void setSelector(juce::ComboBox* box);
    void setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider);
    void setKnobLabel(int slotIndex, const juce::String& label);
    void setNumActiveKnobs(int count) { numActiveKnobs = count; }

private:
    juce::String moduleTitle;
    juce::Colour accent;
    MiniOscilloscopeComponent oscilloscope;

    LedSelectorComponent* ledSelector = nullptr;
    LedSelectorComponent* secondLedSelector = nullptr;
    juce::ComboBox* selectorBox = nullptr;
    juce::Label labels[4];
    RotaryKnobSlider* knobs[4] = {};
    int numActiveKnobs = 4;
};


class TheKlangFarmerAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit TheKlangFarmerAudioProcessorEditor(TheKlangFarmerAudioProcessor&);
    ~TheKlangFarmerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void updateDynamicControls();
    void bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                      const juce::String& paramId, const juce::StringArray& items, int numColumns = 1);

private:
    TheKlangFarmerAudioProcessor& audioProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;

    // Header buttons
    juce::TextButton initButton { "INIT" };
    juce::TextButton triggerButton { "AUDITION HIT" };
    void resetToDefaults();

    // 22 Module Cards
    std::vector<std::unique_ptr<ModuleCardComponent>> cards;

    // Block 1: Carrier 1
    juce::ComboBox carrier1TrackingBox;
    LedSelectorComponent carrier1TrackingSelector;
    RotaryKnobSlider carrier1PitchSlider;
    RotaryKnobSlider carrier1ShapeSlider;
    RotaryKnobSlider carrier1DepthSlider;

    // Block 2: Modulator 1
    juce::ComboBox mod1TrackBox;
    LedSelectorComponent mod1TrackSelector;
    juce::ComboBox mod1TypeBox;
    LedSelectorComponent mod1TypeSelector;
    RotaryKnobSlider mod1ShapeSlider;
    RotaryKnobSlider mod1SpeedSlider;

    // Block 3: Pitch Envelope 1
    juce::ComboBox pitchEnv1TargetBox;
    LedSelectorComponent pitchEnv1TargetSelector;
    RotaryKnobSlider pitchEnv1SlopeSlider;
    RotaryKnobSlider pitchEnv1DepthSlider;
    RotaryKnobSlider pitchEnv1DecaySlider;

    // Block 4: Carrier 2
    juce::ComboBox carrier2TrackingBox;
    LedSelectorComponent carrier2TrackingSelector;
    RotaryKnobSlider carrier2PitchSlider;
    RotaryKnobSlider carrier2ShapeSlider;
    RotaryKnobSlider carrier2DepthSlider;

    // Block 5: Modulator 2
    juce::ComboBox mod2TrackBox;
    LedSelectorComponent mod2TrackSelector;
    juce::ComboBox mod2TypeBox;
    LedSelectorComponent mod2TypeSelector;
    RotaryKnobSlider mod2ShapeSlider;
    RotaryKnobSlider mod2SpeedSlider;

    // Block 6: Pitch Envelope 2
    juce::ComboBox pitchEnv2TargetBox;
    LedSelectorComponent pitchEnv2TargetSelector;
    RotaryKnobSlider pitchEnv2SlopeSlider;
    RotaryKnobSlider pitchEnv2DepthSlider;
    RotaryKnobSlider pitchEnv2DecaySlider;

    // Block 7: Noise Transient
    RotaryKnobSlider noiseShRateSlider;
    RotaryKnobSlider noiseFilterSlider;
    RotaryKnobSlider noiseDriveSlider;
    RotaryKnobSlider noiseDecaySlider;

    // Block 8: Mixer
    RotaryKnobSlider mixerCarrier1LevelSlider;
    RotaryKnobSlider mixerCarrier2LevelSlider;
    RotaryKnobSlider mixerRingModSlider;
    RotaryKnobSlider mixerNoiseLevelSlider;

    // Block 9: Drive
    juce::ComboBox driveLimiterBox;
    LedSelectorComponent driveLimiterSelector;
    RotaryKnobSlider driveAmountSlider;
    RotaryKnobSlider driveBiasSlider;
    RotaryKnobSlider driveFilterSlider;

    // Block 10: Filter
    juce::ComboBox filterTypeBox;
    LedSelectorComponent filterTypeSelector;
    juce::ComboBox filterSlopeBox;
    LedSelectorComponent filterSlopeSelector;
    RotaryKnobSlider filterCutoffSlider;
    RotaryKnobSlider filterResonanceSlider;

    // Block 11: Filter Envelope
    RotaryKnobSlider filterEnvSlopeSlider;
    RotaryKnobSlider filterEnvDepthSlider;
    RotaryKnobSlider filterEnvDecaySlider;
    RotaryKnobSlider filterEnvPostDriveSlider;

    // Block 12: Wave Folder
    juce::ComboBox waveFolderTypeBox;
    LedSelectorComponent waveFolderTypeSelector;
    RotaryKnobSlider waveFolderFoldSlider;
    RotaryKnobSlider waveFolderBiasSlider;
    RotaryKnobSlider waveFolderFilterSlider;

    // Block 13: RingMod
    RotaryKnobSlider ringModShapeSlider;
    RotaryKnobSlider ringModRateSlider;
    RotaryKnobSlider ringModAmountSlider;
    RotaryKnobSlider ringModWidthSlider;

    // Block 14: Frequency Shifter
    RotaryKnobSlider freqShiftShiftSlider;
    RotaryKnobSlider freqShiftRangeSlider;
    RotaryKnobSlider freqShiftBlendSlider;
    RotaryKnobSlider freqShiftWidthSlider;

    // Block 15: Grit FX
    RotaryKnobSlider gritBitsSlider;
    RotaryKnobSlider gritRateSlider;
    RotaryKnobSlider gritLowSlider;
    RotaryKnobSlider gritHighSlider;

    // Block 16: Comb Filter
    juce::ComboBox combTypeBox;
    LedSelectorComponent combTypeSelector;
    RotaryKnobSlider combDampeningSlider;
    RotaryKnobSlider combCutoffSlider;
    RotaryKnobSlider combResonanceSlider;

    // Block 17: Disperser
    juce::ComboBox disperserTypeBox;
    LedSelectorComponent disperserTypeSelector;
    RotaryKnobSlider disperserAmountSlider;
    RotaryKnobSlider disperserCutoffSlider;
    RotaryKnobSlider disperserResonanceSlider;

    // Block 18: EQ (Bell EQ)
    RotaryKnobSlider eqFreqSlider;
    RotaryKnobSlider eqWidthSlider;
    RotaryKnobSlider eqGainSlider;
    RotaryKnobSlider eqFilterSlider;

    // Block 19: Amp
    RotaryKnobSlider ampPanSlider;
    RotaryKnobSlider ampLevelSlider;
    RotaryKnobSlider ampDriveSlider;
    juce::ComboBox ampLimiterBox;
    LedSelectorComponent ampLimiterSelector;

    // Block 20: Amp Envelope
    RotaryKnobSlider ampEnvClapsSlider;
    RotaryKnobSlider ampEnvClapSpeedSlider;
    RotaryKnobSlider ampEnvSlopeSlider;
    RotaryKnobSlider ampEnvDecaySlider;

    // Block 21: Velocity
    RotaryKnobSlider velSlopeSlider;
    RotaryKnobSlider velDecaySlider;
    RotaryKnobSlider velDepthSlider;
    RotaryKnobSlider velVolumeSlider;

    // Block 22: Slop
    RotaryKnobSlider slopFreqSlider;
    RotaryKnobSlider slopDepthSlider;
    RotaryKnobSlider slopDecaySlider;
    RotaryKnobSlider slopPanSlider;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    void setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar = false, double defaultVal = 0.5);
    void setupBox(juce::ComboBox& box);

    int lastCarrier1Track = -1;
    int lastMod1Track = -1;
    int lastMod1Type = -1;
    int lastPitchEnv1Target = -1;
    int lastCarrier2Track = -1;
    int lastMod2Track = -1;
    int lastMod2Type = -1;
    int lastPitchEnv2Target = -1;
    int lastDriveLimiter = -1;
    int lastFilterType = -1;
    int lastFilterSlope = -1;
    int lastWaveFolderType = -1;
    int lastCombType = -1;
    int lastDisperserType = -1;
    int lastAmpLimiter = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangFarmerAudioProcessorEditor)
};
