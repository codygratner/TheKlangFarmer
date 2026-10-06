code = '''
class ThemeColorPropertyComponent : public juce::PropertyComponent {
public:
    ThemeColorPropertyComponent(const juce::String& name, const juce::String& hexStr, std::function<void(const juce::String&)> onHexChanged)
        : juce::PropertyComponent(name), currentHex(hexStr), onChange(onHexChanged) {
        
        setPreferredHeight(30);
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
    
    void resized() override {
        auto b = getLocalBounds();
        b.removeFromLeft(b.getWidth() / 3);
        colorButton.setBounds(b.reduced(4));
    }
    
private:
    juce::TextButton colorButton { "" };
    juce::String currentHex;
    std::function<void(const juce::String&)> onChange;
};
'''
with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

start_idx = content.find('class ThemeColorPropertyComponent')
end_idx = content.find('};', start_idx) + 2

new_content = content[:start_idx] + code + content[end_idx:]

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(new_content)
