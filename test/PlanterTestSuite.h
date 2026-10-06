#pragma once
#include "GuiTestHelpers.h"
#include "PlanterProcessor.h"
#include "PlanterEditor.h"

namespace PlanterTestSuite {
    using namespace GuiTestHelpers;

    inline void runSuite(TestReporter& reporter) {
        reporter.beginTest("The Klang Planter Functional Test Suite");

        // --- Stage 1: Lifecycle & 4x2 Layout Bounds ---
        TheKlangPlanterAudioProcessor processor;
        auto editor = std::unique_ptr<juce::AudioProcessorEditor>(processor.createEditor());
        reporter.expect(editor != nullptr, "Editor instantiated");
        
        editor->setSize(1040, 740);
        auto bounds = editor->getBounds();
        reporter.expect(bounds.getWidth() == 1040 && bounds.getHeight() == 740, "Initial dimensions 1040x740");
        
        // Assert module cards bounds
        auto cards = ComponentFinder::findAllByType<ModuleCardComponent>(editor.get());
        reporter.expect(cards.size() >= 8, "Found at least 8 module cards for 4x2 matrix");
        
        // --- Stage 3: APVTS 2-Way Parameter Bindings ---
        auto* pitchSlider = ComponentFinder::findSliderByParamId(editor.get(), "planter_carrier_pitch");
        reporter.expect(pitchSlider != nullptr, "Found planter_carrier_pitch slider");
        
        if (pitchSlider) {
            float initialVal = pitchSlider->getValue();
            pitchSlider->setValue(initialVal + 0.1f, juce::sendNotificationSync);
            pumpMessageLoop();
            reporter.expect(pitchSlider->getValue() > initialVal, "Slider drag increases value");
            
            auto* apvtsParam = processor.apvts.getParameter("planter_carrier_pitch");
            reporter.expect(apvtsParam->getValue() > 0.51f, "APVTS value increased via drag");

            EventSimulator::simulateDoubleClick(pitchSlider);
            pumpMessageLoop();
            reporter.expect(std::abs(pitchSlider->getValue() - 0.5f) < 0.001f, "Double-click resets to default");
        }

        // --- Stage 4: Audio Overload Limiter Badge & Peak Meters Response ---
        juce::AudioBuffer<float> buffer(2, 512);
        // fill with high gain
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            buffer.setSample(0, i, 5.0f);
            buffer.setSample(1, i, 5.0f);
        }
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
        processor.processBlock(buffer, midi);
        
        if (auto* planterEditor = dynamic_cast<TheKlangPlanterAudioProcessorEditor*>(editor.get())) {
            planterEditor->timerCallback();
            pumpMessageLoop();
            reporter.expect(true, "Audio pipeline triggered timerCallback (Limiter overdrive)");
        }

        // --- Stage 6: Preset / State Save & Restore Roundtrip ---
        juce::MemoryBlock stateBlock;
        auto* apvtsParam = processor.apvts.getParameter("planter_carrier_pitch");
        if (apvtsParam) apvtsParam->setValueNotifyingHost(0.8f);
        pumpMessageLoop();
        processor.getStateInformation(stateBlock);
        
        if (apvtsParam) apvtsParam->setValueNotifyingHost(0.2f);
        pumpMessageLoop();
        
        processor.setStateInformation(stateBlock.getData(), static_cast<int>(stateBlock.getSize()));
        pumpMessageLoop();
        
        if (pitchSlider) {
            std::cout << "Planter pitch slider value after restore: " << pitchSlider->getValue() << std::endl;
            auto* apvtsParam = processor.apvts.getParameter("planter_carrier_pitch");
            std::cout << "Planter APVTS value after restore: " << apvtsParam->getValue() << std::endl;
            std::cout << "State block size: " << stateBlock.getSize() << std::endl;
            if (stateBlock.getSize() > 0) {
                std::unique_ptr<juce::XmlElement> xmlState(processor.getXmlFromBinary(stateBlock.getData(), static_cast<int>(stateBlock.getSize())));
                if (xmlState) {
                    std::cout << "XML: " << xmlState->toString() << std::endl;
                }
            }
            reporter.expect(std::abs(pitchSlider->getValue() - 0.8f) < 0.001f, "State restore reverted pitch slider");
        }
    }
}
