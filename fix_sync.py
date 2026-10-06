code = '''
    formEditor.clear();

    if (isTheme && parsed.isObject()) {
        juce::Array<juce::PropertyComponent*> props;
        auto* obj = parsed.getDynamicObject();
        for (auto& prop : obj->getProperties()) {
            if (prop.value.isString() && prop.value.toString().startsWithIgnoreCase("0x")) {
                auto pc = new ThemeColorPropertyComponent(prop.name.toString(), prop.value.toString(), 
                    [this, propName = prop.name.toString()](const juce::String& newHex) {
                        auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                        if (parsedObj.isObject()) {
                            parsedObj.getDynamicObject()->setProperty(propName, newHex);
                            layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                        }
                    });
                props.add(pc);
            }
        }
        if (!props.isEmpty()) {
            formEditor.addSection("Theme Colors", props);
        }
    }
'''
with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

start_idx = content.find('    formEditor.clear();')
end_idx = start_idx + len('    formEditor.clear();')

new_content = content[:start_idx] + code.strip() + content[end_idx:]

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(new_content)
