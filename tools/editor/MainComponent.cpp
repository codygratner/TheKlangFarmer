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

#include "DiffViewer.h"
#include "SaveConfirmComponent.h"
class PoiRowPropertyComponent : public juce::PropertyComponent {
public:
    PoiRowPropertyComponent(RlyehSound::ControlDef* def, int index, 
                            std::function<void(const juce::String&, const juce::var&)> updateControlsJson,
                            std::function<void()> rebuildPanel)
        : juce::PropertyComponent(juce::String(index)), def(def), index(index),
          updateControlsJson(updateControlsJson), rebuildPanel(rebuildPanel)
    {
        setPreferredHeight(24);
        
        auto& poi = def->snapPoints[index];
        labelInput.setText(poi.label, juce::dontSendNotification);
        valueInput.setText(juce::String(poi.value, 3), juce::dontSendNotification);
        
        labelInput.setEditable(true);
        valueInput.setEditable(true);
        labelInput.setJustificationType(juce::Justification::centredLeft);
        valueInput.setJustificationType(juce::Justification::centredLeft);
        
        labelInput.onTextChange = [this]() {
            this->def->snapPoints[this->index].label = labelInput.getText();
            saveToJson();
        };
        valueInput.onTextChange = [this]() {
            this->def->snapPoints[this->index].value = valueInput.getText().getFloatValue();
            saveToJson();
        };
        
        removeBtn.setButtonText("Remove");
        removeBtn.onClick = [this]() {
            this->def->snapPoints.erase(this->def->snapPoints.begin() + this->index);
            saveToJson();
            if (this->rebuildPanel) this->rebuildPanel();
        };
        
        addAndMakeVisible(labelInput);
        addAndMakeVisible(valueInput);
        addAndMakeVisible(removeBtn);
    }
    
    void saveToJson() {
        // Sort points by value
        std::sort(def->snapPoints.begin(), def->snapPoints.end(), 
                  [](const RlyehSound::SnapPoint& a, const RlyehSound::SnapPoint& b) { return a.value < b.value; });
                  
        juce::Array<juce::var> jsonPoiArr;
        for (auto& p : def->snapPoints) {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("value", p.value);
            obj->setProperty("label", p.label);
            jsonPoiArr.add(juce::var(obj));
        }
        updateControlsJson("snap_points", juce::var(jsonPoiArr));
    }
    
    void refresh() override {}
    
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() / 3);
        labelInput.setBounds(b.removeFromLeft(b.getWidth() / 3).reduced(4, 0));
        valueInput.setBounds(b.removeFromLeft(b.getWidth() / 2).reduced(4, 0));
        removeBtn.setBounds(b.reduced(4, 2));
    }
    
    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds();
        auto nameBounds = b.withWidth(b.getWidth() / 3).reduced(4, 0);
        g.setColour(juce::Colour(0xffe8edf5));
        g.drawText(juce::String(index + 1) + ":", nameBounds, juce::Justification::centredLeft, true);
    }
private:
    RlyehSound::ControlDef* def;
    int index;
    juce::Label labelInput, valueInput;
    juce::TextButton removeBtn;
    std::function<void(const juce::String&, const juce::var&)> updateControlsJson;
    std::function<void()> rebuildPanel;
};

class PoiAddPropertyComponent : public juce::PropertyComponent {
public:
    PoiAddPropertyComponent(RlyehSound::ControlDef* def, 
                            std::function<void(const juce::String&, const juce::var&)> updateControlsJson,
                            std::function<void()> rebuildPanel)
        : juce::PropertyComponent("Add"), def(def), 
          updateControlsJson(updateControlsJson), rebuildPanel(rebuildPanel)
    {
        setPreferredHeight(24);
        addBtn.setButtonText("Add Snap Point");
        addBtn.onClick = [this]() {
            this->def->snapPoints.push_back({0.0f, "New Point"});
            
            std::sort(this->def->snapPoints.begin(), this->def->snapPoints.end(), 
                      [](const RlyehSound::SnapPoint& a, const RlyehSound::SnapPoint& b) { return a.value < b.value; });
                      
            juce::Array<juce::var> jsonPoiArr;
            for (auto& p : this->def->snapPoints) {
                auto* obj = new juce::DynamicObject();
                obj->setProperty("value", p.value);
                obj->setProperty("label", p.label);
                jsonPoiArr.add(juce::var(obj));
            }
            this->updateControlsJson("snap_points", juce::var(jsonPoiArr));
            
            if (this->rebuildPanel) this->rebuildPanel();
        };
        addAndMakeVisible(addBtn);
    }
    void refresh() override {}
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() * 2 / 3);
        addBtn.setBounds(b.reduced(4, 2));
    }
    void paint(juce::Graphics& g) override {
        auto b = getLocalBounds();
        g.setColour(juce::Colour(0xffa0a0a0));
        g.drawText("Snap Points:", b.withWidth(b.getWidth() / 3).reduced(4, 0), juce::Justification::centredLeft, true);
    }
private:
    RlyehSound::ControlDef* def;
    juce::TextButton addBtn;
    std::function<void(const juce::String&, const juce::var&)> updateControlsJson;
    std::function<void()> rebuildPanel;
};

int WhereUsedListModel::getNumRows() {
    return items.size();
}

void WhereUsedListModel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) {
    if (!juce::isPositiveAndBelow(rowNumber, items.size())) return;

    if (rowIsSelected) {
        g.fillAll(juce::Colour(0xff2a344d));
    } else if (rowNumber % 2 == 1) {
        g.fillAll(juce::Colour(0xff161a22));
    }

    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colour(0xffcfd8dc));
    g.setFont(12.0f);
    g.drawText(items[rowNumber].label, 6, 0, width - 12, height, juce::Justification::centredLeft, true);
}

void WhereUsedListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent&) {
    triggerDoubleClick(row);
}

void WhereUsedListModel::triggerDoubleClick(int row) {
    if (juce::isPositiveAndBelow(row, items.size()) && mc != nullptr) {
        mc->navigateToTreeItem(items[row]);
    }
}

class ThemeColorPropertyComponent : public juce::PropertyComponent {
public:
    ThemeColorPropertyComponent(const juce::String& name, const juce::String& initialVal, std::function<void(const juce::String&)> onValChanged)
        : juce::PropertyComponent(name), currentVal(initialVal), onChange(onValChanged) {
        
        setPreferredHeight(30);
        
        addAndMakeVisible(textEditor);
        textEditor.setText(initialVal, juce::dontSendNotification);
        textEditor.onTextChange = [this]() {
            currentVal = textEditor.getText();
            updateButtonColor();
            if (onChange) onChange(currentVal);
        };
        
        addAndMakeVisible(colorButton);
        updateButtonColor();
        colorButton.onClick = [this]() {
            auto hex = resolveToHex(currentVal);
            
            juce::Component::SafePointer<ThemeColorPropertyComponent> safeThis(this);
            
            std::function<void(const juce::String&)> onChangeCopy = onChange;
            auto* picker = new AdvancedColorPickerComponent(juce::Colour::fromString(hex), [safeThis, onChangeCopy](juce::Colour newColor) {
                juce::String newHex = "0x" + newColor.toDisplayString(true).toUpperCase();
                if (safeThis != nullptr) {
                    safeThis->currentVal = newHex;
                    safeThis->textEditor.setText(newHex, juce::dontSendNotification);
                    safeThis->updateButtonColor();
                }
                if (onChangeCopy) onChangeCopy(newHex);
            });
            juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(picker), colorButton.getScreenBounds(), this->getTopLevelComponent());
        };
    }
    
    juce::String resolveToHex(const juce::String& val) {
        if (val.startsWithIgnoreCase("0x")) return val;
        return juce::Colour(0xffcfd8dc).toDisplayString(true);
    }
    
    void updateButtonColor() {
        auto c = juce::Colour::fromString(resolveToHex(currentVal));
        colorButton.setColour(juce::TextButton::buttonColourId, c);
        colorButton.setColour(juce::TextButton::buttonOnColourId, c);
    }
    
    void refresh() override {}
    
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() / 3);
        colorButton.setBounds(b.removeFromRight(40).reduced(4));
        textEditor.setBounds(b.reduced(4));
    }
    
private:
    juce::TextEditor textEditor;
    juce::TextButton colorButton { "Edit" };
    juce::String currentVal;
    std::function<void(const juce::String&)> onChange;
};

class ParamRowPropertyComponent : public juce::PropertyComponent, private juce::Timer {
public:
    ParamRowPropertyComponent(juce::Component* linkedComp, const juce::String& rowTitle, const juce::String& compType, const juce::StringArray& choices,
                              std::function<double()> getRawVal, std::function<juce::String()> getRenVal,
                              std::function<void(double)> onRawEdit = nullptr, std::function<void(juce::String)> onRenEdit = nullptr)
        : juce::PropertyComponent(rowTitle), component(linkedComp), compType(compType), choices(choices), 
          getRaw(getRawVal), getRen(getRenVal), onRawEdit(onRawEdit), onRenEdit(onRenEdit)    {
        setPreferredHeight(24);
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
            else if (compType == "string") rawStr = "-";
            
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

class CalloutTextPropertyComponent : public juce::PropertyComponent {
public:
    CalloutTextPropertyComponent(const juce::String& name, const juce::String& initialVal, std::function<void(const juce::String&)> onValChanged)
        : juce::PropertyComponent(name), onChange(std::move(onValChanged)) {
        setPreferredHeight(30);
        addAndMakeVisible(textEditor);
        textEditor.setText(initialVal, juce::dontSendNotification);
        textEditor.onTextChange = [this]() {
            if (onChange) onChange(textEditor.getText());
        };
    }
    void refresh() override {}
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() / 3);
        textEditor.setBounds(b.reduced(4));
    }
private:
    juce::TextEditor textEditor;
    std::function<void(const juce::String&)> onChange;
};

class CalloutPreviewCard : public juce::Component {
public:
    CalloutPreviewCard(const juce::String& calloutType,
                       const juce::String& title,
                       juce::Colour bgCol,
                       juce::Colour borderCol,
                       float radius)
        : type(calloutType), cardTitle(title), bg(bgCol), border(borderCol), cornerRadius(radius),
          limitSelector(borderCol)
    {
        if (type == "planter_limiter") {
            limitSelector.setAccent(borderCol);
            limitSelector.setItems({ "OFF", "ON" }, 2);
            limitSelector.setSelectedIndex(1, juce::dontSendNotification);
            addAndMakeVisible(limitSelector);

            auto setupMockKnob = [&](RotaryKnobSlider& s, double val) {
                s.setAccentColour(borderCol);
                s.setLabel("");
                s.setRange(0.0, 1.0, 0.01);
                s.setValue(val);
                addAndMakeVisible(s);
            };

            setupMockKnob(knob1, 0.33);
            setupMockKnob(knob2, 1.0);
            setupMockKnob(knob3, 0.63);
        } else {
            textEditor.setText("1.00 kHz", false);
            textEditor.setReadOnly(true);
            textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff12141a));
            textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
            addAndMakeVisible(textEditor);

