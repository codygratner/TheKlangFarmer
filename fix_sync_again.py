with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

# I will find my injected block
start_str = '    formEditor.clear();\n\n    if (isTheme && parsed.isObject()) {'
end_str = '        if (!props.isEmpty()) {\n            formEditor.addSection("Theme Colors", props);\n        }\n    }'

start_idx = content.find(start_str)
end_idx = content.find(end_str) + len(end_str)

block = content[start_idx:end_idx]

# Remove it
content = content[:start_idx] + '    formEditor.clear();' + content[end_idx:]

# Find where to put it:
insert_target = '    previewWrapper.deleteAllChildren();\n    activeSliders.clear();\n    compToParamId.clear();'
insert_idx = content.find(insert_target) + len(insert_target)

content = content[:insert_idx] + '\n\n    formEditor.clear();\n\n' + block.replace('    formEditor.clear();\n\n', '') + content[insert_idx:]

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
