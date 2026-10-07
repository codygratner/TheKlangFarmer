#pragma once
#include "GuiTestHelpers.h"
#include "../source/FarmerProcessor.h"
#include "../source/ParameterManager.h"

class ParameterSchemaAuditTest {
public:
    static void runSuite(GuiTestHelpers::TestReporter& reporter) {
        reporter.beginTest("Parameter Schema Compliance Audit");

        TheKlangFarmerAudioProcessor processor;
        auto& pm = RlyehSound::ParameterManager::getInstance();
        
        // Ensure ParameterManager has loaded JSON
        // Actually it loads them in constructor automatically via TkfAssets
        
        int paramCount = 0;
        int passCount = 0;
        bool allPassed = true;


        
        for (auto* node : processor.getParameters()) {
            if (auto* param = dynamic_cast<juce::AudioProcessorParameterWithID*>(node)) {
                juce::String paramId = param->paramID;
                if (paramId.startsWith("unused") || paramId.isEmpty() || 
                    paramId.startsWith("pre_fx_") || paramId.startsWith("post_fx_")) continue;
                
                paramCount++;
                const auto* def = pm.getControlDef(paramId);
                if (def == nullptr) {
                    reporter.expect(false, "APVTS parameter '" + paramId + "' is missing from JSON schema!");
                    allPassed = false;
                    continue;
                }
                
                if (def->type == "float") {
                    bool minMaxValid = def->min < def->max;
                    bool defaultValid = def->defaultFloat >= def->min && def->defaultFloat <= def->max;
                    bool doubleClickValid = def->doubleClickValue >= def->min && def->doubleClickValue <= def->max;
                    bool skewValid = def->skew > 0.0f;
                    
                    if (!minMaxValid) { reporter.expect(false, paramId + ": min >= max"); allPassed = false; }
                    if (!defaultValid) { reporter.expect(false, paramId + ": default outside bounds (" + juce::String(def->defaultFloat) + ")"); allPassed = false; }
                    if (!doubleClickValid) { reporter.expect(false, paramId + ": double_click outside bounds (" + juce::String(def->doubleClickValue) + ")"); allPassed = false; }
                    if (!skewValid) { reporter.expect(false, paramId + ": invalid skew (" + juce::String(def->skew) + ")"); allPassed = false; }
                    
                    if (minMaxValid && defaultValid && doubleClickValid && skewValid) {
                        passCount++;
                    }
                } else if (def->type == "choice") {
                    passCount++; // Choice params are simpler
                }
            }
        }
        
        reporter.expect(allPassed, "All " + juce::String(paramCount) + " APVTS parameters comply with strict schema constraints.");

        // --- Schema Separation of Concerns Audit ---
        reporter.beginTest("Schema Separation of Concerns Audit");

        auto getAssetsFolder = []() -> juce::File {
            auto cur = juce::File::getCurrentWorkingDirectory();
            while (cur.getParentDirectory() != cur) {
                auto a = cur.getChildFile("assets");
                if (a.isDirectory()) return a;
                cur = cur.getParentDirectory();
            }
            return {};
        };

        auto assetsFolder = getAssetsFolder();
        reporter.expect(assetsFolder.isDirectory(), "Assets folder located successfully");

        if (assetsFolder.isDirectory()) {
            auto controlsDir = assetsFolder.getChildFile("controls");
            reporter.expect(controlsDir.getChildFile("modulator.json").existsAsFile(), "modulator.json (singular) exists");
            reporter.expect(controlsDir.getChildFile("filter.json").existsAsFile(), "filter.json (singular) exists");
            reporter.expect(controlsDir.getChildFile("envelope.json").existsAsFile(), "envelope.json (singular) exists");
            reporter.expect(!controlsDir.getChildFile("modulators.json").existsAsFile(), "modulators.json (plural) does not exist");
            reporter.expect(!controlsDir.getChildFile("filters.json").existsAsFile(), "filters.json (plural) does not exist");
            reporter.expect(!controlsDir.getChildFile("envelopes.json").existsAsFile(), "envelopes.json (plural) does not exist");

            juce::DirectoryIterator iter(controlsDir, false, "*.json");
            int controlFilesAudited = 0;
            bool leakFound = false;

            while (iter.next()) {
                auto f = iter.getFile();
                controlFilesAudited++;
                auto parsed = juce::JSON::parse(f.loadFileAsString());
                reporter.expect(parsed.isObject(), "Control file " + f.getFileName() + " parses as valid JSON object");
                if (parsed.isObject()) {
                    auto* obj = parsed.getDynamicObject();
                    if (obj->hasProperty("ui_strings")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'ui_strings'");
                        leakFound = true;
                    }
                    if (obj->hasProperty("global_strings")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'global_strings'");
                        leakFound = true;
                    }
                    if (obj->hasProperty("ui_colors")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'ui_colors'");
                        leakFound = true;
                    }
                    if (obj->hasProperty("callout_styles")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'callout_styles'");
                        leakFound = true;
                    }
                    if (obj->hasProperty("width")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'width'");
                        leakFound = true;
                    }
                    if (obj->hasProperty("height")) {
                        reporter.expect(false, "Schema leak in " + f.getFileName() + ": contains 'height'");
                        leakFound = true;
                    }
                    for (auto& prop : obj->getProperties()) {
                        auto* pObj = prop.value.getDynamicObject();
                        if (pObj != nullptr) {
                            if (pObj->hasProperty("description")) {
                                reporter.expect(false, "Schema leak in " + f.getFileName() + " (" + prop.name.toString() + "): contains 'description'");
                                leakFound = true;
                            }
                            if (pObj->hasProperty("choice_tooltips")) {
                                reporter.expect(false, "Schema leak in " + f.getFileName() + " (" + prop.name.toString() + "): contains 'choice_tooltips'");
                                leakFound = true;
                            }
                        }
                    }
                }
            }
            reporter.expect(controlFilesAudited > 0, "Audited " + juce::String(controlFilesAudited) + " control JSON files");
            reporter.expect(!leakFound, "assets/controls/*.json contains zero visual styling, description, or choice_tooltip leaks");

            // Audit assets/text/strings.json
            auto stringsFile = assetsFolder.getChildFile("text").getChildFile("strings.json");
            reporter.expect(stringsFile.existsAsFile(), "assets/text/strings.json exists");
            if (stringsFile.existsAsFile()) {
                auto parsedStrings = juce::JSON::parse(stringsFile.loadFileAsString());
                reporter.expect(parsedStrings.isObject(), "strings.json parses as valid JSON object");
                if (parsedStrings.isObject()) {
                    auto* sObj = parsedStrings.getDynamicObject();
                    reporter.expect(sObj->hasProperty("shared"), "strings.json has 'shared' namespace");
                    reporter.expect(sObj->hasProperty("farmer"), "strings.json has 'farmer' namespace");
                    reporter.expect(sObj->hasProperty("planter"), "strings.json has 'planter' namespace");
                }
            }

            // Verify ParameterManager merged strings into ControlDef
            const auto* c1Shape = pm.getControlDef("carrier1_shape");
            reporter.expect(c1Shape != nullptr && c1Shape->description.isNotEmpty(), "ParameterManager merged description for carrier1_shape from strings.json");
            const auto* c1Track = pm.getControlDef("carrier1_tracking");
            reporter.expect(c1Track != nullptr && c1Track->choiceTooltips.size() > 0, "ParameterManager merged choiceTooltips for carrier1_tracking from strings.json");
            const auto* limGain = pm.getControlDef("planter_limiter_gain");
            reporter.expect(limGain != nullptr && limGain->description.isNotEmpty(), "ParameterManager merged description for planter_limiter_gain from strings.json");

            // Audit assets/themes/theme.json
            auto themeFile = assetsFolder.getChildFile("themes").getChildFile("theme.json");
            reporter.expect(themeFile.existsAsFile(), "assets/themes/theme.json exists");
            if (themeFile.existsAsFile()) {
                auto parsedTheme = juce::JSON::parse(themeFile.loadFileAsString());
                reporter.expect(parsedTheme.isObject(), "theme.json parses as valid JSON object");
                if (parsedTheme.isObject()) {
                    auto* obj = parsedTheme.getDynamicObject();
                    reporter.expect(!obj->hasProperty("global_strings"), "theme.json contains zero 'global_strings' (cleanly extracted)");
                    reporter.expect(obj->hasProperty("module_colors"), "theme.json has 'module_colors'");
                    reporter.expect(obj->hasProperty("global_colors"), "theme.json has 'global_colors'");
                }
            }

            // Audit assets/themes/callouts.json
            auto calloutsFile = assetsFolder.getChildFile("themes").getChildFile("callouts.json");
            reporter.expect(calloutsFile.existsAsFile(), "assets/themes/callouts.json exists");
            if (calloutsFile.existsAsFile()) {
                auto parsedCallouts = juce::JSON::parse(calloutsFile.loadFileAsString());
                reporter.expect(parsedCallouts.isObject(), "callouts.json parses as valid JSON object");
                if (parsedCallouts.isObject()) {
                    auto* obj = parsedCallouts.getDynamicObject();
                    reporter.expect(obj->hasProperty("callout_styles"), "callouts.json has 'callout_styles'");
                    if (obj->hasProperty("callout_styles")) {
                        auto* cStyles = obj->getProperty("callout_styles").getDynamicObject();
                        reporter.expect(cStyles != nullptr && cStyles->hasProperty("planter_limiter"), "callouts.json defines 'planter_limiter'");
                        if (cStyles && cStyles->hasProperty("planter_limiter")) {
                            auto* limDef = cStyles->getProperty("planter_limiter").getDynamicObject();
                            reporter.expect(limDef != nullptr && limDef->hasProperty("parameters") && limDef->getProperty("parameters").isArray(),
                                            "'planter_limiter' contains 'parameters' array");
                            if (limDef && limDef->getProperty("parameters").isArray()) {
                                reporter.expect(limDef->getProperty("parameters").getArray()->size() == 4,
                                                "'planter_limiter' parameters array has 4 items");
                            }
                        }
                    }
                }
            }
        }
    }
};
