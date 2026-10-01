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

    auto renderPage = [&](int pageIndex, const juce::String& filename) {
        editor.setPage(pageIndex);
        auto img = editor.createComponentSnapshot(editor.getLocalBounds());
        juce::File file("C:/Dev/TheKlangFarmer/screenshots/" + filename);
        file.getParentDirectory().createDirectory();
        file.deleteFile();
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(img, stream);
            std::cout << "SUCCESS: Snapshot page " << pageIndex << " rendered to " << file.getFullPathName().toStdString() << std::endl;
        }
    };

    renderPage(0, "TheKlangFarmer_GUI.png");
    renderPage(0, "page0_voice1.png");
    renderPage(4, "page4_amplifier.png");
    renderPage(6, "page6_modulations.png");

    return 0;
}
