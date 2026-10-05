#include <iostream>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "FarmerProcessor.h"
#include "FarmerEditor.h"

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

    // Test Carrier Mod Depth visualization (shows affected span on Offset slider WITHOUT indicator needle)
    if (auto* p = processor.apvts.getParameter("carrier1_depth")) {
        p->setValueNotifyingHost(0.75f); // +50% depth (+-2 octaves)
    }
    auto modInfo = processor.getParamModulationInfo("carrier1_pitch");
    std::cout << "DEBUG carrier1_pitch isMod=" << modInfo.isModulated 
              << " min=" << modInfo.rangeMinNorm << " max=" << modInfo.rangeMaxNorm 
              << " needle=" << modInfo.showNeedle << " numSrc=" << modInfo.sources.size() << std::endl;
    editor.timerCallback();
    saveImage(editor.createComponentSnapshot(editor.getLocalBounds()), "page0_carrier_depth_modulated.png");

    // Test Modulator Noise mode (3rd param is DJ Filter, 4th param is Speed 24 kHz)
    if (auto* p = processor.apvts.getParameter("mod1_type")) {
        p->setValueNotifyingHost(1.0f); // Type 2 = Noise
    }
    if (auto* p = processor.apvts.getParameter("mod1_shape")) {
        p->setValueNotifyingHost(0.5f); // DJ Filter default 0%
    }
    if (auto* p = processor.apvts.getParameter("mod1_speed")) {
        p->setValueNotifyingHost(1.0f); // Speed default 24 kHz
    }
    editor.timerCallback();
    saveImage(editor.createComponentSnapshot(editor.getLocalBounds()), "page0_noise_modulator.png");

    // Test Visualizer OFF mode (lit up glowing red, motionless flat line)
    // Find vizCard via editor component or toggle
    for (auto* child : editor.getChildren()) {
        if (auto* viz = dynamic_cast<VisualizationCardComponent*>(child)) {
            viz->setIsOff(true);
            break;
        }
    }
    editor.timerCallback();
    saveImage(editor.createComponentSnapshot(editor.getLocalBounds()), "visualizer_off.png");

    // Test INIT Clean mode (empty FX slots)
    editor.resetToDefaults(true); // Clean: all FX slots set to None
    editor.setPage(3); // PRE-AMP FX page
    editor.timerCallback();
    saveImage(editor.createComponentSnapshot(editor.getLocalBounds()), "pre_fx_clean.png");

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
