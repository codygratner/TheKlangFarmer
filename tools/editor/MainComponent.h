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
    void loadProduct(int productId);
    void loadModule(const juce::String& moduleName);
    void syncJsonToPreview(const juce::String& jsonString = "");

    juce::ComboBox productSelector;
    juce::ComboBox moduleSelector;
    juce::TextButton refreshButton { "Refresh" };
    juce::TextButton saveButton { "Save" };
    juce::TextButton toggleOriginalButton { "Show Original" };
    
    juce::TextEditor filePathDisplay;
    juce::PropertyPanel formEditor;
    juce::Component previewWrapper;
    juce::CodeDocument rawJsonDocument;
    std::unique_ptr<juce::CodeEditorComponent> rawJsonEditor;
    juce::OwnedArray<juce::Component> activeSliders;

    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutResizerBar splitterBar1;
    juce::StretchableLayoutResizerBar splitterBar2;
    
    int currentProductId = 0;
    juce::String currentModuleName;
    juce::String currentParamJsonFile;
    juce::String originalJsonString;
    bool showingOriginal = false;
    
    juce::File getAssetFile(const juce::String& subfolder, const juce::String& name);
    juce::var currentLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
