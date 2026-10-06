with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

# remove setSize from top
content = content.replace('    setSize(520, 360);\n    \n    auto setupSlider = [this]', '    auto setupSlider = [this]')

# add setSize to bottom of constructor
content = content.replace('    originalColor = initialColor;\n    updateFromColor(initialColor, false, true, true);\n}', '    originalColor = initialColor;\n    updateFromColor(initialColor, false, true, true);\n\n    setSize(520, 360);\n}')

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
