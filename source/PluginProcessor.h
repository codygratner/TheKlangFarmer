#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "ModularBlocks.h"

class TheKlangFarmerAudioProcessor : public juce::AudioProcessor {
public:
    TheKlangFarmerAudioProcessor();
    ~TheKlangFarmerAudioProcessor() override = default;

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
    // 1. Carrier
    juce::AudioParameterChoice* carrierTrackingParam = nullptr;
    juce::AudioParameterFloat*  carrierPitchParam    = nullptr;
    juce::AudioParameterFloat*  carrierShapeParam    = nullptr;
    juce::AudioParameterFloat*  carrierDriveParam    = nullptr;

    // 2. Modulator
    juce::AudioParameterChoice* modTypeParam         = nullptr;
    juce::AudioParameterFloat*  modShapeParam        = nullptr;
    juce::AudioParameterFloat*  modDepthParam        = nullptr;
    juce::AudioParameterFloat*  modSpeedParam        = nullptr;

    // 3. Pitch Envelope
    juce::AudioParameterChoice* pitchEnvTargetParam  = nullptr;
    juce::AudioParameterFloat*  pitchEnvSlopeParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnvDepthParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnvDecayParam   = nullptr;

    // 4. Drive
    juce::AudioParameterChoice* driveTypeParam       = nullptr;
    juce::AudioParameterFloat*  driveAmountParam     = nullptr;
    juce::AudioParameterFloat*  driveBiasParam       = nullptr;
    juce::AudioParameterFloat*  driveFilterParam     = nullptr;

    // 5. Noise Transient
    juce::AudioParameterFloat*  noiseShRateParam     = nullptr;
    juce::AudioParameterFloat*  noiseFilterParam     = nullptr;
    juce::AudioParameterFloat*  noiseDriveParam      = nullptr;
    juce::AudioParameterFloat*  noiseDecayParam      = nullptr;

    // 6. Mixer
    juce::AudioParameterFloat*  mixerCarrierLevelParam = nullptr;
    juce::AudioParameterFloat*  mixerNoiseLevelParam   = nullptr;
    juce::AudioParameterFloat*  mixerDriveParam        = nullptr;
    juce::AudioParameterChoice* mixerLimiterParam      = nullptr;

    // 7. Filter
    juce::AudioParameterChoice* filterTypeParam      = nullptr;
    juce::AudioParameterFloat*  filterSlopeParam     = nullptr;
    juce::AudioParameterFloat*  filterCutoffParam    = nullptr;
    juce::AudioParameterFloat*  filterResonanceParam = nullptr;

    // 8. Filter Envelope
    juce::AudioParameterFloat*  filterEnvSlopeParam    = nullptr;
    juce::AudioParameterFloat*  filterEnvDepthParam    = nullptr;
    juce::AudioParameterFloat*  filterEnvDecayParam    = nullptr;
    juce::AudioParameterFloat*  filterEnvPreDriveParam = nullptr;

    // 9. RingMod
    juce::AudioParameterFloat*  ringModShapeParam    = nullptr;
    juce::AudioParameterFloat*  ringModRateParam     = nullptr;
    juce::AudioParameterFloat*  ringModAmountParam   = nullptr;
    juce::AudioParameterFloat*  ringModWidthParam    = nullptr;

    // 10. Frequency Shifter
    juce::AudioParameterFloat*  freqShiftShiftParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftRangeParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftBlendParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftWidthParam  = nullptr;

    // 11. Grit FX
    juce::AudioParameterFloat*  gritBitsParam        = nullptr;
    juce::AudioParameterFloat*  gritRateParam        = nullptr;
    juce::AudioParameterFloat*  gritLowBoostParam    = nullptr;
    juce::AudioParameterFloat*  gritHighBoostParam   = nullptr;

    // 12. Comb Filter
    juce::AudioParameterChoice* combTypeParam        = nullptr;
    juce::AudioParameterFloat*  combDampeningParam   = nullptr;
    juce::AudioParameterFloat*  combCutoffParam      = nullptr;
    juce::AudioParameterFloat*  combResonanceParam   = nullptr;

    // 13. Disperser
    juce::AudioParameterChoice* disperserTypeParam   = nullptr;
    juce::AudioParameterFloat*  disperserAmountParam = nullptr;
    juce::AudioParameterFloat*  disperserCutoffParam = nullptr;
    juce::AudioParameterFloat*  disperserResonanceParam = nullptr;

    // 14. Amp
    juce::AudioParameterFloat*  ampPanParam          = nullptr;
    juce::AudioParameterFloat*  ampLevelParam        = nullptr;
    juce::AudioParameterFloat*  ampDriveParam        = nullptr;
    juce::AudioParameterChoice* ampLimiterParam      = nullptr;

    // 15. Amp Envelope
    juce::AudioParameterFloat*  ampEnvClapsParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvClapSpeedParam = nullptr;
    juce::AudioParameterFloat*  ampEnvSlopeParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvDecayParam     = nullptr;

    // 16. Velocity
    juce::AudioParameterFloat*  velSlopeParam        = nullptr;
    juce::AudioParameterFloat*  velDecayParam        = nullptr;
    juce::AudioParameterFloat*  velDepthParam        = nullptr;
    juce::AudioParameterFloat*  velVolumeParam       = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangFarmerAudioProcessor)
};
