#include "MainComponent.h"
#include "ParameterManager.h"
#include "UIComponents.h"

MainComponent::MainComponent()
    : splitterBar1(&verticalLayout, 1, true),
      splitterBar2(&verticalLayout, 3, true)
{
    addAndMakeVisible(fileSelector);
    fileSelector.addItem("theme.json", 1);
    fileSelector.addItem("carrier.json", 2);
    fileSelector.onChange = [this]() { loadFile(fileSelector.getSelectedId()); };

    filePathDisplay.setMultiLine(true);
    filePathDisplay.setReadOnly(true);
    addAndMakeVisible(filePathDisplay);

    addAndMakeVisible(formEditor);
    addAndMakeVisible(previewWrapper);

    rawJsonEditor = std::make_unique<juce::CodeEditorComponent>(rawJsonDocument, nullptr);
    addAndMakeVisible(rawJsonEditor.get());
    rawJsonDocument.addListener(this);

    addAndMakeVisible(splitterBar1);
    addAndMakeVisible(splitterBar2);

    verticalLayout.setItemLayout(0, -0.1, -0.5, -0.33);
    verticalLayout.setItemLayout(1, 8, 8, 8);
    verticalLayout.setItemLayout(2, -0.1, -0.5, -0.33);
    verticalLayout.setItemLayout(3, 8, 8, 8);
    verticalLayout.setItemLayout(4, -0.1, -0.5, -0.34);

    setSize(1000, 700);

    // Initial load
    fileSelector.setSelectedId(1, juce::sendNotification);
}

MainComponent::~MainComponent() {
    rawJsonDocument.removeListener(this);
}

juce::File MainComponent::getAssetFile(const juce::String& name) {
    // Assuming we run from build directory or similar, look up to find assets
    auto currentDir = juce::File::getCurrentWorkingDirectory();
    while (currentDir.getParentDirectory() != currentDir) {
        auto assetsDir = currentDir.getChildFile("assets");
        if (assetsDir.exists()) {
            return assetsDir.getChildFile("controls").getChildFile(name);
        }
        currentDir = currentDir.getParentDirectory();
    }
    return {};
}

void MainComponent::loadFile(int id) {
    if (id == 1) currentFileId = "theme.json";
    else if (id == 2) currentFileId = "carrier.json";
    else return;

    auto file = getAssetFile(currentFileId);
    filePathDisplay.setText(file.getFullPathName());

    if (file.existsAsFile()) {
        rawJsonDocument.replaceAllContent(file.loadFileAsString());
        syncJsonToPreview();
    }
}

void MainComponent::codeDocumentTextInserted(const juce::String&, int) { startTimer(500); }
void MainComponent::codeDocumentTextDeleted(int, int) { startTimer(500); }

void MainComponent::timerCallback() {
    stopTimer();
    syncJsonToPreview();
}

void MainComponent::syncJsonToPreview() {
    auto jsonString = rawJsonDocument.getAllContent();
    bool isTheme = (currentFileId == "theme.json");
    
    // Attempt parse
    auto parsed = juce::JSON::parse(jsonString);
    if (!parsed.isVoid()) {
        RlyehSound::ParameterManager::getInstance().reloadFromJson(jsonString, isTheme);
        
        // Save to file automatically for live preview
        auto file = getAssetFile(currentFileId);
        if (file.existsAsFile()) {
            file.replaceWithText(jsonString);
        }
        
        // Reload Preview Component
        previewWrapper.deleteAllChildren();
        
        if (isTheme) {
            // Style Guide Mode - just a card to show colors
            auto* card = new ModuleCardComponent("Theme Preview", 
                RlyehSound::ParameterManager::getInstance().getThemeColour("colRed"));
            previewWrapper.addAndMakeVisible(card);
            card->setBounds(10, 10, 280, 200);
        } else if (currentFileId == "carrier.json") {
            auto* card = new ModuleCardComponent("Carrier Preview", 
                RlyehSound::ParameterManager::getInstance().getThemeColour("colCyan"));
            previewWrapper.addAndMakeVisible(card);
            card->setBounds(10, 10, 280, 420);
        }
        previewWrapper.repaint();

        // Populate Form (One-way for now)
        formEditor.clear();
        juce::Array<juce::PropertyComponent*> props;
        if (parsed.isObject()) {
            auto* obj = parsed.getDynamicObject();
            for (auto& prop : obj->getProperties()) {
                if (prop.value.isString()) {
                    auto* pc = new juce::TextPropertyComponent(
                        juce::Value(prop.value.toString()), prop.name.toString(), 256, false);
                    props.add(pc);
                }
            }
        }
        if (!props.isEmpty()) {
            formEditor.addSection("Properties", props);
        }
    }
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
    auto bounds = getLocalBounds();
    auto topBar = bounds.removeFromTop(60);
    
    fileSelector.setBounds(topBar.removeFromTop(24).reduced(4, 0));
    filePathDisplay.setBounds(topBar.reduced(4, 4));

    juce::Component* comps[] = { &formEditor, &splitterBar1, &previewWrapper, &splitterBar2, rawJsonEditor.get() };
    verticalLayout.layOutComponents(comps, 5, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false, true);
}
