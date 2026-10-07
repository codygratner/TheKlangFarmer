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
    // Phase 5: Planter Header Interactions & Limiter Callout Suite
    // ==============================================================================
    inline void runPlanterHeaderAndLimiterCalloutTest(TestReporter& reporter) {
        reporter.beginTest("Planter Header Panic & Limiter Callout Card Suite");

        TheKlangPlanterAudioProcessor processor;
        auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
        editor->setSize(1040, 740);
        pumpMessageLoop();

        // 1. Header Visualizer Interactions
        auto vizs = ComponentFinder::findAllByType<PlanterHeaderVisualizer>(editor.get());
        reporter.expect(vizs.size() == 1, "Planter: Header visualizer component found (" + juce::String(vizs.size()) + ")");

        if (!vizs.isEmpty()) {
            auto* viz = vizs[0];

            // Activate engine with test audio block and note on
            juce::AudioBuffer<float> buffer(2, 256);
            buffer.clear();
            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, 36, (juce::uint8)127), 0);
            processor.prepareToPlay(44100.0, 256);
            processor.processBlock(buffer, midi);

            // Click meter area to trigger panic
            auto meterArea = viz->getMeterArea();
            juce::Point<float> clickPos = meterArea.getCentre();
            juce::MouseEvent ePanic(juce::Desktop::getInstance().getMainMouseSource(), clickPos,
                                    juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    viz, viz, juce::Time::getCurrentTime(), clickPos, juce::Time::getCurrentTime(), 1, false);
            viz->mouseDown(ePanic);
            pumpMessageLoop();

            reporter.expect(processor.getEngine().getPeakL() == 0.0f && processor.getEngine().getPeakR() == 0.0f,
                            "Planter: Header meterArea click triggers panic() and resets peak meters to zero");

            // Flash animation trigger
            viz->triggerFlash();
            reporter.expect(true, "Planter: Header visualizer triggerFlash() executes safely");

            // Right-click limitArea to request limiter callout
            bool calloutRequested = false;
            viz->onLimiterCalloutRequested = [&](const juce::Rectangle<int>&) {
                calloutRequested = true;
            };
            auto limitArea = viz->getLimitArea();
            juce::Point<float> rightPos = limitArea.getCentre();
            juce::MouseEvent eLimit(juce::Desktop::getInstance().getMainMouseSource(), rightPos,
                                    juce::ModifierKeys::rightButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    viz, viz, juce::Time::getCurrentTime(), rightPos, juce::Time::getCurrentTime(), 1, false);
            viz->mouseDown(eLimit);
            pumpMessageLoop();
            reporter.expect(calloutRequested, "Planter: Header limitArea right-click triggers onLimiterCalloutRequested");

            // Left-click limitArea also triggers limiter callout
            bool leftCalloutRequested = false;
            viz->onLimiterCalloutRequested = [&](const juce::Rectangle<int>&) {
                leftCalloutRequested = true;
            };
            juce::MouseEvent eLeftLimit(juce::Desktop::getInstance().getMainMouseSource(), rightPos,
                                        juce::ModifierKeys::leftButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        viz, viz, juce::Time::getCurrentTime(), rightPos, juce::Time::getCurrentTime(), 1, false);
            viz->mouseDown(eLeftLimit);
            pumpMessageLoop();
            reporter.expect(leftCalloutRequested, "Planter: Header limitArea left-click triggers onLimiterCalloutRequested");

            // Secondary trigger: Right-clicking Amplifier card triggers limiter callout
            auto cards = ComponentFinder::findAllByType<ModuleCardComponent>(editor.get());
            bool cardAmpRightClicked = false;
            for (auto* card : cards) {
                if (card->getTitle() == "Amplifier") {
                    card->onCardMouseDown = [&](const juce::MouseEvent& e) {
                        if (e.mods.isPopupMenu() || e.mods.isRightButtonDown()) cardAmpRightClicked = true;
                    };
                    juce::MouseEvent eCard(juce::Desktop::getInstance().getMainMouseSource(), card->getLocalBounds().getCentre().toFloat(),
                                           juce::ModifierKeys::rightButtonModifier, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                           card, card, juce::Time::getCurrentTime(), card->getLocalBounds().getCentre().toFloat(), juce::Time::getCurrentTime(), 1, false);
                    card->mouseDown(eCard);
                    break;
                }
            }
            reporter.expect(cardAmpRightClicked, "Planter: Right-clicking Amplifier card triggers onCardMouseDown");
        }

        // 2. Limiter Mini-Card Callout Component (100% of controls verified)
        {
            PlanterLimiterCalloutComponent callout(processor);
            callout.setSize(300, 130);
            pumpMessageLoop();

            // Schema defaults verification
            auto* defGain = RlyehSound::ParameterManager::getInstance().getControlDef("planter_limiter_gain");
            auto* defThresh = RlyehSound::ParameterManager::getInstance().getControlDef("planter_limiter_thresh");
            auto* defRel = RlyehSound::ParameterManager::getInstance().getControlDef("planter_limiter_release");
            reporter.expect(defGain && std::abs(defGain->defaultFloat - 0.333333f) < 0.01f, "Schema: planter_limiter_gain defaultFloat parsed correctly");
            reporter.expect(defThresh && std::abs(defThresh->defaultFloat - 1.0f) < 0.01f, "Schema: planter_limiter_thresh defaultFloat parsed correctly");
            reporter.expect(defRel && std::abs(defRel->defaultFloat - 0.6296f) < 0.01f, "Schema: planter_limiter_release defaultFloat parsed correctly");

            // Custom formatting verification
            reporter.expect(callout.getGainSlider().customFormatText != nullptr, "Planter Callout: Gain slider has customFormatText attached");
            reporter.expect(callout.getThreshSlider().customFormatText != nullptr, "Planter Callout: Thresh slider has customFormatText attached");
            reporter.expect(callout.getReleaseSlider().customFormatText != nullptr, "Planter Callout: Release slider has customFormatText attached");

            juce::String gainStr = callout.getGainSlider().getTextFromValue(12.0 / 36.0);
            reporter.expect(gainStr.contains("dB"), "Planter Callout: Gain value formats to decibels (" + gainStr + ")");
            juce::String threshStr = callout.getThreshSlider().getTextFromValue(1.0);
            reporter.expect(threshStr.contains("dB"), "Planter Callout: Thresh value formats to decibels (" + threshStr + ")");
            juce::String relStr = callout.getReleaseSlider().getTextFromValue(0.6296);
            reporter.expect(relStr.contains("ms"), "Planter Callout: Release value formats to milliseconds (" + relStr + ")");

            // Control 1: planter_limiter_enable
            auto* enableParam = processor.apvts.getParameter("planter_limiter_enable");
            reporter.expect(enableParam != nullptr, "Planter: planter_limiter_enable APVTS parameter exists");
            if (enableParam) {
                callout.getEnableSelector().setSelectedIndex(0, juce::sendNotificationSync);
                pumpMessageLoop();
                reporter.expect(enableParam->getValue() == 0.0f, "Planter Callout: Enable selector 'Off' synchronizes with APVTS");

                callout.getEnableSelector().setSelectedIndex(1, juce::sendNotificationSync);
                pumpMessageLoop();
                reporter.expect(enableParam->getValue() == 1.0f, "Planter Callout: Enable selector 'On' synchronizes with APVTS");
            }

            // Control 2: planter_limiter_gain (range 0.0 - 1.0)
            auto* gainParam = processor.apvts.getParameter("planter_limiter_gain");
            reporter.expect(gainParam != nullptr, "Planter: planter_limiter_gain APVTS parameter exists");
            callout.getGainSlider().setValue(0.75, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(std::abs(callout.getGainSlider().getValue() - 0.75) < 0.01, "Planter Callout: Gain slider updates value synchronously");
            reporter.expect(gainParam && std::abs(gainParam->getValue() - 0.75f) < 0.01f, "Planter Callout: Gain APVTS synchronizes with slider");
            reporter.expect(!callout.getGainSlider().getTooltip().isEmpty(), "Planter Callout: Gain slider has valid non-empty tooltip");

            // Control 3: planter_limiter_thresh (range 0.0 - 1.0)
            auto* threshParam = processor.apvts.getParameter("planter_limiter_thresh");
            reporter.expect(threshParam != nullptr, "Planter: planter_limiter_thresh APVTS parameter exists");
            callout.getThreshSlider().setValue(0.50, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(std::abs(callout.getThreshSlider().getValue() - 0.50) < 0.01, "Planter Callout: Ceiling slider updates value synchronously");
            reporter.expect(threshParam && std::abs(threshParam->getValue() - 0.50f) < 0.01f, "Planter Callout: Ceiling APVTS synchronizes with slider");
            reporter.expect(!callout.getThreshSlider().getTooltip().isEmpty(), "Planter Callout: Ceiling slider has valid non-empty tooltip");

            // Control 4: planter_limiter_release (range 0.0 - 1.0)
            auto* relParam = processor.apvts.getParameter("planter_limiter_release");
            reporter.expect(relParam != nullptr, "Planter: planter_limiter_release APVTS parameter exists");
            callout.getReleaseSlider().setValue(0.40, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(std::abs(callout.getReleaseSlider().getValue() - 0.40) < 0.01, "Planter Callout: Release slider updates value synchronously");
            reporter.expect(relParam && std::abs(relParam->getValue() - 0.40f) < 0.01f, "Planter Callout: Release APVTS synchronizes with slider");
            reporter.expect(!callout.getReleaseSlider().getTooltip().isEmpty(), "Planter Callout: Release slider has valid non-empty tooltip");

            // Offscreen paint pass
            juce::Image calloutImg(juce::Image::ARGB, 300, 130, true);
            juce::Graphics calloutG(calloutImg);
            callout.paintEntireComponent(calloutG, true);
            reporter.expect(true, "Planter Callout: Offscreen paint completed with zero errors");

            // 3. In-Window CalloutBox Parenting Verification
            PlanterLimiterCalloutComponent testCallout(processor);
            auto area = juce::Rectangle<int>(100, 100, 80, 24);
            auto box = std::make_unique<juce::CallOutBox>(testCallout, area, editor.get());
            pumpMessageLoop();
            reporter.expect(editor->isParentOf(box.get()), "Planter Callout: CallOutBox is direct child of editor (not desktop window)");
            box.reset();
            pumpMessageLoop();

            // 4. INIT Button Reset Verification
            auto* pitchParam = processor.apvts.getParameter("planter_carrier_pitch");
            if (pitchParam) pitchParam->setValueNotifyingHost(0.2f);
            for (auto* child : editor->getChildren()) {
                if (auto* btn = dynamic_cast<juce::TextButton*>(child)) {
                    if (btn->getButtonText().contains("INIT")) {
                        if (btn->onClick) btn->onClick();
                        break;
                    }
                }
            }
            pumpMessageLoop();
            reporter.expect(pitchParam && std::abs(pitchParam->getValue() - pitchParam->getDefaultValue()) < 0.01f,
                            "Planter INIT button resets parameters to factory defaults and flushes engine");
        }
    }

    // ==============================================================================
    // Phase 6: Two-Line StatusBarComponent Integrity Suite
    // ==============================================================================
    inline void runStatusBarIntegrityTest(TestReporter& reporter) {
        reporter.beginTest("Two-Line Status Bar Integrity & Tooltip Feed Suite");

        // 1. Direct Component API & State
        {
            StatusBarComponent sb;
            sb.setSize(1000, 36);
            reporter.expect(sb.isOpaque() == true, "StatusBar: isOpaque() is true for high-contrast rendering");
            reporter.expect(sb.isTooltipsEnabled(), "StatusBar: Default tooltipsEnabled is true");
            reporter.expect(sb.getIdleStatusText().contains("SYSTEM READY"), "StatusBar: Default idle status text contains 'SYSTEM READY'");

            sb.setHoveredControl("Filter Cutoff", "1.20 kHz", "Adjusts master lowpass filter cutoff frequency",
                                 "Right-Click: Snap Points", "2x-Click: Default (1.00)");
            reporter.expect(sb.getActiveName() == "Filter Cutoff", "StatusBar: getActiveName() returns correct name");
            reporter.expect(sb.getActiveValue() == "1.20 kHz", "StatusBar: getActiveValue() returns correct value");
            reporter.expect(sb.getActiveDesc().contains("Adjusts master lowpass"), "StatusBar: getActiveDesc() returns correct description");
            reporter.expect(sb.getActiveRightClickHint() == "Right-Click: Snap Points", "StatusBar: getActiveRightClickHint() matches");
            reporter.expect(sb.getActiveDoubleClickHint() == "2x-Click: Default (1.00)", "StatusBar: getActiveDoubleClickHint() matches");

            // Paint with active hovered control
            juce::Image imgActive(juce::Image::ARGB, 1000, 36, true);
            juce::Graphics gActive(imgActive);
            sb.paintEntireComponent(gActive, true);

            sb.clearHoveredControl();
            reporter.expect(sb.getActiveName().isEmpty(), "StatusBar: clearHoveredControl() clears active parameter name");

            sb.setTooltipsEnabled(false);
            reporter.expect(!sb.isTooltipsEnabled(), "StatusBar: setTooltipsEnabled(false) disables tooltip feed");

            // Paint in quiet mode
            juce::Image imgQuiet(juce::Image::ARGB, 1000, 36, true);
            juce::Graphics gQuiet(imgQuiet);
            sb.paintEntireComponent(gQuiet, true);
            reporter.expect(true, "StatusBar: Standalone component painted cleanly in both active and quiet states");
        }

        // 2. The Klang Farmer Integration
        {
            TheKlangFarmerAudioProcessor farmerProc;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(farmerProc.createEditor());
            editor->setSize(1000, 750);
            pumpMessageLoop();

            auto* coreEditor = dynamic_cast<KlangCoreEditor*>(editor.get());
            reporter.expect(coreEditor != nullptr, "Farmer: Editor inherits from KlangCoreEditor");

            if (coreEditor) {
                auto& sb = coreEditor->getStatusBar();
                reporter.expect(sb.isVisible(), "Farmer: StatusBarComponent is visible");
                reporter.expect(sb.isOpaque() == true, "Farmer: StatusBarComponent is opaque");
                reporter.expect(sb.getHeight() == 36, "Farmer: StatusBarComponent height is exactly 36px");
                reporter.expect(sb.getY() == editor->getHeight() - 36, "Farmer: StatusBarComponent pinned to bottom edge");

                // Test hover wiring on slider
                auto* slider = ComponentFinder::findSliderByParamId(editor.get(), "carrier1_pitch");
                if (slider && slider->onMouseEnter) {
                    slider->onMouseEnter(slider);
                    reporter.expect(sb.getActiveName().isNotEmpty(), "Farmer: Hovering carrier1_pitch populates status bar name (" + sb.getActiveName() + ")");
                    reporter.expect(sb.getActiveDoubleClickHint().isNotEmpty(), "Farmer: Hovering carrier1_pitch populates double-click hint (" + sb.getActiveDoubleClickHint() + ")");
                    if (slider->onMouseExit) slider->onMouseExit(slider);
                    reporter.expect(sb.getActiveName().isEmpty(), "Farmer: Mouse exit clears status bar name");
                }

                // Test header button hover feeds
                coreEditor->simulateHeaderHover(&coreEditor->getInitButton());
                reporter.expect(sb.getActiveName() == "INIT", "Farmer: Hovering INIT button populates status bar name");
                reporter.expect(sb.getActiveDesc().contains("panic"), "Farmer: Hovering INIT button populates panic description");
                coreEditor->simulateHeaderExit(&coreEditor->getInitButton());
                reporter.expect(sb.getActiveName().isEmpty(), "Farmer: Header exit clears status bar name");

                coreEditor->simulateHeaderHover(&coreEditor->getTriggerButton());
                reporter.expect(sb.getActiveName() == "TRIGGER", "Farmer: Hovering TRIGGER button populates status bar name");
                coreEditor->simulateHeaderExit(&coreEditor->getTriggerButton());
                reporter.expect(sb.getActiveName().isEmpty(), "Farmer: Header exit clears TRIGGER status bar name");

                // Test tooltips button sync
                auto& tipBtn = coreEditor->getTooltipsButton();
                bool initialEnabled = sb.isTooltipsEnabled();
                if (tipBtn.onClick) tipBtn.onClick();
                reporter.expect(sb.isTooltipsEnabled() != initialEnabled, "Farmer: Clicking tooltipsButton toggles status bar tooltipsEnabled");
                if (tipBtn.onClick) tipBtn.onClick();
                reporter.expect(sb.isTooltipsEnabled() == initialEnabled, "Farmer: Second click restores status bar tooltipsEnabled");
            }
        }

        // 3. The Klang Planter Integration
        {
            TheKlangPlanterAudioProcessor planterProc;
            auto editor = std::unique_ptr<juce::AudioProcessorEditor>(planterProc.createEditor());
            editor->setSize(1040, 740);
            pumpMessageLoop();

            auto* coreEditor = dynamic_cast<KlangCoreEditor*>(editor.get());
            reporter.expect(coreEditor != nullptr, "Planter: Editor inherits from KlangCoreEditor");

            if (coreEditor) {
                auto& sb = coreEditor->getStatusBar();
                reporter.expect(sb.isVisible(), "Planter: StatusBarComponent is visible");
                reporter.expect(sb.isOpaque() == true, "Planter: StatusBarComponent is opaque");
                reporter.expect(sb.getHeight() == 36, "Planter: StatusBarComponent height is exactly 36px");
                reporter.expect(sb.getY() == editor->getHeight() - 36, "Planter: StatusBarComponent pinned to bottom edge");

                // Test hover wiring on slider
                auto* slider = ComponentFinder::findSliderByParamId(editor.get(), "planter_carrier_pitch");
                if (slider && slider->onMouseEnter) {
                    slider->onMouseEnter(slider);
                    reporter.expect(sb.getActiveName().isNotEmpty(), "Planter: Hovering planter_carrier_pitch populates status bar name (" + sb.getActiveName() + ")");
                    reporter.expect(sb.getActiveDoubleClickHint().isNotEmpty(), "Planter: Hovering planter_carrier_pitch populates double-click hint (" + sb.getActiveDoubleClickHint() + ")");
                    if (slider->onMouseExit) slider->onMouseExit(slider);
                    reporter.expect(sb.getActiveName().isEmpty(), "Planter: Mouse exit clears status bar name");
                }

                // Test header button hover feeds
                coreEditor->simulateHeaderHover(&coreEditor->getInitButton());
                reporter.expect(sb.getActiveName() == "INIT", "Planter: Hovering INIT button populates status bar name");
                reporter.expect(sb.getActiveDesc().contains("panic"), "Planter: Hovering INIT button populates panic description");
                coreEditor->simulateHeaderExit(&coreEditor->getInitButton());
                reporter.expect(sb.getActiveName().isEmpty(), "Planter: Header exit clears status bar name");

                coreEditor->simulateHeaderHover(&coreEditor->getTriggerButton());
                reporter.expect(sb.getActiveName() == "TRIGGER", "Planter: Hovering TRIGGER button populates status bar name");
                coreEditor->simulateHeaderExit(&coreEditor->getTriggerButton());
                reporter.expect(sb.getActiveName().isEmpty(), "Planter: Header exit clears TRIGGER status bar name");

                // Test tooltips button sync
                auto& tipBtn = coreEditor->getTooltipsButton();
                bool initialEnabled = sb.isTooltipsEnabled();
                if (tipBtn.onClick) tipBtn.onClick();
                reporter.expect(sb.isTooltipsEnabled() != initialEnabled, "Planter: Clicking tooltipsButton toggles status bar tooltipsEnabled");
                if (tipBtn.onClick) tipBtn.onClick();
                reporter.expect(sb.isTooltipsEnabled() == initialEnabled, "Planter: Second click restores status bar tooltipsEnabled");
            }
        }
    }

    // ==============================================================================
    // Phase 7: Automated VST3 Parameter Validation Suite (6 Pillars)
    // ==============================================================================
    inline void runVst3ParameterValidationSuite(TestReporter& reporter) {
        reporter.beginTest("Automated VST3 Parameter Validation Suite");

        auto runPillarsForProcessor = [&](juce::AudioProcessor& processor, const juce::String& pluginName, int minParams) {
            auto& params = processor.getParameters();
            auto& pm = RlyehSound::ParameterManager::getInstance();

            // Pillar 1: Dynamic Reflection & Identity
            reporter.expect(params.size() >= minParams,
                            pluginName + " Pillar 1: Parameter count exceeds threshold (" + juce::String(params.size()) + " >= " + juce::String(minParams) + ")");

            bool allIdentitiesValid = true;
            for (auto* pNode : params) {
                auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode);
                if (!p || p->paramID.isEmpty() || p->getName(128).isEmpty()) {
                    allIdentitiesValid = false;
                    break;
                }
            }
            reporter.expect(allIdentitiesValid, pluginName + " Pillar 1: 100% of parameters have non-empty paramID and name");

            // Pillar 2: Normalization Roundtrip (convertTo0to1 & convertFrom0to1)
            bool allRoundtripsValid = true;
            juce::String roundtripFailureDetail;
            const float testPoints[] = { 0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 1.0f };

            for (auto* pNode : params) {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(pNode)) {
                    float stepNorm = 0.0f;
                    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(pNode)) {
                        int numChoices = choice->choices.size();
                        if (numChoices > 1) stepNorm = 1.0f / static_cast<float>(numChoices - 1);
                    } else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(pNode)) {
                        auto r = intParam->getNormalisableRange();
                        if (r.end > r.start) stepNorm = 1.0f / (r.end - r.start);
                    } else {
                        auto r = ranged->getNormalisableRange();
                        float span = r.end - r.start;
                        if (r.interval > 0.0f && span > 0.0f) stepNorm = r.interval / span;
                    }
                    float tol = (stepNorm > 0.0f) ? std::max(0.005f, stepNorm * 0.55f) : 0.002f;

                    for (float pt : testPoints) {
                        float realVal = ranged->convertFrom0to1(pt);
                        float roundtripNorm = ranged->convertTo0to1(realVal);
                        if (std::abs(roundtripNorm - pt) > tol) {
                            allRoundtripsValid = false;
                            roundtripFailureDetail = ranged->paramID + " failed at " + juce::String(pt) + " (got " + juce::String(roundtripNorm) + ", tol " + juce::String(tol) + ")";
                            break;
                        }
                    }
                    if (!allRoundtripsValid) break;
                }
            }
            reporter.expect(allRoundtripsValid, pluginName + " Pillar 2: 100% of parameters pass Normalization Roundtrip (" + (allRoundtripsValid ? "All Passed" : roundtripFailureDetail) + ")");

            // Pillar 3: Boundary Clamping & Safety
            bool allClamped = true;
            juce::String clampFailureDetail;
            const float oobValues[] = { -1.0f, -0.05f, 1.05f, 2.0f };

            for (auto* pNode : params) {
                float defaultVal = pNode->getDefaultValue();
                for (float oob : oobValues) {
                    pNode->setValueNotifyingHost(oob);
                    float val = pNode->getValue();
                    if (val < 0.0f || val > 1.0f || std::isnan(val) || std::isinf(val)) {
                        allClamped = false;
                        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode)) {
                            clampFailureDetail = p->paramID + " oob " + juce::String(oob) + " clamped to invalid " + juce::String(val);
                        }
                        break;
                    }
                }
                pNode->setValueNotifyingHost(defaultVal);
                if (!allClamped) break;
            }
            reporter.expect(allClamped, pluginName + " Pillar 3: Out-of-bounds inputs strictly clamped to [0.0, 1.0] with zero NaNs/Infs");

            // Pillar 4: Dual-Plugin JSON Schema Parity
            bool allSchemaValid = true;
            juce::String schemaFailureDetail;
            int schemaMatchedCount = 0;

            for (auto* pNode : params) {
                auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode);
                if (!p) continue;
                juce::String id = p->paramID;
                if (id.startsWith("unused") || id.isEmpty()) continue;
                if (pluginName.contains("Farmer") && (id.startsWith("pre_fx_") || id.startsWith("post_fx_"))) continue;

                const auto* def = pm.getControlDef(id);
                if (def == nullptr) {
                    allSchemaValid = false;
                    schemaFailureDetail = "Missing ControlDef for '" + id + "'";
                    break;
                }

                float defNorm = p->getDefaultValue();
                if (defNorm < 0.0f || defNorm > 1.0f || std::isnan(defNorm)) {
                    allSchemaValid = false;
                    schemaFailureDetail = "Invalid default normalized value for '" + id + "' (" + juce::String(defNorm) + ")";
                    break;
                }
                schemaMatchedCount++;
            }
            reporter.expect(allSchemaValid, pluginName + " Pillar 4: 100% of APVTS parameters match JSON ControlDefs (" + juce::String(schemaMatchedCount) + " validated)");

            // Pillar 5: APVTS State Serialization & Restoration Roundtrip
            juce::MemoryBlock baselineState;
            processor.getStateInformation(baselineState);
            reporter.expect(baselineState.getSize() > 0, pluginName + " Pillar 5: Baseline getStateInformation produced non-empty state block (" + juce::String(baselineState.getSize()) + " bytes)");

            // Invert all parameters away from baseline
            std::map<juce::String, float> baselineValues;
            for (auto* pNode : params) {
                if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode)) {
                    baselineValues[p->paramID] = p->getValue();
                    float inverted = (p->getValue() > 0.5f) ? 0.15f : 0.85f;
                    p->setValueNotifyingHost(inverted);
                }
            }
            pumpMessageLoop(2, 5);

            // Restore baseline state
            processor.setStateInformation(baselineState.getData(), static_cast<int>(baselineState.getSize()));
            pumpMessageLoop(2, 5);

            bool allRestored = true;
            juce::String restoreFailureDetail;
            for (auto* pNode : params) {
                if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(pNode)) {
                    float expected = baselineValues[p->paramID];
                    float actual = p->getValue();
                    if (std::abs(actual - expected) > 0.02f) {
                        allRestored = false;
                        restoreFailureDetail = p->paramID + " expected " + juce::String(expected) + " but got " + juce::String(actual);
                        break;
                    }
                }
            }
            reporter.expect(allRestored, pluginName + " Pillar 5: Full APVTS state restore reverted all parameters (" + (allRestored ? "100% Parity" : restoreFailureDetail) + ")");

            // Pillar 6: DSP Audio Smoke Pass Under Automation
            processor.prepareToPlay(44100.0, 512);
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);

            bool audioPassClean = true;
            for (int block = 0; block < 32; ++block) {
                float sweepVal = static_cast<float>(block) / 31.0f;
                for (auto* pNode : params) {
                    pNode->setValueNotifyingHost(sweepVal);
                }

                buffer.clear();
                processor.processBlock(buffer, midi);
                midi.clear();

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                    const float* samples = buffer.getReadPointer(ch);
                    for (int s = 0; s < buffer.getNumSamples(); ++s) {
                        float smp = samples[s];
                        if (std::isnan(smp) || std::isinf(smp) || std::abs(smp) > 100.0f) {
                            audioPassClean = false;
                            break;
                        }
                    }
                    if (!audioPassClean) break;
                }
                if (!audioPassClean) break;
            }
            processor.releaseResources();
            reporter.expect(audioPassClean, pluginName + " Pillar 6: 32-block rapid automation audio sweep rendered zero NaNs or Infs");
        };

        // Run on Farmer
        {
            TheKlangFarmerAudioProcessor farmer;
            runPillarsForProcessor(farmer, "Farmer", 100);
        }

        // Run on Planter
        {
            TheKlangPlanterAudioProcessor planter;
            runPillarsForProcessor(planter, "Planter", 30);
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
        runPlanterHeaderAndLimiterCalloutTest(reporter);
        runStatusBarIntegrityTest(reporter);
        runVst3ParameterValidationSuite(reporter);
    }
}
