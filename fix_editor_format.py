import re

with open('tools/editor/MainComponent.cpp', 'r') as f:
    content = f.read()

replacement = '''
                            if (def->format == "waveshape") {
                                slider->customFormatText = formatWaveshape;
                                slider->customParseText = parseWaveshape;
                            } else if (def->format == "wet_dry") {
                                slider->customFormatText = formatWetDry;
                                slider->customParseText = parseWetDry;
                            } else {
                                slider->customFormatText = [paramId](double val) {
                                    if (paramId.containsIgnoreCase("pitch") || paramId.containsIgnoreCase("semi")) return juce::String(juce::roundToInt((val - 0.5) * 96.0)) + " st";
                                    if (paramId.containsIgnoreCase("depth") || paramId.containsIgnoreCase("crossfade") || paramId.containsIgnoreCase("pan") || paramId.containsIgnoreCase("filter")) return (val >= 0.5 ? "+" : "") + juce::String(juce::roundToInt((val - 0.5) * 200.0)) + " %";
                                    if (paramId.containsIgnoreCase("shape") || paramId.containsIgnoreCase("ratio") || paramId.containsIgnoreCase("amount") || paramId.containsIgnoreCase("level") || paramId.containsIgnoreCase("res") || paramId.containsIgnoreCase("mix")) return juce::String(juce::roundToInt(val * 100.0)) + " %";
                                    if (paramId.containsIgnoreCase("decay") || paramId.containsIgnoreCase("speed") || paramId.containsIgnoreCase("time")) return juce::String(val * 2000.0, 0) + " ms";
                                    if (paramId.containsIgnoreCase("cutoff") || paramId.containsIgnoreCase("rate") || paramId.containsIgnoreCase("freq")) return juce::String(val * 20000.0, 0) + " Hz";
                                    if (paramId.containsIgnoreCase("drive") || paramId.containsIgnoreCase("gain")) return (val >= 0.5 ? "+" : "") + juce::String((val - 0.5) * 48.0, 1) + " dB";
                                    if (paramId.containsIgnoreCase("claps")) return juce::String(juce::roundToInt(val * 8.0));
                                    return juce::String(val, 2);
                                };
                            }
'''

pattern = r'slider->customFormatText = \[paramId\]\(double val\) \{.*?\};\n'

content = re.sub(pattern, replacement, content, flags=re.DOTALL)

with open('tools/editor/MainComponent.cpp', 'w') as f:
    f.write(content)
