#include "MainComponent.h"
#include "ParameterManager.h"
#include "UIComponents.h"

class ParamHeaderPropertyComponent : public juce::PropertyComponent {
public:
    ParamHeaderPropertyComponent(const juce::String& name) : juce::PropertyComponent(name) {}
    void refresh() override {}
    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds();
        g.setColour(juce::Colour(0xffa0a0a0));
        auto nameBounds = b.removeFromLeft(b.getWidth() / 3).reduced(4, 0);
        g.drawText(getName(), nameBounds, juce::Justification::centredLeft, true);
        auto juceBounds = b.removeFromLeft(b.getWidth() / 2).reduced(4, 0);
        g.drawText("JUCE", juceBounds, juce::Justification::centredLeft, true);
        g.drawText("Rendered", b.reduced(4, 0), juce::Justification::centredLeft, true);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1.0f);
    }
};

class ParamRowPropertyComponent : public juce::PropertyComponent, private juce::Timer {
public:
    ParamRowPropertyComponent(juce::Component* linkedComp, const juce::String& rowTitle, const juce::String& compType, const juce::StringArray& choices,
                              std::function<double()> getRawVal, std::function<juce::String()> getRenVal,
                              std::function<void(double)> onRawEdit = nullptr, std::function<void(juce::String)> onRenEdit = nullptr)
        : juce::PropertyComponent(rowTitle), component(linkedComp), compType(compType), choices(choices), 
          getRaw(getRawVal), getRen(getRenVal), onRawEdit(onRawEdit), onRenEdit(onRenEdit)
    {
        addAndMakeVisible(rawValueLabel);
        rawValueLabel.setEditable(onRawEdit != nullptr);
        rawValueLabel.setJustificationType(juce::Justification::centredLeft);
        if (onRawEdit) {
            rawValueLabel.onTextChange = [this]() {
                if (this->onRawEdit) this->onRawEdit(rawValueLabel.getText().getDoubleValue());
                editedByHand = true;
                repaint();
            };
        }

        addAndMakeVisible(renderedValueLabel);
        renderedValueLabel.setEditable(onRenEdit != nullptr);
        renderedValueLabel.setJustificationType(juce::Justification::centredLeft);
        if (onRenEdit) {
            renderedValueLabel.onTextChange = [this]() {
                if (this->onRenEdit) this->onRenEdit(renderedValueLabel.getText());
                editedByHand = true;
                repaint();
            };
        }
        
        startTimerHz(15);
    }
    
    void refresh() override {}
    
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() / 3);
        rawValueLabel.setBounds(b.removeFromLeft(b.getWidth() / 2).reduced(4, 0));
        renderedValueLabel.setBounds(b.reduced(4, 0));
    }
    
    void paint(juce::Graphics& g) override {
        bool isHovered = false;
        if (component) {
            if (auto* s = dynamic_cast<RotaryKnobSlider*>(component)) isHovered = s->isMouseButtonDown() || s->isMouseOverOrDragging();
            else if (auto* l = dynamic_cast<LedSelectorComponent*>(component)) isHovered = l->isMouseButtonDown() || l->isMouseOverOrDragging();
        }
        
        auto b = getLocalBounds();
        if (isHovered) {
            g.setColour(juce::Colours::white);
            g.drawRect(b, 1.0f);
        } else {
            g.setColour(juce::Colours::black);
            g.drawRect(b, 1.0f);
        }
        
        auto bgCol = editedByHand ? juce::Colour(0xffe8edf5) : juce::Colours::transparentBlack;
        auto textCol = editedByHand ? juce::Colour(0xff161922) : juce::Colour(0xffe8edf5);
        
        g.setColour(bgCol);
        auto nameBounds = b.withWidth(b.getWidth() / 3).reduced(4, 0);
        g.fillRect(nameBounds);
        
        g.setColour(textCol);
        g.drawText(getName(), nameBounds, juce::Justification::centredLeft, true);
    }
    
    void timerCallback() override { 
        if (!rawValueLabel.isBeingEdited() && !renderedValueLabel.isBeingEdited()) {
            double rVal = getRaw ? getRaw() : 0.0;
            juce::String renStr = getRen ? getRen() : "";
            
            juce::String rawStr = juce::String(rVal, 3);
            if (compType == "choice" && getName() != "Choices") rawStr = juce::String(static_cast<int>(rVal));
            else if (getName() == "Choices") rawStr = juce::String(static_cast<int>(rVal)) + " items";
            
            if (rawValueLabel.getText() != rawStr) rawValueLabel.setText(rawStr, juce::dontSendNotification);
            if (renderedValueLabel.getText() != renStr) renderedValueLabel.setText(renStr, juce::dontSendNotification);
        }
        repaint();
    }
    
