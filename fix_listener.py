with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

content = content.replace('val.addListener(this); // Just in case, standard JUCE listener not fully wired here for custom edit', '')

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
