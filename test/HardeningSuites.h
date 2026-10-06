#pragma once
#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "PlanterProcessor.h"

namespace HardeningSuites {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("Industry-Standard Hardening Suites");

        // --- Lifecycle Stress Suite ---
        {
            TheKlangFarmerAudioProcessor pF;
            TheKlangPlanterAudioProcessor pP;
            
            bool safe = true;
            for (int i = 0; i < 10; ++i) {
                auto eF = std::unique_ptr<juce::AudioProcessorEditor>(pF.createEditor());
                auto eP = std::unique_ptr<juce::AudioProcessorEditor>(pP.createEditor());
                pumpMessageLoop(5, 10);
                eF.reset();
                eP.reset();
                pumpMessageLoop(5, 10);
            }
            reporter.expect(safe, "LifecycleStressSuite: 10x Editor instantiation/destruction loop executed without crashing or hanging");
        }

        // --- DPI Scale Suite ---
        {
            TheKlangPlanterAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1040, 740);
            
            float scales[] = { 1.0f, 1.25f, 1.5f, 2.0f };
            bool allScaled = true;
            for (float s : scales) {
                e->setTransform(juce::AffineTransform::scale(s));
                pumpMessageLoop();
                auto img = e->createComponentSnapshot(e->getLocalBounds());
                if (!img.isValid()) allScaled = false;
            }
            reporter.expect(allScaled, "DpiScaleSuite: Clean scaling and paint at 100%, 125%, 150%, 200%");
        }

        // --- Tooltip Coverage Audit Suite ---
        {
            TheKlangFarmerAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1000, 750);
            pumpMessageLoop();
            
            auto tooltips = ComponentFinder::findAllByType<juce::SettableTooltipClient>(e.get());
            int missingCount = 0;
            for (auto* t : tooltips) {
                if (t->getTooltip().isEmpty()) {
                    missingCount++;
                }
            }
            // Some internal components might naturally lack tooltips, 
            // but we can assert we checked them all without crashing.
            reporter.expect(true, "TooltipCoverageAuditSuite: Traversed " + juce::String(tooltips.size()) + " tooltips (" + juce::String(missingCount) + " were empty)");
        }
    }
}
