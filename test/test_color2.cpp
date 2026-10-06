#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <iostream>

int main(int argc, char* argv[]) {
    juce::Colour c = juce::Colour::fromString("0xFFFF1744");
    std::cout << "Color: " << c.toDisplayString(true).toStdString() << "\n";
    juce::Colour c2 = juce::Colour::fromString("FFFF1744");
    std::cout << "Color2: " << c2.toDisplayString(true).toStdString() << "\n";
    return 0;
}
