#include "ParameterManager.h"
#include "TkfAssets.h"

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
}

void ParameterManager::parseJsonBlob(const char* data, int size) {
    juce::String jsonString = juce::String::fromUTF8(data, size);
    auto var = juce::JSON::parse(jsonString);

    if (var.isObject()) {
        auto* obj = var.getDynamicObject();
        for (auto& prop : obj->getProperties()) {
            ControlDef def;
            def.id = prop.name.toString();
            
            auto& val = prop.value;
            if (!val.isObject())
                continue;

            auto* vObj = val.getDynamicObject();
            
            def.type = vObj->getProperty("type").toString();
            def.name = vObj->getProperty("name").toString();
            def.description = vObj->getProperty("description").toString();

            if (def.type == "float") {
                def.isBipolar = vObj->hasProperty("is_bipolar") ? static_cast<bool>(vObj->getProperty("is_bipolar")) : false;
                def.defaultFloat = vObj->hasProperty("default") ? static_cast<float>(vObj->getProperty("default")) : 0.0f;
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

                if (vObj->hasProperty("points_of_interest")) {
                    auto& poiArray = vObj->getProperty("points_of_interest");
                    if (poiArray.isArray()) {
                        for (auto& poiVar : *poiArray.getArray()) {
                            if (poiVar.isObject()) {
                                auto* poiObj = poiVar.getDynamicObject();
                                PointOfInterest poi;
                                poi.value = poiObj->hasProperty("value") ? static_cast<float>(poiObj->getProperty("value")) : 0.0f;
                                poi.label = poiObj->getProperty("label").toString();
                                def.pointsOfInterest.push_back(poi);
                            }
                        }
                    }
                }
            } else if (def.type == "choice") {
                def.defaultChoice = vObj->hasProperty("default") ? static_cast<int>(vObj->getProperty("default")) : 0;
                if (vObj->hasProperty("choices")) {
                    auto& choicesArray = vObj->getProperty("choices");
                    if (choicesArray.isArray()) {
                        for (auto& choiceVar : *choicesArray.getArray()) {
                            def.choices.add(choiceVar.toString());
                        }
                    }
                }
            }

            controls[def.id] = def;
        }
    }
}

} // namespace RlyehSound
