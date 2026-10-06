#pragma once
#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"
#include "PlanterProcessor.h"
#include "PlanterEditor.h"
#include "../tools/editor/MainComponent.h"

namespace SmokePaintSuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("Smoke Paint Rendering Suite");

        // The Klang Farmer
        {
            TheKlangFarmerAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1000, 750);
            pumpMessageLoop();
            
            auto img = e->createComponentSnapshot(e->getLocalBounds());
            reporter.expect(img.isValid() && img.getWidth() == 1000 && img.getHeight() == 750, "Farmer editor painted offscreen successfully");
        }

        // The Klang Planter
        {
            TheKlangPlanterAudioProcessor p;
            auto e = std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
            e->setSize(1040, 740);
            pumpMessageLoop();
            
            auto img = e->createComponentSnapshot(e->getLocalBounds());
            reporter.expect(img.isValid() && img.getWidth() == 1040 && img.getHeight() == 740, "Planter editor painted offscreen successfully");
        }

        // The Klang Editor
        {
            MainComponent e;
            e.setSize(1200, 800);
            pumpMessageLoop();
            
            auto img = e.createComponentSnapshot(e.getLocalBounds());
            reporter.expect(img.isValid() && img.getWidth() == 1200 && img.getHeight() == 800, "Standalone Editor painted offscreen successfully");
        }
    }
}
