with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

content = content.replace(
    'if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId)) {\n        for (const auto& poi : def->snapPoints) {',
    'if (auto* def = RlyehSound::ParameterManager::getInstance().getControlDef(paramId)) {\n        juce::Logger::writeToLog("SliderCalloutComponent paramId: " + paramId + " snapPoints: " + juce::String(def->snapPoints.size()));\n        for (const auto& poi : def->snapPoints) {'
)

with open('source/UIComponents.cpp', 'w') as f:
    f.write(content)
