code = '''
    if (isTheme && parsed.isObject()) {
        juce::Array<juce::PropertyComponent*> props;
        auto* obj = parsed.getDynamicObject();
        if (obj->hasProperty("colors")) {
            auto* colorsObj = obj->getProperty("colors").getDynamicObject();
            if (colorsObj) {
                for (auto& prop : colorsObj->getProperties()) {
                    if (prop.value.isString() && prop.value.toString().startsWithIgnoreCase("0x")) {
                        auto pc = new ThemeColorPropertyComponent(prop.name.toString(), prop.value.toString(), 
                            [this, propName = prop.name.toString()](const juce::String& newHex) {
                                auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                                if (parsedObj.isObject() && parsedObj.getDynamicObject()->hasProperty("colors")) {
                                    parsedObj.getDynamicObject()->getProperty("colors").getDynamicObject()->setProperty(propName, newHex);
                                    layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                                }
                            });
                        props.add(pc);
                    }
                }
            }
        }
        if (!props.isEmpty()) {
            formEditor.addSection("Theme Colors", props);
        }
    }
'''
with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

start_str = '    if (isTheme && parsed.isObject()) {'
end_str = '        if (!props.isEmpty()) {\n            formEditor.addSection("Theme Colors", props);\n        }\n    }'

start_idx = content.find(start_str)
end_idx = content.find(end_str) + len(end_str)

new_content = content[:start_idx] + code.strip() + content[end_idx:]

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(new_content)
