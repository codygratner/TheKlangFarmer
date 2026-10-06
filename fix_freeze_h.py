with open('source/UIComponents.h', 'r') as f:
    content = f.read()

content = content.replace('    void sliderValueChanged(juce::Slider* slider) override;', '    void sliderValueChanged(juce::Slider* slider) override;\n    void sliderDragEnded(juce::Slider* slider) override;')

with open('source/UIComponents.h', 'w') as f:
    f.write(content)
