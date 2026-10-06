#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <iostream>

int main(int argc, char* argv[]) {
    juce::File file("C:/Dev/TheKlangFarmer/assets/controls/theme.json");
    auto jsonString = file.loadFileAsString();
    auto parsed = juce::JSON::parse(jsonString);
    
    if (parsed.isObject()) {
        auto* obj = parsed.getDynamicObject();
        if (obj->hasProperty("colors")) {
            auto* colorsObj = obj->getProperty("colors").getDynamicObject();
            if (colorsObj) {
                for (auto& prop : colorsObj->getProperties()) {
                    bool isStr = prop.value.isString();
                    bool startsWith = prop.value.toString().startsWithIgnoreCase("0x");
                    std::cout << prop.name.toString().toStdString() << ": isStr=" << isStr << " startsWith=" << startsWith << "\n";
                }
            } else {
                std::cout << "colorsObj is null\n";
            }
        } else {
            std::cout << "no colors property\n";
        }
    } else {
        std::cout << "not an object\n";
    }
    return 0;
}
