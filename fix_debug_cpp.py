with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

# Add debugLabel to constructor
cons_old = '''    addAndMakeVisible(resetButton);'''
cons_new = '''    addAndMakeVisible(resetButton);
    addAndMakeVisible(debugLabel);
    debugLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    debugLabel.setText("Debug", juce::dontSendNotification);'''
content = content.replace(cons_old, cons_new)

# Add bounds to resized
res_old = '''    resetButton.setBounds(sliderX + 120, y, 60, 24);'''
res_new = '''    resetButton.setBounds(sliderX + 120, y, 60, 24);
    debugLabel.setBounds(10, 10, 200, 20);'''
content = content.replace(res_old, res_new)

# Update text in mouseDrag
drag_old = '''        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
    }
}'''
drag_new = '''        updateFromHSVA(false, true, true); // Do NOT notify on every pixel
        debugLabel.setText("Drag: " + juce::String(dx) + ", " + juce::String(dy), juce::dontSendNotification);
    }
}'''
content = content.replace(drag_old, drag_new)

# Update text in mouseDown
down_old = '''void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
    mouseDrag(e);
}'''
down_new = '''void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
    debugLabel.setText("Down", juce::dontSendNotification);
    mouseDrag(e);
}'''
content = content.replace(down_old, down_new)

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
