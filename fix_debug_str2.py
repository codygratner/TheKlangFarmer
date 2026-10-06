with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

content = content.replace('debugLabel.setText(juce::String("Drag: ") + juce::String(dx) + ", " + juce::String(dy), juce::dontSendNotification);', 'debugLabel.setText(juce::String("Drag: ") + juce::String((double)dx) + ", " + juce::String((double)dy), juce::dontSendNotification);')

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
