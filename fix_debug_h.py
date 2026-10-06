with open('source/UIComponents.h', 'r') as f:
    content = f.read()

content = content.replace('    juce::Label headerRgba, headerHsva, hexLabel;', '    juce::Label headerRgba, headerHsva, hexLabel, debugLabel;')

with open('source/UIComponents.h', 'w') as f:
    f.write(content)
