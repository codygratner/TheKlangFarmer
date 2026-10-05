#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

class KlangCoreProcessor : public juce::AudioProcessor {
public:
    KlangCoreProcessor(const BusesProperties& ioLayouts, const juce::String& apvtsName, 
                       juce::AudioProcessorValueTreeState::ParameterLayout layout);
    ~KlangCoreProcessor() override = default;

    // Standard JUCE audio processor methods (Shared)
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // The APVTS (Shared across all derivatives)
    juce::AudioProcessorValueTreeState apvts;
};

