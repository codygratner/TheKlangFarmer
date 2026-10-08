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
// 3. Clickable Swatch Pill Button for Live Theme Tints
// ==============================================================================
class SwatchButton : public juce::Button
{
public:
    SwatchButton(const juce::String& name, juce::Colour color, const juce::String& tooltipText = "")
        : juce::Button(name), swatchColor(color)
    {
        if (tooltipText.isNotEmpty()) setTooltip(tooltipText);
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
    ~SwatchButton() override = default;

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        if (shouldDrawButtonAsDown) bounds.translate(0.0f, 1.0f);

        g.setColour(swatchColor);
        g.fillRoundedRectangle(bounds, 4.0f);

        juce::Colour borderCol = shouldDrawButtonAsHighlighted ? juce::Colours::white : juce::Colour(0xff2a3245);
        g.setColour(borderCol);
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.setColour(swatchColor.getBrightness() > 0.5f ? juce::Colours::black : juce::Colours::white);
        g.drawText(getButtonText(), bounds, juce::Justification::centred, true);
    }

    juce::Colour getSwatchColor() const { return swatchColor; }

private:
    juce::Colour swatchColor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwatchButton)
};

// ==============================================================================
// 4. Settings & About Modal Component
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

    juce::ComboBox& getThemeBox() { return themeBox; }
    const std::vector<std::unique_ptr<SwatchButton>>& getAccentSwatches() const { return accentSwatches; }
    const std::vector<std::unique_ptr<SwatchButton>>& getBackgroundSwatches() const { return backgroundSwatches; }

private:
    juce::String productName;
    juce::TextButton closeButton { "X" };

    // Theme Engine Section
    juce::Label themeLabel;
    juce::ComboBox themeBox;
    juce::Label accentLabel;
    std::vector<std::unique_ptr<SwatchButton>> accentSwatches;
    juce::Label bgLabel;
    std::vector<std::unique_ptr<SwatchButton>> backgroundSwatches;

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
