with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

start_str = '        if (parsed.isObject()) {'
end_str = '        if (!props.isEmpty()) {\n            formEditor.addSection("Card Layout", props);\n        }'

start_idx = content.find(start_str)
end_idx = content.find(end_str) + len(end_str)

print(content[start_idx:end_idx])
