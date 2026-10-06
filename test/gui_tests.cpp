#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <iostream>
#include <cassert>
#include "FarmerProcessor.h"

int main()
{
    // Initialize the JUCE message manager and GUI subsystem
    juce::ScopedJuceInitialiser_GUI guiInit;

    std::cout << "Starting Automated GUI Test Harness..." << std::endl;

    // 1. Instantiate the headless AudioProcessor
    TheKlangFarmerAudioProcessor processor;
    
    // 2. Instantiate the Editor (the GUI)
    auto* editor = processor.createEditor();
    if (editor == nullptr) {
        std::cerr << "FAILED: Could not create TheKlangFarmerAudioProcessorEditor!" << std::endl;
        return 1;
    }

    // Force layout computation
    editor->setSize(1000, 750);
    editor->setVisible(true);

    std::cout << "Editor successfully instantiated and laid out." << std::endl;

    // TODO: Add functional state tests (simulate MouseEvent, verify APVTS change)
    // Example:
    // auto* carrierDepthSlider = findChildComponentByName(editor, "carrier1_depth");
    // simulateMouseDrag(carrierDepthSlider, 0, 100);
    // assert(processor.apvts.getParameter("carrier1_depth")->getValue() == 1.0f);

    std::cout << "GUI Tests Passed." << std::endl;
    delete editor;
    return 0;
}