private:
    juce::Component* component;
    juce::String compType;
    juce::StringArray choices;
    std::function<double()> getRaw;
    std::function<juce::String()> getRen;
    std::function<void(double)> onRawEdit;
    std::function<void(juce::String)> onRenEdit;
    juce::Label rawValueLabel;
    juce::Label renderedValueLabel;
    bool editedByHand = false;
};

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
    // Tree collapsed by default

        addAndMakeVisible(refreshButton);
    refreshButton.onClick = [this]() { 
        auto xml = navigationTree.getOpennessState(false);
        auto* selected = navigationTree.getSelectedItem(0);
        juce::String selName = selected ? static_cast<EditorTreeItem*>(selected)->name : "";
        juce::String selPage = selected ? static_cast<EditorTreeItem*>(selected)->pageId : "";
        juce::String selProd = selected ? static_cast<EditorTreeItem*>(selected)->productId : "";
        
        buildTree();
        
        if (xml) navigationTree.restoreOpennessState(*xml, false);
        
        if (selName.isNotEmpty()) {
            std::function<void(EditorTreeItem*)> findAndSelect = [&](EditorTreeItem* n) {
                if (n->name == selName && n->pageId == selPage && n->productId == selProd) {
                    n->setSelected(true, true);
                    return;
                }
                for (int i=0; i < n->getNumSubItems(); ++i) findAndSelect(static_cast<EditorTreeItem*>(n->getSubItem(i)));
            };
            findAndSelect(static_cast<EditorTreeItem*>(navigationTree.getRootItem()));
        }
    };
    
    addAndMakeVisible(saveButton);
    saveButton.onClick = [this]() { 
        auto parsedLayoutEdit = juce::JSON::parse(layoutJsonDocument.getAllContent());
        auto parsedControlsEdit = juce::JSON::parse(controlsJsonDocument.getAllContent());
        if (parsedLayoutEdit.isVoid() || parsedControlsEdit.isVoid()) return;
        
        juce::String changes = "";
        
        // Layout Diff
        auto oldLayout = juce::JSON::parse(originalLayoutJson).getDynamicObject();
        auto newLayout = parsedLayoutEdit.getDynamicObject();
        if (oldLayout && newLayout) {
            for (auto& prop : newLayout->getProperties()) {
                if (!oldLayout->hasProperty(prop.name) || oldLayout->getProperty(prop.name) != prop.value) changes += "- Layout: " + prop.name.toString() + "\n";
            }
            for (auto& prop : oldLayout->getProperties()) {
                if (!newLayout->hasProperty(prop.name)) changes += "- Layout: " + prop.name.toString() + " (deleted)\n";
            }
        }
        
        // Controls Diff
        auto oldControls = juce::JSON::parse(originalControlsJson).getDynamicObject();
        auto newControls = parsedControlsEdit.getDynamicObject();
        if (oldControls && newControls) {
            for (auto& prop : newControls->getProperties()) {
                if (!oldControls->hasProperty(prop.name) || oldControls->getProperty(prop.name) != prop.value) changes += "- Control: " + prop.name.toString() + "\n";
            }
        }
        
        if (changes.isEmpty()) changes = "No changes detected.";
        
        juce::MessageBoxOptions options = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Confirm Save")
            .withMessage("You are about to save changes to:\nProduct: " + currentProductId + 
                         (currentPageId.isNotEmpty() ? "\nPage: " + currentPageId : "") + 
                         "\nCard: " + currentCardId + "\n\nChanges detected:\n" + changes + "\n\nCommit to disk?")
            .withButton("Commit").withButton("Cancel");
            
        juce::AlertWindow::showAsync(options, [this, parsedLayoutEdit, parsedControlsEdit, newControls](int result) {
            if (result == 1) { // Commit
                // Save Layout
                auto file = getAssetFile(currentProductId == "theme" ? "controls" : "layouts", currentParamJsonFile);
                if (file.existsAsFile()) {
                    if (currentProductId != "theme" && currentLayout.isObject()) {
                        if (currentPageId.isNotEmpty() && currentLayout.getDynamicObject()->hasProperty(currentPageId)) {
                            auto pageObj = currentLayout.getDynamicObject()->getProperty(currentPageId);
                            if (pageObj.isObject()) pageObj.getDynamicObject()->setProperty(currentCardId, parsedLayoutEdit);
                        } else if (currentPageId.isEmpty()) {
                            currentLayout.getDynamicObject()->setProperty(currentCardId, parsedLayoutEdit);
                        }
                        file.replaceWithText(juce::JSON::toString(currentLayout));
                    } else if (currentProductId == "theme") {
                        file.replaceWithText(layoutJsonDocument.getAllContent());
                    }
                }
                
                // Save Controls
                if (currentProductId != "theme" && newControls) {
                    for (auto& prop : newControls->getProperties()) {
                        juce::String paramId = prop.name.toString();
                        if (paramToFileMap.count(paramId) > 0) {
                            auto cFile = getAssetFile("controls", paramToFileMap[paramId]);
                            if (cFile.existsAsFile()) {
                                auto fullC = juce::JSON::parse(cFile.loadFileAsString());
                                if (fullC.isObject()) {
                                    fullC.getDynamicObject()->setProperty(paramId, prop.value);
                                    cFile.replaceWithText(juce::JSON::toString(fullC));
                                }
                            }
                        }
                    }
                }
                
                originalLayoutJson = juce::JSON::toString(parsedLayoutEdit);
                originalControlsJson = juce::JSON::toString(parsedControlsEdit);
            }
        });
    };
    
    addAndMakeVisible(toggleOriginalButton);
    toggleOriginalButton.onClick = [this]() {
        showingOriginal = !showingOriginal;
        if (showingOriginal) {
            toggleOriginalButton.setButtonText("Show Edited");
            syncJsonToPreview(originalLayoutJson);
        } else {
            toggleOriginalButton.setButtonText("Show Original");
            syncJsonToPreview(layoutJsonDocument.getAllContent());
        }
    };

    filePathDisplay.setMultiLine(true);
    filePathDisplay.setReadOnly(true);
    addAndMakeVisible(filePathDisplay);

    addAndMakeVisible(formEditor);
    
        emptyPlaceholder.setJustificationType(juce::Justification::centred);
    emptyPlaceholder.setFont(16.0f);
    emptyPlaceholder.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(emptyPlaceholder); // Sibling of previewWrapper, not a child!
    addAndMakeVisible(previewWrapper);
    previewWrapper.addMouseListener(this, true); // Listen for clicks on children!

        addAndMakeVisible(jsonContainer);
    
    layoutJsonEditor = std::make_unique<juce::CodeEditorComponent>(layoutJsonDocument, nullptr);
    jsonContainer.addAndMakeVisible(layoutJsonEditor.get());
    layoutJsonDocument.addListener(this);
    
    controlsJsonEditor = std::make_unique<juce::CodeEditorComponent>(controlsJsonDocument, nullptr);
    jsonContainer.addAndMakeVisible(controlsJsonEditor.get());
    controlsJsonDocument.addListener(this);
    
    jsonSplitterBar = std::make_unique<juce::StretchableLayoutResizerBar>(&jsonSplitterLayout, 1, false);
    jsonContainer.addAndMakeVisible(jsonSplitterBar.get());
    
    jsonSplitterLayout.setItemLayout(0, -0.1, -0.9, -0.5); // Layout JSON
    jsonSplitterLayout.setItemLayout(1, 8, 8, 8);          // Splitter
    jsonSplitterLayout.setItemLayout(2, -0.1, -0.9, -0.5); // Controls JSON

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
    layoutJsonDocument.removeListener(this);
    controlsJsonDocument.removeListener(this);
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
                for (auto& prop : lObj->getProperties()) {
                    if (prop.value.isObject()) {
                        auto* pObj = prop.value.getDynamicObject();
                        if (pObj->hasProperty("parameters")) {
                            // It's a card directly
                            juce::String cardName = prop.name.toString();
                            auto* cardNode = new EditorTreeItem(this, cardName, "card", productIds[p], "", cardName);
                            cardNode->addSubItem(new EditorTreeItem(this, "Main Theme", "card_theme", productIds[p], "", cardName));
                            auto params = pObj->getProperty("parameters");
                            if (params.isArray()) {
                                int i = 1;
                                for (auto& param : *params.getArray()) {
                                    cardNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", productIds[p], "", cardName, param.toString()));
                                }
                            }
                            prodNode->addSubItem(cardNode);
                        } else {
                            // It's a page
                            juce::String pageName = prop.name.toString();
                            auto* pageNode = new EditorTreeItem(this, pageName, "page", productIds[p], pageName);
                            for (auto& cardProp : pObj->getProperties()) {
                                juce::String cardName = cardProp.name.toString();
                                auto* cardNode = new EditorTreeItem(this, cardName, "card", productIds[p], pageName, cardName);
                                cardNode->addSubItem(new EditorTreeItem(this, "Main Theme", "card_theme", productIds[p], pageName, cardName));
                                if (cardProp.value.isObject()) {
                                    auto params = cardProp.value.getDynamicObject()->getProperty("parameters");
                                    if (params.isArray()) {
                                        int i = 1;
                                        for (auto& param : *params.getArray()) {
                                            cardNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", productIds[p], pageName, cardName, param.toString()));
                                        }
                                    }
                                }
                                pageNode->addSubItem(cardNode);
                            }
                            prodNode->addSubItem(pageNode);
                        }
                    }
                }
            }
        }
        root->addSubItem(prodNode);
    }
    
    navigationTree.setRootItem(root);
    navigationTree.setRootItemVisible(false);
}

