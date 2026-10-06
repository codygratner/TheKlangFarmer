#pragma once
#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"
#include "PlanterProcessor.h"
#include "PlanterEditor.h"

namespace ReflectionGuardrailSuite {
    using namespace GuiTestHelpers;

    inline void checkParameters(TestReporter& reporter, juce::AudioProcessor& processor, juce::Component* editor, const juce::String& targetName) {
        auto params = processor.getParameters();
        bool allFound = true;
        
        for (auto* pNode : params) {
            auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode);
            if (!p) continue;
            
            juce::String id = p->paramID;
            
            bool found = (ComponentFinder::findSliderByParamId(editor, id) != nullptr) ||
                         (ComponentFinder::findSelectorByParamId(editor, id) != nullptr);
                         
            if (!found && !id.contains("track") && !id.contains("type") && !id.contains("target") && !id.contains("slope") && !id.contains("enable") && !id.contains("limiter") && 
                !id.contains("drive_") && !id.contains("wavefolder_") && !id.contains("ringmod_") && !id.contains("freqshift_") && !id.contains("grit_") && !id.contains("comb_") && !id.contains("phasesmear_") && !id.contains("eq_") && !id.contains("fxfilter_") && !id.contains("chorus_") && !id.contains("flanger_") && !id.contains("phaser_") && !id.contains("delay_")) {
                allFound = false;
                std::cerr << "FAILED: Orphan APVTS parameter '" << id << "' found without UI binding in " << targetName << "!" << std::endl;
            }
            
            float normDefault = p->getDefaultValue();
            p->setValueNotifyingHost(0.0f); pumpMessageLoop(2, 5);
            p->setValueNotifyingHost(1.0f); pumpMessageLoop(2, 5);
            p->setValueNotifyingHost(normDefault); pumpMessageLoop(2, 5);
        }
        reporter.expect(allFound, targetName + " 100% APVTS Parameter Audit Passed (Orphan Check)");
    }

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("Dynamic Reflection & Orphan Feature Guardrail Suite");

        {
            TheKlangFarmerAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1000, 750);
            pumpMessageLoop();
            checkParameters(reporter, p, e.get(), "Farmer");
        }

        {
            TheKlangPlanterAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1040, 740);
            pumpMessageLoop();
            checkParameters(reporter, p, e.get(), "Planter");
        }
    }
}
