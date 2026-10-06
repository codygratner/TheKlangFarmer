#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "KlangCoreProcessor.h"
#include "UIComponents.h"
#include "SettingsModal.h"
#include "VersionChecker.h"

class KlangCoreEditor : public juce::AudioProcessorEditor
{
public:
    explicit KlangCoreEditor(KlangCoreProcessor& p);
    ~KlangCoreEditor() override = default;

protected:
    KlangCoreProcessor& coreProcessor;
    RotaryKnobLookAndFeel knobLookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
    bool tooltipsEnabled = false;

    // Header buttons (shared)
    GearButton settingsButton;
    UpdateBadgeButton updateBadgeButton;
    juce::TextButton tooltipsButton;
    juce::TextButton initButton;
    juce::TextButton triggerButton;
    juce::TextButton guideButton;

    std::unique_ptr<SettingsModalComponent> settingsModal;
    StatusBarComponent statusBar;

public:
    StatusBarComponent& getStatusBar() { return statusBar; }
    juce::TextButton& getTooltipsButton() { return tooltipsButton; }

protected:
    void setTooltipsEnabled(bool enabled);
};
