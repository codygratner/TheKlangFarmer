#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "ModularBlocks.h"

class BiaEr1AudioProcessor : public juce::AudioProcessor {
public:
    BiaEr1AudioProcessor();
    ~BiaEr1AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

    TbdAudio::ModularDrumEngine& getEngine() { return engine; }

private:
    TbdAudio::ModularDrumEngine engine;

    // Direct parameter pointers for fast, thread-safe access
    juce::AudioParameterChoice* carrierTrackingParam = nullptr;
    juce::AudioParameterFloat*  carrierPitchParam    = nullptr;
    juce::AudioParameterFloat*  carrierShapeParam    = nullptr;
    juce::AudioParameterFloat*  carrierLevelParam    = nullptr;

    juce::AudioParameterChoice* modTypeParam         = nullptr;
    juce::AudioParameterFloat*  modShapeParam        = nullptr;
    juce::AudioParameterFloat*  modDepthParam        = nullptr;
    juce::AudioParameterFloat*  modSpeedParam        = nullptr;

    juce::AudioParameterChoice* driveTypeParam       = nullptr;
    juce::AudioParameterFloat*  driveAmountParam     = nullptr;
    juce::AudioParameterFloat*  driveBiasParam       = nullptr;
    juce::AudioParameterFloat*  driveFilterParam     = nullptr;

    juce::AudioParameterFloat*  noiseShRateParam     = nullptr;
    juce::AudioParameterFloat*  noiseFilterParam     = nullptr;
    juce::AudioParameterFloat*  noiseLevelParam      = nullptr;
    juce::AudioParameterFloat*  noiseDecayParam      = nullptr;

    juce::AudioParameterChoice* filterTypeParam      = nullptr;
    juce::AudioParameterFloat*  filterCutoffParam    = nullptr;
    juce::AudioParameterFloat*  filterDepthParam     = nullptr;
    juce::AudioParameterFloat*  filterDecayParam     = nullptr;

    juce::AudioParameterFloat*  ringModShapeParam    = nullptr;
    juce::AudioParameterFloat*  ringModRateParam     = nullptr;
    juce::AudioParameterFloat*  ringModAmountParam   = nullptr;
    juce::AudioParameterFloat*  ringModWidthParam    = nullptr;

    juce::AudioParameterFloat*  gritBitsParam        = nullptr;
    juce::AudioParameterFloat*  gritRateParam        = nullptr;

    juce::AudioParameterFloat*  freqShiftShiftParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftRangeParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftBlendParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftWidthParam  = nullptr;

    juce::AudioParameterFloat*  ampPanParam          = nullptr;
    juce::AudioParameterFloat*  ampLevelParam        = nullptr;
    juce::AudioParameterFloat*  ampDriveParam        = nullptr;
    juce::AudioParameterFloat*  ampLowBoostParam     = nullptr;

    juce::AudioParameterChoice* ampEnvTypeParam      = nullptr;
    juce::AudioParameterFloat*  ampEnvClapsParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvShapeParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvDecayParam     = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BiaEr1AudioProcessor)
};