            const char* snaps[] = { "0.5x", "1.0x", "2.0x", "Default" };
            for (int i = 0; i < 4; ++i) {
                auto* btn = new juce::TextButton(snaps[i]);
                btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181d26));
                btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
                snapButtons.add(btn);
                addAndMakeVisible(btn);
            }
        }
    }

    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.getWidth() <= 1.0f || bounds.getHeight() <= 1.0f) return;

        // Chassis background & rounded border
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, cornerRadius);
        g.setColour(border.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds.reduced(0.75f), cornerRadius, 1.5f);

        // Header bar
        auto headerArea = bounds.removeFromTop(24.0f);
        g.setColour(bg.brighter(0.08f));
        g.fillRoundedRectangle(headerArea.getX(), headerArea.getY(), headerArea.getWidth(), headerArea.getHeight(), cornerRadius);
        g.fillRect(headerArea.removeFromBottom(cornerRadius));

        g.setColour(border);
        g.drawHorizontalLine(24, 0.0f, bounds.getWidth());

        // Header indicator dot
        g.setColour(border);
        g.fillEllipse(10.0f, 8.0f, 8.0f, 8.0f);

        // Title text
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xfff1f5f9));
        g.drawText(cardTitle, 24, 0, getWidth() - 30, 24, juce::Justification::centredLeft, true);

        if (type == "planter_limiter") {
            // Column labels
            g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            g.setColour(juce::Colour(0xffcfd8dc));
            if (limitSelector.getWidth() > 0)
                g.drawText("LIMIT", limitSelector.getX(), 27, limitSelector.getWidth(), 16, juce::Justification::centred, true);
            if (knob1.getWidth() > 0)
                g.drawText("GAIN", knob1.getX(), 27, knob1.getWidth(), 16, juce::Justification::centred, true);
            if (knob2.getWidth() > 0)
                g.drawText("CEIL", knob2.getX(), 27, knob2.getWidth(), 16, juce::Justification::centred, true);
            if (knob3.getWidth() > 0)
                g.drawText("REL", knob3.getX(), 27, knob3.getWidth(), 16, juce::Justification::centred, true);
        } else {
            // Subtitle
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.setColour(border.brighter(0.2f));
            g.drawText("QUICK SNAP VALUES", 10, 64, getWidth() - 20, 16, juce::Justification::centredLeft, true);
        }
    }

    void resized() override {
        if (type == "planter_limiter") {
            int topY = 46;
            int h = std::max(10, getHeight() - topY - 10);
            limitSelector.setBounds(10, topY, 46, h);
            int knobW = std::max(10, (getWidth() - 72) / 3);
            int curX = 62;
            knob1.setBounds(curX, topY, knobW, h); curX += knobW + 4;
            knob2.setBounds(curX, topY, knobW, h); curX += knobW + 4;
            knob3.setBounds(curX, topY, knobW, h);
        } else {
            textEditor.setBounds(10, 32, std::max(10, getWidth() - 20), 26);
            int btnW = std::max(10, (getWidth() - 32) / 4);
            int curX = 10;
            for (auto* btn : snapButtons) {
                btn->setBounds(curX, 86, btnW, 26);
                curX += btnW + 4;
            }
        }
    }

    void setTitle(const juce::String& newTitle)
    {
        cardTitle = newTitle;
        repaint();
    }

    void setBackgroundColour(juce::Colour c)
    {
        bg = c;
        repaint();
    }

    void setBorderColour(juce::Colour c)
    {
        border = c;
        if (type == "planter_limiter") {
            limitSelector.setAccent(c);
            knob1.setAccentColour(c);
            knob2.setAccentColour(c);
            knob3.setAccentColour(c);
        }
        repaint();
    }

    void setCornerRadius(float r)
    {
        cornerRadius = r;
        repaint();
    }

private:
    juce::String type;
    juce::String cardTitle;
    juce::Colour bg;
    juce::Colour border;
    float cornerRadius = 5.0f;

    LedSelectorComponent limitSelector;
    RotaryKnobSlider knob1;
    RotaryKnobSlider knob2;
    RotaryKnobSlider knob3;

    juce::TextEditor textEditor;
    juce::OwnedArray<juce::TextButton> snapButtons;
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

void EditorTreeItem::itemClicked(const juce::MouseEvent& e) {
    if (e.mods.isPopupMenu()) {
        juce::PopupMenu menu;
        menu.addItem(1, "Expand All");
        menu.addItem(2, "Collapse All");
        menu.addItem(3, "Collapse Others");
        
        menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) {
            std::function<void(juce::TreeViewItem*, bool)> setOpenRec = [&](juce::TreeViewItem* item, bool open) {
                if (!item) return;
                item->setOpen(open);
                for (int i = 0; i < item->getNumSubItems(); ++i) {
                    setOpenRec(item->getSubItem(i), open);
                }
            };
            
            if (result == 1) {
                setOpenRec(this, true);
            } else if (result == 2) {
                setOpenRec(this, false);
            } else if (result == 3) {
                if (auto* parent = this->getParentItem()) {
                    for (int i = 0; i < parent->getNumSubItems(); ++i) {
                        auto* sibling = parent->getSubItem(i);
                        if (sibling != this) {
                            setOpenRec(sibling, false);
                        }
                    }
                }
                setOpenRec(this, true);
            }
        });
    }
}

