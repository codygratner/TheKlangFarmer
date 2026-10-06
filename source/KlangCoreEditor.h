#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "KlangCoreProcessor.h"
#include "UIComponents.h"

class KlangCoreEditor : public juce::AudioProcessorEditor {
public:
    KlangCoreEditor(KlangCoreProcessor& p);
    ~KlangCoreEditor() override = default;


protected:
    KlangCoreProcessor& coreProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
    bool tooltipsEnabled = false;

    // Header buttons (shared)
    juce::TextButton tooltipsButton;
    juce::TextButton initButton;
    juce::TextButton triggerButton;
    juce::TextButton guideButton;

    void setTooltipsEnabled(bool enabled);
};





