with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

content = content.replace('juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(picker), colorButton.getScreenBounds(), nullptr);', 'juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(picker), colorButton.getScreenBounds(), this->getTopLevelComponent());')

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