MainComponent::MainComponent()
    : splitterBar1(&verticalLayout, 1, true),
      splitterBar2(&verticalLayout, 3, true),
      treeSplitter(&horizontalLayout, 1, false)
{
    addAndMakeVisible(navigationTabs);
    navigationTabs.addTab("CONTROLS", juce::Colours::transparentBlack, &controlsTree, false);
    navigationTabs.addTab("LAYOUTS", juce::Colours::transparentBlack, &layoutsTree, false);
    layoutsTree.setMultiSelectEnabled(false);
    controlsTree.setMultiSelectEnabled(false);
    navigationTabs.getTabbedButtonBar().setMinimumTabScaleFactor(0.5);

    whereUsedLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    whereUsedLabel.setColour(juce::Label::textColourId, juce::Colour(0xff90a4ae));
    whereUsedLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff12141a));
    whereUsedLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(whereUsedLabel);

    whereUsedListBox.setRowHeight(20);
    whereUsedListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff101218));
    whereUsedListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff252b3b));
    addAndMakeVisible(whereUsedListBox);

    // Tree collapsed by default
    addAndMakeVisible(expandAllButton);
    addAndMakeVisible(collapseAllButton);
    expandAllButton.onClick = [this]() {
        auto* tree = navigationTabs.getCurrentTabIndex() == 0 ? &controlsTree : &layoutsTree;
        std::function<void(juce::TreeViewItem*, bool)> setOpenRec = [&](juce::TreeViewItem* item, bool open) {
            if (!item) return;
            item->setOpen(open);
            for (int i = 0; i < item->getNumSubItems(); ++i) setOpenRec(item->getSubItem(i), open);
        };
        setOpenRec(tree->getRootItem(), true);
    };
    collapseAllButton.onClick = [this]() {
        auto* tree = navigationTabs.getCurrentTabIndex() == 0 ? &controlsTree : &layoutsTree;
        std::function<void(juce::TreeViewItem*, bool)> setOpenRec = [&](juce::TreeViewItem* item, bool open) {
            if (!item) return;
            item->setOpen(open);
            for (int i = 0; i < item->getNumSubItems(); ++i) setOpenRec(item->getSubItem(i), open);
        };
        setOpenRec(tree->getRootItem(), false);
    };

    addAndMakeVisible(refreshButton);
    addAndMakeVisible(exportSnapshotButton);
    addAndMakeVisible(importSnapshotButton);
    addAndMakeVisible(restoreFactoryButton);

    exportSnapshotButton.onClick = [this]() {
        juce::DynamicObject::Ptr snapshot = new juce::DynamicObject();
        juce::DynamicObject::Ptr controlsObj = new juce::DynamicObject();
        juce::DynamicObject::Ptr layoutsObj = new juce::DynamicObject();
        
        auto controlsDir = getAssetFile("controls", "");
        juce::DirectoryIterator iterC(controlsDir, false, "*.json");
        while (iterC.next()) {
            auto f = iterC.getFile();
            auto p = juce::JSON::parse(f.loadFileAsString());
            if (p.isObject()) controlsObj->setProperty(f.getFileName(), p);
        }
        
        auto layoutsDir = getAssetFile("layouts", "");
        juce::DirectoryIterator iterL(layoutsDir, false, "*.json");
        while (iterL.next()) {
            auto f = iterL.getFile();
            auto p = juce::JSON::parse(f.loadFileAsString());
            if (p.isObject()) layoutsObj->setProperty(f.getFileName(), p);
        }
        
        snapshot->setProperty("controls", juce::var(controlsObj.get()));
        snapshot->setProperty("layouts", juce::var(layoutsObj.get()));
        
        auto fc = std::make_shared<juce::FileChooser>("Export Snapshot", juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("snapshot.json"), "*.json");
        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, 
            [snapshot, fc](const juce::FileChooser& chooser) {
                auto result = chooser.getResult();
                if (result != juce::File{}) {
                    result.replaceWithText(juce::JSON::toString(juce::var(snapshot.get())));
                }
            });
    };

    importSnapshotButton.onClick = [this]() {
        auto fc = std::make_shared<juce::FileChooser>("Import Snapshot", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.json");
        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, fc](const juce::FileChooser& chooser) {
                auto f = chooser.getResult();
                if (f != juce::File{}) {
                    auto parsed = juce::JSON::parse(f.loadFileAsString());
                    if (parsed.isObject()) {
                        auto* root = parsed.getDynamicObject();
                        if (root->hasProperty("controls")) {
                            auto* controlsObj = root->getProperty("controls").getDynamicObject();
                            if (controlsObj) {
                                for (auto& prop : controlsObj->getProperties()) {
                                    getAssetFile("controls", prop.name.toString()).replaceWithText(juce::JSON::toString(prop.value));
                                }
                            }
                        }
                        if (root->hasProperty("layouts")) {
                            auto* layoutsObj = root->getProperty("layouts").getDynamicObject();
                            if (layoutsObj) {
                                for (auto& prop : layoutsObj->getProperties()) {
                                    getAssetFile("layouts", prop.name.toString()).replaceWithText(juce::JSON::toString(prop.value));
                                }
                            }
                        }
                        refreshButton.triggerClick();
                    }
                }
            });
    };

    restoreFactoryButton.onClick = [this]() {
        auto f = getAssetFile("factory_defaults_snapshot.json", "");
        auto parsed = juce::JSON::parse(f.loadFileAsString());
        if (parsed.isObject()) {
            auto* root = parsed.getDynamicObject();
            if (root->hasProperty("controls")) {
                auto* controlsObj = root->getProperty("controls").getDynamicObject();
                if (controlsObj) {
                    for (auto& prop : controlsObj->getProperties()) {
                        getAssetFile("controls", prop.name.toString()).replaceWithText(juce::JSON::toString(prop.value));
                    }
                }
            }
            if (root->hasProperty("layouts")) {
                auto* layoutsObj = root->getProperty("layouts").getDynamicObject();
                if (layoutsObj) {
                    for (auto& prop : layoutsObj->getProperties()) {
                        getAssetFile("layouts", prop.name.toString()).replaceWithText(juce::JSON::toString(prop.value));
                    }
                }
            }
            refreshButton.triggerClick();
        }
    };

    refreshButton.onClick = [this]() { 
        RlyehSound::ParameterManager::getInstance().reloadFromJson(controlsJsonDocument.getAllContent());
        auto xmlL = layoutsTree.getOpennessState(false);
        auto xmlC = controlsTree.getOpennessState(false);
        auto* tree = navigationTabs.getCurrentTabIndex() == 0 ? &controlsTree : &layoutsTree;
        auto* selected = tree->getSelectedItem(0);
        juce::String selName = selected ? static_cast<EditorTreeItem*>(selected)->name : "";
        juce::String selPage = selected ? static_cast<EditorTreeItem*>(selected)->pageId : "";
        juce::String selProd = selected ? static_cast<EditorTreeItem*>(selected)->productId : "";
        
        buildTree();
        buildReferencesIndex();
        
        if (xmlL) layoutsTree.restoreOpennessState(*xmlL, false);
        if (xmlC) controlsTree.restoreOpennessState(*xmlC, false);
        
        
                if (selName.isNotEmpty()) {
            std::function<void(EditorTreeItem*)> findAndSelect = [&](EditorTreeItem* n) {
                if (n->name == selName && n->pageId == selPage && n->productId == selProd) {
                    n->setSelected(true, true);
                    return;
                }
                for (int i=0; i < n->getNumSubItems(); ++i) findAndSelect(static_cast<EditorTreeItem*>(n->getSubItem(i)));
            };
            findAndSelect(static_cast<EditorTreeItem*>(tree->getRootItem()));
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
        
        juce::String oldLayoutStr = juce::JSON::toString(juce::JSON::parse(originalLayoutJson));
        juce::String newLayoutStr = juce::JSON::toString(parsedLayoutEdit);
        juce::String oldControlsStr = juce::JSON::toString(juce::JSON::parse(originalControlsJson));
        juce::String newControlsStr = juce::JSON::toString(parsedControlsEdit);

        auto* saveComp = new SaveConfirmComponent(oldLayoutStr, newLayoutStr, oldControlsStr, newControlsStr,
            [this, parsedLayoutEdit, parsedControlsEdit, newControls](SaveConfirmComponent* comp) {
                // Commit
                auto file = getAssetFile(currentProductId == "theme" || currentProductId == "callouts" ? "themes" : "layouts", currentParamJsonFile);
                if (file.existsAsFile()) {
                    if (currentProductId == "callouts") {
                        auto parsedGlobal = juce::JSON::parse(file.loadFileAsString());
                        if (parsedGlobal.isObject()) {
                            auto* root = parsedGlobal.getDynamicObject();
                            if (root->hasProperty("callout_styles")) {
                                auto* calloutsObj = root->getProperty("callout_styles").getDynamicObject();
                                if (calloutsObj) {
                                    calloutsObj->setProperty(currentCardId, parsedLayoutEdit);
                                    file.replaceWithText(juce::JSON::toString(parsedGlobal));
                                }
                            }
                        }
                    } else if (currentProductId != "theme" && currentLayout.isObject()) {
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
                if (currentProductId != "theme" && currentProductId != "callouts" && newControls) {
                    for (auto& prop : newControls->getProperties()) {
                        juce::String paramId = prop.name.toString();
                        if (paramToFileMap.count(paramId) > 0) {
                            auto cfile = getAssetFile("controls", paramToFileMap[paramId]);
                            if (cfile.existsAsFile()) {
                                auto parsedC = juce::JSON::parse(cfile.loadFileAsString());
                                if (parsedC.isObject()) {
                                    parsedC.getDynamicObject()->setProperty(paramId, prop.value);
                                    cfile.replaceWithText(juce::JSON::toString(parsedC));
                                }
                            }
                        }
                    }
                }
                
                RlyehSound::ParameterManager::getInstance().reloadFromJson(controlsJsonDocument.getAllContent());
                buildTree();
                
                if (auto* dw = comp->findParentComponentOfClass<juce::DialogWindow>()) {
                    dw->exitModalState(0);
                    dw->setVisible(false);
                }
            },
            [](SaveConfirmComponent* comp) {
                if (auto* dw = comp->findParentComponentOfClass<juce::DialogWindow>()) {
                    dw->exitModalState(0);
                    dw->setVisible(false);
                }
            });
            
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(saveComp);
        opts.dialogTitle = "Confirm Save";
        opts.dialogBackgroundColour = juce::Colour(0xff1e1e1e);
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = false;
        opts.launchAsync();
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
    buildReferencesIndex();
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
    auto* lRoot = new EditorTreeItem(this, "Root", "root");
    auto* cRoot = new EditorTreeItem(this, "Root", "root");

    const char* products[] = { "The Klang Farmer", "The Klang Mill", "The Klang Planter", "The Klang Seed" };
    const char* productIds[] = { "tkf", "tkm", "tkp", "tks" };
    
    for (int p = 0; p < 4; ++p) {
        auto* lProdNode = new EditorTreeItem(this, products[p], "product", productIds[p]);
        auto* cProdNode = new EditorTreeItem(this, products[p], "product", productIds[p]);
        
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
                            juce::String cardName = prop.name.toString();
                            lProdNode->addSubItem(new EditorTreeItem(this, cardName, "card_theme", productIds[p], "", cardName));
                            
                            auto* cCardNode = new EditorTreeItem(this, cardName, "card", productIds[p], "", cardName);
                            auto params = pObj->getProperty("parameters");
                            if (params.isArray()) {
                                int i = 1;
                                for (auto& param : *params.getArray()) {
                                    cCardNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", productIds[p], "", cardName, param.toString()));
                                }
                            }
                            cProdNode->addSubItem(cCardNode);
                        } else {
                            juce::String pageName = prop.name.toString();
                            auto* lPageNode = new EditorTreeItem(this, pageName, "page", productIds[p], pageName);
                            auto* cPageNode = new EditorTreeItem(this, pageName, "page", productIds[p], pageName);
                            
                            for (auto& cardProp : pObj->getProperties()) {
                                juce::String cardName = cardProp.name.toString();
                                lPageNode->addSubItem(new EditorTreeItem(this, cardName, "card_theme", productIds[p], pageName, cardName));
                                
                                auto* cCardNode = new EditorTreeItem(this, cardName, "card", productIds[p], pageName, cardName);
                                if (cardProp.value.isObject()) {
                                    auto params = cardProp.value.getDynamicObject()->getProperty("parameters");
                                    if (params.isArray()) {
                                        int i = 1;
                                        for (auto& param : *params.getArray()) {
                                            cCardNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", productIds[p], pageName, cardName, param.toString()));
                                        }
                                    }
                                }
                                cPageNode->addSubItem(cCardNode);
                            }
                            lProdNode->addSubItem(lPageNode);
                            cProdNode->addSubItem(cPageNode);
                        }
                    }
                }
            }
        }
        
        // Mount Product-Specific Callouts (e.g. Master Limiter for TKP)
        auto calloutsFile = getAssetFile("themes", "callouts.json");
        if (calloutsFile.existsAsFile()) {
            auto parsedCallouts = juce::JSON::parse(calloutsFile.loadFileAsString());
            if (parsedCallouts.isObject() && parsedCallouts.getDynamicObject()->hasProperty("callout_styles")) {
                auto* cStyles = parsedCallouts.getDynamicObject()->getProperty("callout_styles").getDynamicObject();
                if (cStyles) {
                    for (auto& cProp : cStyles->getProperties()) {
                        if (cProp.value.isObject()) {
                            auto* cDef = cProp.value.getDynamicObject();
                            if (cDef->getProperty("product").toString() == productIds[p]) {
                                juce::String calloutKey = cProp.name.toString();
                                juce::String title = (calloutKey == "planter_limiter") ? "Master Limiter" : cDef->getProperty("title").toString();
                                juce::String displayTitle = "[Callout] " + title;

                                lProdNode->addSubItem(new EditorTreeItem(this, displayTitle, "callout_preview", productIds[p], "", calloutKey));

                                auto* cCalloutNode = new EditorTreeItem(this, displayTitle, "callout_preview", productIds[p], "", calloutKey);
                                auto params = cDef->getProperty("parameters");
                                if (params.isArray()) {
                                    int i = 1;
                                    for (auto& param : *params.getArray()) {
                                        cCalloutNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", productIds[p], "", calloutKey, param.toString()));
                                    }
                                }
                                cProdNode->addSubItem(cCalloutNode);
                            }
                        }
                    }
                }
            }
        }

        lRoot->addSubItem(lProdNode);
        cRoot->addSubItem(cProdNode);
    }
    
    auto* layoutsRootNode = new EditorTreeItem(this, "Raw Layout JSONs", "product", "all_layouts");
    juce::DirectoryIterator iterL(getAssetFile("layouts", ""), false, "*.json");
    while (iterL.next()) {
        auto f = iterL.getFile();
        layoutsRootNode->addSubItem(new EditorTreeItem(this, f.getFileName(), "layout_file", "all_layouts", f.getFileNameWithoutExtension()));
    }
    lRoot->addSubItem(layoutsRootNode);
    
    auto* controlsRootNode = new EditorTreeItem(this, "Raw Control JSONs", "product", "all_controls");
    juce::DirectoryIterator iterC(getAssetFile("controls", ""), false, "*.json");
    while (iterC.next()) {
        auto f = iterC.getFile();
        if (f.getFileName() == "theme.json") continue; 
        
        juce::String fileName = f.getFileName();
        juce::String pageId = f.getFileNameWithoutExtension();
        auto* fileNode = new EditorTreeItem(this, fileName, "control_file", "all_controls", pageId);
        
        auto parsedC = juce::JSON::parse(f.loadFileAsString());
        if (parsedC.isObject()) {
            for (auto& prop : parsedC.getDynamicObject()->getProperties()) {
                if (prop.value.isObject()) {
                    fileNode->addSubItem(new EditorTreeItem(this, prop.name.toString(), "card_param", "all_controls", pageId, pageId, prop.name.toString()));
                }
            }
        }
        controlsRootNode->addSubItem(fileNode);
    }
    cRoot->addSubItem(controlsRootNode);

    auto makeCalloutsNode = [this]() {
        auto* calloutsNode = new EditorTreeItem(this, "Callouts & Overlays", "product", "callouts");
        auto* limiterNode = new EditorTreeItem(this, "Planter Master Limiter", "callout_preview", "tkp", "", "planter_limiter");
        auto* modNode = new EditorTreeItem(this, "Slider Modulation & Snaps", "callout_preview", "tkf", "", "slider_modulation");

        auto cFile = getAssetFile("themes", "callouts.json");
        if (cFile.existsAsFile()) {
            auto parsedGlobal = juce::JSON::parse(cFile.loadFileAsString());
            if (parsedGlobal.isObject() && parsedGlobal.getDynamicObject()->hasProperty("callout_styles")) {
                auto* cStyles = parsedGlobal.getDynamicObject()->getProperty("callout_styles").getDynamicObject();
                if (cStyles) {
                    if (cStyles->hasProperty("planter_limiter")) {
                        auto* pDef = cStyles->getProperty("planter_limiter").getDynamicObject();
                        if (pDef && pDef->hasProperty("parameters")) {
                            auto params = pDef->getProperty("parameters");
                            if (params.isArray()) {
                                int i = 1;
                                for (auto& param : *params.getArray()) {
                                    limiterNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", "tkp", "", "planter_limiter", param.toString()));
                                }
                            }
                        }
                    }
                    if (cStyles->hasProperty("slider_modulation")) {
                        auto* mDef = cStyles->getProperty("slider_modulation").getDynamicObject();
                        if (mDef && mDef->hasProperty("parameters")) {
                            auto params = mDef->getProperty("parameters");
                            if (params.isArray()) {
                                int i = 1;
                                for (auto& param : *params.getArray()) {
                                    modNode->addSubItem(new EditorTreeItem(this, juce::String(i++) + ": " + param.toString(), "card_param", "tkf", "", "slider_modulation", param.toString()));
                                }
                            }
                        }
                    }
                }
            }
        }

        calloutsNode->addSubItem(limiterNode);
        calloutsNode->addSubItem(modNode);
        return calloutsNode;
    };
    lRoot->addSubItem(makeCalloutsNode());
    cRoot->addSubItem(makeCalloutsNode());
    
    layoutsTree.setRootItem(lRoot);
    layoutsTree.setRootItemVisible(false);
    controlsTree.setRootItem(cRoot);
    controlsTree.setRootItemVisible(false);
}

