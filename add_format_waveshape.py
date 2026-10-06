import re

with open('source/UIComponents.h', 'r') as f:
    content = f.read()

if 'juce::String formatWaveshape(double val);' not in content:
    content = content.replace('juce::String formatWetDry(double val);', 'juce::String formatWaveshape(double val);\ndouble parseWaveshape(const juce::String& text);\n\njuce::String formatWetDry(double val);')
    with open('source/UIComponents.h', 'w') as f:
        f.write(content)

with open('source/UIComponents.cpp', 'r') as f:
    content = f.read()

impl = '''
juce::String formatWaveshape(double val) {
    if (val < 0.1) return juce::String("Sine");
    if (val < 0.3) return juce::String("Triangle");
    if (val < 0.5) return juce::String("Sawtooth");
    if (val < 0.8) return juce::String("Square");
    return juce::String("PWM");
}

double parseWaveshape(const juce::String& text) {
    if (text.containsIgnoreCase("sin")) return 0.0;
    if (text.containsIgnoreCase("tri")) return 0.2;
    if (text.containsIgnoreCase("saw")) return 0.4;
    if (text.containsIgnoreCase("sq") || text.containsIgnoreCase("pul")) return 0.6;
    if (text.containsIgnoreCase("pwm")) return 1.0;
    return 0.0;
}
'''

if 'formatWaveshape(' not in content:
    content = content.replace('juce::String formatWetDry(double val) {', impl + '\njuce::String formatWetDry(double val) {')
    with open('source/UIComponents.cpp', 'w') as f:
        f.write(content)
