with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'compToParamId[slider] = paramId;',
    'compToParamId[slider] = paramId;\n                            slider->setParamId(paramId);'
)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
