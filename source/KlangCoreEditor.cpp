#include "KlangCoreEditor.h"
#include "ParameterManager.h"

KlangCoreEditor::KlangCoreEditor(KlangCoreProcessor& p)
    : AudioProcessorEditor(p), coreProcessor(p)
{
    initButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_init", "INIT"));
    triggerButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_trigger", "TRIGGER"));
    tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_off", "TIPS: OFF"));
    guideButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_guide", "QUICKSTART GUIDE"));
    setLookAndFeel(&knobLookAndFeel);

    // Default tooltips window
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 700);
    tooltipWindow->setOpaque(false);
    tooltipWindow->setMillisecondsBeforeTipAppears(400);

    // Settings Modal & Button
    settingsModal = std::make_unique<SettingsModalComponent>(coreProcessor.getName());
    addChildComponent(*settingsModal);

    settingsButton.onClick = [this] {
        if (settingsModal) {
            settingsModal->setVisible(true);
            settingsModal->toFront(true);
        }
    };
    addAndMakeVisible(settingsButton);

    // Update Badge Button (hidden until an update is discovered)
    addChildComponent(updateBadgeButton);

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

    addAndMakeVisible(statusBar);

    // Trigger update check on launch if enabled
    if (VersionChecker::isCheckOnLaunchEnabled())
        VersionChecker::getInstance().checkForUpdates(false);
}

void KlangCoreEditor::setTooltipsEnabled(bool enabled)
{
    tooltipsEnabled = enabled;
    statusBar.setTooltipsEnabled(enabled);
    if (enabled) {
        tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_on", "TIPS: ON"));
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff33ccff));
        tooltipWindow->setMillisecondsBeforeTipAppears(400);
    } else {
        tooltipsButton.setButtonText(RlyehSound::ParameterManager::getInstance().getGlobalString("btn_tooltips_off", "TIPS: OFF"));
        tooltipsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8b99a6));
        tooltipWindow->setMillisecondsBeforeTipAppears(9999999);
        tooltipWindow->hideTip();
    }
}
