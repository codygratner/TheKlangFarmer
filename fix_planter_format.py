with open('source/PlanterEditor.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'carrierShapeSlider.customFormatText = formatPercent;\n    carrierShapeSlider.customParseText  = parsePercent;',
    'carrierShapeSlider.customFormatText = formatWaveshape;\n    carrierShapeSlider.customParseText  = parseWaveshape;'
)

content = content.replace(
    'modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n        modShapeSlider.setBipolar(false);',
    'modShapeSlider.diagramType = RotaryKnobSlider::DiagramType::Waveform;\n        modShapeSlider.customFormatText = formatWaveshape;\n        modShapeSlider.customParseText = parseWaveshape;\n        modShapeSlider.setBipolar(false);'
)

with open('source/PlanterEditor.cpp', 'w') as f:
    f.write(content)
