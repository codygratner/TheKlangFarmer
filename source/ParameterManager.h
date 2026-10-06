#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <unordered_map>
#include <vector>

namespace RlyehSound {

struct SnapPoint {
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
    juce::String format;
    float min { 0.0f };
    float max { 1.0f };
    float step { 0.0005f };
    float skew { 1.0f };
    float defaultFloat { 0.0f };
    float doubleClickValue { 0.0f };
    std::vector<SnapPoint> snapPoints;

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


    juce::Colour getModuleColor(const juce::String& colorId, juce::Colour defaultFallback = juce::Colours::transparentBlack) const;
    void reloadFromJson(const juce::String& jsonString);
private:
    ParameterManager();
    ~ParameterManager() = default;

    ParameterManager(const ParameterManager&) = delete;
    ParameterManager& operator=(const ParameterManager&) = delete;

    std::unordered_map<juce::String, juce::Colour> moduleColors;
    std::unordered_map<juce::String, ControlDef> controls;

    void parseJsonBlob(const char* data, int size);
};

} // namespace RlyehSound






