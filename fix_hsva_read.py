with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

old_func = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
    currentHue = hSlider.getValue() / 360.0;
    currentSat = sSlider.getValue() / 100.0;
    currentVal = vSlider.getValue() / 100.0;
    currentAlpha = aSlider2.getValue() / 100.0;
    
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    
    isUpdating = true;
    if (updateRGB) {
        rSlider.setValue(currentColor.getRed(), juce::dontSendNotification);
        gSlider.setValue(currentColor.getGreen(), juce::dontSendNotification);
        bSlider.setValue(currentColor.getBlue(), juce::dontSendNotification);
        aSlider1.setValue(currentColor.getAlpha(), juce::dontSendNotification);
    }
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}'''

new_func = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    
    isUpdating = true;
    if (updateRGB) {
        rSlider.setValue(currentColor.getRed(), juce::dontSendNotification);
        gSlider.setValue(currentColor.getGreen(), juce::dontSendNotification);
        bSlider.setValue(currentColor.getBlue(), juce::dontSendNotification);
        aSlider1.setValue(currentColor.getAlpha(), juce::dontSendNotification);
    }
    hSlider.setValue(currentHue * 360.0, juce::dontSendNotification);
    sSlider.setValue(currentSat * 100.0, juce::dontSendNotification);
    vSlider.setValue(currentVal * 100.0, juce::dontSendNotification);
    aSlider2.setValue(currentAlpha * 100.0, juce::dontSendNotification);
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}'''

content = content.replace(old_func, new_func)

# We need to make sure sliderValueChanged exists and is correct.
# Wait, let's see what sliderValueChanged currently looks like in UIComponents.cpp!
