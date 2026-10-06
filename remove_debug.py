with open('source/UIComponents.h', 'r') as f:
    content = f.read()
content = content.replace('    juce::Label headerRgba, headerHsva, hexLabel, debugLabel;', '    juce::Label headerRgba, headerHsva, hexLabel;')
with open('source/UIComponents.h', 'w') as f:
    f.write(content)

with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

content = content.replace('    addAndMakeVisible(debugLabel);\n    debugLabel.setColour(juce::Label::textColourId, juce::Colours::white);\n    debugLabel.setText("Debug", juce::dontSendNotification);', '')
content = content.replace('    debugLabel.setBounds(10, 10, 200, 20);', '')
content = content.replace('        debugLabel.setText(juce::String("Drag: ") + juce::String((double)dx) + ", " + juce::String((double)dy), juce::dontSendNotification);\n', '')
content = content.replace('    debugLabel.setText("Down", juce::dontSendNotification);\n', '')

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
