with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

old_mouse = '''void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
    mouseDrag(e);
}

void AdvancedColorPickerComponent::mouseDrag(const juce::MouseEvent& e) {
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float distance = std::sqrt(dx*dx + dy*dy);
    
    if (distance >= 60.0f && distance <= 140.0f) {
        float angle = std::atan2(dx, -dy);
        if (angle < 0) angle += juce::MathConstants<float>::twoPi;
        currentHue = angle / juce::MathConstants<float>::twoPi;
        updateFromHSVA(true, true, true);
    }
}'''

new_mouse = '''void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float distance = std::sqrt(dx*dx + dy*dy);
    if (distance >= 60.0f && distance <= 140.0f) {
        isDraggingRing = true;
        mouseDrag(e);
    } else {
        isDraggingRing = false;
    }
}

void AdvancedColorPickerComponent::mouseDrag(const juce::MouseEvent& e) {
    if (!isDraggingRing) return;
    auto b = getRingBounds();
    float dx = e.x - b.getCentreX();
    float dy = e.y - b.getCentreY();
    float angle = std::atan2(dx, -dy);
    if (angle < 0) angle += juce::MathConstants<float>::twoPi;
    currentHue = angle / juce::MathConstants<float>::twoPi;
    updateFromHSVA(true, true, true);
}

void AdvancedColorPickerComponent::mouseUp(const juce::MouseEvent& e) {
    isDraggingRing = false;
}'''

content = content.replace(old_mouse, new_mouse)

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
