with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

# Remove notify argument entirely from update functions to be safe, or just ignore it.
# Actually let's just leave the signature but not call onColorChanged from them if it's during drag.
# But updateFromHSVA is called from 	extEditorTextChanged where we DO want to notify.
# Let's change updateFromHSVA and updateFromRGBA to NOT notify by default, and only notify manually.

old_update_h = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
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

new_update_h = '''void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
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
content = content.replace(old_update_h, new_update_h) # no change needed if we just pass false


# Update mouseDrag to pass false for notify
old_mouse = '''void AdvancedColorPickerComponent::mouseDrag(const juce::MouseEvent& e) {
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float distance = std::sqrt(dx*dx + dy*dy);
    
    if (distance > 10.0f) {
        float angle = std::atan2(dx, -dy);
        if (angle < 0) angle += juce::MathConstants<float>::twoPi;
        currentHue = angle / juce::MathConstants<float>::twoPi;
        updateFromHSVA(true, true, true);
    }
}

void AdvancedColorPickerComponent::mouseUp(const juce::MouseEvent& e) {
}'''

new_mouse = '''void AdvancedColorPickerComponent::mouseDrag(const juce::MouseEvent& e) {
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float distance = std::sqrt(dx*dx + dy*dy);
    
    if (distance > 10.0f) {
        float angle = std::atan2(dx, -dy);
        if (angle < 0) angle += juce::MathConstants<float>::twoPi;
        currentHue = angle / juce::MathConstants<float>::twoPi;
        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
    }
}

void AdvancedColorPickerComponent::mouseUp(const juce::MouseEvent& e) {
    if (onColorChanged) onColorChanged(currentColor); // Notify only when released
}'''

content = content.replace(old_mouse, new_mouse)


# Update sliderValueChanged to pass false for notify
old_slider = '''void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
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
}'''

new_slider = '''void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(false, true, true); // Do NOT notify on every pixel
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        currentHue = hSlider.getValue() / 360.0f;
        currentSat = sSlider.getValue() / 100.0f;
        currentVal = vSlider.getValue() / 100.0f;
        currentAlpha = aSlider2.getValue() / 100.0f;
        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
    }
}

void AdvancedColorPickerComponent::sliderDragEnded(juce::Slider* slider) {
    if (onColorChanged) onColorChanged(currentColor); // Notify only when released
}'''

content = content.replace(old_slider, new_slider)


with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