void MainComponent::onTreeItemSelected(EditorTreeItem* item) {
    if (item->itemType != "card" && item->itemType != "card_theme" && item->itemType != "card_param" && !(item->itemType == "product" && item->productId == "theme")) return;
    
    currentParamTarget = item->paramId;
    
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
    filePathDisplay.setText(file.getFullPathName() + (currentProductId != "theme" ? " -> [" + currentPageId + "] -> [" + currentCardId + "]" : "") + (currentParamTarget.isNotEmpty() ? " -> [" + currentParamTarget + "]" : ""));

    if (file.existsAsFile()) {
        auto fullJsonString = file.loadFileAsString();
        
        if (currentProductId != "theme") {
            currentLayout = juce::JSON::parse(fullJsonString);
            juce::var cardJson;
            if (currentLayout.isObject()) {
                auto* root = currentLayout.getDynamicObject();
                if (currentPageId.isNotEmpty() && root->hasProperty(currentPageId)) {
                    auto* page = root->getProperty(currentPageId).getDynamicObject();
                    if (page && page->hasProperty(currentCardId)) {
                        cardJson = page->getProperty(currentCardId);
                        originalLayoutJson = juce::JSON::toString(cardJson);
                    }
                } else if (currentPageId.isEmpty() && root->hasProperty(currentCardId)) {
                    cardJson = root->getProperty(currentCardId);
                    originalLayoutJson = juce::JSON::toString(cardJson);
                }
            }
            
            juce::DynamicObject::Ptr controlsObj = new juce::DynamicObject();
            paramToFileMap.clear();
            if (cardJson.isObject()) {
                auto paramsArray = cardJson.getProperty("parameters", juce::var());
                if (paramsArray.isArray()) {
                    for (auto& paramIdVar : *paramsArray.getArray()) {
                        juce::String paramId = paramIdVar.toString();
                        juce::DirectoryIterator iter(getAssetFile("controls", ""), false, "*.json");
                        while (iter.next()) {
                            auto f = iter.getFile();
                            auto p = juce::JSON::parse(f.loadFileAsString());
                            if (p.isObject() && p.getDynamicObject()->hasProperty(paramId)) {
                                paramToFileMap[paramId] = f.getFileName();
                                if (item->itemType != "card_param" || currentParamTarget == paramId) {
                                    controlsObj->setProperty(paramId, p.getDynamicObject()->getProperty(paramId));
                                }
                                break;
                            }
                        }
                    }
                }
            }
            originalControlsJson = juce::JSON::toString(juce::var(controlsObj.get()));
            
            if (item->itemType == "card_param") {
                // Keep the layout intact in memory for preview, but maybe clear the editor
            }
        } else {
            originalLayoutJson = fullJsonString;
            originalControlsJson = "{}";
        }
        
        layoutJsonDocument.replaceAllContent(originalLayoutJson);
        controlsJsonDocument.replaceAllContent(originalControlsJson);
        
        if (item->itemType == "card_theme") {
            jsonSplitterLayout.setItemLayout(0, -1.0, -1.0, -1.0); // Full layout
            jsonSplitterLayout.setItemLayout(1, 0, 0, 0);          // Hide splitter
            jsonSplitterLayout.setItemLayout(2, 0, 0, 0);          // Hide controls
        } else if (item->itemType == "card_param") {
            jsonSplitterLayout.setItemLayout(0, 0, 0, 0);          // Hide layout
            jsonSplitterLayout.setItemLayout(1, 0, 0, 0);          // Hide splitter
            jsonSplitterLayout.setItemLayout(2, -1.0, -1.0, -1.0); // Full controls
        } else {
            jsonSplitterLayout.setItemLayout(0, -0.1, -0.9, -0.5);
            jsonSplitterLayout.setItemLayout(1, 8, 8, 8);
            jsonSplitterLayout.setItemLayout(2, -0.1, -0.9, -0.5);
        }
        resized();
        
        syncJsonToPreview(originalLayoutJson);
    }
}

