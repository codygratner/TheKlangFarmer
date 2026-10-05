#include "MainComponent.h"
#include "ParameterManager.h"
#include "UIComponents.h"

MainComponent::MainComponent()
    : splitterBar1(&verticalLayout, 1, true),
      splitterBar2(&verticalLayout, 3, true)
{
    addAndMakeVisible(productSelector);
    productSelector.addItem("Global Theme", 1);
    productSelector.addItem("The Klang Farmer (TKF)", 2);
    productSelector.addItem("The Klang Planter (TKP)", 3);
    productSelector.onChange = [this]() { loadProduct(productSelector.getSelectedId()); };

    addAndMakeVisible(moduleSelector);
    moduleSelector.onChange = [this]() { loadModule(moduleSelector.getText()); };

    addAndMakeVisible(refreshButton);
    refreshButton.onClick = [this]() { loadModule(moduleSelector.getText()); };
    
    addAndMakeVisible(saveButton);
    saveButton.onClick = [this]() { 
        auto file = getAssetFile(currentProductId == 1 ? "controls" : "layouts", currentParamJsonFile);
        if (file.existsAsFile()) {
            file.replaceWithText(rawJsonDocument.getAllContent());
            originalJsonString = rawJsonDocument.getAllContent();
        }
    };
    
    addAndMakeVisible(toggleOriginalButton);
    toggleOriginalButton.onClick = [this]() {
        showingOriginal = !showingOriginal;
        if (showingOriginal) {
            toggleOriginalButton.setButtonText("Show Edited");
            auto parsed = juce::JSON::parse(originalJsonString);
            if (!parsed.isVoid()) {
                syncJsonToPreview(originalJsonString);
            }
        } else {
            toggleOriginalButton.setButtonText("Show Original");
            syncJsonToPreview(rawJsonDocument.getAllContent());
        }
    };

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

    productSelector.setSelectedId(2, juce::sendNotification);
}

MainComponent::~MainComponent() {
    rawJsonDocument.removeListener(this);
}

juce::File MainComponent::getAssetFile(const juce::String& subfolder, const juce::String& name) {
    auto currentDir = juce::File::getCurrentWorkingDirectory();
    while (currentDir.getParentDirectory() != currentDir) {
        auto assetsDir = currentDir.getChildFile("assets");
        if (assetsDir.exists()) {
            return assetsDir.getChildFile(subfolder).getChildFile(name);
        }
        currentDir = currentDir.getParentDirectory();
    }
    return {};
}

void MainComponent::loadProduct(int productId) {
    currentProductId = productId;
    moduleSelector.clear(juce::dontSendNotification);

    if (productId == 1) {
        moduleSelector.addItem("theme.json", 1);
        moduleSelector.setSelectedId(1, juce::sendNotification);
    } else {
        juce::String layoutFile = (productId == 2) ? "tkf_layout.json" : "tkp_layout.json";
        auto file = getAssetFile("layouts", layoutFile);
        if (file.existsAsFile()) {
            currentLayout = juce::JSON::parse(file.loadFileAsString());
            if (currentLayout.isObject()) {
                int id = 1;
                for (auto& prop : currentLayout.getDynamicObject()->getProperties()) {
                    moduleSelector.addItem(prop.name.toString(), id++);
                }
            }
            if (moduleSelector.getNumItems() > 0)
                moduleSelector.setSelectedId(1, juce::sendNotification);
        }
    }
}

void MainComponent::loadModule(const juce::String& moduleName) {
    if (moduleName.isEmpty()) return;
    currentModuleName = moduleName;

    if (currentProductId == 1) {
        currentParamJsonFile = "theme.json";
    } else {
        currentParamJsonFile = (currentProductId == 2) ? "tkf_layout.json" : "tkp_layout.json";
    }

    auto file = getAssetFile(currentProductId == 1 ? "controls" : "layouts", currentParamJsonFile);
    filePathDisplay.setText(file.getFullPathName());

    if (file.existsAsFile()) {
        originalJsonString = file.loadFileAsString();
        rawJsonDocument.replaceAllContent(originalJsonString);
        syncJsonToPreview(originalJsonString);
    }
}

void MainComponent::codeDocumentTextInserted(const juce::String&, int) { if (!showingOriginal) startTimer(500); }
void MainComponent::codeDocumentTextDeleted(int, int) { if (!showingOriginal) startTimer(500); }

void MainComponent::timerCallback() {
    stopTimer();
    syncJsonToPreview();
}

