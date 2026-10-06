with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

content = content.replace('rSlider.setValue(currentColor.getRed());', 'rSlider.setValue(currentColor.getRed(), juce::dontSendNotification);')
content = content.replace('gSlider.setValue(currentColor.getGreen());', 'gSlider.setValue(currentColor.getGreen(), juce::dontSendNotification);')
content = content.replace('bSlider.setValue(currentColor.getBlue());', 'bSlider.setValue(currentColor.getBlue(), juce::dontSendNotification);')
content = content.replace('aSlider1.setValue(currentColor.getAlpha());', 'aSlider1.setValue(currentColor.getAlpha(), juce::dontSendNotification);')

content = content.replace('hSlider.setValue(currentHue * 360.0);', 'hSlider.setValue(currentHue * 360.0, juce::dontSendNotification);')
content = content.replace('sSlider.setValue(currentSat * 100.0);', 'sSlider.setValue(currentSat * 100.0, juce::dontSendNotification);')
content = content.replace('vSlider.setValue(currentVal * 100.0);', 'vSlider.setValue(currentVal * 100.0, juce::dontSendNotification);')
content = content.replace('aSlider2.setValue(currentAlpha * 100.0);', 'aSlider2.setValue(currentAlpha * 100.0, juce::dontSendNotification);')

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
