#include "MainComponent.h"
#include "ParameterManager.h"
#include "UIComponents.h"

MainComponent::MainComponent()
    : splitterBar1(&verticalLayout, 1, true),
      splitterBar2(&verticalLayout, 3, true)
{
    addAndMakeVisible(fileSelector);
    fileSelector.onChange = [this]() { loadFile(fileSelector.getText()); };

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

    // Scan for JSON files
    auto assetsDir = getAssetFile("").getParentDirectory();
    if (assetsDir.exists()) {
        juce::Array<juce::File> files;
        assetsDir.findChildFiles(files, juce::File::findFiles, false, "*.json");
        int id = 1;
        for (const auto& f : files) {
            fileSelector.addItem(f.getFileName(), id++);
        }
    }
    
    // Initial load
    fileSelector.setSelectedId(1, juce::sendNotification);
}

MainComponent::~MainComponent() {
    rawJsonDocument.removeListener(this);
}

juce::File MainComponent::getAssetFile(const juce::String& name) {
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

void MainComponent::loadFile(const juce::String& filename) {
    if (filename.isEmpty()) return;
    currentFileId = filename;

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
    
    auto parsed = juce::JSON::parse(jsonString);
    if (parsed.isVoid()) return;

    RlyehSound::ParameterManager::getInstance().reloadFromJson(jsonString, isTheme);
    
    auto file = getAssetFile(currentFileId);
    if (file.existsAsFile()) {
        file.replaceWithText(jsonString);
    }
    
    previewWrapper.deleteAllChildren();
    activeSliders.clear();
    
    if (isTheme) {
        auto* card = new ModuleCardComponent("Theme Preview", 
            RlyehSound::ParameterManager::getInstance().getThemeColour("colRed"));
        previewWrapper.addAndMakeVisible(card);
        card->setBounds(10, 10, 280, 200);
    } else {
        auto* card = new ModuleCardComponent(currentFileId.upToLastOccurrenceOf(".json", false, false).toUpperCase(), 
            RlyehSound::ParameterManager::getInstance().getThemeColour("colCyan"));
        
        // Dynamically add knobs based on JSON properties
        if (parsed.isObject()) {
            auto* obj = parsed.getDynamicObject();
            int slot = 0;
            for (auto& prop : obj->getProperties()) {
                if (prop.value.isObject()) {
                    auto* vObj = prop.value.getDynamicObject();
                    juce::String name = vObj->hasProperty("name") ? vObj->getProperty("name").toString() : prop.name.toString();
                    juce::String type = vObj->hasProperty("type") ? vObj->getProperty("type").toString() : "float";
                    juce::String desc = vObj->hasProperty("description") ? vObj->getProperty("description").toString() : "";
                    
                    if (type == "float" && slot < 4) {
                        auto* slider = new RotaryKnobSlider();
                        activeSliders.add(slider);
                        slider->setTooltip(desc);
                        slider->setRange(
                            vObj->hasProperty("range") && vObj->getProperty("range").isObject() && vObj->getProperty("range").getDynamicObject()->hasProperty("min") ? static_cast<double>(vObj->getProperty("range").getDynamicObject()->getProperty("min")) : 0.0,
                            vObj->hasProperty("range") && vObj->getProperty("range").isObject() && vObj->getProperty("range").getDynamicObject()->hasProperty("max") ? static_cast<double>(vObj->getProperty("range").getDynamicObject()->getProperty("max")) : 1.0,
                            vObj->hasProperty("range") && vObj->getProperty("range").isObject() && vObj->getProperty("range").getDynamicObject()->hasProperty("step") ? static_cast<double>(vObj->getProperty("range").getDynamicObject()->getProperty("step")) : 0.001
                        );
                        slider->setValue(vObj->hasProperty("default") ? static_cast<double>(vObj->getProperty("default")) : 0.0);
                        
                        card->addAndMakeVisible(slider);
                        card->setKnob(slot, name, slider);
                        slot++;
                    }
                }
            }
        }
        
        previewWrapper.addAndMakeVisible(card);
        card->setBounds(10, 10, 280, 420);
    }
    previewWrapper.repaint();

    // Populate Form
    formEditor.clear();
    juce::Array<juce::PropertyComponent*> props;
    if (parsed.isObject()) {
        auto* obj = parsed.getDynamicObject();
        for (auto& prop : obj->getProperties()) {
            if (prop.value.isObject()) {
                auto* pc = new juce::TextPropertyComponent(
                    juce::Value(prop.value.getDynamicObject()->hasProperty("name") ? prop.value.getDynamicObject()->getProperty("name").toString() : ""), 
                    prop.name.toString(), 256, false);
                props.add(pc);
            }
        }
    }
    if (!props.isEmpty()) {
        formEditor.addSection("Properties", props);
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