void MainComponent::codeDocumentTextInserted(const juce::String&, int) { if (!showingOriginal) startTimer(500); }
void MainComponent::codeDocumentTextDeleted(int, int) { if (!showingOriginal) startTimer(500); }

void MainComponent::timerCallback() {
    stopTimer();
    syncJsonToPreview();
}

void MainComponent::syncJsonToPreview(const juce::String& forcedJson) {
    auto jsonString = forcedJson.isNotEmpty() ? forcedJson : layoutJsonDocument.getAllContent();
    bool isTheme = (currentProductId == "theme");
    
    auto parsed = juce::JSON::parse(jsonString);
    if (parsed.isVoid() || jsonString.isEmpty()) {
        emptyPlaceholder.setVisible(true);
        previewWrapper.deleteAllChildren();
        activeSliders.clear();
        compToParamId.clear();
        formEditor.clear();
        return;
    }
    emptyPlaceholder.setVisible(false);

    if (isTheme) {
        RlyehSound::ParameterManager::getInstance().reloadFromJson(jsonString, true);
    }
    
    previewWrapper.deleteAllChildren();
    activeSliders.clear();
    compToParamId.clear();
    
    if (isTheme) {
        auto* card = new ModuleCardComponent("Theme Preview", 
            RlyehSound::ParameterManager::getInstance().getThemeColour("colRed"));
        previewWrapper.addAndMakeVisible(card);
        card->setBounds(10, 10, 280, 200);
    } else {
        if (parsed.isObject()) {
            auto moduleConfig = parsed;
            
            juce::Colour c = RlyehSound::ParameterManager::getInstance().getThemeColour(
                moduleConfig.getProperty("color", "colSilver").toString());
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();
            auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
            
            auto* card = new ModuleCardComponent(currentCardId, c, style);
            
                        auto paramsArray = moduleConfig.getProperty("parameters", juce::var());
            if (paramsArray.isArray()) {
                int slot = 0;
                int choiceCount = 0;
                for (auto& paramIdVar : *paramsArray.getArray()) {
                    juce::String paramId = paramIdVar.toString();
                    auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                    
                    if (def) {
                        if (def->type == "float" && slot < 4) {
                            auto* slider = new RotaryKnobSlider();
                            activeSliders.add(slider);
                            compToParamId[slider] = paramId;
                            
                            slider->setTooltip(def->description);
                            slider->setRange(def->min, def->max, def->step);
                            slider->setValue(def->defaultFloat);
                            
                            slider->setDoubleClickReturnValue(true, def->doubleClickValue);
                            slider->getDefaultValue = [val = def->doubleClickValue]() { return val; };
                            
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
                            auto* box = new LedSelectorComponent(c);
                            activeSliders.add(box);
                            compToParamId[box] = paramId;
                            
                            box->setItems(def->choices, def->choices.size() > 3 ? 4 : def->choices.size());
                            box->setTooltip(def->description);
                            box->setSelectedIndex(def->defaultChoice, juce::dontSendNotification);
                            
                                                        if (paramId.containsIgnoreCase("target")) {
                                card->setSelectorAtBottom(true);
                                if (currentProductId == "tkp") {
                                    juce::Colour colRed = RlyehSound::ParameterManager::getInstance().getThemeColour("colRed");
                                    juce::Colour colCyan = RlyehSound::ParameterManager::getInstance().getThemeColour("colCyan");
                                    box->setItemStyle(0, { colRed,  std::nullopt, std::nullopt });
                                    box->setItemStyle(1, { colCyan, std::nullopt, std::nullopt });
                                    box->setItemStyle(2, { colCyan, colRed,       colRed       });
                                    box->setItemStyle(3, { colRed,  colCyan,      colCyan      });
                                } else {
                                    juce::Colour colCarrier = RlyehSound::ParameterManager::getInstance().getThemeColour("colCyan");
                                    juce::Colour colMod = RlyehSound::ParameterManager::getInstance().getThemeColour("colOrange");
                                    box->setItemStyle(0, { colCarrier, std::nullopt, std::nullopt });
                                    box->setItemStyle(1, { colMod, std::nullopt, std::nullopt });
                                    box->setItemStyle(2, { colMod, colCarrier, colCarrier });
                                    box->setItemStyle(3, { colCarrier, colMod, colMod });
                                }
                            }
                            
                            card->addAndMakeVisible(box);
                            if (choiceCount == 0) card->setLedSelector(box);
                            else card->setSecondLedSelector(box);
                            choiceCount++;
                        }
                    }
                }
            }
            previewWrapper.addAndMakeVisible(card);
            card->setBounds(10, 10, 280, 420);
        }
    }
    previewWrapper.repaint();

    auto* selected = navigationTree.getSelectedItem(0);
    juce::String selType = selected ? static_cast<EditorTreeItem*>(selected)->itemType : "";
    bool showLayout = (selType != "card_param");
    bool showParams = (selType != "card_theme");

    formEditor.clear();
    juce::Array<juce::PropertyComponent*> props;
    if (showLayout && parsed.isObject()) {
        auto* obj = parsed.getDynamicObject();
        for (auto& prop : obj->getProperties()) {
            juce::String valStr;
            if (prop.value.isArray()) {
                juce::StringArray arr;
                for (auto& v : *prop.value.getArray()) arr.add(v.toString());
                valStr = "[ " + arr.joinIntoString(", ") + " ]";
            } else if (!prop.value.isObject()) {
                valStr = prop.value.toString();
            } else {
                continue;
            }
            
                        juce::Value val (valStr);
            auto* pc = new juce::TextPropertyComponent(val, prop.name.toString(), 256, false);
            
                        auto origParsed = juce::JSON::parse(originalJsonString);
            if (origParsed.isObject()) {
                auto* origObj = origParsed.getDynamicObject();
                if (!origObj->hasProperty(prop.name) || origObj->getProperty(prop.name) != prop.value) {
                    pc->setColour(juce::PropertyComponent::backgroundColourId, juce::Colour(0xffe8edf5));
                    pc->setColour(juce::PropertyComponent::labelTextColourId, juce::Colour(0xff161922));
                }
            }
            props.add(pc);
        }
    }
    if (!props.isEmpty()) {
        formEditor.addSection("Card Layout", props);
    }
    
    // Add detailed parameter properties
    if (showParams && parsed.isObject()) {
        auto paramsArray = parsed.getDynamicObject()->getProperty("parameters");
        if (paramsArray.isArray()) {
            for (auto& paramIdVar : *paramsArray.getArray()) {
                juce::String paramId = paramIdVar.toString();
                if (currentParamTarget.isNotEmpty() && currentParamTarget != paramId) continue;
                auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                if (def) {
                    juce::Array<juce::PropertyComponent*> pProps;
                    juce::Component* linkedComp = nullptr;
                    for (int j = 0; j < activeSliders.size(); ++j) {
                        if (compToParamId[activeSliders[j]] == paramId) {
                            linkedComp = activeSliders[j];
                            break;
                        }
                    }
                    
                    juce::String sectionTitle = (currentProductId == "tkf" ? "TKF: " : (currentProductId == "tkp" ? "TKP: " : "TKS: ")) + currentCardId + ": " + def->name;
                    pProps.add(new ParamHeaderPropertyComponent(sectionTitle));
                    
                    pProps.add(new ParamRowPropertyComponent(linkedComp, "Slider Value", def->type, def->choices,
                        [linkedComp, def]() -> double {
                            if (def->type == "float" && linkedComp) return dynamic_cast<RotaryKnobSlider*>(linkedComp)->getValue();
                            if (def->type == "choice" && linkedComp) return dynamic_cast<LedSelectorComponent*>(linkedComp)->getSelectedIndex();
                            return 0.0;
                        },
                        [linkedComp, def]() -> juce::String {
                            if (def->type == "float" && linkedComp) {
                                auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp);
                                return s->getTextFromValue(s->getValue());
                            }
                            if (def->type == "choice" && linkedComp) {
                                auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp);
                                int idx = l->getSelectedIndex();
                                return (idx >= 0 && idx < def->choices.size()) ? def->choices[idx] : "";
                            }
                            return "";
                        },
                        [linkedComp, def](double v) {
                            if (def->type == "float" && linkedComp) dynamic_cast<RotaryKnobSlider*>(linkedComp)->setValue(v, juce::sendNotificationAsync);
                            if (def->type == "choice" && linkedComp) dynamic_cast<LedSelectorComponent*>(linkedComp)->setSelectedIndex((int)v, juce::sendNotificationAsync);
                        },
                        [linkedComp, def](juce::String s) {
                            if (def->type == "float" && linkedComp) {
                                auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp);
                                kn->setValue(kn->getValueFromText(s), juce::sendNotificationAsync);
                            }
                            if (def->type == "choice" && linkedComp) {
                                int idx = def->choices.indexOf(s);
                                if (idx >= 0) dynamic_cast<LedSelectorComponent*>(linkedComp)->setSelectedIndex(idx, juce::sendNotificationAsync);
                            }
                        }
                    ));

                    if (def->type == "float") {
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Minimum", def->type, def->choices,
                            [def]() { return def->min; },
                            [linkedComp, def]() { return linkedComp ? dynamic_cast<RotaryKnobSlider*>(linkedComp)->getTextFromValue(def->min) : ""; }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Maximum", def->type, def->choices,
                            [def]() { return def->max; },
                            [linkedComp, def]() { return linkedComp ? dynamic_cast<RotaryKnobSlider*>(linkedComp)->getTextFromValue(def->max) : ""; }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Default", def->type, def->choices,
                            [def]() { return def->defaultFloat; },
                            [linkedComp, def]() { return linkedComp ? dynamic_cast<RotaryKnobSlider*>(linkedComp)->getTextFromValue(def->defaultFloat) : ""; }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Double-Click", def->type, def->choices,
                            [def]() { return def->doubleClickValue; },
                            [linkedComp, def]() { return linkedComp ? dynamic_cast<RotaryKnobSlider*>(linkedComp)->getTextFromValue(def->doubleClickValue) : ""; }
                        ));
                    } else if (def->type == "choice") {
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Default", def->type, def->choices,
                            [def]() { return def->defaultChoice; },
                            [def]() { return (def->defaultChoice >= 0 && def->defaultChoice < def->choices.size()) ? def->choices[def->defaultChoice] : ""; }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Choices", def->type, def->choices,
                            [def]() { return def->choices.size(); },
                            [def]() { return def->choices.joinIntoString(", "); }
                        ));
                    }

                    formEditor.addSection("Param: " + def->name, pProps);
                }
            }
        }
    }
}

