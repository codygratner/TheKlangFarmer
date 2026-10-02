#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PlanterEngine.h"
#include "UIComponents.h"

class TheKlangPlanterAudioProcessor : public juce::AudioProcessor {
public:
    TheKlangPlanterAudioProcessor();
    ~TheKlangPlanterAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "The Klang Planter"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    TbdAudio::PlanterDrumEngine& getEngine() { return engine; }

    float getLimiterActivity() const { return engine.getLimiterActivity(); }

    using ModSourceDetail = ::ModSourceDetail;
    using ParamModulationInfo = ::ParamModulationInfo;
    ParamModulationInfo getParamModulationInfo(const juce::String& paramId) const;

private:
    TbdAudio::PlanterDrumEngine engine;

    // Direct parameter pointers for lock-free audio thread access
    // 1. Carrier
    juce::AudioParameterChoice* carrierTrackingParam = nullptr;
    juce::AudioParameterFloat*  carrierPitchParam    = nullptr;
    juce::AudioParameterFloat*  carrierShapeParam    = nullptr;
    juce::AudioParameterFloat*  carrierDepthParam    = nullptr;

    // 2. Modulator
    juce::AudioParameterChoice* modTrackParam        = nullptr;
    juce::AudioParameterChoice* modTypeParam         = nullptr;
    juce::AudioParameterFloat*  modShapeParam        = nullptr;
    juce::AudioParameterFloat*  modSpeedParam        = nullptr;

    // 3. Pitch Envelope
    juce::AudioParameterChoice* pitchEnvTargetParam  = nullptr;
    juce::AudioParameterFloat*  pitchEnvSlopeParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnvDepthParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnvDecayParam   = nullptr;

    // 4. Noise Transient (S&H, DJ Filter, Decay, Crossfade)
    juce::AudioParameterFloat*  noiseShRateParam     = nullptr;
    juce::AudioParameterFloat*  noiseFilterParam     = nullptr;
    juce::AudioParameterFloat*  noiseDecayParam      = nullptr;
    juce::AudioParameterFloat*  noiseCrossfadeParam  = nullptr;

    // 5. Filter
    juce::AudioParameterChoice* filterTypeParam      = nullptr;
    juce::AudioParameterChoice* filterSlopeParam     = nullptr;
    juce::AudioParameterFloat*  filterCutoffParam    = nullptr;
    juce::AudioParameterFloat*  filterResoParam      = nullptr;

    // 6. Filter Envelope (Slope, Depth, Decay, Pre-Filter Drive)
    juce::AudioParameterFloat*  filterEnvSlopeParam  = nullptr;
    juce::AudioParameterFloat*  filterEnvDepthParam  = nullptr;
    juce::AudioParameterFloat*  filterEnvDecayParam  = nullptr;
    juce::AudioParameterFloat*  filterEnvDriveParam  = nullptr;

    // 7. Amplifier (Drive, Pan, Vel Slope, Vel Floor)
    juce::AudioParameterFloat*  ampDriveParam        = nullptr;
    juce::AudioParameterFloat*  ampPanParam          = nullptr;
    juce::AudioParameterFloat*  ampVelSlopeParam     = nullptr;
    juce::AudioParameterFloat*  ampVelFloorParam     = nullptr;

    // 8. Amp Envelope
    juce::AudioParameterFloat*  ampEnvClapsParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvClapSpeedParam = nullptr;
    juce::AudioParameterFloat*  ampEnvSlopeParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvDecayParam     = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangPlanterAudioProcessor)
};
