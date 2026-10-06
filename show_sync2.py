with open('tools/editor/MainComponent.cpp', 'r') as f:
    lines = f.read().splitlines()

start = -1
for i, line in enumerate(lines):
    if 'void MainComponent::syncJsonToPreview' in line:
        start = i
        break

if start != -1:
    for i in range(start, start+45):
        print(lines[i])
