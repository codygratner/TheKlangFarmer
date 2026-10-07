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
    juce::String defaultLabel;

    // Choice specifics
    juce::StringArray choices;
    int defaultChoice { 0 };
    juce::StringArray choiceTooltips;
};

struct FxAlgorithmDef {
    juce::String id;
    juce::String name;
    juce::String description;
    juce::StringArray knobParams; // Virtual parameter IDs mapped to knobs 1-4
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

    const FxAlgorithmDef* getFxAlgorithmDef(int fxIndex) const {
        auto it = fxAlgorithms.find(fxIndex);
        if (it != fxAlgorithms.end()) {
            return &it->second;
        }
        return nullptr;
    }

    juce::String getGlobalString(const juce::String& id, const juce::String& fallback = "") const {
        auto it = globalStrings.find(id);
        if (it != globalStrings.end()) {
            return it->second;
        }
        return fallback;
    }

    juce::String getTooltip(const juce::String& id) const {
        if (auto def = getControlDef(id))
            return def->description;
        return {};
    }


    juce::Colour getModuleColor(const juce::String& colorId, juce::Colour defaultFallback = juce::Colours::transparentBlack) const;
    juce::Colour getGlobalColor(const juce::String& colorId, juce::Colour defaultFallback = juce::Colours::transparentBlack) const;
    void reloadFromJson(const juce::String& jsonString);
private:
    ParameterManager();
    ~ParameterManager() = default;

    ParameterManager(const ParameterManager&) = delete;
    ParameterManager& operator=(const ParameterManager&) = delete;

    std::unordered_map<juce::String, juce::Colour> moduleColors;
    std::unordered_map<juce::String, juce::Colour> globalColors;
    std::unordered_map<juce::String, ControlDef> controls;
    std::unordered_map<int, FxAlgorithmDef> fxAlgorithms;
    std::unordered_map<juce::String, juce::String> globalStrings;

    void parseJsonBlob(const char* data, int size);
};

} // namespace RlyehSound