void MainComponent::mouseDown(const juce::MouseEvent& e) {
    if (e.originalComponent && compToParamId.find(e.originalComponent) != compToParamId.end()) {
        juce::String pId = compToParamId[e.originalComponent];
        
        for (int i = 0; i < layoutJsonDocument.getNumLines(); ++i) {
            juce::String lineText = layoutJsonDocument.getLine(i);
            if (lineText.contains("\"" + pId + "\"") || lineText.contains(pId)) {
                juce::CodeDocument::Position startPos(layoutJsonDocument, i, 0);
                juce::CodeDocument::Position endPos(layoutJsonDocument, i, lineText.length());
                
                layoutJsonEditor->selectRegion(startPos, endPos);
                layoutJsonEditor->scrollToLine(i);
                break;
            }
        }
        for (int i = 0; i < controlsJsonDocument.getNumLines(); ++i) {
            juce::String lineText = controlsJsonDocument.getLine(i);
            if (lineText.contains("\"" + pId + "\"") || lineText.contains(pId)) {
                juce::CodeDocument::Position startPos(controlsJsonDocument, i, 0);
                juce::CodeDocument::Position endPos(controlsJsonDocument, i, lineText.length());
                
                controlsJsonEditor->selectRegion(startPos, endPos);
                controlsJsonEditor->scrollToLine(i);
                break;
            }
        }
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

        juce::Component* rightComps[] = { &formEditor, &splitterBar1, &previewWrapper, &splitterBar2, &jsonContainer };
    
    auto treeBounds = bounds.removeFromLeft(200);
    treeSplitter.setBounds(bounds.removeFromLeft(8));
    navigationTree.setBounds(treeBounds);
    
    verticalLayout.layOutComponents(rightComps, 5, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false, true);
    
    juce::Component* jsonComps[] = { layoutJsonEditor.get(), jsonSplitterBar.get(), controlsJsonEditor.get() };
    jsonSplitterLayout.layOutComponents(jsonComps, 3, 0, 0, jsonContainer.getWidth(), jsonContainer.getHeight(), true, true);
    
    emptyPlaceholder.setBounds(previewWrapper.getBounds());
}






















