juce::String MainComponent::getControlFileForParam(const juce::String& paramId) {
    juce::DirectoryIterator iterC(getAssetFile("controls", ""), false, "*.json");
    while (iterC.next()) {
        auto f = iterC.getFile();
        if (f.getFileName() == "theme.json") continue;
        auto parsed = juce::JSON::parse(f.loadFileAsString());
        if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(paramId)) {
            return f.getFileName();
        }
    }
    return "";
}

void MainComponent::onTreeItemSelected(EditorTreeItem* item) {
    if (!item) {
        updateWhereUsed(nullptr);
        return;
    }
    juce::Logger::writeToLog("onTreeItemSelected: " + item->name);
    if (item->itemType != "card" && item->itemType != "card_theme" && item->itemType != "card_param" && !(item->itemType == "product" && item->productId == "theme") && item->itemType != "control_file" && item->itemType != "layout_file" && item->itemType != "page" && item->itemType != "product" && item->itemType != "callout_preview") return;
    
    if (item->itemType == "product" && item->productId == "callouts") return;

    currentParamTarget = item->paramId;
    updateWhereUsed(item);
    
    if (item->itemType == "callout_preview") {
        currentProductId = "callouts";
        currentParamJsonFile = "callouts.json";
        currentPageId = "";
        currentCardId = item->cardId;
        activePreviewProduct = item->productId;
    } else if (item->itemType == "product" && item->productId == "theme") {
        currentProductId = "theme";
        currentParamJsonFile = "theme.json";
        currentPageId = "";
        currentCardId = "theme";
    } else if (item->itemType == "control_file" || item->itemType == "card_param") {
        if (item->productId != "all_controls" && item->productId != "theme" && item->productId != "all_layouts") {
            activePreviewProduct = item->productId;
        }
        currentProductId = "all_controls";
        
        if (item->itemType == "card_param" && item->productId != "all_controls") {
            currentParamJsonFile = getControlFileForParam(item->paramId);
        } else {
            currentParamJsonFile = item->pageId + ".json";
        }
        
        currentPageId = "";
        currentCardId = "";
    } else if (item->productId == "all_layouts" || item->itemType == "layout_file") {
        currentProductId = "all_layouts";
        currentParamJsonFile = item->pageId + ".json";
        currentPageId = "";
        currentCardId = "";
    } else {
        currentProductId = item->productId;
        currentParamJsonFile = currentProductId + "_layout.json";
        currentPageId = item->pageId;
        currentCardId = item->cardId;
    }

    auto file = getAssetFile(currentProductId == "theme" || currentProductId == "callouts" ? "themes" : (currentProductId == "all_controls" ? "controls" : "layouts"), currentParamJsonFile);
    if (currentProductId == "callouts") {
        filePathDisplay.setText(file.getFullPathName() + " -> [callout_styles] -> [" + currentCardId + "]");
    } else {
        filePathDisplay.setText(file.getFullPathName() + (currentProductId != "theme" && currentProductId != "all_controls" && currentProductId != "all_layouts" ? " -> [" + currentPageId + "] -> [" + currentCardId + "]" : "") + (currentParamTarget.isNotEmpty() ? " -> [" + currentParamTarget + "]" : ""));
    }

    juce::var cardJson;
    if (file.existsAsFile()) {
        auto fullJsonString = file.loadFileAsString();
        
        if (currentProductId == "callouts") {
            auto parsedGlobal = juce::JSON::parse(fullJsonString);
            if (parsedGlobal.isObject()) {
                auto* obj = parsedGlobal.getDynamicObject();
                if (obj->hasProperty("callout_styles")) {
                    auto* calloutsObj = obj->getProperty("callout_styles").getDynamicObject();
                    if (calloutsObj && calloutsObj->hasProperty(currentCardId)) {
                        cardJson = calloutsObj->getProperty(currentCardId);
                        originalLayoutJson = juce::JSON::toString(cardJson);
                    }
                }
            }
            originalControlsJson = "{}";
        } else if (currentProductId == "all_layouts") {
            cardJson = juce::JSON::parse(fullJsonString);
            originalLayoutJson = fullJsonString;
            originalControlsJson = "{}";
        } else if (currentProductId != "theme" && currentProductId != "all_controls") {
            currentLayout = juce::JSON::parse(fullJsonString);
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
        } else if (currentProductId == "all_controls") {
            originalLayoutJson = "{}";
            originalControlsJson = fullJsonString;
            
            juce::DynamicObject::Ptr fakeCard = new juce::DynamicObject();
            juce::Array<juce::var> paramArray;
            auto parsedC = juce::JSON::parse(originalControlsJson);
            paramToFileMap.clear();
            if (parsedC.isObject()) {
                for (auto& prop : parsedC.getDynamicObject()->getProperties()) {
                    paramArray.add(prop.name.toString());
                    paramToFileMap[prop.name.toString()] = currentParamJsonFile;
                }
            }
            fakeCard->setProperty("parameters", juce::var(paramArray));
            cardJson = juce::var(fakeCard.get());
        } else {
            originalLayoutJson = fullJsonString;
            originalControlsJson = "{}";
        }
        
        layoutJsonDocument.replaceAllContent(originalLayoutJson);
        controlsJsonDocument.replaceAllContent(originalControlsJson);
        
        if (currentProductId == "callouts") {
            jsonSplitterLayout.setItemLayout(0, -1.0, -1.0, -1.0);
            jsonSplitterLayout.setItemLayout(1, 0, 0, 0);
            jsonSplitterLayout.setItemLayout(2, 0, 0, 0);
        } else if (navigationTabs.getCurrentTabIndex() == 1) { // LAYOUTS
            jsonSplitterLayout.setItemLayout(0, -1.0, -1.0, -1.0);
            jsonSplitterLayout.setItemLayout(1, 0, 0, 0);          
            jsonSplitterLayout.setItemLayout(2, 0, 0, 0);          
        } else { // CONTROLS
            jsonSplitterLayout.setItemLayout(0, 0, 0, 0);          
            jsonSplitterLayout.setItemLayout(1, 0, 0, 0);          
            jsonSplitterLayout.setItemLayout(2, -1.0, -1.0, -1.0); 
        }
        resized();
        formEditor.clear();
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
    juce::Logger::writeToLog("syncJsonToPreview");
    auto jsonString = forcedJson.isNotEmpty() ? forcedJson : layoutJsonDocument.getAllContent();
    bool isTheme = (currentProductId == "theme");
    bool isCallout = (currentProductId == "callouts");
    
    auto parsed = juce::JSON::parse(jsonString);
    if (parsed.isVoid() || jsonString.isEmpty()) {
        emptyPlaceholder.setVisible(true);
        activeSliders.clear();
        previewWrapper.deleteAllChildren();
        compToParamId.clear();
        formEditor.clear();
        return;
    }
    emptyPlaceholder.setVisible(false);

    activeSliders.clear();
    previewWrapper.deleteAllChildren();
    compToParamId.clear();

    bool shouldKeepForm = isCallout && !formEditor.isEmpty() && (layoutJsonEditor == nullptr || !layoutJsonEditor->hasKeyboardFocus(true));
    if (!shouldKeepForm && !formEditor.hasKeyboardFocus(true)) {
        formEditor.clear();
    }

    if (isTheme) {
        RlyehSound::ParameterManager::getInstance().reloadFromJson(jsonString);
    } else if (!isCallout) {
        RlyehSound::ParameterManager::getInstance().reloadFromJson(controlsJsonDocument.getAllContent());
    }

if (isTheme && parsed.isObject()) {
        juce::Array<juce::PropertyComponent*> props;
        auto* obj = parsed.getDynamicObject();
        if (obj->hasProperty("colors")) {
            auto* colorsObj = obj->getProperty("colors").getDynamicObject();
            if (colorsObj) {
                for (auto& prop : colorsObj->getProperties()) {
                    auto pc = new ThemeColorPropertyComponent(prop.name.toString(), prop.value.toString(), 
                        [this, propName = prop.name.toString()](const juce::String& newHex) {
                            auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                            if (parsedObj.isObject() && parsedObj.getDynamicObject()->hasProperty("colors")) {
                                parsedObj.getDynamicObject()->getProperty("colors").getDynamicObject()->setProperty(propName, newHex);
                                layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                            }
                        });
                    props.add(pc);
                }
            }
        }
        if (!props.isEmpty()) {
            formEditor.addSection("Theme Colors", props);
        }
    }
    
    if (isTheme) {
        // We removed theme node, but keep this just in case
        auto* card = new ModuleCardComponent("Theme Preview", juce::Colour(0xffff3b30));
        previewWrapper.addAndMakeVisible(card);
        card->setBounds(10, 10, 280, 200);
    } else if (currentProductId == "all_layouts") {
        if (parsed.isObject()) {
            int yOffset = 10;
            auto* lObj = parsed.getDynamicObject();
            for (auto& prop : lObj->getProperties()) {
                if (prop.value.isObject()) {
                    auto* pObj = prop.value.getDynamicObject();
                    auto processCard = [&](const juce::String& cardName, juce::DynamicObject* cardObj) {
                        if (!cardObj->hasProperty("parameters")) return;
                        auto paramsArr = cardObj->getProperty("parameters");
                        if (!paramsArr.isArray()) return;
                        
                        juce::Colour c = juce::Colour(0xffcfd8dc);
                        if (cardObj->hasProperty("color")) {
                            c = juce::Colour::fromString(cardObj->getProperty("color").toString());
                        }
                        auto styleStr = cardObj->hasProperty("style") ? cardObj->getProperty("style").toString() : "StandardDark";
                        auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
                        
                        auto* card = new ModuleCardComponent(cardName, c, style);
                        
                        int slot = 0;
                        int choiceCount = 0;
                        for (auto& paramIdVar : *paramsArr.getArray()) {
                            juce::String paramId = paramIdVar.toString();
                            auto* constDef = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                            if (constDef) {
                                if (constDef->type == "float" && slot < 4) {
                                    auto* slider = new RotaryKnobSlider();
                                    activeSliders.add(slider);
                                    compToParamId[slider] = paramId;
                                    slider->setParamId(paramId);
                                    slider->setRange(constDef->min, constDef->max, constDef->step);
                                    slider->setValue(constDef->defaultFloat);
                                    card->addAndMakeVisible(slider);
                                    card->setKnob(slot, constDef->name, slider);
                                    slot++;
                                } else if (constDef->type == "choice" && choiceCount < 2) {
                                    auto* box = new LedSelectorComponent(c);
                                    activeSliders.add(box);
                                    compToParamId[box] = paramId;
                                    box->setItems(constDef->choices, constDef->choices.size() > 3 ? 4 : constDef->choices.size());
                                    box->setSelectedIndex(constDef->defaultChoice, juce::dontSendNotification);
                                    card->addAndMakeVisible(box);
                                    if (choiceCount == 0) card->setLedSelector(box);
                                    else card->setSecondLedSelector(box);
                                    choiceCount++;
                                }
                            }
                        }
                        
                        auto* label = new juce::Label({}, cardName);
                        label->setColour(juce::Label::textColourId, juce::Colours::white);
                        label->setFont(16.0f);
                        previewWrapper.addAndMakeVisible(label);
                        label->setBounds(10, yOffset, 280, 20);
                        yOffset += 25;
                        
                        previewWrapper.addAndMakeVisible(card);
                        card->setBounds(10, yOffset, 280, 420);
                        yOffset += 430;
                    };
                    
                    if (pObj->hasProperty("parameters")) {
                        processCard(prop.name.toString(), pObj);
                    } else {
                        for (auto& cardProp : pObj->getProperties()) {
                            if (cardProp.value.isObject()) {
                                processCard(cardProp.name.toString(), cardProp.value.getDynamicObject());
                            }
                        }
                    }
                }
            }
        }
    } else if (currentProductId == "all_controls") {
        if (parsed.isObject()) {
            const char* products[] = { "tkf", "tkm", "tkp", "tks" };
            const char* prodNames[] = { "The Klang Farmer", "The Klang Mill", "The Klang Planter", "The Klang Seed" };
            const char* btnNames[] = { "Farmer", "Mill", "Planter", "Seed" };
            
            int btnX = 10;
            for (int p = 0; p < 4; ++p) {
                auto* btn = new juce::TextButton(btnNames[p]);
                btn->setClickingTogglesState(true);
                btn->setRadioGroupId(100);
                btn->setToggleState(activePreviewProduct == products[p], juce::dontSendNotification);
                btn->onClick = [this, prod = juce::String(products[p])]() {
                    activePreviewProduct = prod;
                    syncJsonToPreview("");
                };
                previewWrapper.addAndMakeVisible(btn);
                btn->setBounds(btnX, 10, 60, 24);
                btnX += 65;
            }
            
            int yOffset = 45;
            int cardsRendered = 0;
            
            for (int p = 0; p < 4; ++p) {
                if (activePreviewProduct != products[p]) continue;
                juce::File layoutFile = getAssetFile("layouts", juce::String(products[p]) + "_layout.json");
                if (!layoutFile.existsAsFile()) continue;
                
                auto layoutJson = juce::JSON::parse(layoutFile.loadFileAsString());
                if (!layoutJson.isObject()) continue;
                
                auto* lObj = layoutJson.getDynamicObject();
                for (auto& prop : lObj->getProperties()) {
                    if (prop.value.isObject()) {
                        auto* pObj = prop.value.getDynamicObject();
                        auto processCard = [&](const juce::String& cardName, juce::DynamicObject* cardObj) {
                            if (!cardObj->hasProperty("parameters")) return;
                            auto paramsArr = cardObj->getProperty("parameters");
                            if (!paramsArr.isArray()) return;
                            
                            bool matches = false;
                            for (auto& paramVar : *paramsArr.getArray()) {
                                juce::String pId = paramVar.toString();
                                if (currentParamTarget.isNotEmpty() && currentParamTarget == pId) {
                                    matches = true; break;
                                } else if (currentParamTarget.isEmpty() && parsed.getDynamicObject()->hasProperty(pId)) {
                                    matches = true; break;
                                }
                            }
                            if (!matches) return;
                            
                            // Render this card!
                            juce::Colour c = juce::Colour(0xffcfd8dc);
                            if (cardObj->hasProperty("color")) {
                                c = juce::Colour::fromString(cardObj->getProperty("color").toString());
                            }
                            auto styleStr = cardObj->hasProperty("style") ? cardObj->getProperty("style").toString() : "StandardDark";
                            auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
                            
                            auto* card = new ModuleCardComponent(cardName, c, style);
                            
                            // Load parameters into card
                            int slot = 0;
                            int choiceCount = 0;
                            for (auto& paramIdVar : *paramsArr.getArray()) {
                                juce::String paramId = paramIdVar.toString();
                                auto* constDef = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                                if (constDef) {
                                    if (constDef->type == "float" && slot < 4) {
                                        auto* slider = new RotaryKnobSlider();
                                        activeSliders.add(slider);
                                        compToParamId[slider] = paramId;
                                        slider->setParamId(paramId);
                                        slider->setRange(constDef->min, constDef->max, constDef->step);
                                        slider->setValue(constDef->defaultFloat);
                                        card->addAndMakeVisible(slider);
                                        card->setKnob(slot, constDef->name, slider);
                                        slot++;
                                    } else if (constDef->type == "choice" && choiceCount < 2) {
                                        auto* box = new LedSelectorComponent(c);
                                        activeSliders.add(box);
                                        compToParamId[box] = paramId;
                                        box->setItems(constDef->choices, constDef->choices.size() > 3 ? 4 : constDef->choices.size());
                                        box->setSelectedIndex(constDef->defaultChoice, juce::dontSendNotification);
                                        card->addAndMakeVisible(box);
                                        if (choiceCount == 0) card->setLedSelector(box);
                                        else card->setSecondLedSelector(box);
                                        choiceCount++;
                                    }
                                }
                            }
                            
                            auto* label = new juce::Label({}, juce::String(prodNames[p]) + " - " + cardName);
                            label->setColour(juce::Label::textColourId, juce::Colours::white);
                            label->setFont(16.0f);
                            previewWrapper.addAndMakeVisible(label);
                            label->setBounds(10, yOffset, 280, 20);
                            yOffset += 25;
                            
                            previewWrapper.addAndMakeVisible(card);
                            card->setBounds(10, yOffset, 280, 420);
                            yOffset += 430;
                            cardsRendered++;
                        };
                        
                        if (pObj->hasProperty("parameters")) {
                            processCard(prop.name.toString(), pObj);
                        } else {
                            for (auto& cardProp : pObj->getProperties()) {
                                if (cardProp.value.isObject()) {
                                    processCard(cardProp.name.toString(), cardProp.value.getDynamicObject());
                                }
                            }
                        }
                    }
                }
            }
            
            if (cardsRendered == 0) {
                auto* emptyLbl = new juce::Label({}, "No cards in this plugin use these parameters.");
                emptyLbl->setColour(juce::Label::textColourId, juce::Colours::grey);
                emptyLbl->setJustificationType(juce::Justification::centred);
                previewWrapper.addAndMakeVisible(emptyLbl);
                emptyLbl->setBounds(10, yOffset + 20, 280, 40);
            }
        }
    } else if (isCallout) {
        if (parsed.isObject()) {
            juce::String title = parsed.getProperty("title", "CALLOUT").toString();
            int w = parsed.getProperty("width", 300);
            int h = parsed.getProperty("height", 130);
            juce::Colour bg = juce::Colour::fromString(parsed.getProperty("background_colour", "0xff1e222b").toString());
            juce::Colour border = juce::Colour::fromString(parsed.getProperty("border_colour", "0xffe53935").toString());
            float radius = (float) parsed.getProperty("corner_radius", 5.0);

            auto* card = new CalloutPreviewCard(currentCardId, title, bg, border, radius);
            previewWrapper.addAndMakeVisible(card);

            int pwW = previewWrapper.getWidth();
            int pwH = previewWrapper.getHeight();
            if (pwW <= 0) pwW = 600;
            if (pwH <= 0) pwH = 400;

            int cardX = std::max(10, (pwW - w) / 2);
            int cardY = std::max(10, (pwH - h) / 2);
            card->setBounds(cardX, cardY, w, h);
        }
    } else {
        if (parsed.isObject()) {
            auto moduleConfig = parsed;
            
            juce::Colour c = juce::Colour(0xffcfd8dc);
            if (moduleConfig.hasProperty("color")) {
                c = juce::Colour::fromString(moduleConfig.getProperty("color", "").toString());
            }
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();
            auto style = (styleStr == "DoepferSilver") ? ModuleCardComponent::PanelStyle::DoepferSilver : ModuleCardComponent::PanelStyle::StandardDark;
            
            auto* card = new ModuleCardComponent(currentCardId, c, style);
            
                        auto paramsArray = moduleConfig.getProperty("parameters", juce::var());
            if (paramsArray.isArray()) {
                int slot = 0;
                int choiceCount = 0;
                for (auto& paramIdVar : *paramsArray.getArray()) {
                    juce::String paramId = paramIdVar.toString();
                    auto* constDef = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                    auto* def = const_cast<RlyehSound::ControlDef*>(constDef);
                    if (def) {
                        auto updateControlsJson = [this, paramId](const juce::String& key, const juce::var& newValue) {
                            auto parsed = juce::JSON::parse(controlsJsonDocument.getAllContent());
                            if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(paramId)) {
                                parsed.getDynamicObject()->getProperty(paramId).getDynamicObject()->setProperty(key, newValue);
                                controlsJsonDocument.replaceAllContent(juce::JSON::toString(parsed));
                            }
                        };
                        
                        auto updateChoicesJson = [this, paramId](const juce::StringArray& choices) {
                            auto parsed = juce::JSON::parse(controlsJsonDocument.getAllContent());
                            if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(paramId)) {
                                juce::Array<juce::var> arr;
                                for (auto& c : choices) arr.add(c);
                                parsed.getDynamicObject()->getProperty(paramId).getDynamicObject()->setProperty("choices", juce::var(arr));
                                controlsJsonDocument.replaceAllContent(juce::JSON::toString(parsed));
                            }
                        };
                        if (def->type == "float" && slot < 4) {
                            auto* slider = new RotaryKnobSlider();
                            activeSliders.add(slider);
                            compToParamId[slider] = paramId;
                            slider->setParamId(paramId);
                            
                            slider->setTooltip(def->description);
                            slider->setRange(def->min, def->max, def->step);
                            slider->setValue(def->defaultFloat);
                            
                            slider->setDoubleClickReturnValue(true, def->doubleClickValue);
                            slider->getDefaultValue = [val = def->doubleClickValue]() { return val; };
                            auto nameLower = def->name.toLowerCase();
                            if (nameLower.contains("hz") || paramId.contains("freq") || paramId.contains("cutoff") || paramId.contains("filter")) {
                                slider->customFormatText = formatFreqHz;
                                slider->customParseText = parseFreqHz;
                            } else if (nameLower.contains("ms") || paramId.contains("decay") || paramId.contains("attack") || paramId.contains("release") || paramId.contains("sh_rate")) {
                                slider->customFormatText = formatTimeMs;
                                slider->customParseText = parseTimeMs;
                            } else if (paramId.contains("crossfade") || paramId.contains("mix")) {
                                slider->customFormatText = formatCrossfade;
                                slider->customParseText = parseCrossfade;
                            } else if (paramId.contains("pan") || paramId.contains("detune")) {
                                slider->customFormatText = formatBipolarPercent;
                                slider->customParseText = parseBipolarPercent;
                            } else if (paramId.contains("semi")) {
                                slider->customFormatText = formatSemi;
                                slider->customParseText = parseSemi;
                            } else if (paramId.contains("db") || paramId.contains("drive") || paramId.contains("gain")) {
                                slider->customFormatText = def->isBipolar ? formatBipolarDb : formatDb;
                                slider->customParseText = def->isBipolar ? parseBipolarDb : parseDb;
                            } else if (def->isBipolar) {
                                slider->customFormatText = formatBipolarPercent;
                                slider->customParseText = parseBipolarPercent;
                            } else {
                                slider->customFormatText = formatPercent;
                                slider->customParseText = parsePercent;
                            }
                            if (paramId.containsIgnoreCase("shape") || paramId.containsIgnoreCase("waveform")) {
                                slider->diagramType = RotaryKnobSlider::DiagramType::Waveform;
                            } else if (paramId.containsIgnoreCase("slope")) {
                                slider->diagramType = RotaryKnobSlider::DiagramType::EnvelopeSlope;
                            }
                            
                            
                            if (def->format == "waveshape") {
                                slider->customFormatText = formatWaveshape;
                                slider->customParseText = parseWaveshape;
                            } else if (def->format == "wet_dry") {
                                slider->customFormatText = formatWetDry;
                                slider->customParseText = parseWetDry;
                            } else {
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
                            }
                            
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
                                    juce::Colour colRed = juce::Colour(0xffff3b30);
                                    juce::Colour colCyan = juce::Colour(0xffcfd8dc);
                                    box->setItemStyle(0, { colRed,  std::nullopt, std::nullopt });
                                    box->setItemStyle(1, { colCyan, std::nullopt, std::nullopt });
                                    box->setItemStyle(2, { colCyan, colRed,       colRed       });
                                    box->setItemStyle(3, { colRed,  colCyan,      colCyan      });
                                } else {
                                    juce::Colour colCarrier = juce::Colour(0xffcfd8dc);
                                    juce::Colour colMod = juce::Colour(0xffcfd8dc);
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

    auto* tree = navigationTabs.getCurrentTabIndex() == 0 ? &controlsTree : &layoutsTree;
    auto* selected = tree->getSelectedItem(0);
    juce::String selType = selected ? static_cast<EditorTreeItem*>(selected)->itemType : "";
    bool showLayout = (navigationTabs.getCurrentTabIndex() == 1); // Only in LAYOUTS tab
    bool showParams = (navigationTabs.getCurrentTabIndex() == 0); // Only in CONTROLS tab

    juce::Array<juce::PropertyComponent*> props;
    if (!isCallout && showLayout && parsed.isObject()) {
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
            
            juce::String pName = prop.name.toString();
            if (pName == "parameters") continue; // Hide the parameters array from the UI!
            
            juce::PropertyComponent* pc = nullptr;
            if (pName == "color" || pName == "accent" || pName == "background" || pName.startsWithIgnoreCase("col")) {
                pc = new ThemeColorPropertyComponent(pName, valStr, [this, pName](const juce::String& newHex) {
                    auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                    if (parsedObj.isObject()) {
                        parsedObj.getDynamicObject()->setProperty(pName, newHex);
                        layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                    }
                });
            } else {
                juce::Value val (valStr);
                
                pc = new juce::TextPropertyComponent(val, pName, 256, false);
            }
            
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

    if (isCallout && parsed.isObject()) {
        if (formEditor.isEmpty()) {
            juce::Array<juce::PropertyComponent*> calloutProps;
            auto* obj = parsed.getDynamicObject();

            auto updateCalloutJson = [this](const juce::String& key, const juce::var& newVal) {
                auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                if (parsedObj.isObject()) {
                    parsedObj.getDynamicObject()->setProperty(key, newVal);
                    layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                }
            };

            auto getPreviewCard = [this]() -> CalloutPreviewCard* {
                if (previewWrapper.getNumChildComponents() > 0)
                    return dynamic_cast<CalloutPreviewCard*>(previewWrapper.getChildComponent(0));
                return nullptr;
            };

            // Title
            juce::String titleVal = obj->getProperty("title").toString();
            calloutProps.add(new CalloutTextPropertyComponent("Title", titleVal, [updateCalloutJson, getPreviewCard](const juce::String& val) {
                if (auto* c = getPreviewCard()) c->setTitle(val);
                updateCalloutJson("title", val);
            }));

            // Width
            juce::String widthVal = obj->getProperty("width").toString();
            calloutProps.add(new CalloutTextPropertyComponent("Width", widthVal, [this, updateCalloutJson, getPreviewCard](const juce::String& val) {
                int w = val.getIntValue();
                if (auto* c = getPreviewCard()) {
                    c->setSize(w, c->getHeight());
                    int cx = std::max(10, (previewWrapper.getWidth() - w) / 2);
                    int cy = std::max(10, (previewWrapper.getHeight() - c->getHeight()) / 2);
                    c->setTopLeftPosition(cx, cy);
                }
                updateCalloutJson("width", w);
            }));

            // Height
            juce::String heightVal = obj->getProperty("height").toString();
            calloutProps.add(new CalloutTextPropertyComponent("Height", heightVal, [this, updateCalloutJson, getPreviewCard](const juce::String& val) {
                int h = val.getIntValue();
                if (auto* c = getPreviewCard()) {
                    c->setSize(c->getWidth(), h);
                    int cx = std::max(10, (previewWrapper.getWidth() - c->getWidth()) / 2);
                    int cy = std::max(10, (previewWrapper.getHeight() - h) / 2);
                    c->setTopLeftPosition(cx, cy);
                }
                updateCalloutJson("height", h);
            }));

            // Background Colour
            juce::String bgVal = obj->getProperty("background_colour").toString();
            calloutProps.add(new ThemeColorPropertyComponent("Background Colour", bgVal, [updateCalloutJson, getPreviewCard](const juce::String& val) {
                if (auto* c = getPreviewCard()) c->setBackgroundColour(juce::Colour::fromString(val));
                updateCalloutJson("background_colour", val);
            }));

            // Border Colour
            juce::String borderVal = obj->getProperty("border_colour").toString();
            calloutProps.add(new ThemeColorPropertyComponent("Border Colour", borderVal, [updateCalloutJson, getPreviewCard](const juce::String& val) {
                if (auto* c = getPreviewCard()) c->setBorderColour(juce::Colour::fromString(val));
                updateCalloutJson("border_colour", val);
            }));

            // Corner Radius
            juce::String radiusVal = obj->getProperty("corner_radius").toString();
            calloutProps.add(new CalloutTextPropertyComponent("Corner Radius", radiusVal, [updateCalloutJson, getPreviewCard](const juce::String& val) {
                float r = val.getFloatValue();
                if (auto* c = getPreviewCard()) c->setCornerRadius(r);
                updateCalloutJson("corner_radius", r);
            }));

            formEditor.addSection("Callout Style: " + currentCardId, calloutProps);
        }
    }
    
    // Add detailed parameter properties
    if (!isCallout && showParams && parsed.isObject()) {
        juce::StringArray paramIds;
        if (currentProductId == "all_controls") {
            auto parsedC = juce::JSON::parse(controlsJsonDocument.getAllContent());
            if (parsedC.isObject()) {
                for (auto& prop : parsedC.getDynamicObject()->getProperties()) {
                    paramIds.add(prop.name.toString());
                }
            }
        } else {
            auto paramsArray = parsed.getDynamicObject()->getProperty("parameters");
            if (paramsArray.isArray()) {
                for (auto& paramIdVar : *paramsArray.getArray()) {
                    paramIds.add(paramIdVar.toString());
                }
            }
        }
        
        if (!paramIds.isEmpty()) {
            for (auto& paramId : paramIds) {
                if (currentParamTarget.isNotEmpty() && currentParamTarget != paramId) continue;
                auto* constDef = RlyehSound::ParameterManager::getInstance().getControlDef(paramId);
                auto* def = const_cast<RlyehSound::ControlDef*>(constDef);
                    if (def) {
                        auto updateControlsJson = [this, paramId](const juce::String& key, const juce::var& newValue) {
                            auto parsed = juce::JSON::parse(controlsJsonDocument.getAllContent());
                            if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(paramId)) {
                                if (key == "min" || key == "max" || key == "step" || key == "skew") {
                                    auto* paramObj = parsed.getDynamicObject()->getProperty(paramId).getDynamicObject();
                                    if (!paramObj->hasProperty("range")) paramObj->setProperty("range", juce::var(new juce::DynamicObject()));
                                    paramObj->getProperty("range").getDynamicObject()->setProperty(key, newValue);
                                } else {
                                    parsed.getDynamicObject()->getProperty(paramId).getDynamicObject()->setProperty(key, newValue);
                                }
                                controlsJsonDocument.replaceAllContent(juce::JSON::toString(parsed));
                            }
                        };
                        
                        auto updateChoicesJson = [this, paramId](const juce::StringArray& choices) {
                            auto parsed = juce::JSON::parse(controlsJsonDocument.getAllContent());
                            if (parsed.isObject() && parsed.getDynamicObject()->hasProperty(paramId)) {
                                juce::Array<juce::var> arr;
                                for (auto& c : choices) arr.add(c);
                                parsed.getDynamicObject()->getProperty(paramId).getDynamicObject()->setProperty("choices", juce::var(arr));
                                controlsJsonDocument.replaceAllContent(juce::JSON::toString(parsed));
                            }
                        };
                    juce::Array<juce::PropertyComponent*> pProps;
                    juce::Component* linkedComp = nullptr;
                    for (int j = 0; j < activeSliders.size(); ++j) {
                        if (compToParamId[activeSliders[j]] == paramId) {
                            linkedComp = activeSliders[j];
                            break;
                        }
                    }
                    
                    juce::String sectionTitle = currentProductId.toUpperCase() + ": " + currentCardId + ": " + def->name;
                    pProps.add(new ParamHeaderPropertyComponent(sectionTitle));
                    
                    pProps.add(new ParamRowPropertyComponent(linkedComp, "Slider Value", def->type, def->choices,
                        [linkedComp, def]() -> double {
                            if (def->type == "float" && linkedComp) return ([&]() { auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getValue() : 0.0; })();
                            if (def->type == "choice" && linkedComp) return ([&]() { auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp); return l ? l->getSelectedIndex() : 0; })();
                            return 0.0;
                        },
                        [linkedComp, def]() -> juce::String {
                            if (def->type == "float" && linkedComp) {
                                auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getTextFromValue(s->getValue()) : "";
                            }
                            if (def->type == "choice" && linkedComp) {
                                auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp); int idx = l ? l->getSelectedIndex() : -1;
                                return (idx >= 0 && idx < def->choices.size()) ? def->choices[idx] : "";
                            }
                            return "";
                        },
                        [linkedComp, def](double v) {
                            if (def->type == "float" && linkedComp) if (auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp)) s->setValue(v, juce::sendNotificationAsync);
                            if (def->type == "choice" && linkedComp) if (auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp)) l->setSelectedIndex((int)v, juce::sendNotificationAsync);
                        },
                        [linkedComp, def](juce::String s) {
                            if (def->type == "float" && linkedComp) {
                                if (auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp)) kn->setValue(kn->getValueFromText(s), juce::sendNotificationAsync);
                            }
                            if (def->type == "choice" && linkedComp) {
                                int idx = def->choices.indexOf(s);
                                if (idx >= 0) { if (auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp)) l->setSelectedIndex(idx, juce::sendNotificationAsync); }
                            }
                        }
                    ));

                    if (def->type == "float") {
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Minimum", def->type, def->choices,
                            [def]() { return def->min; },
                            [linkedComp, def]() { return linkedComp ? ([&]() { auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getTextFromValue(def->min) : ""; })() : ""; },
                            nullptr,
                            [def, updateControlsJson, linkedComp](juce::String s) {
                                if (linkedComp) {
                                    double v = 0.0; if (auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp)) v = kn->getValueFromText(s);
                                    def->min = v;
                                    updateControlsJson("min", v);
                                    if (auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp)) s->setRange(def->min, def->max, def->step);
                                }
                            }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Maximum", def->type, def->choices,
                            [def]() { return def->max; },
                            [linkedComp, def]() { return linkedComp ? ([&]() { auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getTextFromValue(def->max) : ""; })() : ""; },
                            nullptr,
                            [def, updateControlsJson, linkedComp](juce::String s) {
                                if (linkedComp) {
                                    double v = 0.0; if (auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp)) v = kn->getValueFromText(s);
                                    def->max = v;
                                    updateControlsJson("max", v);
                                    if (auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp)) s->setRange(def->min, def->max, def->step);
                                }
                            }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Default", def->type, def->choices,
                            [def]() { return def->defaultFloat; },
                            [linkedComp, def]() { return linkedComp ? ([&]() { auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getTextFromValue(def->defaultFloat) : ""; })() : ""; },
                            [def, updateControlsJson](double v) { def->defaultFloat = v; updateControlsJson("default", v); },
                            [def, updateControlsJson, linkedComp](juce::String s) {
                                if (linkedComp) {
                                    double v = 0.0; if (auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp)) v = kn->getValueFromText(s);
                                    def->defaultFloat = v;
                                    updateControlsJson("default", v);
                                }
                            }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Double-Click", def->type, def->choices,
                            [def]() { return def->doubleClickValue; },
                            [linkedComp, def]() { return linkedComp ? ([&]() { auto* s = dynamic_cast<RotaryKnobSlider*>(linkedComp); return s ? s->getTextFromValue(def->doubleClickValue) : ""; })() : ""; },
                            [def, updateControlsJson](double v) { def->doubleClickValue = v; updateControlsJson("double_click", v); },
                            [def, updateControlsJson, linkedComp](juce::String s) {
                                if (linkedComp) {
                                    double v = 0.0; if (auto* kn = dynamic_cast<RotaryKnobSlider*>(linkedComp)) v = kn->getValueFromText(s);
                                    def->doubleClickValue = v;
                                    updateControlsJson("double_click", v);
                                }
                            }
                        ));
                    } else if (def->type == "choice") {
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Default", def->type, def->choices,
                            [def]() { return def->defaultChoice; },
                            [def]() { return (def->defaultChoice >= 0 && def->defaultChoice < def->choices.size()) ? def->choices[def->defaultChoice] : ""; },
                            [def, updateControlsJson](double v) { def->defaultChoice = (int)v; updateControlsJson("default", (int)v); },
                            [def, updateControlsJson](juce::String s) {
                                int idx = def->choices.indexOf(s);
                                if (idx >= 0) { def->defaultChoice = idx; updateControlsJson("default", idx); }
                            }
                        ));
                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Choices", def->type, def->choices,
                            [def]() { return def->choices.size(); },
                            [def]() { return def->choices.joinIntoString(", "); },
                            nullptr,
                            [def, updateChoicesJson, linkedComp](juce::String s) {
                                juce::StringArray arr;
                                arr.addTokens(s, ",", "\"");
                                for (auto& token : arr) token = token.trim();
                                def->choices = arr;
                                updateChoicesJson(arr);
                                if (linkedComp) {
                                    auto* l = dynamic_cast<LedSelectorComponent*>(linkedComp);
                                    if (l) l->setItems(arr);
                                }
                            }
                        ));
                    }                        pProps.add(new ParamRowPropertyComponent(linkedComp, "Tooltip", "string", juce::StringArray(),
                            []() { return 0.0; },
                            [def]() { return def->description; },
                            nullptr,
                            [def, updateControlsJson](juce::String s) {
                                def->description = s;
                                updateControlsJson("description", s);
                            }
                        ));
                        
                        auto refreshPanel = [this]() {
                            syncJsonToPreview("");
                        };
                        
                        for (int i = 0; i < def->snapPoints.size(); ++i) {
                            pProps.add(new PoiRowPropertyComponent(def, i, updateControlsJson, refreshPanel));
                        }
                        pProps.add(new PoiAddPropertyComponent(def, updateControlsJson, refreshPanel));
                        
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
    exportSnapshotButton.setBounds(row1.removeFromRight(120).reduced(4, 0));
    importSnapshotButton.setBounds(row1.removeFromRight(120).reduced(4, 0));
    restoreFactoryButton.setBounds(row1.removeFromRight(120).reduced(4, 0));
    
    filePathDisplay.setBounds(topBar.reduced(4, 4));

        juce::Component* rightComps[] = { &formEditor, &splitterBar1, &previewWrapper, &splitterBar2, &jsonContainer };
    
    auto treeBounds = bounds.removeFromLeft(200);
    auto treeToolbar = treeBounds.removeFromTop(24);
    expandAllButton.setBounds(treeToolbar.removeFromLeft(treeToolbar.getWidth() / 2).reduced(2));
    collapseAllButton.setBounds(treeToolbar.reduced(2));
    
    auto whereUsedArea = treeBounds.removeFromBottom(180);
    auto whereUsedHeader = whereUsedArea.removeFromTop(20);
    whereUsedLabel.setBounds(whereUsedHeader);
    whereUsedListBox.setBounds(whereUsedArea);

    treeSplitter.setBounds(bounds.removeFromLeft(8));
    navigationTabs.setBounds(treeBounds);
    
    verticalLayout.layOutComponents(rightComps, 5, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), false, true);
    
    juce::Component* jsonComps[] = { layoutJsonEditor.get(), jsonSplitterBar.get(), controlsJsonEditor.get() };
    jsonSplitterLayout.layOutComponents(jsonComps, 3, 0, 0, jsonContainer.getWidth(), jsonContainer.getHeight(), true, true);
    
    emptyPlaceholder.setBounds(previewWrapper.getBounds());

    if (currentProductId == "callouts" && previewWrapper.getNumChildComponents() > 0) {
        if (auto* c = previewWrapper.getChildComponent(0)) {
            int cx = std::max(10, (previewWrapper.getWidth() - c->getWidth()) / 2);
            int cy = std::max(10, (previewWrapper.getHeight() - c->getHeight()) / 2);
            c->setTopLeftPosition(cx, cy);
        }
    }
}
































































































