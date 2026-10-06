with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

content = content.replace('juce::String newHex = newColor.toDisplayString(true).toUpperCase();', 'juce::String newHex = \"0x\" + newColor.toDisplayString(true).toUpperCase();')

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
