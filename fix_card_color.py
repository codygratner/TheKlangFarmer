code = '''
            juce::PropertyComponent* pc = nullptr;
            juce::String pName = prop.name.toString();
            if (pName == "color" || pName == "accent" || pName == "background" || pName.startsWithIgnoreCase("col")) {
                pc = new ThemeColorPropertyComponent(pName, valStr, [this, pName](const juce::String& newHex) {
                    auto parsedObj = juce::JSON::parse(layoutJsonDocument.getAllContent());
                    if (parsedObj.isObject()) {
                        auto* root = parsedObj.getDynamicObject();
                        if (currentPageId.isNotEmpty() && root->hasProperty(currentPageId)) {
                            auto* page = root->getProperty(currentPageId).getDynamicObject();
                            if (page && page->hasProperty(currentCardId)) {
                                page->getProperty(currentCardId).getDynamicObject()->setProperty(pName, newHex);
                                layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                            }
                        } else if (currentPageId.isEmpty() && root->hasProperty(currentCardId)) {
                            root->getProperty(currentCardId).getDynamicObject()->setProperty(pName, newHex);
                            layoutJsonDocument.replaceAllContent(juce::JSON::toString(parsedObj));
                        }
                    }
                });
            } else {
                juce::Value val (valStr);
                val.addListener(this); // Just in case, standard JUCE listener not fully wired here for custom edit
                pc = new juce::TextPropertyComponent(val, pName, 256, false);
            }
'''

with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

start_str = '            juce::Value val (valStr);\n            auto* pc = new juce::TextPropertyComponent(val, prop.name.toString(), 256, false);'
content = content.replace(start_str, code.strip())

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
