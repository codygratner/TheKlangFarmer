#include "PluginProcessor.h"
#include "PluginEditor.h"

TheKlangFarmerAudioProcessor::TheKlangFarmerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Retrieve direct raw parameter pointers
    // 1. Carrier 1
    carrier1TrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier1_tracking"));
    carrier1PitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_pitch"));
    carrier1ShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_shape"));
    carrier1DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier1_depth"));

    // 2. Modulator 1
    mod1TrackParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod1_track"));
    mod1TypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod1_type"));
    mod1ShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod1_shape"));
    mod1SpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod1_speed"));

    // 3. Pitch Envelope 1
    pitchEnv1TargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pitchenv1_target"));
    pitchEnv1SlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_slope"));
    pitchEnv1DepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_depth"));
    pitchEnv1DecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv1_decay"));

    // 4. Carrier 2
    carrier2TrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier2_tracking"));
    carrier2PitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_pitch"));
    carrier2ShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_shape"));
    carrier2DepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier2_depth"));

    // 5. Modulator 2
    mod2TrackParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod2_track"));
    mod2TypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod2_type"));
    mod2ShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod2_shape"));
    mod2SpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod2_speed"));

    // 6. Pitch Envelope 2
    pitchEnv2TargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pitchenv2_target"));
    pitchEnv2SlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_slope"));
    pitchEnv2DepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_depth"));
    pitchEnv2DecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv2_decay"));

    // 7. Noise Transient
    noiseShRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_sh_rate"));
    noiseFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_filter"));
    noiseDriveParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_drive"));
    noiseDecayParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_decay"));

    // 8. Mixer
    mixerCarrier1LevelParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_carrier1_level"));
    mixerCarrier2LevelParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_carrier2_level"));
    mixerRingModParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_ringmod"));
    mixerNoiseLevelParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_noise_level"));

    // 9. Drive
    driveAmountParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_amount"));
    driveBiasParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_bias"));
    driveFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_filter"));
    driveLimiterParam    = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("drive_limiter"));

    // Voice 1 Filter & Env
    filter1TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter1_type"));
    filter1SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter1_slope"));
    filter1CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter1_cutoff"));
    filter1ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter1_resonance"));

    filterEnv1SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_slope"));
    filterEnv1DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_depth"));
    filterEnv1DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_decay"));
    filterEnv1PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv1_postdrive"));

    // Voice 2 Filter & Env
    filter2TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter2_type"));
    filter2SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter2_slope"));
    filter2CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter2_cutoff"));
    filter2ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter2_resonance"));

    filterEnv2SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_slope"));
    filterEnv2DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_depth"));
    filterEnv2DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_decay"));
    filterEnv2PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv2_postdrive"));

    // Transients Filter & Env
    filter3TypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter3_type"));
    filter3SlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter3_slope"));
    filter3CutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter3_cutoff"));
    filter3ResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter3_resonance"));

    filterEnv3SlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_slope"));
    filterEnv3DepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_depth"));
    filterEnv3DecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_decay"));
    filterEnv3PostDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv3_postdrive"));

    // Standalone FX Filter
    fxFilterTypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("fxfilter_type"));
    fxFilterSlopeParam     = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("fxfilter_slope"));
    fxFilterCutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("fxfilter_cutoff"));
    fxFilterResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("fxfilter_resonance"));

    // Limiters
    preLimiterEnableParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_limiter_enable"));
    preLimiterGainParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_gain"));
    preLimiterThreshParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_thresh"));
    preLimiterReleaseParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pre_limiter_release"));

    postLimiterEnableParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_limiter_enable"));
    postLimiterGainParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_gain"));
    postLimiterThreshParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_thresh"));
    postLimiterReleaseParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("post_limiter_release"));

    // FX Pickers
    preFX1TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_1_type"));
    preFX2TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_2_type"));
    preFX3TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_3_type"));
    preFX4TypeParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pre_fx_4_type"));

    postFX1TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_1_type"));
    postFX2TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_2_type"));
    postFX3TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_3_type"));
    postFX4TypeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("post_fx_4_type"));

    // 12. Wave Folder
    waveFolderTypeParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("wavefolder_type"));
    waveFolderFoldParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("wavefolder_fold"));
    waveFolderBiasParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("wavefolder_bias"));
    waveFolderFilterParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("wavefolder_filter"));

    // 13. RingMod
    ringModShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_shape"));
    ringModRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_rate"));
    ringModAmountParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_amount"));
    ringModWidthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_width"));

    // 14. Frequency Shifter
    freqShiftShiftParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_shift"));
    freqShiftRangeParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_range"));
    freqShiftBlendParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_blend"));
    freqShiftWidthParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_width"));

    // 15. Grit FX
    gritBitsParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_bits"));
    gritRateParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_rate"));
    gritLowParam         = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_low"));
    gritHighParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_high"));

    // 16. Comb Filter
    combTypeParam        = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("comb_type"));
    combDampeningParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("comb_dampening"));
    combCutoffParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("comb_cutoff"));
    combResonanceParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("comb_resonance"));

    // 17. Disperser
    disperserTypeParam   = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("disperser_type"));
    disperserAmountParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("disperser_amount"));
    disperserCutoffParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("disperser_cutoff"));
    disperserResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("disperser_resonance"));

    // 18. EQ (bell EQ)
    eqFreqParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("eq_freq"));
    eqWidthParam         = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("eq_width"));
    eqGainParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("eq_gain"));
    eqFilterParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("eq_filter"));

    // 19. Amp
    ampLevelParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_level"));
    ampPanParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_pan"));
    ampDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_drive"));
    ampLimiterParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("amp_limiter"));

    // 20. Amp Envelope
    ampEnvClapsParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_claps"));
    ampEnvClapSpeedParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_clapspeed"));
    ampEnvSlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_slope"));
    ampEnvDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_decay"));

    // 21. Velocity
    velSlopeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_slope"));
    velDecayParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_decay"));
    velDepthParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_depth"));
    velVolumeParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("vel_volume"));

    // 22. Slop
    slopFreqParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_freq"));
    slopDepthParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_depth"));
    slopDecayParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_decay"));
    slopPanParam         = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("slop_pan"));

    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            preFXParam[s][p] = dynamic_cast<juce::AudioParameterFloat*>(
                apvts.getParameter("pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1)));
            postFXParam[s][p] = dynamic_cast<juce::AudioParameterFloat*>(
                apvts.getParameter("post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1)));
        }
    }
}

void TheKlangFarmerAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    engine.init(static_cast<float>(sampleRate));
}

