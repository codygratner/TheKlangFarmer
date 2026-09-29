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
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
};

// Mini Oscilloscope widget inside each card
class MiniOscilloscopeComponent : public juce::Component {
public:
    MiniOscilloscopeComponent(juce::Colour traceColour);
    void updateData(const float* data, int numPoints);
    void paint(juce::Graphics& g) override;

private:
    juce::Colour traceCol;
    std::vector<float> points;
};

// Rotary knob with right-click hovering text box editor and bipolar arc support
class RotaryKnobSlider : public juce::Slider {
public:
    RotaryKnobSlider();
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void openHoveringEditor();

    bool isBipolar = false;
    void setBipolar(bool bipolar) { isBipolar = bipolar; repaint(); }

    std::function<double()> getDefaultValue;
    std::function<juce::String(double)> customFormatText;
    std::function<double(const juce::String&)> customParseText;

    juce::String getTextFromValue(double val) override;
    double getValueFromText(const juce::String& text) override;
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

// Card component representing one of the 13 blocks
class ModuleCardComponent : public juce::Component {
public:
    ModuleCardComponent(const juce::String& title, juce::Colour accentColour);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateScope(const float* data, int numSamples);
    void setLedSelector(LedSelectorComponent* selector);
    void setSelector(juce::ComboBox* box);
    void setKnob(int slotIndex, const juce::String& label, RotaryKnobSlider* slider);
    void setKnobLabel(int slotIndex, const juce::String& label);
    void setNumActiveKnobs(int count) { numActiveKnobs = count; }

private:
    juce::String moduleTitle;
    juce::Colour accent;
    MiniOscilloscopeComponent oscilloscope;

    LedSelectorComponent* ledSelector = nullptr;
    juce::ComboBox* selectorBox = nullptr;
    juce::Label labels[4];
    RotaryKnobSlider* knobs[4] = {};
    int numActiveKnobs = 4;
};

class BiaEr1AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit BiaEr1AudioProcessorEditor(BiaEr1AudioProcessor&);
    ~BiaEr1AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void updateDynamicControls();
    void bindSelector(LedSelectorComponent& selector, juce::ComboBox& box,
                      const juce::String& paramId, const juce::StringArray& items, int numColumns = 1);

private:
    BiaEr1AudioProcessor& audioProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;

    // Header buttons
    juce::TextButton initButton { "INIT" };
    juce::TextButton triggerButton { "AUDITION HIT" };
    void resetToDefaults();

    // 13 Module Cards
    std::vector<std::unique_ptr<ModuleCardComponent>> cards;

    // Block 1: Carrier
    juce::ComboBox carrierTrackingBox;
    LedSelectorComponent carrierTrackingSelector;
    RotaryKnobSlider carrierPitchSlider;
    RotaryKnobSlider carrierShapeSlider;
    RotaryKnobSlider carrierDriveSlider;

    // Block 2: Modulator
    juce::ComboBox modTypeBox;
    LedSelectorComponent modTypeSelector;
    RotaryKnobSlider modShapeSlider;
    RotaryKnobSlider modDepthSlider;
    RotaryKnobSlider modSpeedSlider;

    // Block 3: Pitch Envelope
    juce::ComboBox pitchEnvTargetBox;
    LedSelectorComponent pitchEnvTargetSelector;
    RotaryKnobSlider pitchEnvSlopeSlider;
    RotaryKnobSlider pitchEnvDepthSlider;
    RotaryKnobSlider pitchEnvDecaySlider;

    // Block 4: Drive
    juce::ComboBox driveTypeBox;
    LedSelectorComponent driveTypeSelector;
    RotaryKnobSlider driveAmountSlider;
    RotaryKnobSlider driveBiasSlider;
    RotaryKnobSlider driveFilterSlider;

    // Block 5: Noise Transient
    RotaryKnobSlider noiseShRateSlider;
    RotaryKnobSlider noiseFilterSlider;
    RotaryKnobSlider noiseDriveSlider;
    RotaryKnobSlider noiseDecaySlider;

    // Block 6: Mixer
    RotaryKnobSlider mixerCarrierLevelSlider;
    RotaryKnobSlider mixerNoiseLevelSlider;
    RotaryKnobSlider mixerDriveSlider;
    juce::ComboBox mixerLimiterBox;
    LedSelectorComponent mixerLimiterSelector;

    // Block 7: Filter
    juce::ComboBox filterTypeBox;
    LedSelectorComponent filterTypeSelector;
    RotaryKnobSlider filterStyleSlider;
    RotaryKnobSlider filterCutoffSlider;
    RotaryKnobSlider filterResonanceSlider;

    // Block 8: Filter Envelope
    RotaryKnobSlider filterEnvSlopeSlider;
    RotaryKnobSlider filterEnvDepthSlider;
    RotaryKnobSlider filterEnvDecaySlider;
    RotaryKnobSlider filterEnvPreDriveSlider;

    // Block 9: RingMod
    RotaryKnobSlider ringModShapeSlider;
    RotaryKnobSlider ringModRateSlider;
    RotaryKnobSlider ringModAmountSlider;
    RotaryKnobSlider ringModWidthSlider;

    // Block 10: Frequency Shifter
    RotaryKnobSlider freqShiftShiftSlider;
    RotaryKnobSlider freqShiftRangeSlider;
    RotaryKnobSlider freqShiftBlendSlider;
    RotaryKnobSlider freqShiftWidthSlider;

    // Block 11: Grit FX
    RotaryKnobSlider gritBitsSlider;
    RotaryKnobSlider gritRateSlider;
    RotaryKnobSlider gritLowBoostSlider;
    RotaryKnobSlider gritHighBoostSlider;

    // Block 12: Comb Filter
    juce::ComboBox combTypeBox;
    LedSelectorComponent combTypeSelector;
    RotaryKnobSlider combDampeningSlider;
    RotaryKnobSlider combCutoffSlider;
    RotaryKnobSlider combResonanceSlider;

    // Block 13: Disperser
    juce::ComboBox disperserTypeBox;
    LedSelectorComponent disperserTypeSelector;
    RotaryKnobSlider disperserAmountSlider;
    RotaryKnobSlider disperserCutoffSlider;
    RotaryKnobSlider disperserResonanceSlider;

    // Block 14: Amp
    RotaryKnobSlider ampPanSlider;
    RotaryKnobSlider ampLevelSlider;
    RotaryKnobSlider ampDriveSlider;
    juce::ComboBox ampLimiterBox;
    LedSelectorComponent ampLimiterSelector;

    // Block 15: Amp Envelope
    RotaryKnobSlider ampEnvClapsSlider;
    RotaryKnobSlider ampEnvClapSpeedSlider;
    RotaryKnobSlider ampEnvSlopeSlider;
    RotaryKnobSlider ampEnvDecaySlider;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    void setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour, bool isBipolar = false, double defaultVal = 0.5);
    void setupBox(juce::ComboBox& box);

    int lastCarrierTrack = -1;
    int lastModType = -1;
    int lastPitchEnvTarget = -1;
    int lastDriveType = -1;
    int lastMixerLimiter = -1;
    int lastFilterType = -1;
    int lastCombType = -1;
    int lastDisperserType = -1;
    int lastAmpLimiter = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiaEr1AudioProcessorEditor)
};
