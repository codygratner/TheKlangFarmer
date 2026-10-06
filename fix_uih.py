with open('source/UIComponents.h', 'r') as f:
    lines = f.read().splitlines()

insert_idx = 0
for i, line in enumerate(lines):
    if line.startswith('class RotaryKnobLookAndFeel'):
        insert_idx = i
        break

code = '''
class AdvancedColorPickerComponent : public juce::Component, public juce::Slider::Listener, public juce::TextEditor::Listener {
public:
    AdvancedColorPickerComponent(juce::Colour initialColor, std::function<void(juce::Colour)> onColorChangedFunc);
    ~AdvancedColorPickerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    void sliderValueChanged(juce::Slider* slider) override;
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

private:
    juce::Colour originalColor;
    juce::Colour currentColor;
    float currentHue, currentSat, currentVal, currentAlpha;
    bool isUpdating = false;

    std::function<void(juce::Colour)> onColorChanged;

    void updateFromColor(juce::Colour newColor, bool notify, bool updateSliders, bool updateHex);
    void updateFromHSVA(bool notify, bool updateRGB, bool updateHex);
    void updateFromRGBA(bool notify, bool updateHSV, bool updateHex);
    void updateHexFromColor();
    void updateSlidersFromColor();
    
    void loadPreferences();
    void savePreferences();

    juce::Rectangle<float> getRingBounds() const;
    juce::Rectangle<float> getInnerSquareBounds() const;

    juce::Slider rSlider, gSlider, bSlider, aSlider1;
    juce::Slider hSlider, sSlider, vSlider, aSlider2;
    juce::Label rLabel, gLabel, bLabel, aLabel1;
    juce::Label hLabel, sLabel, vLabel, aLabel2;
    juce::Label headerRgba, headerHsva, hexLabel;
    juce::TextEditor hexInput;
    juce::TextButton resetButton { "Reset" };

    class PaletteSwatch;
    std::array<juce::Colour, 16> customColors;
    juce::OwnedArray<PaletteSwatch> swatches;
};
'''
lines.insert(insert_idx, code)

with open('source/UIComponents.h', 'w') as f:
    f.write('\n'.join(lines))
