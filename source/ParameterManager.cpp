#include "ParameterManager.h"
#include "TkfAssets.h"
#include "DevLogger.h"

namespace RlyehSound {

ParameterManager::ParameterManager() {
    for (int i = 0; i < TkfAssets::namedResourceListSize; ++i) {
        int dataSizeInBytes = 0;
        const char* data = TkfAssets::getNamedResource(TkfAssets::namedResourceList[i], dataSizeInBytes);
        
        // Ensure it's a JSON file based on name
        juce::String resourceName(TkfAssets::namedResourceList[i]);
        if (resourceName.endsWithIgnoreCase(".json") || resourceName.endsWithIgnoreCase("_json")) {
            parseJsonBlob(data, dataSizeInBytes);
        }
    }
    mergeTextIntoControls();
    TKS_LOG_INFO("ParameterManager: initialized with " + juce::String(controls.size()) + " controls, " + juce::String(textDescriptions.size()) + " descriptions, and " + juce::String(textChoiceTooltips.size()) + " choice tooltips");
}

juce::Colour ParameterManager::getModuleColor(const juce::String& colorId, juce::Colour defaultFallback) const {
    auto it = moduleColors.find(colorId);
    if (it != moduleColors.end()) {
        return it->second;
    }
    return defaultFallback;
}

juce::Colour ParameterManager::getGlobalColor(const juce::String& colorId, juce::Colour defaultFallback) const {
    auto it = globalColors.find(colorId);
    if (it != globalColors.end()) {
        return it->second;
    }
    auto itMod = moduleColors.find(colorId);
    if (itMod != moduleColors.end()) {
        return itMod->second;
    }
    return defaultFallback;
}

void ParameterManager::parseJsonBlob(const char* data, int size) {
    juce::String jsonString = juce::String::fromUTF8(data, size);
    auto var = juce::JSON::parse(jsonString);

    if (var.isObject()) {
        auto* obj = var.getDynamicObject();
        if (obj->hasProperty("shared") || obj->hasProperty("farmer") || obj->hasProperty("planter")) {
            parseStringsJson(var);
            return;
        }

        for (auto& prop : obj->getProperties()) {
            ControlDef def;
            def.id = prop.name.toString();
            
            auto& val = prop.value;
            auto* vObj = val.getDynamicObject();
            if (vObj == nullptr)
                continue;
            
            if (def.id == "ui_colors" || def.id == "module_colors") {
                for (auto& colorProp : vObj->getProperties()) {
                    juce::String hex = colorProp.value.toString();
                    if (hex.startsWithIgnoreCase("0x")) hex = hex.substring(2);
                    if (hex.startsWithIgnoreCase("#")) hex = hex.substring(1);
                    moduleColors[colorProp.name.toString()] = juce::Colour::fromString(hex.length() == 6 ? "ff" + hex : hex);
                }
                continue;
            }

            if (def.id == "global_colors") {
                for (auto& colorProp : vObj->getProperties()) {
                    juce::String hex = colorProp.value.toString();
                    if (hex.startsWithIgnoreCase("0x")) hex = hex.substring(2);
                    if (hex.startsWithIgnoreCase("#")) hex = hex.substring(1);
                    globalColors[colorProp.name.toString()] = juce::Colour::fromString(hex.length() == 6 ? "ff" + hex : hex);
                }
                continue;
            }

            if (def.id == "ui_strings" || def.id == "global_strings") {
                for (auto& strProp : vObj->getProperties()) {
                    globalStrings[strProp.name.toString()] = strProp.value.toString();
                }
                continue;
            }

            def.type = vObj->getProperty("type").toString();
            def.name = vObj->getProperty("name").toString();
            def.description = vObj->getProperty("description").toString();

            if (def.type == "float") {
                def.isBipolar = vObj->hasProperty("is_bipolar") ? static_cast<bool>(vObj->getProperty("is_bipolar")) : false;
                def.format = vObj->hasProperty("format") ? vObj->getProperty("format").toString() : juce::String();
                if (vObj->hasProperty("default"))
                    def.defaultFloat = static_cast<float>(vObj->getProperty("default"));
                else if (vObj->hasProperty("defaultFloat"))
                    def.defaultFloat = static_cast<float>(vObj->getProperty("defaultFloat"));
                else
                    def.defaultFloat = 0.0f;
                def.defaultLabel = vObj->hasProperty("default_label") ? vObj->getProperty("default_label").toString() : juce::String();
                def.doubleClickValue = vObj->hasProperty("double_click") ? static_cast<float>(vObj->getProperty("double_click")) : def.defaultFloat;

                if (vObj->hasProperty("range")) {
                    auto* rangeObj = vObj->getProperty("range").getDynamicObject();
                    if (rangeObj) {
                        def.min = rangeObj->hasProperty("min") ? static_cast<float>(rangeObj->getProperty("min")) : 0.0f;
                        def.max = rangeObj->hasProperty("max") ? static_cast<float>(rangeObj->getProperty("max")) : 1.0f;
                        def.step = rangeObj->hasProperty("step") ? static_cast<float>(rangeObj->getProperty("step")) : 0.01f;
                        def.skew = rangeObj->hasProperty("skew") ? static_cast<float>(rangeObj->getProperty("skew")) : 1.0f;
                    }
                }

                if (vObj->hasProperty("snap_points")) {
                    auto& poiArray = vObj->getProperty("snap_points");
                    if (poiArray.isArray()) {
                        for (auto& poiVar : *poiArray.getArray()) {
                            if (poiVar.isObject()) {
                                auto* poiObj = poiVar.getDynamicObject();
                                SnapPoint poi;
                                poi.value = poiObj->hasProperty("value") ? static_cast<float>(double(poiObj->getProperty("value"))) : 0.0f;
                                poi.label = poiObj->getProperty("label").toString();
                                def.snapPoints.push_back(poi);
                            }
                        }
                    }
                }
            } else if (def.type == "choice") {
                if (vObj->hasProperty("default"))
                    def.defaultChoice = static_cast<int>(vObj->getProperty("default"));
                else if (vObj->hasProperty("defaultChoice"))
                    def.defaultChoice = static_cast<int>(vObj->getProperty("defaultChoice"));
                else
                    def.defaultChoice = 0;
                if (vObj->hasProperty("choices")) {
                    auto& choicesArray = vObj->getProperty("choices");
                    if (choicesArray.isArray()) {
                        for (auto& choiceVar : *choicesArray.getArray()) {
                            def.choices.add(choiceVar.toString());
                        }
                    }
                }
                if (vObj->hasProperty("choice_tooltips")) {
                    auto& tooltipsArray = vObj->getProperty("choice_tooltips");
                    if (tooltipsArray.isArray()) {
                        for (auto& tVar : *tooltipsArray.getArray()) {
                            def.choiceTooltips.add(tVar.toString());
                        }
                    }
                }
            }

            controls[def.id] = def;
        }
    }
}

void ParameterManager::parseStringsJson(const juce::var& var) {
    if (!var.isObject()) return;
    auto* rootObj = var.getDynamicObject();

    // 1. Shared strings
    if (rootObj->hasProperty("shared")) {
        auto& sharedVar = rootObj->getProperty("shared");
        if (sharedVar.isObject()) {
            auto* sObj = sharedVar.getDynamicObject();
            for (auto& prop : sObj->getProperties()) {
                globalStrings[prop.name.toString()] = prop.value.toString();
            }
        }
    }

    // 2. Namespaced product strings (farmer, planter)
    auto parseProductStrings = [this](const juce::var& pVar) {
        if (!pVar.isObject()) return;
        auto* pObj = pVar.getDynamicObject();
        for (auto& modProp : pObj->getProperties()) {
            if (!modProp.value.isObject()) continue;
            auto* modObj = modProp.value.getDynamicObject();
            for (auto& paramProp : modObj->getProperties()) {
                juce::String paramId = paramProp.name.toString();
                if (!paramProp.value.isObject()) continue;
                auto* entryObj = paramProp.value.getDynamicObject();
                if (entryObj->hasProperty("description")) {
                    textDescriptions[paramId] = entryObj->getProperty("description").toString();
                }
                if (entryObj->hasProperty("choice_tooltips")) {
                    auto& tVar = entryObj->getProperty("choice_tooltips");
                    if (tVar.isArray()) {
                        juce::StringArray tooltips;
                        for (auto& item : *tVar.getArray()) {
                            tooltips.add(item.toString());
                        }
                        textChoiceTooltips[paramId] = tooltips;
                    }
                }
            }
        }
    };

    if (rootObj->hasProperty("farmer"))
        parseProductStrings(rootObj->getProperty("farmer"));
    if (rootObj->hasProperty("planter"))
        parseProductStrings(rootObj->getProperty("planter"));
}

void ParameterManager::mergeTextIntoControls() {
    for (auto& [paramId, desc] : textDescriptions) {
        auto it = controls.find(paramId);
        if (it != controls.end()) {
            it->second.description = desc;
        }
    }
    for (auto& [paramId, tooltips] : textChoiceTooltips) {
        auto it = controls.find(paramId);
        if (it != controls.end()) {
            it->second.choiceTooltips = tooltips;
        }
    }
}

void ParameterManager::reloadFromJson(const juce::String& jsonString) {
    auto stdString = jsonString.toStdString();
    parseJsonBlob(stdString.c_str(), static_cast<int>(stdString.size()));
    mergeTextIntoControls();
    TKS_LOG_INFO("ParameterManager: reloaded from JSON, total controls: " + juce::String(controls.size()));
}

} // namespace RlyehSound

