#include <iostream>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

int main(int argc, char* argv[]) {
    juce::ScopedJuceInitialiser_GUI guiInit;

    TheKlangFarmerAudioProcessor processor;
    TheKlangFarmerAudioProcessorEditor editor(processor);
    editor.setSize(1040, 740);

    auto saveImage = [](const juce::Image& img, const juce::String& filename) {
        juce::File file("C:/Dev/TheKlangFarmer/screenshots/" + filename);
        file.getParentDirectory().createDirectory();
        file.deleteFile();
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "SUCCESS: Snapshot rendered to " << file.getFullPathName().toStdString() << std::endl;
        }
    };

    auto renderPage = [&](int pageIndex, const juce::String& filename) {
        std::cout << "  setPage(" << pageIndex << ")..." << std::endl;
        editor.setPage(pageIndex);
        std::cout << "  timerCallback..." << std::endl;
        editor.timerCallback();
        std::cout << "  createComponentSnapshot..." << std::endl;
        auto img = editor.createComponentSnapshot(editor.getLocalBounds());
        std::cout << "  saveImage..." << std::endl;
        saveImage(img, filename);
    };

    std::cout << "Rendering page 0..." << std::endl;
    renderPage(0, "TheKlangFarmer_GUI.png");
    renderPage(0, "page0_voice1.png");
    std::cout << "Rendering page 1..." << std::endl;
    renderPage(1, "page1_voice2.png");
    std::cout << "Rendering page 4..." << std::endl;
    renderPage(4, "page4_amplifier.png");
    std::cout << "Rendering page 6..." << std::endl;
    renderPage(6, "page6_modulations.png");
    std::cout << "Page 6 done." << std::endl;

    // Test active modulation rendering on Page 0:
    // Set Filter 1 Cutoff to 0.40 (~260 Hz)
    if (auto* p = processor.apvts.getParameter("filter1_cutoff")) {
        p->setValueNotifyingHost(0.40f);
    }
    // Set Filter 1 Env Depth to +5.0 oct (0.75f in 0..1 range)
    if (auto* p = processor.apvts.getParameter("filterenv1_depth")) {
        p->setValueNotifyingHost(0.75f);
    }
    // Trigger audio block so engine has active envelope value
    processor.getEngine().trigger(1.0f);
    juce::AudioBuffer<float> testBuf(2, 64);
    testBuf.clear();
    juce::MidiBuffer midi;
    processor.processBlock(testBuf, midi);

    // Render Page 0 with active modulation bar and needle on Cutoff
    editor.setPage(0);
    editor.timerCallback();
    saveImage(editor.createComponentSnapshot(editor.getLocalBounds()), "page0_voice1_modulated.png");

    // Also test and render the SliderCalloutComponent standalone
    RotaryKnobSlider testSlider;
    testSlider.setLabel("Filter 1 Cutoff");
    testSlider.setAccentColour(juce::Colour(0xff00d2ff));
    testSlider.setParamId("filter1_cutoff");
    testSlider.customFormatText = [](double val) {
        float hz = 0.1f * std::pow(24000.0f / 0.1f, static_cast<float>(val));
        if (hz >= 1000.0f) return juce::String(hz / 1000.0f, 2) + " kHz";
        return juce::String(hz, 1) + " Hz";
    };
    testSlider.setValue(0.40);

    SliderCalloutComponent callout(testSlider, "filter1_cutoff", [&](const juce::String& pid) {
        return processor.getParamModulationInfo(pid);
    });
    callout.setSize(220, 140);
    saveImage(callout.createComponentSnapshot(callout.getLocalBounds()), "callout_popup_modulated.png");

    return 0;
}
