with open('tools/editor/MainComponent.cpp', 'r') as f:
    lines = f.read().splitlines()
for i, line in enumerate(lines):
    if 'void MainComponent::syncJsonToPreview' in line:
        for j in range(i, i+60):
            print(lines[j])
        break