void TheKlangFarmerAudioProcessor::releaseResources() {
}

bool TheKlangFarmerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TheKlangFarmerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    auto getNorm = [](juce::AudioParameterFloat* p) {
        return p ? p->range.convertTo0to1(p->get()) : 0.0f;
    };

    if (auto* playHead = getPlayHead()) {
        if (auto posOpt = playHead->getPosition()) {
            if (auto bpmOpt = posOpt->getBpm()) {
                engine.setBpm(static_cast<float>(*bpmOpt));
            }
        }
    }

    // 1. Carrier 1
    if (carrier1TrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 0, static_cast<float>(carrier1TrackingParam->getIndex()) / 2.0f);
    if (carrier1PitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, getNorm(carrier1PitchParam));
    if (carrier1ShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 2, getNorm(carrier1ShapeParam));
    if (carrier1DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 3, getNorm(carrier1DepthParam));

    // 2. Modulator 1
    if (mod1TrackParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 0, static_cast<float>(mod1TrackParam->getIndex()) / 2.0f);
    if (mod1TypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 1, static_cast<float>(mod1TypeParam->getIndex()) / 2.0f);
    if (mod1ShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 2, getNorm(mod1ShapeParam));
    if (mod1SpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR1, 3, getNorm(mod1SpeedParam));

    // 3. Pitch Envelope 1
    if (pitchEnv1TargetParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 0, static_cast<float>(pitchEnv1TargetParam->getIndex()) / 3.0f);
    if (pitchEnv1SlopeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 1, getNorm(pitchEnv1SlopeParam));
    if (pitchEnv1DepthParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 2, getNorm(pitchEnv1DepthParam));
    if (pitchEnv1DecayParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 3, getNorm(pitchEnv1DecayParam));

    // 4. Carrier 2
    if (carrier2TrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 0, static_cast<float>(carrier2TrackingParam->getIndex()) / 2.0f);
    if (carrier2PitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 1, getNorm(carrier2PitchParam));
    if (carrier2ShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 2, getNorm(carrier2ShapeParam));
    if (carrier2DepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER2, 3, getNorm(carrier2DepthParam));

    // 5. Modulator 2
    if (mod2TrackParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 0, static_cast<float>(mod2TrackParam->getIndex()) / 2.0f);
    if (mod2TypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 1, static_cast<float>(mod2TypeParam->getIndex()) / 2.0f);
    if (mod2ShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 2, getNorm(mod2ShapeParam));
    if (mod2SpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR2, 3, getNorm(mod2SpeedParam));

    // 6. Pitch Envelope 2
    if (pitchEnv2TargetParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 0, static_cast<float>(pitchEnv2TargetParam->getIndex()) / 3.0f);
    if (pitchEnv2SlopeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 1, getNorm(pitchEnv2SlopeParam));
    if (pitchEnv2DepthParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 2, getNorm(pitchEnv2DepthParam));
    if (pitchEnv2DecayParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 3, getNorm(pitchEnv2DecayParam));

    // 7. Noise Transient
    if (noiseShRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 0, getNorm(noiseShRateParam));
    if (noiseFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 1, getNorm(noiseFilterParam));
    if (noiseDriveParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 2, getNorm(noiseDriveParam));
    if (noiseDecayParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 3, getNorm(noiseDecayParam));

    // 8. Mixer
    if (mixerCarrier1LevelParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, getNorm(mixerCarrier1LevelParam));
    if (mixerCarrier2LevelParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, getNorm(mixerCarrier2LevelParam));
    if (mixerRingModParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, getNorm(mixerRingModParam));
    if (mixerNoiseLevelParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 3, getNorm(mixerNoiseLevelParam));

    // 9. Drive
    if (driveAmountParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 0, getNorm(driveAmountParam));
    if (driveBiasParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 1, getNorm(driveBiasParam));
    if (driveFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 2, getNorm(driveFilterParam));
    if (driveLimiterParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 3, static_cast<float>(driveLimiterParam->getIndex()));

    // Voice 1 Filter & Env
    if (filter1TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 0, static_cast<float>(filter1TypeParam->getIndex()) / 4.0f);
    if (filter1SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 1, static_cast<float>(filter1SlopeParam->getIndex()) / 4.0f);
    if (filter1CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 2, getNorm(filter1CutoffParam));
    if (filter1ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 3, getNorm(filter1ResonanceParam));

    if (filterEnv1SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 0, getNorm(filterEnv1SlopeParam));
    if (filterEnv1DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 1, getNorm(filterEnv1DepthParam));
    if (filterEnv1DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 2, getNorm(filterEnv1DecayParam));
    if (filterEnv1PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV1, 3, getNorm(filterEnv1PostDriveParam));

    // Voice 2 Filter & Env
    if (filter2TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 0, static_cast<float>(filter2TypeParam->getIndex()) / 4.0f);
    if (filter2SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 1, static_cast<float>(filter2SlopeParam->getIndex()) / 4.0f);
    if (filter2CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 2, getNorm(filter2CutoffParam));
    if (filter2ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER2, 3, getNorm(filter2ResonanceParam));

    if (filterEnv2SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 0, getNorm(filterEnv2SlopeParam));
    if (filterEnv2DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 1, getNorm(filterEnv2DepthParam));
    if (filterEnv2DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 2, getNorm(filterEnv2DecayParam));
    if (filterEnv2PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV2, 3, getNorm(filterEnv2PostDriveParam));

    // Transients Filter & Env
    if (filter3TypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 0, static_cast<float>(filter3TypeParam->getIndex()) / 4.0f);
    if (filter3SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 1, static_cast<float>(filter3SlopeParam->getIndex()) / 4.0f);
    if (filter3CutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 2, getNorm(filter3CutoffParam));
    if (filter3ResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER3, 3, getNorm(filter3ResonanceParam));

    if (filterEnv3SlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 0, getNorm(filterEnv3SlopeParam));
    if (filterEnv3DepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 1, getNorm(filterEnv3DepthParam));
    if (filterEnv3DecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 2, getNorm(filterEnv3DecayParam));
    if (filterEnv3PostDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV3, 3, getNorm(filterEnv3PostDriveParam));

    // Standalone FX Filter
    if (fxFilterTypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FXFILTER, 0, static_cast<float>(fxFilterTypeParam->getIndex()) / 4.0f);
    if (fxFilterSlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FXFILTER, 1, static_cast<float>(fxFilterSlopeParam->getIndex()) / 4.0f);
    if (fxFilterCutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FXFILTER, 2, getNorm(fxFilterCutoffParam));
    if (fxFilterResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FXFILTER, 3, getNorm(fxFilterResonanceParam));

    // Limiters
    if (preLimiterEnableParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 0, static_cast<float>(preLimiterEnableParam->getIndex()));
    if (preLimiterGainParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 1, getNorm(preLimiterGainParam));
    if (preLimiterThreshParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 2, getNorm(preLimiterThreshParam));
    if (preLimiterReleaseParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PRE_LIMITER, 3, getNorm(preLimiterReleaseParam));

    if (postLimiterEnableParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 0, static_cast<float>(postLimiterEnableParam->getIndex()));
    if (postLimiterGainParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 1, getNorm(postLimiterGainParam));
    if (postLimiterThreshParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 2, getNorm(postLimiterThreshParam));
    if (postLimiterReleaseParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_POST_LIMITER, 3, getNorm(postLimiterReleaseParam));

    // FX Pickers
    if (preFX1TypeParam)  engine.setPreFXType(0, preFX1TypeParam->getIndex());
    if (preFX2TypeParam)  engine.setPreFXType(1, preFX2TypeParam->getIndex());
    if (preFX3TypeParam)  engine.setPreFXType(2, preFX3TypeParam->getIndex());
    if (preFX4TypeParam)  engine.setPreFXType(3, preFX4TypeParam->getIndex());

    if (postFX1TypeParam) engine.setPostFXType(0, postFX1TypeParam->getIndex());
    if (postFX2TypeParam) engine.setPostFXType(1, postFX2TypeParam->getIndex());
    if (postFX3TypeParam) engine.setPostFXType(2, postFX3TypeParam->getIndex());
    if (postFX4TypeParam) engine.setPostFXType(3, postFX4TypeParam->getIndex());

    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            if (preFXParam[s][p]) engine.setPreFXParam(s, p, getNorm(preFXParam[s][p]));
            if (postFXParam[s][p]) engine.setPostFXParam(s, p, getNorm(postFXParam[s][p]));
        }
    }

    // 12. Wave Folder
    if (waveFolderTypeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 0, static_cast<float>(waveFolderTypeParam->getIndex()));
    if (waveFolderFoldParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 1, getNorm(waveFolderFoldParam));
    if (waveFolderBiasParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 2, getNorm(waveFolderBiasParam));
    if (waveFolderFilterParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 3, getNorm(waveFolderFilterParam));

    // 13. RingMod
    if (ringModShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 0, getNorm(ringModShapeParam));
    if (ringModRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 1, getNorm(ringModRateParam));
    if (ringModAmountParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 2, getNorm(ringModAmountParam));
    if (ringModWidthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 3, getNorm(ringModWidthParam));

    // 14. Frequency Shifter
    if (freqShiftShiftParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 0, getNorm(freqShiftShiftParam));
    if (freqShiftRangeParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 1, getNorm(freqShiftRangeParam));
    if (freqShiftBlendParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 2, getNorm(freqShiftBlendParam));
    if (freqShiftWidthParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 3, getNorm(freqShiftWidthParam));

    // 15. Grit FX
    if (gritBitsParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 0, getNorm(gritBitsParam));
    if (gritRateParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 1, getNorm(gritRateParam));
    if (gritLowParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 2, getNorm(gritLowParam));
    if (gritHighParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 3, getNorm(gritHighParam));

    // 16. Comb Filter
    if (combTypeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 0, static_cast<float>(combTypeParam->getIndex()));
    if (combDampeningParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 1, getNorm(combDampeningParam));
    if (combCutoffParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 2, getNorm(combCutoffParam));
    if (combResonanceParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 3, getNorm(combResonanceParam));

    // 17. Disperser
    if (disperserTypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 0, static_cast<float>(disperserTypeParam->getIndex()));
    if (disperserAmountParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 1, getNorm(disperserAmountParam));
    if (disperserCutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 2, getNorm(disperserCutoffParam));
    if (disperserResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 3, getNorm(disperserResonanceParam));

    // 18. EQ (bell EQ)
    if (eqFreqParam)             engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_EQ, 0, getNorm(eqFreqParam));
    if (eqWidthParam)            engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_EQ, 1, getNorm(eqWidthParam));
    if (eqGainParam)             engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_EQ, 2, getNorm(eqGainParam));
    if (eqFilterParam)           engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_EQ, 3, getNorm(eqFilterParam));

    // 19. Amp
    if (ampLevelParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 0, getNorm(ampLevelParam));
    if (ampPanParam)          engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 1, getNorm(ampPanParam));
    if (ampDriveParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 2, getNorm(ampDriveParam));
    if (ampLimiterParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 3, static_cast<float>(ampLimiterParam->getIndex()));

    // 20. Amp Envelope
    if (ampEnvClapsParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, getNorm(ampEnvClapsParam));
    if (ampEnvClapSpeedParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, getNorm(ampEnvClapSpeedParam));
    if (ampEnvSlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, getNorm(ampEnvSlopeParam));
    if (ampEnvDecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, getNorm(ampEnvDecayParam));

    // 21. Velocity
    if (velSlopeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 0, getNorm(velSlopeParam));
    if (velDecayParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, getNorm(velDecayParam));
    if (velDepthParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, getNorm(velDepthParam));
    if (velVolumeParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, getNorm(velVolumeParam));

    // 22. Slop
    if (slopFreqParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 0, getNorm(slopFreqParam));
    if (slopDepthParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 1, getNorm(slopDepthParam));
    if (slopDecayParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 2, getNorm(slopDecayParam));
    if (slopPanParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 3, getNorm(slopPanParam));

    int numSamples = buffer.getNumSamples();
    float* left = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    int currentSample = 0;

    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        int eventSample = static_cast<int>(metadata.samplePosition);

        // Sanity check eventSample (should be within [0, numSamples])
        if (eventSample < currentSample) eventSample = currentSample;
        if (eventSample > numSamples) eventSample = numSamples;

        // Process audio up to this event
        if (eventSample > currentSample) {
            int samplesToProcess = eventSample - currentSample;
            float* leftPtr = left ? left + currentSample : nullptr;
            float* rightPtr = right ? right + currentSample : nullptr;
            engine.processStereo(leftPtr, rightPtr, samplesToProcess);
            currentSample = eventSample;
        }

        // Handle MIDI triggers
        if (msg.isNoteOn()) {
            engine.setMidiPitch(msg.getNoteNumber());
            engine.trigger(msg.getFloatVelocity());
        }
    }

    // Process remaining samples in the block
    if (currentSample < numSamples) {
        int samplesToProcess = numSamples - currentSample;
        float* leftPtr = left ? left + currentSample : nullptr;
        float* rightPtr = right ? right + currentSample : nullptr;
        engine.processStereo(leftPtr, rightPtr, samplesToProcess);
    }
}

