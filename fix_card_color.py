with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

old_multi = '''                            // Render this card!
                            juce::Colour c = juce::Colour(0xffcfd8dc);
                            auto styleStr = cardObj->hasProperty("style") ? cardObj->getProperty("style").toString() : "StandardDark";'''

new_multi = '''                            // Render this card!
                            juce::Colour c = juce::Colour(0xffcfd8dc);
                            if (cardObj->hasProperty("color")) {
                                c = juce::Colour::fromString(cardObj->getProperty("color").toString());
                            }
                            auto styleStr = cardObj->hasProperty("style") ? cardObj->getProperty("style").toString() : "StandardDark";'''

content = content.replace(old_multi, new_multi)

old_single = '''        if (parsed.isObject()) {
            auto moduleConfig = parsed;
            
            juce::Colour c = juce::Colour(0xffcfd8dc);
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();'''

new_single = '''        if (parsed.isObject()) {
            auto moduleConfig = parsed;
            
            juce::Colour c = juce::Colour(0xffcfd8dc);
            if (moduleConfig.hasProperty("color")) {
                c = juce::Colour::fromString(moduleConfig.getProperty("color").toString());
            }
            auto styleStr = moduleConfig.getProperty("style", "StandardDark").toString();'''

content = content.replace(old_single, new_single)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
