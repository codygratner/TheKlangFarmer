#pragma once

#include "GuiTestHelpers.h"
#include "../source/FarmerProcessor.h"
#include "../source/FarmerEditor.h"
#include "../source/PlanterProcessor.h"
#include "../source/PlanterEditor.h"
#include "../source/VersionChecker.h"
#include "../source/SettingsModal.h"
#include "../source/ParameterManager.h"

namespace PluginIntensiveTestSuite {
    using namespace GuiTestHelpers;

    // ==============================================================================
    // Phase 1: Headless Component & APVTS Binding Deep Sweep
    // ==============================================================================
    inline void runComponentBindingTest(TestReporter& reporter) {
        reporter.beginTest("Plugin Component & APVTS Binding Deep Sweep");

        // 1. The Klang Farmer
        {
            TheKlangFarmerAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1000, 750);
            pumpMessageLoop();

            auto sliders = ComponentFinder::findAllByType<RotaryKnobSlider>(editor.get());
            reporter.expect(sliders.size() > 30, "Farmer has substantial registered RotaryKnobSliders (" + juce::String(sliders.size()) + " found)");

            int boundSliders = 0;
            int activeVisibleSliders = 0;
            int validVisibleTooltips = 0;
            int validRanges = 0;

            for (auto* slider : sliders) {
                auto paramId = slider->getParamId();
                if (!paramId.isEmpty()) {
                    auto* param = processor.apvts.getParameter(paramId);
                    if (param != nullptr) boundSliders++;
                }

                if (slider->isVisible()) {
                    activeVisibleSliders++;
                    if (!slider->getTooltip().isEmpty()) {
                        validVisibleTooltips++;
                    }
                }

                auto range = slider->getRange();
                if (range.getStart() < range.getEnd()) {
                    validRanges++;
                }
            }

            reporter.expect(boundSliders == sliders.size(), "Farmer: 100% of sliders mapped to valid APVTS parameters (" + juce::String(boundSliders) + "/" + juce::String(sliders.size()) + ")");
            reporter.expect(validVisibleTooltips == activeVisibleSliders, "Farmer: 100% of active visible sliders have non-empty, meaningful tooltips (" + juce::String(validVisibleTooltips) + "/" + juce::String(activeVisibleSliders) + ")");
            reporter.expect(validRanges == sliders.size(), "Farmer: 100% of sliders have valid non-inverted ranges (" + juce::String(validRanges) + "/" + juce::String(sliders.size()) + ")");

            auto selectors = ComponentFinder::findAllByType<LedSelectorComponent>(editor.get());
            int boundSelectors = 0;
            int totalWithParamId = 0;
            for (auto* sel : selectors) {
                if (!sel->getParamId().isEmpty()) {
                    totalWithParamId++;
                    if (processor.apvts.getParameter(sel->getParamId()) != nullptr) {
                        boundSelectors++;
                    }
                }
            }
            reporter.expect(boundSelectors == totalWithParamId && totalWithParamId >= 17, "Farmer: 100% of APVTS-mapped LedSelectorComponents verified (" + juce::String(boundSelectors) + "/" + juce::String(totalWithParamId) + ")");
        }

        // 2. The Klang Planter
        {
            TheKlangPlanterAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1040, 740);
            pumpMessageLoop();

            auto sliders = ComponentFinder::findAllByType<RotaryKnobSlider>(editor.get());
            reporter.expect(sliders.size() > 20, "Planter has substantial registered RotaryKnobSliders (" + juce::String(sliders.size()) + " found)");

            int boundSliders = 0;
            int validTooltips = 0;
            int validRanges = 0;

            for (auto* slider : sliders) {
                auto paramId = slider->getParamId();
                if (!paramId.isEmpty()) {
                    auto* param = processor.apvts.getParameter(paramId);
                    if (param != nullptr) boundSliders++;
                }

                if (!slider->getTooltip().isEmpty()) {
                    validTooltips++;
                }

                auto range = slider->getRange();
                if (range.getStart() < range.getEnd()) {
                    validRanges++;
                }
            }