void MainComponent::NavTabbedComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) {
    juce::TabbedComponent::currentTabChanged(newCurrentTabIndex, newCurrentTabName);
    if (mc) mc->onTabChanged();
}

void MainComponent::onTabChanged() {
    auto* tree = navigationTabs.getCurrentTabIndex() == 0 ? &controlsTree : &layoutsTree;
    auto* selected = tree->getSelectedItem(0);
    if (selected) {
        onTreeItemSelected(static_cast<EditorTreeItem*>(selected));
    } else {
        formEditor.clear();
        previewWrapper.deleteAllChildren();
    }
}

void MainComponent::updateWhereUsed(EditorTreeItem* item) {
    whereUsedModel.items.clear();
    if (item == nullptr) {
        whereUsedListBox.updateContent();
        whereUsedListBox.repaint();
        return;
    }

    juce::String key;
    if (item->itemType == "card_param" && item->paramId.isNotEmpty()) {
        key = item->paramId;
    } else if (item->itemType == "callout_preview") {
        key = item->cardId;
    } else if (item->itemType == "card" || item->itemType == "card_theme") {
        key = item->cardId;
    } else if (item->itemType == "control_file") {
        key = item->pageId;
    } else if (item->itemType == "layout_file") {
        key = item->pageId;
    }

    if (key.isNotEmpty() && referencesMap.count(key) > 0) {
        whereUsedModel.items = referencesMap[key];
    }

    whereUsedListBox.updateContent();
    whereUsedListBox.repaint();
}

