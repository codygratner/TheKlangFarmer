#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "VersionChecker.h"
#include "ParameterManager.h"

// ==============================================================================
// 1. Sleek Vector Gear Button for Header
// ==============================================================================
class GearButton : public juce::Button
{
public:
    GearButton();
    ~GearButton() override = default;

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearButton)
};

// ==============================================================================
// 2. Glowing Update Available Notification Badge Button
// ==============================================================================
class UpdateBadgeButton : public juce::Button, public juce::ChangeListener
{
public:
    UpdateBadgeButton();
    ~UpdateBadgeButton() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    juce::String candidateVersion;
    juce::String releaseUrl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateBadgeButton)
};

// ==============================================================================
// 3. Settings & About Modal Component
// ==============================================================================
class SettingsModalComponent : public juce::Component, public juce::ChangeListener
{
public:
    explicit SettingsModalComponent(const juce::String& pluginName = "The Klang Farmer");
    ~SettingsModalComponent() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void setPluginName(const juce::String& name);

private:
    juce::String productName;
    juce::TextButton closeButton { "X" };

    juce::ToggleButton checkOnLaunchToggle;
    juce::TextButton checkNowButton;
    juce::TextButton downloadButton;
    juce::Label statusLabel;

    juce::TextButton githubButton;
    juce::TextButton issuesButton;

    juce::Rectangle<int> getCardBounds() const;
    void updateStatusDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsModalComponent)
};
