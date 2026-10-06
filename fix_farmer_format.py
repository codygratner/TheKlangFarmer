with open('source/FarmerEditor.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;',
    'carrier1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n    carrier1ShapeSlider.customFormatText = formatWaveshape;\n    carrier1ShapeSlider.customParseText = parseWaveshape;'
)

content = content.replace(
    'carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;',
    'carrier2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n    carrier2ShapeSlider.customFormatText = formatWaveshape;\n    carrier2ShapeSlider.customParseText = parseWaveshape;'
)

content = content.replace(
    'mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n            mod1ShapeSlider.setBipolar(false);',
    'mod1ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n            mod1ShapeSlider.customFormatText = formatWaveshape;\n            mod1ShapeSlider.customParseText = parseWaveshape;\n            mod1ShapeSlider.setBipolar(false);'
)

content = content.replace(
    'mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n            mod2ShapeSlider.setBipolar(false);',
    'mod2ShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n            mod2ShapeSlider.customFormatText = formatWaveshape;\n            mod2ShapeSlider.customParseText = parseWaveshape;\n            mod2ShapeSlider.setBipolar(false);'
)

with open('source/FarmerEditor.cpp', 'w') as f:
    f.write(content)
