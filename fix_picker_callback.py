with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

old_lambda = '''            auto* picker = new AdvancedColorPickerComponent(juce::Colour::fromString(hex), [safeThis](juce::Colour newColor) {
                if (safeThis != nullptr) {
                    safeThis->currentVal = newColor.toDisplayString(true).toUpperCase();
                    safeThis->textEditor.setText(safeThis->currentVal, juce::dontSendNotification);
                    safeThis->updateButtonColor();
                    if (safeThis->onChange) safeThis->onChange(safeThis->currentVal);
                }
            });'''

new_lambda = '''            std::function<void(const juce::String&)> onChangeCopy = onChange;
            auto* picker = new AdvancedColorPickerComponent(juce::Colour::fromString(hex), [safeThis, onChangeCopy](juce::Colour newColor) {
                juce::String newHex = newColor.toDisplayString(true).toUpperCase();
                if (safeThis != nullptr) {
                    safeThis->currentVal = newHex;
                    safeThis->textEditor.setText(newHex, juce::dontSendNotification);
                    safeThis->updateButtonColor();
                }
                if (onChangeCopy) onChangeCopy(newHex);
            });'''

content = content.replace(old_lambda, new_lambda)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
