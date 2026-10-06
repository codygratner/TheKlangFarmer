code = '''
class ThemeColorPropertyComponent : public juce::PropertyComponent {
public:
    ThemeColorPropertyComponent(const juce::String& name, const juce::String& hexStr, std::function<void(const juce::String&)> onHexChanged)
        : juce::PropertyComponent(name), currentHex(hexStr), onChange(onHexChanged) {
        
        colorButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString(hexStr));
        colorButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromString(hexStr));
        colorButton.onClick = [this]() {
            auto* picker = new AdvancedColorPickerComponent(juce::Colour::fromString(currentHex), [this](juce::Colour newColor) {
                currentHex = newColor.toDisplayString(true).toUpperCase();
                colorButton.setColour(juce::TextButton::buttonColourId, newColor);
                colorButton.setColour(juce::TextButton::buttonOnColourId, newColor);
                if (onChange) onChange(currentHex);
            });
            juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(picker), colorButton.getScreenBounds(), nullptr);
        };
        addAndMakeVisible(colorButton);
    }
    
    void refresh() override {}
    
private:
    juce::TextButton colorButton { "" };
    juce::String currentHex;
    std::function<void(const juce::String&)> onChange;
};
'''
with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

insert_idx = content.find('class ParamRowPropertyComponent')
new_content = content[:insert_idx] + code + '\n' + content[insert_idx:]

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(new_content)
