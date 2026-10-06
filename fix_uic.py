code = '''
// ==============================================================================
// AdvancedColorPickerComponent
// ==============================================================================

class AdvancedColorPickerComponent::PaletteSwatch : public juce::Component {
public:
    juce::Colour color;
    std::function<void(juce::Colour)> onSelect;
    std::function<void(PaletteSwatch*)> onSave;
    
    void paint(juce::Graphics& g) override {
        g.fillAll(color);
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(getLocalBounds(), 1);
    }
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown()) {
            if (onSave) onSave(this);
        } else {
            if (onSelect) onSelect(color);
        }
    }
};

AdvancedColorPickerComponent::AdvancedColorPickerComponent(juce::Colour initialColor, std::function<void(juce::Colour)> onColorChangedFunc)
    : onColorChanged(onColorChangedFunc) {
    
    setSize(520, 360);
    
    auto setupSlider = [this](juce::Slider& s, juce::Label& l, const juce::String& text, double maxVal, const juce::String& suffix) {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        s.setRange(0.0, maxVal, 1.0);
        s.setTextValueSuffix(suffix);
        s.addListener(this);
        
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.attachToComponent(&s, true);
    };

    setupSlider(rSlider, rLabel, "R", 255.0, "");
    setupSlider(gSlider, gLabel, "G", 255.0, "");
    setupSlider(bSlider, bLabel, "B", 255.0, "");
    setupSlider(aSlider1, aLabel1, "A", 255.0, "");

    setupSlider(hSlider, hLabel, "H", 360.0, " deg");
    setupSlider(sSlider, sLabel, "S", 100.0, "%");
    setupSlider(vSlider, vLabel, "V", 100.0, "%");
    setupSlider(aSlider2, aLabel2, "A", 100.0, "%");
    
    addAndMakeVisible(headerRgba);
    headerRgba.setText("R G B A", juce::dontSendNotification);
    addAndMakeVisible(headerHsva);
    headerHsva.setText("H S V A", juce::dontSendNotification);

    addAndMakeVisible(hexInput);
    hexInput.addListener(this);
    addAndMakeVisible(hexLabel);
    hexLabel.setText("#", juce::dontSendNotification);
    hexLabel.attachToComponent(&hexInput, true);

    addAndMakeVisible(resetButton);
    resetButton.onClick = [this]() {
        updateFromColor(originalColor, true, true, true);
    };

    for (int i = 0; i < 16; ++i) {
        customColors[i] = juce::Colour(0xff303030);
    }
    loadPreferences();

    for (int i = 0; i < 16; ++i) {
        auto* swatch = new PaletteSwatch();
        swatch->color = customColors[i];
        swatch->onSelect = [this](juce::Colour c) { updateFromColor(c, true, true, true); };
        swatch->onSave = [this, i](PaletteSwatch* s) { 
            s->color = currentColor; 
            customColors[i] = currentColor;
            s->repaint();
            savePreferences();
        };
        swatches.add(swatch);
        addAndMakeVisible(swatch);
    }

    originalColor = initialColor;
    updateFromColor(initialColor, false, true, true);
}

AdvancedColorPickerComponent::~AdvancedColorPickerComponent() {}

void AdvancedColorPickerComponent::loadPreferences() {
    juce::File prefsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("RlyehSound").getChildFile("TheKlangFarmer").getChildFile("preferences.json");
    if (prefsFile.existsAsFile()) {
        auto parsed = juce::JSON::parse(prefsFile);
        if (parsed.isObject()) {
            auto arr = parsed.getProperty("custom_palette", juce::var());
            if (arr.isArray()) {
                auto* array = arr.getArray();
                for (int i = 0; i < juce::jmin(16, array->size()); ++i) {
                    customColors[i] = juce::Colour::fromString(array->getReference(i).toString());
                }
            }
        }
    }
}

void AdvancedColorPickerComponent::savePreferences() {
    juce::File prefsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("RlyehSound").getChildFile("TheKlangFarmer").getChildFile("preferences.json");
    prefsFile.getParentDirectory().createDirectory();
    
    juce::var prefsObj(new juce::DynamicObject());
    if (prefsFile.existsAsFile()) {
        auto existing = juce::JSON::parse(prefsFile);
        if (existing.isObject()) prefsObj = existing;
    }
    
    juce::Array<juce::var> arr;
    for (int i = 0; i < 16; ++i) {
        arr.add(customColors[i].toDisplayString(true));
    }
    prefsObj.getDynamicObject()->setProperty("custom_palette", arr);
    
    prefsFile.replaceWithText(juce::JSON::toString(prefsObj));
}

juce::Rectangle<float> AdvancedColorPickerComponent::getRingBounds() const {
    return juce::Rectangle<float>(20, 20, 240, 240);
}

juce::Rectangle<float> AdvancedColorPickerComponent::getInnerSquareBounds() const {
    auto b = getRingBounds();
    return juce::Rectangle<float>(b.getCentreX() - 55, b.getCentreY() - 55, 110, 110);
}

void AdvancedColorPickerComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff2b2d31));

    auto ringBounds = getRingBounds();
    float centerX = ringBounds.getCentreX();
    float centerY = ringBounds.getCentreY();
    float outerRadius = 120.0f;
    float innerRadius = 80.0f;

    for (float angle = 0.0f; angle < juce::MathConstants<float>::twoPi; angle += 0.02f) {
        float hue = angle / juce::MathConstants<float>::twoPi;
        g.setColour(juce::Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f));
        juce::Path p;
        p.addCentredArc(centerX, centerY, (outerRadius+innerRadius)*0.5f, (outerRadius+innerRadius)*0.5f, 
                        0.0f, angle, angle + 0.03f, true);
        g.strokePath(p, juce::PathStrokeType(outerRadius - innerRadius));
    }

    float cursorAngle = currentHue * juce::MathConstants<float>::twoPi;
    float cursorRadius = (outerRadius + innerRadius) * 0.5f;
    float cx = centerX + cursorRadius * std::sin(cursorAngle);
    float cy = centerY - cursorRadius * std::cos(cursorAngle);
    g.setColour(juce::Colours::white);
    g.drawEllipse(cx - 10, cy - 10, 20, 20, 2.5f);

    auto innerSquare = getInnerSquareBounds();
    g.setColour(currentColor);
    g.fillRoundedRectangle(innerSquare, 10.0f);

    g.setColour(currentColor.contrasting());
    g.setFont(16.0f);
    g.drawText(currentColor.toDisplayString(true).toUpperCase(), 
               innerSquare.withTrimmedTop(80).toNearestInt(), juce::Justification::centred, false);
}

void AdvancedColorPickerComponent::resized() {
    int sliderX = 320;
    int sliderW = 180;
    int y = 20;

    headerRgba.setBounds(sliderX, y, sliderW, 20); y += 24;
    rSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    gSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    bSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    aSlider1.setBounds(sliderX, y, sliderW, 20); y += 30;

    headerHsva.setBounds(sliderX, y, sliderW, 20); y += 24;
    hSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    sSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    vSlider.setBounds(sliderX, y, sliderW, 20); y += 24;
    aSlider2.setBounds(sliderX, y, sliderW, 20); y += 30;

    hexInput.setBounds(sliderX, y, 110, 24);
    resetButton.setBounds(sliderX + 120, y, 60, 24);
    
    // 2x8 palette grid
    int px = 20;
    int py = 290;
    int pw = 25;
    for (int i = 0; i < 16; ++i) {
        swatches[i]->setBounds(px + (i % 8) * (pw + 5), py + (i / 8) * (pw + 5), pw, pw);
    }
}

void AdvancedColorPickerComponent::mouseDown(const juce::MouseEvent& e) {
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
}

void AdvancedColorPickerComponent::sliderValueChanged(juce::Slider* slider) {
    if (isUpdating) return;
    
    if (slider == &rSlider || slider == &gSlider || slider == &bSlider || slider == &aSlider1) {
        updateFromRGBA(true, true, true);
    } else if (slider == &hSlider || slider == &sSlider || slider == &vSlider || slider == &aSlider2) {
        updateFromHSVA(true, true, true);
    }
}

void AdvancedColorPickerComponent::textEditorTextChanged(juce::TextEditor& editor) {
    if (isUpdating) return;
    if (&editor == &hexInput) {
        juce::String hex = hexInput.getText().toUpperCase();
        if (hex.startsWith("0X") && hex.length() == 10) {
            updateFromColor(juce::Colour::fromString(hex), true, true, false);
        }
    }
}

void AdvancedColorPickerComponent::textEditorReturnKeyPressed(juce::TextEditor& editor) {
    textEditorTextChanged(editor);
}

void AdvancedColorPickerComponent::updateFromColor(juce::Colour newColor, bool notify, bool updateSliders, bool updateHex) {
    currentColor = newColor;
    currentColor.getHSB(currentHue, currentSat, currentVal);
    currentAlpha = currentColor.getFloatAlpha();
    
    if (updateSliders) updateSlidersFromColor();
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateFromHSVA(bool notify, bool updateRGB, bool updateHex) {
    currentHue = hSlider.getValue() / 360.0;
    currentSat = sSlider.getValue() / 100.0;
    currentVal = vSlider.getValue() / 100.0;
    currentAlpha = aSlider2.getValue() / 100.0;
    
    currentColor = juce::Colour(currentHue, currentSat, currentVal, currentAlpha);
    
    isUpdating = true;
    if (updateRGB) {
        rSlider.setValue(currentColor.getRed());
        gSlider.setValue(currentColor.getGreen());
        bSlider.setValue(currentColor.getBlue());
        aSlider1.setValue(currentColor.getAlpha());
    }
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateFromRGBA(bool notify, bool updateHSV, bool updateHex) {
    currentColor = juce::Colour(
        (juce::uint8)rSlider.getValue(),
        (juce::uint8)gSlider.getValue(),
        (juce::uint8)bSlider.getValue(),
        (juce::uint8)aSlider1.getValue()
    );
    currentColor.getHSB(currentHue, currentSat, currentVal);
    currentAlpha = currentColor.getFloatAlpha();
    
    isUpdating = true;
    if (updateHSV) {
        hSlider.setValue(currentHue * 360.0);
        sSlider.setValue(currentSat * 100.0);
        vSlider.setValue(currentVal * 100.0);
        aSlider2.setValue(currentAlpha * 100.0);
    }
    isUpdating = false;
    
    if (updateHex) updateHexFromColor();
    repaint();
    if (notify && onColorChanged) onColorChanged(currentColor);
}

void AdvancedColorPickerComponent::updateHexFromColor() {
    isUpdating = true;
    hexInput.setText(currentColor.toDisplayString(true).toUpperCase(), juce::dontSendNotification);
    isUpdating = false;
}

void AdvancedColorPickerComponent::updateSlidersFromColor() {
    isUpdating = true;
    rSlider.setValue(currentColor.getRed());
    gSlider.setValue(currentColor.getGreen());
    bSlider.setValue(currentColor.getBlue());
    aSlider1.setValue(currentColor.getAlpha());
    
    hSlider.setValue(currentHue * 360.0);
    sSlider.setValue(currentSat * 100.0);
    vSlider.setValue(currentVal * 100.0);
    aSlider2.setValue(currentAlpha * 100.0);
    isUpdating = false;
}
'''
with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content + '\n' + code)
