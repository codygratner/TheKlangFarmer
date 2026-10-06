import re

with open('source/UIComponents.h', 'r') as f:
    content = f.read()

# I will just revert everything and do it safely
content = content.replace('    void mouseDrag(const juce::MouseEvent& e) override;\n    void mouseUp(const juce::MouseEvent& e) override;', '    void mouseDrag(const juce::MouseEvent& e) override;')

# Now find AdvancedColorPickerComponent block
start_idx = content.find('class AdvancedColorPickerComponent')
end_idx = content.find('};', start_idx)

block = content[start_idx:end_idx]
block = block.replace('    void mouseDrag(const juce::MouseEvent& e) override;', '    void mouseDrag(const juce::MouseEvent& e) override;\n    void mouseUp(const juce::MouseEvent& e) override;')

content = content[:start_idx] + block + content[end_idx:]

with open('source/UIComponents.h', 'w') as f:
    f.write(content)
