with open('source/UIComponents.h', 'r') as f:
    content = f.read()

content = content.replace('    void mouseDrag(const juce::MouseEvent& e) override;', '    void mouseDrag(const juce::MouseEvent& e) override;\n    void mouseUp(const juce::MouseEvent& e) override;')
content = content.replace('    bool isUpdating = false;', '    bool isUpdating = false;\n    bool isDraggingRing = false;')

with open('source/UIComponents.h', 'w') as f:
    f.write(content)