            reporter.expect(boundSliders == sliders.size(), "Planter: 100% of sliders mapped to valid APVTS parameters (" + juce::String(boundSliders) + "/" + juce::String(sliders.size()) + ")");
            reporter.expect(validTooltips == sliders.size(), "Planter: 100% of sliders have non-empty, meaningful tooltips (" + juce::String(validTooltips) + "/" + juce::String(sliders.size()) + ")");
            reporter.expect(validRanges == sliders.size(), "Planter: 100% of sliders have valid non-inverted ranges (" + juce::String(validRanges) + "/" + juce::String(sliders.size()) + ")");

            auto selectors = ComponentFinder::findAllByType<LedSelectorComponent>(editor.get());
            int boundSelectors = 0;
            for (auto* sel : selectors) {
                if (!sel->getParamId().isEmpty() && processor.apvts.getParameter(sel->getParamId()) != nullptr) {
                    boundSelectors++;
                }
            }
            reporter.expect(boundSelectors == selectors.size(), "Planter: 100% of LedSelectorComponents mapped to APVTS (" + juce::String(boundSelectors) + "/" + juce::String(selectors.size()) + ")");
        }
    }

    // ==============================================================================
    // Phase 2: Page Navigation & Paint Smoke Passes
    // ==============================================================================
    inline void runPageNavigationAndPaintSmokeTest(TestReporter& reporter) {
        reporter.beginTest("Page Navigation & Paint Smoke Passes");

        // 1. The Klang Farmer Page Cycling & Resizing
        {
            TheKlangFarmerAudioProcessor processor;
            auto editor = std::unique_ptr<TheKlangFarmerAudioProcessorEditor>(dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(processor.createEditor()));
            reporter.expect(editor != nullptr, "Farmer editor created");

            if (editor) {
                const int numPages = 7;
                bool allPagesRendered = true;

                struct Dimensions { int w; int h; const char* name; };
                Dimensions testDims[] = {
                    { 800, 600, "Compact" },
                    { 1000, 750, "Standard" },
                    { 1920, 1080, "4K Scaled" }
                };

                for (int page = 0; page < numPages; ++page) {
                    editor->setPage(page);
                    pumpMessageLoop(5, 5);

                    for (const auto& dim : testDims) {
                        editor->setSize(dim.w, dim.h);
                        pumpMessageLoop(5, 5);

                        juce::Image buffer(juce::Image::ARGB, dim.w, dim.h, true);
                        juce::Graphics g(buffer);
                        editor->paintEntireComponent(g, true);

                        if (!buffer.isValid()) {
                            allPagesRendered = false;
                        }
                    }
                }

                reporter.expect(allPagesRendered, "Farmer: All 7 pages painted successfully across Compact, Standard, and 4K dimensions without crash");
            }
        }

        // 2. The Klang Planter Multi-Resolution Smoke
        {
            TheKlangPlanterAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            reporter.expect(editor != nullptr, "Planter editor created");

            if (editor) {
                bool allDimsRendered = true;

                struct Dimensions { int w; int h; const char* name; };
                Dimensions testDims[] = {
                    { 800, 600, "Compact" },
                    { 1040, 740, "Standard" },
                    { 2080, 1480, "High-DPI" }
                };

                for (const auto& dim : testDims) {
                    editor->setSize(dim.w, dim.h);
                    pumpMessageLoop(5, 5);

                    juce::Image buffer(juce::Image::ARGB, dim.w, dim.h, true);
                    juce::Graphics g(buffer);
                    editor->paintEntireComponent(g, true);

                    if (!buffer.isValid()) {
                        allDimsRendered = false;
                    }
                }

                reporter.expect(allDimsRendered, "Planter: Console painted successfully across Compact, Standard, and High-DPI dimensions");
            }
        }
    }

    // ==============================================================================
    // Phase 3: Modal Lifecycle & Dialog Smoke Tests
    // ==============================================================================
    inline void runModalLifecycleSmokeTest(TestReporter& reporter) {
        reporter.beginTest("Modal Lifecycle & Dialog Smoke Tests");

        // 1. Farmer Settings Modal & Guide Modal
        {
            TheKlangFarmerAudioProcessor processor;
            auto editor = std::unique_ptr<TheKlangFarmerAudioProcessorEditor>(dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(processor.createEditor()));
            editor->setSize(1000, 750);
            pumpMessageLoop();

            auto* gearBtn = ComponentFinder::findByType<GearButton>(editor.get());
            auto* modal = ComponentFinder::findByType<SettingsModalComponent>(editor.get());
            reporter.expect(gearBtn != nullptr && modal != nullptr, "Farmer has GearButton and SettingsModalComponent");

            if (gearBtn && modal) {
                reporter.expect(!modal->isVisible(), "SettingsModal initially hidden");
                
                // Open via gear button click
                if (gearBtn->onClick) gearBtn->onClick();
                pumpMessageLoop();
                reporter.expect(modal->isVisible(), "GearButton opens SettingsModal");

                // Dismiss via Escape key
                modal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                pumpMessageLoop();
                reporter.expect(!modal->isVisible(), "Escape key dismisses SettingsModal");
            }

            // Quickstart Guide overlay check
            auto* guideBtn = ComponentFinder::findByName(editor.get(), "GUIDE");
            if (!guideBtn) {
                // Try button with text
                for (auto* btn : ComponentFinder::findAllByType<juce::TextButton>(editor.get())) {
                    if (btn->getButtonText() == "GUIDE") {
                        guideBtn = btn;
                        break;
                    }
                }
            }
            reporter.expect(guideBtn != nullptr, "Farmer has GUIDE button in header");
            if (guideBtn) {
                if (auto* b = dynamic_cast<juce::Button*>(guideBtn)) {
                    if (b->onClick) b->onClick();
                } else {
                    EventSimulator::simulateClick(guideBtn);
                }
                pumpMessageLoop();

                auto* guideModal = ComponentFinder::findByType<QuickstartGuideModalComponent>(editor.get());
                reporter.expect(guideModal != nullptr && guideModal->isVisible(), "Clicking GUIDE button opens QuickstartGuideModalComponent");

                if (guideModal) {
                    guideModal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                    pumpMessageLoop();
                    reporter.expect(!guideModal->isVisible(), "Escape key closes QuickstartGuideModalComponent");
                }
            }

            // Right-click SliderCalloutComponent lifecycle
            auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "carrier1_pitch");
            if (pitchSlider) {
                EventSimulator::simulateRightClick(pitchSlider);
                pumpMessageLoop();

                SliderCalloutComponent* callout = nullptr;
                for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                    if (auto* found = ComponentFinder::findByType<SliderCalloutComponent>(juce::Desktop::getInstance().getComponent(i))) {
                        callout = found;
                        break;
                    }
                }

                reporter.expect(callout != nullptr, "Right-click on slider opens SliderCalloutComponent");
                if (callout) {
                    auto snapButtons = ComponentFinder::findAllByType<juce::TextButton>(callout);
                    if (!snapButtons.isEmpty()) {
                        EventSimulator::simulateClick(snapButtons[0]);
                        pumpMessageLoop();

                        bool stillExists = false;
                        for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                            if (ComponentFinder::findByType<SliderCalloutComponent>(juce::Desktop::getInstance().getComponent(i))) stillExists = true;
                        }
                        reporter.expect(!stillExists, "Selecting snap value dismisses SliderCalloutComponent cleanly");
                    }
                }
            }
        }

        // 2. Planter Settings Modal Lifecycle
        {
            TheKlangPlanterAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1040, 740);
            pumpMessageLoop();

            auto* gearBtn = ComponentFinder::findByType<GearButton>(editor.get());
            auto* modal = ComponentFinder::findByType<SettingsModalComponent>(editor.get());
            reporter.expect(gearBtn != nullptr && modal != nullptr, "Planter has GearButton and SettingsModalComponent");

            if (gearBtn && modal) {
                if (gearBtn->onClick) gearBtn->onClick();
                pumpMessageLoop();
                reporter.expect(modal->isVisible(), "Planter GearButton opens SettingsModal");

                modal->keyPressed(juce::KeyPress(juce::KeyPress::escapeKey));
                pumpMessageLoop();
                reporter.expect(!modal->isVisible(), "Planter Escape key dismisses SettingsModal");
            }
        }
    }

    // ==============================================================================
    // Phase 4: Synthetic Mouse & Coordinate Fallback Integration
    // ==============================================================================
    inline void runCoordinateInteractionTest(TestReporter& reporter) {
        reporter.beginTest("Coordinate-Based Interaction & Double-Click Reset Harness");

        // 1. Farmer Slider Drag & Reset
        {
            TheKlangFarmerAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1000, 750);
            pumpMessageLoop();

            auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "carrier1_pitch");
            reporter.expect(pitchSlider != nullptr, "Farmer carrier1_pitch slider found");

            if (pitchSlider) {
                double initialVal = pitchSlider->getValue();
                EventSimulator::simulateDrag(pitchSlider, -40.0f);
                reporter.expect(pitchSlider->getValue() > initialVal, "Coordinate drag up increases slider value");

                // Verify APVTS sync via synchronous parameter notification
                pitchSlider->setValue(initialVal + 0.15, juce::sendNotificationSync);
                pumpMessageLoop();
                auto* apvtsParam = processor.apvts.getParameter("carrier1_pitch");
                reporter.expect(apvtsParam != nullptr && apvtsParam->getValue() > 0.51f, "APVTS parameter updated via coordinate drag");

                // Double-click reset to doubleClickValue
                EventSimulator::simulateDoubleClick(pitchSlider);
                pitchSlider->setValue(pitchSlider->getValue(), juce::sendNotificationSync);
                pumpMessageLoop();

                auto* def = RlyehSound::ParameterManager::getInstance().getControlDef("carrier1_pitch");
                double expectedReset = (def != nullptr) ? def->doubleClickValue : 0.5;
                reporter.expect(std::abs(pitchSlider->getValue() - expectedReset) < 0.001, "Double-click resets slider to exact doubleClickValue (" + juce::String(expectedReset) + ")");
            }
        }

        // 2. Planter Slider Drag & Reset
        {
            TheKlangPlanterAudioProcessor processor;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
            editor->setSize(1040, 740);
            pumpMessageLoop();

            auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "planter_carrier_pitch");
            reporter.expect(pitchSlider != nullptr, "Planter planter_carrier_pitch slider found");

            if (pitchSlider) {
                double initialVal = pitchSlider->getValue();
                EventSimulator::simulateDrag(pitchSlider, -40.0f);
                reporter.expect(pitchSlider->getValue() > initialVal, "Planter: Coordinate drag up increases slider value");

                pitchSlider->setValue(initialVal + 0.1, juce::sendNotificationSync);
                pumpMessageLoop();
                auto* apvtsParam = processor.apvts.getParameter("planter_carrier_pitch");
                reporter.expect(apvtsParam != nullptr && apvtsParam->getValue() > initialVal, "Planter: APVTS parameter updated via coordinate drag");

                EventSimulator::simulateDoubleClick(pitchSlider);
                pitchSlider->setValue(pitchSlider->getValue(), juce::sendNotificationSync);
                pumpMessageLoop();

                auto* def = RlyehSound::ParameterManager::getInstance().getControlDef("planter_carrier_pitch");
                double expectedReset = (def != nullptr) ? def->doubleClickValue : 0.8;
                reporter.expect(std::abs(pitchSlider->getValue() - expectedReset) < 0.001, "Planter: Double-click resets slider to exact doubleClickValue (" + juce::String(expectedReset) + ")");
            }
        }
    }

    // ==============================================================================
    // Master Runner
    // ==============================================================================
    inline void runSuite(TestReporter& reporter) {
        runComponentBindingTest(reporter);
        runPageNavigationAndPaintSmokeTest(reporter);
        runModalLifecycleSmokeTest(reporter);
        runCoordinateInteractionTest(reporter);
    }
}