bool MainComponent::navigateToTreeItem(const ReferenceItem& ref) {
    auto searchInTree = [&](juce::TreeView& tree) -> EditorTreeItem* {
        std::function<EditorTreeItem*(juce::TreeViewItem*)> searchRec = [&](juce::TreeViewItem* cur) -> EditorTreeItem* {
            if (!cur) return nullptr;
            auto* eti = dynamic_cast<EditorTreeItem*>(cur);
            if (eti) {
                if (ref.targetType == "card_param") {
                    if (eti->itemType == "card_param" && eti->paramId == ref.paramId) {
                        if (ref.cardId.isEmpty() || eti->cardId == ref.cardId) {
                            if (ref.productId.isEmpty() || eti->productId == ref.productId) {
                                return eti;
                            }
                        }
                    }
                } else if (ref.targetType == "callout_preview") {
                    if (eti->itemType == "callout_preview" && eti->cardId == ref.cardId) {
                        return eti;
                    }
                } else if (ref.targetType == "card" || ref.targetType == "card_theme") {
                    if ((eti->itemType == "card" || eti->itemType == "card_theme") && eti->cardId == ref.cardId) {
                        if (ref.productId.isEmpty() || eti->productId == ref.productId) {
                            return eti;
                        }
                    }
                } else if (ref.targetType == "control_file") {
                    if (eti->itemType == "control_file" && (eti->pageId == ref.pageId || eti->name == ref.pageId || eti->name == ref.pageId + ".json")) {
                        return eti;
                    }
                } else if (ref.targetType == "product") {
                    if (eti->itemType == "product" && eti->productId == ref.productId) {
                        return eti;
                    }
                }
            }
            for (int i = 0; i < cur->getNumSubItems(); ++i) {
                if (auto* found = searchRec(cur->getSubItem(i))) return found;
            }
            return nullptr;
        };
        return searchRec(tree.getRootItem());
    };

    juce::TreeView* primaryTree = (navigationTabs.getCurrentTabIndex() == 0) ? &controlsTree : &layoutsTree;
    juce::TreeView* secondaryTree = (navigationTabs.getCurrentTabIndex() == 0) ? &layoutsTree : &controlsTree;
    int secondaryTab = (navigationTabs.getCurrentTabIndex() == 0) ? 1 : 0;

    EditorTreeItem* target = searchInTree(*primaryTree);
    if (!target) {
        target = searchInTree(*secondaryTree);
        if (target) {
            navigationTabs.setCurrentTabIndex(secondaryTab);
        }
    }

    if (target) {
        auto* p = target->getParentItem();
        while (p != nullptr) {
            p->setOpen(true);
            p = p->getParentItem();
        }
        target->setSelected(true, true);
        if (navigationTabs.getCurrentTabIndex() == 0) {
            controlsTree.scrollToKeepItemVisible(target);
        } else {
            layoutsTree.scrollToKeepItemVisible(target);
        }
        onTreeItemSelected(target);
        return true;
    }
    return false;
}

