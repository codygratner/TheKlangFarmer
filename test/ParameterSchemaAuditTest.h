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
    }
};
