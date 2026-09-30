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

    auto image = editor.createComponentSnapshot(editor.getLocalBounds());
    
    juce::File outputFile("C:/Dev/TheKlangFarmer/screenshots/TheKlangFarmer_GUI.png");
    outputFile.getParentDirectory().createDirectory();
    outputFile.deleteFile();

    juce::FileOutputStream stream(outputFile);
    if (stream.openedOk()) {
        juce::PNGImageFormat png;
        png.writeImageToStream(image, stream);
        std::cout << "SUCCESS: Snapshot rendered (" << image.getWidth() << "x" << image.getHeight() << ") to " 
                  << outputFile.getFullPathName().toStdString() << std::endl;
    } else {
        std::cerr << "ERROR: Failed to open output file stream." << std::endl;
        return 1;
    }

    return 0;
}
