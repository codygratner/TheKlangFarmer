#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class MainComponent;

class EditorTreeItem : public juce::TreeViewItem {
public:
    EditorTreeItem(MainComponent* owner, const juce::String& text, const juce::String& type, const juce::String& prodId = "", const juce::String& pgId = "", const juce::String& cId = "", const juce::String& pId = "")
        : mainComp(owner), name(text), itemType(type), productId(prodId), pageId(pgId), cardId(cId), paramId(pId) {}

    bool mightContainSubItems() override { return getNumSubItems() > 0; }
    void paintItem(juce::Graphics& g, int width, int height) override;
    void itemSelectionChanged(bool isNowSelected) override;

    MainComponent* mainComp;
    juce::String name, itemType, productId, pageId, cardId, paramId;
};
class MainComponent : public juce::Component, public juce::Timer, public juce::CodeDocument::Listener {
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    
    void timerCallback() override;
    void codeDocumentTextInserted(const juce::String&, int) override;
    void codeDocumentTextDeleted(int, int) override;

    void onTreeItemSelected(EditorTreeItem* item);

private:
    void buildTree();
    void syncJsonToPreview(const juce::String& forcedJson = "");

    juce::TreeView navigationTree;
    juce::TextButton refreshButton { "Refresh" };
    juce::TextButton saveButton { "Save" };
    juce::TextButton toggleOriginalButton { "Show Original" };
    
    juce::TextEditor filePathDisplay;
            juce::PropertyPanel formEditor;
    juce::Component previewWrapper;
    juce::Label emptyPlaceholder { {}, "Pick a module from the left tree to edit its UI card" };
    
    juce::CodeDocument layoutJsonDocument;
    std::unique_ptr<juce::CodeEditorComponent> layoutJsonEditor;
    juce::CodeDocument controlsJsonDocument;
    std::unique_ptr<juce::CodeEditorComponent> controlsJsonEditor;
    juce::Component jsonContainer;
    juce::StretchableLayoutManager jsonSplitterLayout;
    std::unique_ptr<juce::StretchableLayoutResizerBar> jsonSplitterBar;
    
    juce::OwnedArray<juce::Component> activeSliders;
    std::unordered_map<juce::Component*, juce::String> compToParamId;

    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutManager horizontalLayout;
    juce::StretchableLayoutResizerBar splitterBar1;
    juce::StretchableLayoutResizerBar splitterBar2;
    juce::StretchableLayoutResizerBar treeSplitter;
    
    juce::String currentProductId;
    juce::String activePreviewProduct { "tkf" };
    juce::String currentPageId;
    juce::String currentCardId;
    juce::String currentParamTarget;
    juce::String currentParamJsonFile;
    juce::String originalJsonString;
    bool showingOriginal = false;
    
    juce::File getAssetFile(const juce::String& subfolder, const juce::String& name);
        juce::var currentLayout;
    juce::String originalLayoutJson;
    juce::String originalControlsJson;
    std::unordered_map<juce::String, juce::String> paramToFileMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};








