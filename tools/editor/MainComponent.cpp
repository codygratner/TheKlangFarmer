#include "MainComponent.h"
#include "ParameterManager.h"
#include "UIComponents.h"

void EditorTreeItem::paintItem(juce::Graphics& g, int width, int height) {
    if (isSelected()) g.fillAll(juce::Colours::lightblue.withAlpha(0.2f));
    g.setColour(juce::Colours::white);
    g.setFont(itemType == "product" ? 16.0f : (itemType == "page" ? 14.0f : 13.0f));
    juce::String text = name;
    if (itemType == "product") text = text.toUpperCase();
    g.drawText(text, 4, 0, width - 4, height, juce::Justification::centredLeft, true);
}

void EditorTreeItem::itemSelectionChanged(bool isNowSelected) {
    if (isNowSelected && mainComp) {
        mainComp->onTreeItemSelected(this);
    }
}

MainComponent::MainComponent()
    : splitterBar1(&verticalLayout, 1, true),
      splitterBar2(&verticalLayout, 3, true),
      treeSplitter(&horizontalLayout, 1, false)
{
    addAndMakeVisible(navigationTree);
    navigationTree.setDefaultOpenness(true);

    addAndMakeVisible(refreshButton);
    refreshButton.onClick = [this]() { buildTree(); };
    
    addAndMakeVisible(saveButton);
    saveButton.onClick = [this]() { 
        auto file = getAssetFile(currentProductId == "theme" ? "controls" : "layouts", currentParamJsonFile);
        if (file.existsAsFile()) {
            if (currentProductId != "theme" && currentLayout.isObject()) {
                if (currentLayout.getDynamicObject()->hasProperty(currentPageId)) {
                    auto pageObj = currentLayout.getDynamicObject()->getProperty(currentPageId);
                    if (pageObj.isObject()) {
                        auto parsedEdit = juce::JSON::parse(rawJsonDocument.getAllContent());
                        if (!parsedEdit.isVoid()) {
                            pageObj.getDynamicObject()->setProperty(currentCardId, parsedEdit);
                            juce::String fullJson = juce::JSON::toString(currentLayout);
                            file.replaceWithText(fullJson);
                            originalJsonString = rawJsonDocument.getAllContent();
                        }
                    }
                }
            } else if (currentProductId == "theme") {
                file.replaceWithText(rawJsonDocument.getAllContent());
                originalJsonString = rawJsonDocument.getAllContent();
            }
        }
    };
    
    addAndMakeVisible(toggleOriginalButton);
    toggleOriginalButton.onClick = [this]() {
        showingOriginal = !showingOriginal;
        if (showingOriginal) {
            toggleOriginalButton.setButtonText("Show Edited");
            syncJsonToPreview(originalJsonString);
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
    addAndMakeVisible(treeSplitter);

    horizontalLayout.setItemLayout(0, -0.1, -0.3, -0.2);
    horizontalLayout.setItemLayout(1, 8, 8, 8);
    horizontalLayout.setItemLayout(2, -0.1, -0.9, -0.8);

    verticalLayout.setItemLayout(0, -0.1, -0.5, -0.33);
    verticalLayout.setItemLayout(1, 8, 8, 8);
    verticalLayout.setItemLayout(2, -0.1, -0.5, -0.33);
    verticalLayout.setItemLayout(3, 8, 8, 8);
    verticalLayout.setItemLayout(4, -0.1, -0.5, -0.34);

    setSize(1100, 750);

    buildTree();
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

void MainComponent::buildTree() {
    auto* root = new EditorTreeItem(this, "Root", "root");
    auto* themeNode = new EditorTreeItem(this, "Global Theme", "product", "theme");
    root->addSubItem(themeNode);

    const char* products[] = { "The Klang Farmer", "The Klang Planter", "The Klang Seed" };
    const char* productIds[] = { "tkf", "tkp", "tks" };
    
    for (int p = 0; p < 3; ++p) {
        auto* prodNode = new EditorTreeItem(this, products[p], "product", productIds[p]);
        
        juce::String layoutFile = juce::String(productIds[p]) + "_layout.json";
        auto file = getAssetFile("layouts", layoutFile);
        if (file.existsAsFile()) {
            auto layout = juce::JSON::parse(file.loadFileAsString());
            if (layout.isObject()) {
                auto* lObj = layout.getDynamicObject();
                for (auto& pageProp : lObj->getProperties()) {
                    juce::String pageName = pageProp.name.toString();
                    auto* pageNode = new EditorTreeItem(this, pageName, "page", productIds[p], pageName);
                    
                    if (pageProp.value.isObject()) {
                        auto* pObj = pageProp.value.getDynamicObject();
                        for (auto& cardProp : pObj->getProperties()) {
                            juce::String cardName = cardProp.name.toString();
                            pageNode->addSubItem(new EditorTreeItem(this, cardName, "card", productIds[p], pageName, cardName));
                        }
                    }
                    prodNode->addSubItem(pageNode);
                }
            }
        }
        root->addSubItem(prodNode);
    }
    
    navigationTree.setRootItem(root);
    navigationTree.setRootItemVisible(false);

    // Default Selection
    if (root->getNumSubItems() > 1) {
        if (auto* tkf = root->getSubItem(1)) {
            if (tkf->getNumSubItems() > 0) {
                if (auto* osc = tkf->getSubItem(0)) {
                    if (osc->getNumSubItems() > 0) {
                        osc->getSubItem(0)->setSelected(true, true);
                    }
                }
            }
        }
    }
}void MainComponent::onTreeItemSelected(EditorTreeItem* item) {
    if (item->itemType != "card" && !(item->itemType == "product" && item->productId == "theme")) return;
    
    if (item->itemType == "product" && item->productId == "theme") {
        currentProductId = "theme";
        currentParamJsonFile = "theme.json";
        currentPageId = "";
        currentCardId = "theme";
    } else {
        currentProductId = item->productId;
        currentParamJsonFile = currentProductId + "_layout.json";
        currentPageId = item->pageId;
        currentCardId = item->cardId;
    }

    auto file = getAssetFile(currentProductId == "theme" ? "controls" : "layouts", currentParamJsonFile);
    juce::String debugInfo = file.getFullPathName() + " -> [" + currentPageId + "] -> [" + currentCardId + "]";

    if (file.existsAsFile()) {
        auto fullJsonString = file.loadFileAsString();
        
        if (currentProductId == "theme") {
            originalJsonString = fullJsonString;
        } else {
            currentLayout = juce::JSON::parse(fullJsonString);
            if (currentLayout.isObject()) {
                if (currentLayout.getDynamicObject()->hasProperty(currentPageId)) {
                    auto pageObj = currentLayout.getDynamicObject()->getProperty(currentPageId);
                    if (pageObj.isObject()) {
                        if (pageObj.getDynamicObject()->hasProperty(currentCardId)) {
                            auto cardObj = pageObj.getDynamicObject()->getProperty(currentCardId);
                            originalJsonString = juce::JSON::toString(cardObj);
                            debugInfo += " | JSON LOADED: " + juce::String(originalJsonString.length()) + " bytes";
                        } else debugInfo += " | ERROR: Card ID missing in page obj";
                    } else debugInfo += " | ERROR: Page is not an object";
                } else debugInfo += " | ERROR: Page ID missing in root layout";
            } else debugInfo += " | ERROR: Root layout is not an object";
        }
        
        rawJsonDocument.replaceAllContent(originalJsonString);
        syncJsonToPreview(originalJsonString);
    } else {
        debugInfo += " | ERROR: FILE NOT FOUND";
    }
    
    filePathDisplay.setText(debugInfo);
}void MainComponent::codeDocumentTextInserted(const juce::String&, int) { if (!showingOriginal) startTimer(500); }
void MainComponent::codeDocumentTextDeleted(int, int) { if (!showingOriginal) startTimer(500); }

void MainComponent::timerCallback() {
    stopTimer();
    syncJsonToPreview();
}

void MainComponent::syncJsonToPreview(const juce::String& forcedJson) {
    auto jsonString = forcedJson.isNotEmpty() ? forcedJson : rawJsonDocument.getAllContent();
    bool isTheme = (currentProductId == "theme");
    
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
        if (parsed.isObject()) {
            auto moduleConfig = parsed; // The raw editor now ONLY holds this card's config!
            
            juce::Colour c = RlyehSound::ParameterManager::getInstance().getThemeColour(
                moduleConfig.getProperty("color", "colSilver").toString());
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();
            auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
            
            auto* card = new ModuleCardComponent(currentCardId, c, style);
            
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
                                if (paramId.containsIgnoreCase("depth") || paramId.containsIgnoreCase("crossfade") || paramId.containsIgnoreCase("pan") || paramId.containsIgnoreCase("filter")) return (val >= 0.5 ? "+" : "") + juce::String(juce::roundToInt((val - 0.5) * 200.0)) + " %";
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
            if (!prop.value.isObject() && !prop.value.isArray()) {
                // We need a stable juce::Value to bind to for TextPropertyComponent. 
                // Since this is just a visualizer for now, we'll bind to a local Value.
                // In a real editor, this would sync back to the JSON.
                juce::Value val (prop.value.toString());
                auto* pc = new juce::TextPropertyComponent(val, prop.name.toString(), 256, false);
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
    refreshButton.setBounds(row1.removeFromLeft(80).reduced(4, 0));
    saveButton.setBounds(row1.removeFromLeft(80).reduced(4, 0));
    toggleOriginalButton.setBounds(row1.removeFromLeft(120).reduced(4, 0));
    
    filePathDisplay.setBounds(topBar.reduced(4, 4));

    juce::Component* rightComps[] = { &formEditor, &splitterBar1, &previewWrapper, &splitterBar2, rawJsonEditor.get() };
    
    // We can't nest layOutComponents easily. We just use standard positioning.
    auto treeBounds = bounds.removeFromLeft(200);
    treeSplitter.setBounds(bounds.removeFromLeft(8));
    navigationTree.setBounds(treeBounds);
    
    verticalLayout.layOutComponents(rightComps, 5, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false, true);
}