void MainComponent::buildReferencesIndex() {
    referencesMap.clear();

    const char* products[] = { "The Klang Farmer", "The Klang Mill", "The Klang Planter", "The Klang Seed" };
    const char* productIds[] = { "tkf", "tkm", "tkp", "tks" };

    for (int p = 0; p < 4; ++p) {
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
                            juce::String cardName = prop.name.toString();
                            auto params = pObj->getProperty("parameters");
                            if (params.isArray()) {
                                for (auto& param : *params.getArray()) {
                                    juce::String pId = param.toString();
                                    ReferenceItem refCard;
                                    refCard.label = "Card: " + cardName + " (" + juce::String(productIds[p]).toUpperCase() + ")";
                                    refCard.targetType = "card";
                                    refCard.productId = productIds[p];
                                    refCard.cardId = cardName;
                                    referencesMap[pId].add(refCard);

                                    ReferenceItem refParam;
                                    refParam.label = "Param: " + pId;
                                    refParam.targetType = "card_param";
                                    refParam.productId = productIds[p];
                                    refParam.cardId = cardName;
                                    refParam.paramId = pId;
                                    referencesMap[cardName].add(refParam);
                                }
                            }
                        } else {
                            juce::String pageName = prop.name.toString();
                            for (auto& cardProp : pObj->getProperties()) {
                                juce::String cardName = cardProp.name.toString();
                                if (cardProp.value.isObject()) {
                                    auto params = cardProp.value.getDynamicObject()->getProperty("parameters");
                                    if (params.isArray()) {
                                        for (auto& param : *params.getArray()) {
                                            juce::String pId = param.toString();
                                            ReferenceItem refCard;
                                            refCard.label = "Card: " + cardName + " (" + juce::String(productIds[p]).toUpperCase() + " -> " + pageName + ")";
                                            refCard.targetType = "card";
                                            refCard.productId = productIds[p];
                                            refCard.pageId = pageName;
                                            refCard.cardId = cardName;
                                            referencesMap[pId].add(refCard);

                                            ReferenceItem refParam;
                                            refParam.label = "Param: " + pId;
                                            refParam.targetType = "card_param";
                                            refParam.productId = productIds[p];
                                            refParam.pageId = pageName;
                                            refParam.cardId = cardName;
                                            refParam.paramId = pId;
                                            referencesMap[cardName].add(refParam);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    auto calloutsFile = getAssetFile("themes", "callouts.json");
    if (calloutsFile.existsAsFile()) {
        auto parsedGlobal = juce::JSON::parse(calloutsFile.loadFileAsString());
        if (parsedGlobal.isObject() && parsedGlobal.getDynamicObject()->hasProperty("callout_styles")) {
            auto* cStyles = parsedGlobal.getDynamicObject()->getProperty("callout_styles").getDynamicObject();
            if (cStyles) {
                for (auto& prop : cStyles->getProperties()) {
                    if (prop.value.isObject()) {
                        juce::String calloutKey = prop.name.toString();
                        auto* cDef = prop.value.getDynamicObject();
                        juce::String title = cDef->getProperty("title").toString();
                        if (title.isEmpty()) title = calloutKey;
                        juce::String prodId = cDef->getProperty("product").toString();

                        ReferenceItem refProd;
                        refProd.label = "Product: " + prodId.toUpperCase();
                        refProd.targetType = "product";
                        refProd.productId = prodId;
                        referencesMap[calloutKey].add(refProd);

                        auto params = cDef->getProperty("parameters");
                        if (params.isArray()) {
                            for (auto& paramVar : *params.getArray()) {
                                juce::String pId = paramVar.toString();

                                ReferenceItem refCallout;
                                refCallout.label = "Callout: " + title + " (" + prodId.toUpperCase() + ")";
                                refCallout.targetType = "callout_preview";
                                refCallout.productId = prodId;
                                refCallout.cardId = calloutKey;
                                referencesMap[pId].add(refCallout);

                                ReferenceItem refParam;
                                refParam.label = "Param: " + pId;
                                refParam.targetType = "card_param";
                                refParam.productId = prodId;
                                refParam.cardId = calloutKey;
                                refParam.paramId = pId;
                                referencesMap[calloutKey].add(refParam);
                            }
                        }
                    }
                }
            }
        }
    }

    juce::DirectoryIterator iterC(getAssetFile("controls", ""), false, "*.json");
    while (iterC.next()) {
        auto f = iterC.getFile();
        auto parsedC = juce::JSON::parse(f.loadFileAsString());
        if (parsedC.isObject()) {
            for (auto& prop : parsedC.getDynamicObject()->getProperties()) {
                if (prop.value.isObject()) {
                    juce::String pId = prop.name.toString();
                    ReferenceItem refFile;
                    refFile.label = "File: " + f.getFileName();
                    refFile.targetType = "control_file";
                    refFile.productId = "all_controls";
                    refFile.pageId = f.getFileNameWithoutExtension();
                    refFile.paramId = pId;
                    referencesMap[pId].add(refFile);
                }
            }
        }
    }
}










