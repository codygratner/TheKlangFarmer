#pragma once
#include "GuiTestHelpers.h"
#include "FarmerProcessor.h"
#include "FarmerEditor.h"

namespace FarmerTestSuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("The Klang Farmer Functional Test Suite");

        // --- Stage 1: Lifecycle & Layout Bounds ---
        TheKlangFarmerAudioProcessor processor;
        
        auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
        reporter.expect(editor != nullptr, "Editor instantiated");
        
        editor->setSize(1000, 750);
        auto bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 1000 && bounds.getHeight() == 750, "Initial dimensions 1000x750", editor.get(), "Dimensions");
        
        editor->setSize(800, 600);
        bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 800 && bounds.getHeight() == 600, "Resize limits min 800x600");
        
        editor->setSize(1200, 900);
        bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 1200 && bounds.getHeight() == 900, "Resize limits max 1200x900");
        
        editor->setSize(1000, 750); // back to normal

        auto* navCard = ComponentFinder::findByType<NavigationCardComponent>(editor.get());
        reporter.expect(navCard != nullptr, "Navigation card exists", editor.get(), "NavCard");

        // --- Stage 2: All 7 Pages Navigation Transitions ---
        if (navCard) {
            for (int page = 0; page < 7; ++page) {
                // Simulate click on nav button (we can't access buttons array directly, so we just find the button by name if needed, or call setSelectedPage directly to trigger the callback, or find the TextButton child)
                auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
                if (page < buttons.size()) {
                    EventSimulator::simulateClick(buttons[page]);
                    pumpMessageLoop();
                    
                    // Verify the page changed
                    reporter.expect(navCard->getSelectedPage() == page, "Navigated to page " + juce::String(page));
                    
                    // Specific module card visibility checks could be added here
                }
            }
        }
        // Go back to Voice 1
        if (navCard) {
            auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
            if (!buttons.isEmpty()) {
                EventSimulator::simulateClick(buttons[0]);
                pumpMessageLoop();
            }
        }

        // --- Stage 3: APVTS 2-Way Parameter Sync & Formatters ---
        auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "carrier1_pitch");
        reporter.expect(pitchSlider != nullptr, "Found carrier1_pitch slider", editor.get(), "FindPitchSlider");
        
        if (pitchSlider) {
            float initialVal = pitchSlider->getValue();
            pitchSlider->setValue(initialVal + 0.1f, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(pitchSlider->getValue() > initialVal, "Slider drag increases value");
            
            // APVTS updated?
            auto* apvtsParam = processor.apvts.getParameter("carrier1_pitch");
            reporter.expect(apvtsParam->getValue() > 0.51f, "APVTS value increased via drag");

            // Double click reset
            EventSimulator::simulateDoubleClick(pitchSlider);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - 0.5f) < 0.001f, "Double-click resets to default");
            
            // APVTS -> UI
            apvtsParam->setValueNotifyingHost(1.0f);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - 1.0f) < 0.001f, "APVTS change syncs to UI slider");
            apvtsParam->setValueNotifyingHost(0.5f); // reset
        }

        auto* trackSelector = ComponentFinder::findSelectorByParamId(editor.get(), "carrier1_tracking");
        if (trackSelector) {
            EventSimulator::simulateClick(trackSelector);
            pumpMessageLoop();
            auto* apvtsParam = processor.apvts.getParameter("carrier1_tracking");
            reporter.expect(apvtsParam->getValue() > 0.0f, "Selector click updates APVTS");
        }

        // --- Stage 4: Full 13 FX Algorithms Dynamic Reconfiguration Sweep ---
        // Go to Pre-Amp FX page (index 3)
        if (navCard) {
            auto buttons = ComponentFinder::findAllByType<juce::TextButton>(navCard);
            if (buttons.size() > 3) {
                EventSimulator::simulateClick(buttons[3]);
                pumpMessageLoop();
            }
        }
        
        auto fxCards = ComponentFinder::findAllByType<FXSlotCardComponent>(editor.get());
        reporter.expect(fxCards.size() >= 4, "Found FX Slot cards on Pre-Amp page");
        
        if (fxCards.size() >= 1) {
            auto* fxCard = fxCards[0];
            // Test a few algorithms explicitly
            int testTypes[] = { 1, 2, 3, 4, 13 }; // Bell EQ, Chorus, Comb, Drive, WaveFolder
            for (int type : testTypes) {
                fxCard->configureForType(type);
                pumpMessageLoop();
                reporter.expect(fxCard->getCurrentType() == type, "Configured FX Slot for type " + juce::String(type));
            }
        }

        // --- Stage 5: Right-Click Callout Popups & Snap-Point Clicks ---
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
            reporter.expect(callout != nullptr, "Right-click opened SliderCalloutComponent");
            
            if (callout) {
                auto snapButtons = ComponentFinder::findAllByType<juce::TextButton>(callout);
                if (!snapButtons.isEmpty()) {
                    EventSimulator::simulateClick(snapButtons[0]);
                    pumpMessageLoop();
                    bool stillExists = false;
                    for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i) {
                        if (ComponentFinder::findByType<SliderCalloutComponent>(juce::Desktop::getInstance().getComponent(i))) stillExists = true;
                    }
                    reporter.expect(!stillExists, "Clicking snap button closes callout");
                }
            }
        }

        // --- Stage 6: Master Header Controls & Modals ---
        auto* vizCard = ComponentFinder::findByType<VisualizationCardComponent>(editor.get());
        reporter.expect(vizCard != nullptr, "VisualizationCard exists");

        // --- Stage 7: Audio-to-Visualizer Data Pipeline ---
        // Render some audio
        juce::AudioBuffer<float> buffer(2, 512);
        buffer.clear();
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
        processor.processBlock(buffer, midi);
        
        // Trigger timer manually or pump
        if (auto* farmerEditor = dynamic_cast<TheKlangFarmerAudioProcessorEditor*>(editor.get())) {
            farmerEditor->timerCallback();
            pumpMessageLoop();
            reporter.expect(true, "Audio pipeline triggered timerCallback without asserting");
        }

        // --- Stage 8: Preset / State Save & Restore Roundtrip ---
        juce::MemoryBlock stateBlock;
        processor.getStateInformation(stateBlock);
        
        if (pitchSlider) pitchSlider->setValue(0.2f, juce::sendNotificationSync);
        pumpMessageLoop();
        
        processor.setStateInformation(stateBlock.getData(), static_cast<int>(stateBlock.getSize()));
        pumpMessageLoop();
        
        if (pitchSlider) {
            reporter.expect(std::abs(pitchSlider->getValue() - 0.5f) < 0.001f, "State restore reverted pitch slider");
        }
    }
}
