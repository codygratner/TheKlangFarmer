with open('tools/editor/MainComponent.cpp', 'r') as f:
    lines = f.read().splitlines()

start = -1
for i, line in enumerate(lines):
    if 'if (isTheme && parsed.isObject()) {' in line:
        start = i
        break

if start != -1:
    for i in range(start, start+35):
        print(lines[i])