juce::AudioProcessorEditor* TheKlangFarmerAudioProcessor::createEditor() {
    return new TheKlangFarmerAudioProcessorEditor(*this);
}

bool TheKlangFarmerAudioProcessor::hasEditor() const {
    return true;
}

const juce::String TheKlangFarmerAudioProcessor::getName() const {
    return "The Klang Farmer";
}

bool TheKlangFarmerAudioProcessor::acceptsMidi() const {
    return true;
}

bool TheKlangFarmerAudioProcessor::producesMidi() const {
    return false;
}

bool TheKlangFarmerAudioProcessor::isMidiEffect() const {
    return false;
}

double TheKlangFarmerAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int TheKlangFarmerAudioProcessor::getNumPrograms() {
    return 1;
}

int TheKlangFarmerAudioProcessor::getCurrentProgram() {
    return 0;
}

void TheKlangFarmerAudioProcessor::setCurrentProgram(int) {
}

const juce::String TheKlangFarmerAudioProcessor::getProgramName(int) {
    return {};
}

void TheKlangFarmerAudioProcessor::changeProgramName(int, const juce::String&) {
}

void TheKlangFarmerAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TheKlangFarmerAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout TheKlangFarmerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto makeFloatParam = [](const juce::String& id, const juce::String& name, float defaultVal) {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0005f), defaultVal);
    };

    // --- 1. CARRIER 1 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("carrier1_tracking", 1), "Carrier 1: Tracking",
        juce::StringArray{ "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 2)); // default: MIDI Pitch
    layout.add(makeFloatParam("carrier1_pitch", "Carrier 1: Pitch / Freq", 0.5f));
    layout.add(makeFloatParam("carrier1_shape", "Carrier 1: Shape", 0.0f));         // Sine (0%)
    layout.add(makeFloatParam("carrier1_depth", "Carrier 1: Modulation Depth", 0.5f)); // 0% Depth (-200% to +200%)

    // --- 2. MODULATOR 1 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod1_track", 1), "Modulator 1: Pitch Tracking",
        juce::StringArray{ "Fixed", "Following", "FM Operator" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod1_type", 1), "Modulator 1: Type",
        juce::StringArray{ "Oscillator", "Cyclic", "Noise" }, 0));
    layout.add(makeFloatParam("mod1_shape", "Modulator 1: Shape", 0.0f));          // Sine (0%)
    layout.add(makeFloatParam("mod1_speed", "Modulator 1: Speed", 0.50934f));      // 55 Hz (0.50934)

    // --- 3. PITCH ENVELOPE 1 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pitchenv1_target", 1), "PitchEnv 1: Target",
        juce::StringArray{ "Off", "Carrier", "Modulator", "Both" }, 0));
    layout.add(makeFloatParam("pitchenv1_slope", "PitchEnv 1: Slope", 0.0f));      // Exponential
    layout.add(makeFloatParam("pitchenv1_depth", "PitchEnv 1: Depth", 0.5f));      // 0%
    layout.add(makeFloatParam("pitchenv1_decay", "PitchEnv 1: Decay", 0.3806f));   // 333 ms

    // --- 4. CARRIER 2 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("carrier2_tracking", 1), "Carrier 2: Tracking",
        juce::StringArray{ "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 2)); // default: MIDI Pitch
    layout.add(makeFloatParam("carrier2_pitch", "Carrier 2: Pitch / Freq", 0.5f));
    layout.add(makeFloatParam("carrier2_shape", "Carrier 2: Shape", 0.0f));         // Sine (0%)
    layout.add(makeFloatParam("carrier2_depth", "Carrier 2: Modulation Depth", 0.5f)); // 0% Depth (-200% to +200%)

    // --- 5. MODULATOR 2 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod2_track", 1), "Modulator 2: Pitch Tracking",
        juce::StringArray{ "Fixed", "Following", "FM Operator" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod2_type", 1), "Modulator 2: Type",
        juce::StringArray{ "Oscillator", "Cyclic", "Noise" }, 0));
    layout.add(makeFloatParam("mod2_shape", "Modulator 2: Shape", 0.0f));          // Sine (0%)
    layout.add(makeFloatParam("mod2_speed", "Modulator 2: Speed", 0.50934f));      // 55 Hz (0.50934)

    // --- 6. PITCH ENVELOPE 2 ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pitchenv2_target", 1), "PitchEnv 2: Target",
        juce::StringArray{ "Off", "Carrier", "Modulator", "Both" }, 0));
    layout.add(makeFloatParam("pitchenv2_slope", "PitchEnv 2: Slope", 0.0f));      // Exponential
    layout.add(makeFloatParam("pitchenv2_depth", "PitchEnv 2: Depth", 0.5f));      // 0%
    layout.add(makeFloatParam("pitchenv2_decay", "PitchEnv 2: Decay", 0.3806f));   // 333 ms

    // --- 7. NOISE TRANSIENT ---
    layout.add(makeFloatParam("noise_sh_rate", "Noise: S&H Rate", 1.0f));       // 24 kHz
    layout.add(makeFloatParam("noise_filter", "Noise: Filter", 0.5f));          // Flat (50%)
    layout.add(makeFloatParam("noise_drive", "Noise: Drive", 0.5f));            // 0 dB (default)
    layout.add(makeFloatParam("noise_decay", "Noise: Decay", 0.3078f));         // 100 ms

    // --- 8. MIXER ---
    layout.add(makeFloatParam("mixer_carrier1_level", "Mixer: Carrier 1 Level", 0.5f)); // 100%
    layout.add(makeFloatParam("mixer_carrier2_level", "Mixer: Carrier 2 Level", 0.0f)); // 0% (double click returns 100%)
    layout.add(makeFloatParam("mixer_ringmod", "Mixer: Ring Mod Level", 0.0f));        // 0% (double click returns 100%)
    layout.add(makeFloatParam("mixer_noise_level", "Mixer: Noise Level", 0.0f));       // 0% (double click returns 100%)

    // --- 9. DRIVE ---
    layout.add(makeFloatParam("drive_amount", "Drive: Amount", 0.5f));              // 0 dB
    layout.add(makeFloatParam("drive_bias", "Drive: Bias", 0.5f));                  // 0 DC Bias
    layout.add(makeFloatParam("drive_filter", "Drive: Filter", 0.5f));              // Flat (50%)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("drive_limiter", 1), "Drive: Limiter",
        juce::StringArray{ "Off", "On" }, 1));                                      // On

    // --- VOICE 1 FILTER & FILTER ENVELOPE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter1_type", 1), "Filter 1: Type",
        juce::StringArray{ "Off", "LPF", "BPF", "HPF", "BRF" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter1_slope", 1), "Filter 1: Slope",
        juce::StringArray{ "-6dB/oct", "-12dB/oct", "-18dB/oct", "-24dB/oct", "-36dB/oct" }, 1));
    layout.add(makeFloatParam("filter1_cutoff", "Filter 1: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter1_resonance", "Filter 1: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv1_slope", "FilterEnv 1: Slope", 0.0f));
    layout.add(makeFloatParam("filterenv1_depth", "FilterEnv 1: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv1_decay", "FilterEnv 1: Decay", 0.3806f));
    layout.add(makeFloatParam("filterenv1_postdrive", "FilterEnv 1: Post-Drive", 0.5f));

    // --- VOICE 2 FILTER & FILTER ENVELOPE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter2_type", 1), "Filter 2: Type",
        juce::StringArray{ "Off", "LPF", "BPF", "HPF", "BRF" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter2_slope", 1), "Filter 2: Slope",
        juce::StringArray{ "-6dB/oct", "-12dB/oct", "-18dB/oct", "-24dB/oct", "-36dB/oct" }, 1));
    layout.add(makeFloatParam("filter2_cutoff", "Filter 2: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter2_resonance", "Filter 2: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv2_slope", "FilterEnv 2: Slope", 0.0f));
    layout.add(makeFloatParam("filterenv2_depth", "FilterEnv 2: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv2_decay", "FilterEnv 2: Decay", 0.3806f));
    layout.add(makeFloatParam("filterenv2_postdrive", "FilterEnv 2: Post-Drive", 0.5f));

    // --- TRANSIENTS FILTER & FILTER ENVELOPE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter3_type", 1), "Filter 3: Type",
        juce::StringArray{ "Off", "LPF", "BPF", "HPF", "BRF" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter3_slope", 1), "Filter 3: Slope",
        juce::StringArray{ "-6dB/oct", "-12dB/oct", "-18dB/oct", "-24dB/oct", "-36dB/oct" }, 1));
    layout.add(makeFloatParam("filter3_cutoff", "Filter 3: Cutoff", 1.0f));
    layout.add(makeFloatParam("filter3_resonance", "Filter 3: Resonance", 0.0f));

    layout.add(makeFloatParam("filterenv3_slope", "FilterEnv 3: Slope", 0.0f));
    layout.add(makeFloatParam("filterenv3_depth", "FilterEnv 3: Depth", 0.5f));
    layout.add(makeFloatParam("filterenv3_decay", "FilterEnv 3: Decay", 0.3078f));
    layout.add(makeFloatParam("filterenv3_postdrive", "FilterEnv 3: Post-Drive", 0.5f));

    // --- STANDALONE FX FILTER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("fxfilter_type", 1), "FX Filter: Type",
        juce::StringArray{ "Off", "LPF", "BPF", "HPF", "BRF" }, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("fxfilter_slope", 1), "FX Filter: Slope",
        juce::StringArray{ "-6dB/oct", "-12dB/oct", "-18dB/oct", "-24dB/oct", "-36dB/oct" }, 1));
    layout.add(makeFloatParam("fxfilter_cutoff", "FX Filter: Cutoff", 1.0f));
    layout.add(makeFloatParam("fxfilter_resonance", "FX Filter: Resonance", 0.0f));

    // --- LIMITERS ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pre_limiter_enable", 1), "Pre-Limiter: Enable",
        juce::StringArray{ "Off", "On" }, 1));
    layout.add(makeFloatParam("pre_limiter_gain", "Pre-Limiter: Input Gain", 12.0f / 36.0f)); // 0 dB
    layout.add(makeFloatParam("pre_limiter_thresh", "Pre-Limiter: Threshold", 1.0f));          // 0 dB
    layout.add(makeFloatParam("pre_limiter_release", "Pre-Limiter: Release", 0.6296f));       // 50 ms

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("post_limiter_enable", 1), "Post-Limiter: Enable",
        juce::StringArray{ "Off", "On" }, 1));
    layout.add(makeFloatParam("post_limiter_gain", "Post-Limiter: Input Gain", 12.0f / 36.0f)); // 0 dB
    layout.add(makeFloatParam("post_limiter_thresh", "Post-Limiter: Threshold", 1.0f));          // 0 dB
    layout.add(makeFloatParam("post_limiter_release", "Post-Limiter: Release", 0.6296f));       // 50 ms

    // --- FX PICKERS ---
    const juce::StringArray fxChoices { "None", "Drive", "Filter", "Wave Folder", "RingMod", "Frequency Shifter", "Grit FX", "Comb Filter", "Disperser", "Bell EQ", "Chorus", "Phaser", "Flanger", "Tempo Delay" };
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pre_fx_1_type", 1), "Pre FX 1: Type", fxChoices, 1)); // Drive
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pre_fx_2_type", 1), "Pre FX 2: Type", fxChoices, 3)); // Wave Folder
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pre_fx_3_type", 1), "Pre FX 3: Type", fxChoices, 4)); // RingMod
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pre_fx_4_type", 1), "Pre FX 4: Type", fxChoices, 5)); // Frequency Shifter

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("post_fx_1_type", 1), "Post FX 1: Type", fxChoices, 6)); // Grit FX
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("post_fx_2_type", 1), "Post FX 2: Type", fxChoices, 7)); // Comb Filter
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("post_fx_3_type", 1), "Post FX 3: Type", fxChoices, 8)); // Disperser
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("post_fx_4_type", 1), "Post FX 4: Type", fxChoices, 9)); // Bell EQ

    // --- 32 MULTI-INSTANCE FX SLOT PARAMETERS ---
    for (int s = 0; s < 4; ++s) {
        for (int p = 0; p < 4; ++p) {
            juce::String preId = "pre_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            juce::String preName = "Pre FX " + juce::String(s + 1) + ": Param " + juce::String(p + 1);
            float defPre = 0.5f;
            if (s == 0) { const float d[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; defPre = d[p]; }
            else if (s == 1) { const float d[4] = { 0.0f, 0.0f, 0.5f, 0.5f }; defPre = d[p]; }
            else if (s == 2) { const float d[4] = { 0.0f, 0.50934f, 0.0f, 0.5f }; defPre = d[p]; }
            else if (s == 3) { const float d[4] = { 0.5f, TbdAudio::rangeHzToNorm(3.0f), 0.5f, 0.5f }; defPre = d[p]; }
            layout.add(makeFloatParam(preId, preName, defPre));

            juce::String postId = "post_fx_" + juce::String(s + 1) + "_p" + juce::String(p + 1);
            juce::String postName = "Post FX " + juce::String(s + 1) + ": Param " + juce::String(p + 1);
            float defPost = 0.5f;
            if (s == 0) { const float d[4] = { 1.0f, 1.0f, 0.5f, 0.5f }; defPost = d[p]; }
            else if (s == 1) { const float d[4] = { 0.0f, 1.0f, 1.0f, 0.5f }; defPost = d[p]; }
            else if (s == 2) { const float d[4] = { 0.0f, 4.0f / 32.0f, 0.62124f, 0.5f }; defPost = d[p]; }
            else if (s == 3) { const float d[4] = { 1.0f, 0.0f, 0.5f, 0.5f }; defPost = d[p]; }
            layout.add(makeFloatParam(postId, postName, defPost));
        }
    }

    // --- 12. WAVE FOLDER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("wavefolder_type", 1), "WaveFolder: Type",
        juce::StringArray{ "Off", "On" }, 0));
    layout.add(makeFloatParam("wavefolder_fold", "WaveFolder: Fold", 0.0f));        // 0 folds
    layout.add(makeFloatParam("wavefolder_bias", "WaveFolder: Bias", 0.5f));        // 0 DC Bias
    layout.add(makeFloatParam("wavefolder_filter", "WaveFolder: Filter", 0.5f));    // Flat (50%)

    // --- 13. RING MOD ---
    layout.add(makeFloatParam("ringmod_shape", "RingMod: Waveform", 0.0f));     // Sine
    layout.add(makeFloatParam("ringmod_rate", "RingMod: Rate", 0.50934f));      // 55 Hz (0.50934)
    layout.add(makeFloatParam("ringmod_amount", "RingMod: Amount", 0.0f));      // 0% (Dry)
    layout.add(makeFloatParam("ringmod_width", "RingMod: Width", 0.5f));        // Center (0%)

    // --- 14. FREQUENCY SHIFTER ---
    layout.add(makeFloatParam("freqshift_shift", "FreqShift: Shift", 0.5f));    // 0 Hz
    layout.add(makeFloatParam("freqshift_range", "FreqShift: Range", TbdAudio::rangeHzToNorm(3.0f))); // 3 Hz default
    layout.add(makeFloatParam("freqshift_blend", "FreqShift: Blend", 0.5f));    // Dry (0%)
    layout.add(makeFloatParam("freqshift_width", "FreqShift: Width", 0.5f));    // Center (0%)

    // --- 15. GRIT FX ---
    layout.add(makeFloatParam("grit_bits", "Grit: Bit Reduction", 1.0f));       // 16.0 bits
    layout.add(makeFloatParam("grit_rate", "Grit: Sample Rate", 1.0f));         // 24 kHz
    layout.add(makeFloatParam("grit_low", "Grit: Low Shelf", 0.5f));            // 0 dB
    layout.add(makeFloatParam("grit_high", "Grit: High Shelf", 0.5f));          // 0 dB

    // --- 16. COMB FILTER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("comb_type", 1), "Comb: Type",
        juce::StringArray{ "Off", "On" }, 0));
    layout.add(makeFloatParam("comb_dampening", "Comb: Dampening", 1.0f));      // 24 kHz
    layout.add(makeFloatParam("comb_cutoff", "Comb: Cutoff", 1.0f));            // 24 kHz
    layout.add(makeFloatParam("comb_resonance", "Comb: Resonance", 0.5f));      // 0% (bipolar center)

    // --- 17. DISPERSER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("disperser_type", 1), "Disperser: Type",
        juce::StringArray{ "Off", "On" }, 0));
    layout.add(makeFloatParam("disperser_amount", "Disperser: Amount", 4.0f / 32.0f)); // 4 APFs
    layout.add(makeFloatParam("disperser_cutoff", "Disperser: Cutoff", 0.62124f));     // 220 Hz
    layout.add(makeFloatParam("disperser_resonance", "Disperser: Resonance", 0.5f));   // 0% (bipolar center)

    // --- 18. EQ (bell EQ) ---
    layout.add(makeFloatParam("eq_freq", "EQ: Frequency", 1.0f));               // 24 kHz
    layout.add(makeFloatParam("eq_width", "EQ: Width", 0.0f));                  // 0.1 octaves
    layout.add(makeFloatParam("eq_gain", "EQ: Gain", 0.5f));                    // 0 dB
    layout.add(makeFloatParam("eq_filter", "EQ: DJ Filter", 0.5f));             // Flat (50%)

    // --- 19. AMP ---
    layout.add(makeFloatParam("amp_level", "Amp: Level", 1.0f));                // 100%
    layout.add(makeFloatParam("amp_pan", "Amp: Pan", 0.5f));                    // Center
    layout.add(makeFloatParam("amp_drive", "Amp: Drive", 0.5f));                // 0 dB
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("amp_limiter", 1), "Amp: Limiter",
        juce::StringArray{ "Off", "On" }, 1));                                  // On

    // --- 20. AMP ENVELOPE ---
    layout.add(makeFloatParam("ampenv_claps", "AmpEnv: Claps", 0.0f));          // 0 claps
    layout.add(makeFloatParam("ampenv_clapspeed", "AmpEnv: Clap Speed", 0.1429f)); // 3 ms
    layout.add(makeFloatParam("ampenv_slope", "AmpEnv: Slope", 0.0f));          // Exponential
    layout.add(makeFloatParam("ampenv_decay", "AmpEnv: Decay", 0.3806f));       // 333 ms

    // --- 21. VELOCITY ---
    layout.add(makeFloatParam("vel_slope", "Velocity: Slope", 0.0f));            // Exponential
    layout.add(makeFloatParam("vel_decay", "Velocity: Decay", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("vel_depth", "Velocity: Depth", 0.5f));            // 0% (bipolar center)
    layout.add(makeFloatParam("vel_volume", "Velocity: Volume", 0.0f));          // 0% (unipolar min)

    // --- 22. SLOP ---
    layout.add(makeFloatParam("slop_freq", "Slop: Frequency", 0.0f));           // 0% (unipolar min)
    layout.add(makeFloatParam("slop_depth", "Slop: Depth", 0.0f));              // 0% (unipolar min)
    layout.add(makeFloatParam("slop_decay", "Slop: Decay", 0.0f));              // 0% (unipolar min)
    layout.add(makeFloatParam("slop_pan", "Slop: Pan", 0.0f));                  // 0% (unipolar min)

    return layout;
}

// JUCE plugin entry point factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new TheKlangFarmerAudioProcessor();
}