#include "KlangCoreEditor.h"

KlangCoreEditor::KlangCoreEditor(KlangCoreProcessor& p)
    : AudioProcessorEditor(p), coreProcessor(p)
{
    setLookAndFeel(&knobLookAndFeel);
    
    // Default tooltips window
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 700);
    tooltipWindow->setOpaque(false);
    tooltipWindow->setMillisecondsBeforeTipAppears(400);

    // Setup buttons
    tooltipsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    tooltipsButton.onClick = [this] {
        setTooltipsEnabled(!tooltipsEnabled);
    };
    addAndMakeVisible(tooltipsButton);

    initButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    initButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    addAndMakeVisible(initButton);

    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
    addAndMakeVisible(triggerButton);
}

void KlangCoreEditor::setTooltipsEnabled(bool enabled) {
    tooltipsEnabled = enabled;
    if (enabled) {
        tooltipsButton.setButtonText("TIPS: ON");
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff33ccff));
        tooltipWindow->setMillisecondsBeforeTipAppears(400);
    } else {
        tooltipsButton.setButtonText("TIPS: OFF");
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
        tooltipWindow->setMillisecondsBeforeTipAppears(9999999);
        tooltipWindow->hideTip();
    }
}


