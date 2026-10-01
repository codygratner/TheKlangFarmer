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
    // 1. Carrier 1
    juce::AudioParameterChoice* carrier1TrackingParam = nullptr;
    juce::AudioParameterFloat*  carrier1PitchParam    = nullptr;
    juce::AudioParameterFloat*  carrier1ShapeParam    = nullptr;
    juce::AudioParameterFloat*  carrier1DepthParam    = nullptr;

    // 2. Modulator 1
    juce::AudioParameterChoice* mod1TrackParam        = nullptr;
    juce::AudioParameterChoice* mod1TypeParam         = nullptr;
    juce::AudioParameterFloat*  mod1ShapeParam        = nullptr;
    juce::AudioParameterFloat*  mod1SpeedParam        = nullptr;

    // 3. Pitch Envelope 1
    juce::AudioParameterChoice* pitchEnv1TargetParam  = nullptr;
    juce::AudioParameterFloat*  pitchEnv1SlopeParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnv1DepthParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnv1DecayParam   = nullptr;

    // 4. Carrier 2
    juce::AudioParameterChoice* carrier2TrackingParam = nullptr;
    juce::AudioParameterFloat*  carrier2PitchParam    = nullptr;
    juce::AudioParameterFloat*  carrier2ShapeParam    = nullptr;
    juce::AudioParameterFloat*  carrier2DepthParam    = nullptr;

    // 5. Modulator 2
    juce::AudioParameterChoice* mod2TrackParam        = nullptr;
    juce::AudioParameterChoice* mod2TypeParam         = nullptr;
    juce::AudioParameterFloat*  mod2ShapeParam        = nullptr;
    juce::AudioParameterFloat*  mod2SpeedParam        = nullptr;

    // 6. Pitch Envelope 2
    juce::AudioParameterChoice* pitchEnv2TargetParam  = nullptr;
    juce::AudioParameterFloat*  pitchEnv2SlopeParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnv2DepthParam   = nullptr;
    juce::AudioParameterFloat*  pitchEnv2DecayParam   = nullptr;

    // 7. Noise Transient
    juce::AudioParameterFloat*  noiseShRateParam     = nullptr;
    juce::AudioParameterFloat*  noiseFilterParam     = nullptr;
    juce::AudioParameterFloat*  noiseDriveParam      = nullptr;
    juce::AudioParameterFloat*  noiseDecayParam      = nullptr;

    // 8. Mixer
    juce::AudioParameterFloat*  mixerCarrier1LevelParam = nullptr;
    juce::AudioParameterFloat*  mixerCarrier2LevelParam = nullptr;
    juce::AudioParameterFloat*  mixerRingModParam       = nullptr;
    juce::AudioParameterFloat*  mixerNoiseLevelParam    = nullptr;

    // 9. Drive
    juce::AudioParameterFloat*  driveAmountParam     = nullptr;
    juce::AudioParameterFloat*  driveBiasParam       = nullptr;
    juce::AudioParameterFloat*  driveFilterParam     = nullptr;
    juce::AudioParameterChoice* driveLimiterParam    = nullptr;

    // Voice 1 Filter & Env
    juce::AudioParameterChoice* filter1TypeParam      = nullptr;
    juce::AudioParameterChoice* filter1SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filter1CutoffParam    = nullptr;
    juce::AudioParameterFloat*  filter1ResonanceParam = nullptr;

    juce::AudioParameterFloat*  filterEnv1SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv1DepthParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv1DecayParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv1PostDriveParam = nullptr;

    // Voice 2 Filter & Env
    juce::AudioParameterChoice* filter2TypeParam      = nullptr;
    juce::AudioParameterChoice* filter2SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filter2CutoffParam    = nullptr;
    juce::AudioParameterFloat*  filter2ResonanceParam = nullptr;

    juce::AudioParameterFloat*  filterEnv2SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv2DepthParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv2DecayParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv2PostDriveParam = nullptr;

    // Transients Filter & Env
    juce::AudioParameterChoice* filter3TypeParam      = nullptr;
    juce::AudioParameterChoice* filter3SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filter3CutoffParam    = nullptr;
    juce::AudioParameterFloat*  filter3ResonanceParam = nullptr;

    juce::AudioParameterFloat*  filterEnv3SlopeParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv3DepthParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv3DecayParam     = nullptr;
    juce::AudioParameterFloat*  filterEnv3PostDriveParam = nullptr;

    // Standalone FX Filter
    juce::AudioParameterChoice* fxFilterTypeParam      = nullptr;
    juce::AudioParameterChoice* fxFilterSlopeParam     = nullptr;
    juce::AudioParameterFloat*  fxFilterCutoffParam    = nullptr;
    juce::AudioParameterFloat*  fxFilterResonanceParam = nullptr;

    // Limiters
    juce::AudioParameterChoice* preLimiterEnableParam   = nullptr;
    juce::AudioParameterFloat*  preLimiterGainParam     = nullptr;
    juce::AudioParameterFloat*  preLimiterThreshParam   = nullptr;
    juce::AudioParameterFloat*  preLimiterReleaseParam  = nullptr;

    juce::AudioParameterChoice* postLimiterEnableParam  = nullptr;
    juce::AudioParameterFloat*  postLimiterGainParam    = nullptr;
    juce::AudioParameterFloat*  postLimiterThreshParam  = nullptr;
    juce::AudioParameterFloat*  postLimiterReleaseParam = nullptr;

    // FX Pickers (Pre-Amp & Post-Amp)
    juce::AudioParameterChoice* preFX1TypeParam  = nullptr;
    juce::AudioParameterChoice* preFX2TypeParam  = nullptr;
    juce::AudioParameterChoice* preFX3TypeParam  = nullptr;
    juce::AudioParameterChoice* preFX4TypeParam  = nullptr;

    juce::AudioParameterChoice* postFX1TypeParam = nullptr;
    juce::AudioParameterChoice* postFX2TypeParam = nullptr;
    juce::AudioParameterChoice* postFX3TypeParam = nullptr;
    juce::AudioParameterChoice* postFX4TypeParam = nullptr;

    // 12. Wave Folder
    juce::AudioParameterChoice* waveFolderTypeParam   = nullptr;
    juce::AudioParameterFloat*  waveFolderFoldParam   = nullptr;
    juce::AudioParameterFloat*  waveFolderBiasParam   = nullptr;
    juce::AudioParameterFloat*  waveFolderFilterParam = nullptr;

    // 13. RingMod
    juce::AudioParameterFloat*  ringModShapeParam    = nullptr;
    juce::AudioParameterFloat*  ringModRateParam     = nullptr;
    juce::AudioParameterFloat*  ringModAmountParam   = nullptr;
    juce::AudioParameterFloat*  ringModWidthParam    = nullptr;

    // 14. Frequency Shifter
    juce::AudioParameterFloat*  freqShiftShiftParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftRangeParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftBlendParam  = nullptr;
    juce::AudioParameterFloat*  freqShiftWidthParam  = nullptr;

    // 15. Grit FX
    juce::AudioParameterFloat*  gritBitsParam        = nullptr;
    juce::AudioParameterFloat*  gritRateParam        = nullptr;
    juce::AudioParameterFloat*  gritLowParam         = nullptr;
    juce::AudioParameterFloat*  gritHighParam        = nullptr;

    // 16. Comb Filter
    juce::AudioParameterChoice* combTypeParam        = nullptr;
    juce::AudioParameterFloat*  combDampeningParam   = nullptr;
    juce::AudioParameterFloat*  combCutoffParam      = nullptr;
    juce::AudioParameterFloat*  combResonanceParam   = nullptr;

    // 17. Disperser
    juce::AudioParameterChoice* disperserTypeParam   = nullptr;
    juce::AudioParameterFloat*  disperserAmountParam = nullptr;
    juce::AudioParameterFloat*  disperserCutoffParam = nullptr;
    juce::AudioParameterFloat*  disperserResonanceParam = nullptr;

    // 18. EQ (bell EQ)
    juce::AudioParameterFloat*  eqFreqParam          = nullptr;
    juce::AudioParameterFloat*  eqWidthParam         = nullptr;
    juce::AudioParameterFloat*  eqGainParam          = nullptr;
    juce::AudioParameterFloat*  eqFilterParam        = nullptr;

    // 19. Amp
    juce::AudioParameterFloat*  ampLevelParam        = nullptr;
    juce::AudioParameterFloat*  ampPanParam          = nullptr;
    juce::AudioParameterFloat*  ampDriveParam        = nullptr;
    juce::AudioParameterChoice* ampLimiterParam      = nullptr;

    // 20. Amp Envelope
    juce::AudioParameterFloat*  ampEnvClapsParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvClapSpeedParam = nullptr;
    juce::AudioParameterFloat*  ampEnvSlopeParam     = nullptr;
    juce::AudioParameterFloat*  ampEnvDecayParam     = nullptr;

    // 21. Velocity
    juce::AudioParameterFloat*  velSlopeParam        = nullptr;
    juce::AudioParameterFloat*  velDepthParam        = nullptr;
    juce::AudioParameterFloat*  velDecayParam        = nullptr;
    juce::AudioParameterFloat*  velVolumeParam       = nullptr;

    // 22. Key Tracking
    juce::AudioParameterFloat*  keySlopeParam        = nullptr;
    juce::AudioParameterFloat*  keyDepthParam        = nullptr;
    juce::AudioParameterFloat*  keyDecayParam        = nullptr;
    juce::AudioParameterFloat*  keyVolumeParam       = nullptr;

    // 23. Slop
    juce::AudioParameterFloat*  slopFreqParam        = nullptr;
    juce::AudioParameterFloat*  slopDepthParam       = nullptr;
    juce::AudioParameterFloat*  slopDecayParam       = nullptr;
    juce::AudioParameterFloat*  slopPanParam         = nullptr;

    // 24. Mod Envelopes 1..3
    juce::AudioParameterFloat*  modEnv1SlopeParam    = nullptr;
    juce::AudioParameterFloat*  modEnv1DepthParam    = nullptr;
    juce::AudioParameterFloat*  modEnv1DecayParam    = nullptr;
    juce::AudioParameterChoice* modEnv1TargetParam   = nullptr;

    juce::AudioParameterFloat*  modEnv2SlopeParam    = nullptr;
    juce::AudioParameterFloat*  modEnv2DepthParam    = nullptr;
    juce::AudioParameterFloat*  modEnv2DecayParam    = nullptr;
    juce::AudioParameterChoice* modEnv2TargetParam   = nullptr;

    juce::AudioParameterFloat*  modEnv3SlopeParam    = nullptr;
    juce::AudioParameterFloat*  modEnv3DepthParam    = nullptr;
    juce::AudioParameterFloat*  modEnv3DecayParam    = nullptr;
    juce::AudioParameterChoice* modEnv3TargetParam   = nullptr;

    // 25. Multi-Instance FX Slots
    juce::AudioParameterFloat* preFXParam[4][4]  = { {nullptr} };
    juce::AudioParameterFloat* postFXParam[4][4] = { {nullptr} };

    std::vector<juce::AudioParameterFloat*> continuousParams;
    void applyBaseParameters();
    void applyModulationTargets(int target1, int target2, int target3);

public:
    enum class ModTargetType {
        PageBlock,
        PreFX,
        PostFX
    };

    struct ModDestDescriptor {
        const char* id;
        const char* name;
        ModTargetType type;
        int blockOrSlot;
        int paramIndex;
    };

    static const std::vector<ModDestDescriptor>& getModDestinations();
    static juce::StringArray getModDestinationChoices();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TheKlangFarmerAudioProcessor)
};
