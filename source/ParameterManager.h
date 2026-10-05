#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>
#include <vector>

namespace RlyehSound {

struct PointOfInterest {
    float value { 0.0f };
    juce::String label;
};

struct ControlDef {
    juce::String id;
    juce::String type;
    juce::String name;
    juce::String description;

    // Float specifics
    bool isBipolar { false };
    float min { 0.0f };
    float max { 1.0f };
    float step { 0.0005f };
    float skew { 1.0f };
    float defaultFloat { 0.0f };
    float doubleClickValue { 0.0f };
    std::vector<PointOfInterest> pointsOfInterest;

    // Choice specifics
    juce::StringArray choices;
    int defaultChoice { 0 };
};

class ParameterManager {
public:
    static ParameterManager& getInstance() {
        static ParameterManager instance;
        return instance;
    }

    const ControlDef* getControlDef(const juce::String& id) const {
        auto it = controls.find(id);
        if (it != controls.end())
            return &(it->second);
        return nullptr;
    }

    const std::unordered_map<juce::String, ControlDef>& getAllControls() const {
        return controls;
    }

    juce::String getTooltip(const juce::String& id) const {
        if (auto def = getControlDef(id))
            return def->description;
        return {};
    }

    juce::Colour getThemeColour(const juce::String& colorId, juce::Colour defaultColour = juce::Colours::transparentBlack) const;
    const juce::var& getThemeData() const { return themeData; }

private:
    ParameterManager();
    ~ParameterManager() = default;

    ParameterManager(const ParameterManager&) = delete;
    ParameterManager& operator=(const ParameterManager&) = delete;

    std::unordered_map<juce::String, ControlDef> controls;
    juce::var themeData;

    void parseJsonBlob(const char* data, int size, bool isTheme = false);
};

} // namespace RlyehSound
