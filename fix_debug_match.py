with open('source/UIComponents.cpp', 'r') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if 'currentAlpha = aSlider2.getValue()' in line:
        # Check if the next few lines have the bad debugLabel.setText
        for j in range(i, min(i+5, len(lines))):
            if 'debugLabel.setText' in lines[j] and 'dx' in lines[j]:
                lines[j] = ''
                
with open('source/UIComponents.cpp', 'w') as f:
    f.writelines(lines)
