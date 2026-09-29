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

// Rotary knob with right-click hovering text box editor
class RotaryKnobSlider : public juce::Slider {
public:
    RotaryKnobSlider();
    void mouseDown(const juce::MouseEvent& e) override;
    void openHoveringEditor();

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

// Card component representing one of the 10 blocks
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

private:
    BiaEr1AudioProcessor& audioProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;

    // Header Trigger button for rapid auditioning
    juce::TextButton triggerButton { "AUDITION HIT" };

    // 10 Module Cards
    std::vector<std::unique_ptr<ModuleCardComponent>> cards;

    // Block 1: Carrier
    juce::ComboBox carrierTrackingBox;
    LedSelectorComponent carrierTrackingSelector;
    RotaryKnobSlider carrierPitchSlider;
    RotaryKnobSlider carrierShapeSlider;
    RotaryKnobSlider carrierLevelSlider;

    // Block 2: Modulator
    juce::ComboBox modTypeBox;
    LedSelectorComponent modTypeSelector;
    RotaryKnobSlider modShapeSlider;
    RotaryKnobSlider modDepthSlider;
    RotaryKnobSlider modSpeedSlider;

    // Block 3: Drive
    juce::ComboBox driveTypeBox;
    LedSelectorComponent driveTypeSelector;
    RotaryKnobSlider driveAmountSlider;
    RotaryKnobSlider driveBiasSlider;
    RotaryKnobSlider driveFilterSlider;

    // Block 4: Noise Transient
    RotaryKnobSlider noiseShRateSlider;
    RotaryKnobSlider noiseFilterSlider;
    RotaryKnobSlider noiseLevelSlider;
    RotaryKnobSlider noiseDecaySlider;

    // Block 5: Filter
    juce::ComboBox filterTypeBox;
    LedSelectorComponent filterTypeSelector;
    RotaryKnobSlider filterCutoffSlider;
    RotaryKnobSlider filterDepthSlider;
    RotaryKnobSlider filterDecaySlider;

    // Block 6: RingMod
    RotaryKnobSlider ringModShapeSlider;
    RotaryKnobSlider ringModRateSlider;
    RotaryKnobSlider ringModAmountSlider;
    RotaryKnobSlider ringModWidthSlider;

    // Block 7: Grit FX
    RotaryKnobSlider gritBitsSlider;
    RotaryKnobSlider gritRateSlider;

    // Block 8: Frequency Shifter
    RotaryKnobSlider freqShiftShiftSlider;
    RotaryKnobSlider freqShiftRangeSlider;
    RotaryKnobSlider freqShiftBlendSlider;
    RotaryKnobSlider freqShiftWidthSlider;

    // Block 9: Amp
    RotaryKnobSlider ampPanSlider;
    RotaryKnobSlider ampLevelSlider;
    RotaryKnobSlider ampDriveSlider;
    RotaryKnobSlider ampLowBoostSlider;

    // Block 10: Amp Envelope
    juce::ComboBox ampEnvTypeBox;
    LedSelectorComponent ampEnvTypeSelector;
    RotaryKnobSlider ampEnvClapsSlider;
    RotaryKnobSlider ampEnvShapeSlider;
    RotaryKnobSlider ampEnvDecaySlider;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> boxAttachments;

    void setupKnob(RotaryKnobSlider& slider, juce::Colour trackColour);
    void setupBox(juce::ComboBox& box);

    int lastCarrierTrack = -1;
    int lastModType = -1;
    int lastDriveType = -1;
    int lastFilterType = -1;
    int lastAmpEnvType = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiaEr1AudioProcessorEditor)
};
