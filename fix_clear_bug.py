with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

# Remove the second formEditor.clear();
content = content.replace('    bool showParams = (selType != "card_theme");\n\n    formEditor.clear();\n    juce::Array<juce::PropertyComponent*> props;', '    bool showParams = (selType != "card_theme");\n\n    juce::Array<juce::PropertyComponent*> props;')

# Remove the debug block
content = content.replace('        if (!props.isEmpty()) {\n            formEditor.addSection("Theme Colors", props);\n        } else {\n            // Debug if empty\n            auto pc = new juce::TextPropertyComponent(juce::Value("empty"), "DEBUG", 256, false);\n            props.add(pc);\n            formEditor.addSection("Theme Colors", props);\n        }', '        if (!props.isEmpty()) {\n            formEditor.addSection("Theme Colors", props);\n        }')

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
