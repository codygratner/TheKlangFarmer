#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <iostream>

int main(int argc, char* argv[]) {
    juce::Colour c = juce::Colour::fromString("colRed");
    std::cout << "Color: " << c.toDisplayString(true).toStdString() << "\n";
    return 0;
}
