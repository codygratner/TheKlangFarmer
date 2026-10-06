#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include <iostream>

int main(int argc, char* argv[]) {
    juce::MessageManager::getInstance();
    std::cout << "OK\n";
    juce::MessageManager::deleteInstance();
    return 0;
}
