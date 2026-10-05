#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class MainComponent : public juce::Component, public juce::Timer, public juce::CodeDocument::Listener {
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    void codeDocumentTextInserted(const juce::String&, int) override;
    void codeDocumentTextDeleted(int, int) override;

private:
    void loadFile(int id);
    void syncJsonToPreview();

    juce::ComboBox fileSelector;
    juce::TextEditor filePathDisplay;
    juce::PropertyPanel formEditor;
    juce::Component previewWrapper;
    juce::CodeDocument rawJsonDocument;
    std::unique_ptr<juce::CodeEditorComponent> rawJsonEditor;

    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutResizerBar splitterBar1;
    juce::StretchableLayoutResizerBar splitterBar2;
    
    juce::String currentFileId;
    juce::File getAssetFile(const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
