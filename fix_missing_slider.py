with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

slider_func = '''
void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(true, true, true);
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        currentHue = hSlider.getValue() / 360.0f;
        currentSat = sSlider.getValue() / 100.0f;
        currentVal = vSlider.getValue() / 100.0f;
        currentAlpha = aSlider2.getValue() / 100.0f;
        updateFromHSVA(true, true, true);
    }
}
'''

content += slider_func

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
