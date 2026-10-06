with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

old_func = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateSliders, bool updateHex) {
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    if (updateSliders) updateSlidersFromColor();
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}'''

new_func = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateSliders, bool updateHex) {
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    if (updateSliders) updateSlidersFromColor();
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(true, true, true);
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        currentHue = hSlider.getValue() / 360.0f;
        currentSat = sSlider.getValue() / 100.0f;
        currentVal = vSlider.getValue() / 100.0f;
        currentAlpha = aSlider2.getValue() / 100.0f;
        updateFromHSVA(true, false, true);
    }
}'''

content = content.replace(old_func, new_func)

# Remove the old sliderValueChanged
old_slider = '''void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(true, true, true);
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        updateFromHSVA(true, true, true);
    }
}'''
content = content.replace(old_slider, '')

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
