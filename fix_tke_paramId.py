with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'slider->setAccentColour(knobColour);',
    'slider->setAccentColour(knobColour);\n                            slider->setParamId(paramId);'
)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
