import re
with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

code = '''class ThemeColorPropertyComponent : public juce::PropertyComponent {
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
            auto* picker = new AdvancedColorPickerComponent(juce::Colour::fromString(hex), [this](juce::Colour newColor) {
                currentVal = newColor.toDisplayString(true).toUpperCase();
                textEditor.setText(currentVal, juce::dontSendNotification);
                updateButtonColor();
                if (onChange) onChange(currentVal);
            });
            juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(picker), colorButton.getScreenBounds(), nullptr);
        };
    }
    
    juce::String resolveToHex(const juce::String& val) {
        if (val.startsWithIgnoreCase("0x")) return val;
        return RlyehSound::ParameterManager::getInstance().getThemeColour(val).toDisplayString(true);
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
};'''

content = re.sub(r'class ThemeColorPropertyComponent.*?};\s*class ParamRowPropertyComponent', code + '\n\nclass ParamRowPropertyComponent', content, flags=re.DOTALL)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
