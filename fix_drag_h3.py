import re

with open('source/UIComponents.h', 'r') as f:
    content = f.read()

content = content.replace('    bool isUpdating = false;\n    bool isDraggingRing = false;', '    bool isUpdating = false;')

start_idx = content.find('class AdvancedColorPickerComponent')
end_idx = content.find('};', start_idx)

block = content[start_idx:end_idx]
block = block.replace('    bool isUpdating = false;', '    bool isUpdating = false;\n    bool isDraggingRing = false;')

content = content[:start_idx] + block + content[end_idx:]

with open('source/UIComponents.h', 'w') as f:
    f.write(content)