void MainComponent::syncJsonToPreview(const juce::String& forcedJson) {
    auto jsonString = forcedJson.isNotEmpty() ? forcedJson : rawJsonDocument.getAllContent();
    bool isTheme = (currentProductId == 1);
    
    auto parsed = juce::JSON::parse(jsonString);
    if (parsed.isVoid()) return;

    if (isTheme) {
        RlyehSound::ParameterManager::getInstance().reloadFromJson(jsonString, true);
    }
    
    previewWrapper.deleteAllChildren();
    activeSliders.clear();
    
    if (isTheme) {
        auto* card = new ModuleCardComponent("Theme Preview", 
            RlyehSound::ParameterManager::getInstance().getThemeColour("colRed"));
        previewWrapper.addAndMakeVisible(card);
        card->setBounds(10, 10, 280, 200);
    } else {
        if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(currentModuleName)) {
            auto moduleConfig = parsed.getDynamicObject()->getProperty(currentModuleName);
            
            juce::Colour c = RlyehSound::ParameterManager::getInstance().getThemeColour(
                moduleConfig.getProperty("color", "colSilver").toString());
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();
            auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
            
            auto* card = new ModuleCardComponent(currentModuleName, c, style);
            
            auto paramsArray = moduleConfig.getProperty("parameters", juce::var());
            if (paramsArray.isArray()) {
                int slot = 0;
                for (auto& paramIdVar : *paramsArray.getArray()) {
                    juce::String paramId = paramIdVar.toString();
                    auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                    
                    if (def) {
                        if (def->type == "float" && slot < 4) {
                            auto* slider = new RotaryKnobSlider();
                            activeSliders.add(slider);
                            slider->setTooltip(def->description);
                            slider->setRange(def->min, def->max, def->step);
                            slider->setValue(def->defaultFloat);
                            
                                                        if (paramId.containsIgnoreCase("shape") || paramId.containsIgnoreCase("waveform")) {
                                slider->diagramType = RotaryKnobSlider::DiagramType::Waveform;
                            } else if (paramId.containsIgnoreCase("slope")) {
                                slider->diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
                            }
                            
                            slider->customFormatText = [paramId](double val) {
                                if (paramId.containsIgnoreCase("pitch") || paramId.containsIgnoreCase("semi")) return juce::String(juce::roundToInt((val - 0.5) * 96.0)) + " st";
                                if (paramId.containsIgnoreCase("depth") || paramId.containsIgnoreCase("crossfade") || paramId.containsIgnoreCase("pan")) return (val >= 0.5 ? "+" : "") + juce::String(juce::roundToInt((val - 0.5) * 200.0)) + " %";
                                if (paramId.containsIgnoreCase("shape") || paramId.containsIgnoreCase("ratio") || paramId.containsIgnoreCase("amount") || paramId.containsIgnoreCase("level") || paramId.containsIgnoreCase("res") || paramId.containsIgnoreCase("mix")) return juce::String(juce::roundToInt(val * 100.0)) + " %";
                                if (paramId.containsIgnoreCase("decay") || paramId.containsIgnoreCase("speed") || paramId.containsIgnoreCase("time")) return juce::String(val * 2000.0, 0) + " ms";
                                if (paramId.containsIgnoreCase("cutoff") || paramId.containsIgnoreCase("rate") || paramId.containsIgnoreCase("freq")) return juce::String(val * 20000.0, 0) + " Hz";
                                if (paramId.containsIgnoreCase("drive") || paramId.containsIgnoreCase("gain")) return (val >= 0.5 ? "+" : "") + juce::String((val - 0.5) * 48.0, 1) + " dB";
                                if (paramId.containsIgnoreCase("claps")) return juce::String(juce::roundToInt(val * 8.0));
                                return juce::String(val, 2);
                            };
                            
                            card->addAndMakeVisible(slider);
                            card->setKnob(slot, def->name, slider);
                            slot++;
                        } else if (def->type == "choice") {
                            auto* box = new juce::ComboBox();
                            activeSliders.add(box);
                            box->addItemList(def->choices, 1);
                            box->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff161922));
                            box->setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8edf5));
                            box->setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2d3342));
                            box->setJustificationType(juce::Justification::centredLeft);
                            box->setTooltip(def->description);
                            box->setSelectedItemIndex(def->defaultChoice, juce::dontSendNotification);
                            card->addAndMakeVisible(box);
                            card->setSelector(box);
                            if (slot > 1) card->setSelectorAtBottom(true);
                        }
                    }
                }
            }
            previewWrapper.addAndMakeVisible(card);
            card->setBounds(10, 10, 280, 420);
        }
    }
    previewWrapper.repaint();

    formEditor.clear();
    juce::Array<juce::PropertyComponent*> props;
    if (parsed.isObject()) {
        auto* obj = parsed.getDynamicObject();
        for (auto& prop : obj->getProperties()) {
            if (prop.value.isObject()) {
                auto* pc = new juce::TextPropertyComponent(
                    juce::Value(prop.value.getDynamicObject()->hasProperty("name") ? prop.value.getDynamicObject()->getProperty("name").toString() : prop.name.toString()), 
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
    
    auto row1 = topBar.removeFromTop(24);
    productSelector.setBounds(row1.removeFromLeft(bounds.getWidth() / 4).reduced(4, 0));
    moduleSelector.setBounds(row1.removeFromLeft(bounds.getWidth() / 4).reduced(4, 0));
    refreshButton.setBounds(row1.removeFromLeft(80).reduced(4, 0));
    saveButton.setBounds(row1.removeFromLeft(80).reduced(4, 0));
    toggleOriginalButton.setBounds(row1.removeFromLeft(120).reduced(4, 0));
    
    filePathDisplay.setBounds(topBar.reduced(4, 4));

    juce::Component* comps[] = { &formEditor, &splitterBar1, &previewWrapper, &splitterBar2, rawJsonEditor.get() };
    verticalLayout.layOutComponents(comps, 5, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false, true);
}

