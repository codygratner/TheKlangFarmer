#include "PluginProcessor.h"
#include "PluginEditor.h"

BiaEr1AudioProcessor::BiaEr1AudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Retrieve direct raw parameter pointers
    // 1. Carrier
    carrierTrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier_tracking"));
    carrierPitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_pitch"));
    carrierShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_shape"));
    carrierDriveParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_drive"));

    // 2. Modulator
    modTypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod_type"));
    modShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_shape"));
    modDepthParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_depth"));
    modSpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_speed"));

    // 3. Pitch Envelope
    pitchEnvTargetParam  = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("pitchenv_target"));
    pitchEnvSlopeParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv_slope"));
    pitchEnvDepthParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv_depth"));
    pitchEnvDecayParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("pitchenv_decay"));

    // 4. Drive
    driveTypeParam       = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("drive_type"));
    driveAmountParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_amount"));
    driveBiasParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_bias"));
    driveFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_filter"));

    // 5. Noise Transient
    noiseShRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_sh_rate"));
    noiseFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_filter"));
    noiseDriveParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_drive"));
    noiseDecayParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_decay"));

    // 6. Mixer
    mixerCarrierLevelParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_carrier_level"));
    mixerNoiseLevelParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_noise_level"));
    mixerDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mixer_drive"));
    mixerLimiterParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mixer_limiter"));

    // 7. Filter
    filterTypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter_type"));
    filterStyleParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_style"));
    filterCutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_cutoff"));
    filterResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_resonance"));

    // 8. Filter Envelope
    filterEnvSlopeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv_slope"));
    filterEnvDepthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv_depth"));
    filterEnvDecayParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv_decay"));
    filterEnvPreDriveParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterenv_predrive"));

    // 9. RingMod
    ringModShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_shape"));
    ringModRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_rate"));
    ringModAmountParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_amount"));
    ringModWidthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_width"));

    // 10. Frequency Shifter
    freqShiftShiftParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_shift"));
    freqShiftRangeParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_range"));
    freqShiftBlendParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_blend"));
    freqShiftWidthParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_width"));

    // 11. Grit FX
    gritBitsParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_bits"));
    gritRateParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_rate"));
    gritLowBoostParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_low_boost"));
    gritHighBoostParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_high_boost"));

    // 12. Amp
    ampPanParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_pan"));
    ampLevelParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_level"));
    ampDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_drive"));
    ampLimiterParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("amp_limiter"));

    // 13. Amp Envelope
    ampEnvClapsParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_claps"));
    ampEnvClapSpeedParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_clapspeed"));
    ampEnvSlopeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_slope"));
    ampEnvDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ampenv_decay"));
}

void BiaEr1AudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    engine.init(static_cast<float>(sampleRate));
}

void BiaEr1AudioProcessor::releaseResources() {
}

bool BiaEr1AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void BiaEr1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;

    // Handle MIDI triggers
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            engine.setMidiPitch(msg.getNoteNumber());
            engine.trigger(msg.getFloatVelocity());
        }
    }

    auto getNorm = [](juce::AudioParameterFloat* p) {
        return p ? p->range.convertTo0to1(p->get()) : 0.0f;
    };

    // 1. Carrier
    if (carrierTrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 0, static_cast<float>(carrierTrackingParam->getIndex()) / 2.0f);
    if (carrierPitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 1, getNorm(carrierPitchParam));
    if (carrierShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 2, getNorm(carrierShapeParam));
    if (carrierDriveParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 3, getNorm(carrierDriveParam));

    // 2. Modulator
    if (modTypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 0, static_cast<float>(modTypeParam->getIndex()) / 6.0f);
    if (modShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 1, getNorm(modShapeParam));
    if (modDepthParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 2, getNorm(modDepthParam));
    if (modSpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 3, getNorm(modSpeedParam));

    // 3. Pitch Envelope
    if (pitchEnvTargetParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 0, static_cast<float>(pitchEnvTargetParam->getIndex()) / 3.0f);
    if (pitchEnvSlopeParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 1, getNorm(pitchEnvSlopeParam));
    if (pitchEnvDepthParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 2, getNorm(pitchEnvDepthParam));
    if (pitchEnvDecayParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 3, getNorm(pitchEnvDecayParam));

    // 4. Drive
    if (driveTypeParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 0, static_cast<float>(driveTypeParam->getIndex()) / 2.0f);
    if (driveAmountParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 1, getNorm(driveAmountParam));
    if (driveBiasParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 2, getNorm(driveBiasParam));
    if (driveFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 3, getNorm(driveFilterParam));

    // 5. Noise Transient
    if (noiseShRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 0, getNorm(noiseShRateParam));
    if (noiseFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 1, getNorm(noiseFilterParam));
    if (noiseDriveParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 2, getNorm(noiseDriveParam));
    if (noiseDecayParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 3, getNorm(noiseDecayParam));

    // 6. Mixer
    if (mixerCarrierLevelParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, getNorm(mixerCarrierLevelParam));
    if (mixerNoiseLevelParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, getNorm(mixerNoiseLevelParam));
    if (mixerDriveParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, getNorm(mixerDriveParam));
    if (mixerLimiterParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 3, static_cast<float>(mixerLimiterParam->getIndex()));

    // 7. Filter
    if (filterTypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, static_cast<float>(filterTypeParam->getIndex()) / 6.0f);
    if (filterStyleParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, getNorm(filterStyleParam));
    if (filterCutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, getNorm(filterCutoffParam));
    if (filterResonanceParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, getNorm(filterResonanceParam));

    // 8. Filter Envelope
    if (filterEnvSlopeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV, 0, getNorm(filterEnvSlopeParam));
    if (filterEnvDepthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV, 1, getNorm(filterEnvDepthParam));
    if (filterEnvDecayParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV, 2, getNorm(filterEnvDecayParam));
    if (filterEnvPreDriveParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTERENV, 3, getNorm(filterEnvPreDriveParam));

    // 9. RingMod
    if (ringModShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 0, getNorm(ringModShapeParam));
    if (ringModRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 1, getNorm(ringModRateParam));
    if (ringModAmountParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 2, getNorm(ringModAmountParam));
    if (ringModWidthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 3, getNorm(ringModWidthParam));

    // 10. Frequency Shifter
    if (freqShiftShiftParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 0, getNorm(freqShiftShiftParam));
    if (freqShiftRangeParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 1, getNorm(freqShiftRangeParam));
    if (freqShiftBlendParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 2, getNorm(freqShiftBlendParam));
    if (freqShiftWidthParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 3, getNorm(freqShiftWidthParam));

    // 11. Grit FX
    if (gritBitsParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 0, getNorm(gritBitsParam));
    if (gritRateParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 1, getNorm(gritRateParam));
    if (gritLowBoostParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 2, getNorm(gritLowBoostParam));
    if (gritHighBoostParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 3, getNorm(gritHighBoostParam));

    // 12. Amp
    if (ampPanParam)          engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 0, getNorm(ampPanParam));
    if (ampLevelParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 1, getNorm(ampLevelParam));
    if (ampDriveParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 2, getNorm(ampDriveParam));
    if (ampLimiterParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 3, static_cast<float>(ampLimiterParam->getIndex()));

    // 13. Amp Envelope
    if (ampEnvClapsParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, getNorm(ampEnvClapsParam));
    if (ampEnvClapSpeedParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, getNorm(ampEnvClapSpeedParam));
    if (ampEnvSlopeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, getNorm(ampEnvSlopeParam));
    if (ampEnvDecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, getNorm(ampEnvDecayParam));

    int numSamples = buffer.getNumSamples();
    float* left = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    float* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    engine.processStereo(left, right, numSamples);
}

juce::AudioProcessorEditor* BiaEr1AudioProcessor::createEditor() {
    return new BiaEr1AudioProcessorEditor(*this);
}

bool BiaEr1AudioProcessor::hasEditor() const {
    return true;
}

const juce::String BiaEr1AudioProcessor::getName() const {
    return "BIA ER-1 Voice";
}

bool BiaEr1AudioProcessor::acceptsMidi() const {
    return true;
}

bool BiaEr1AudioProcessor::producesMidi() const {
    return false;
}

bool BiaEr1AudioProcessor::isMidiEffect() const {
    return false;
}

double BiaEr1AudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int BiaEr1AudioProcessor::getNumPrograms() {
    return 1;
}

int BiaEr1AudioProcessor::getCurrentProgram() {
    return 0;
}

void BiaEr1AudioProcessor::setCurrentProgram(int) {
}

const juce::String BiaEr1AudioProcessor::getProgramName(int) {
    return {};
}

void BiaEr1AudioProcessor::changeProgramName(int, const juce::String&) {
}

void BiaEr1AudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BiaEr1AudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout BiaEr1AudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto makeFloatParam = [](const char* id, const char* name, float defaultVal) {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0005f), defaultVal);
    };

    // --- 1. CARRIER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("carrier_tracking", 1), "Carrier: Tracking",
        juce::StringArray{ "Fixed Freq", "Fixed Pitch", "MIDI Pitch" }, 2)); // default: MIDI Pitch
    layout.add(makeFloatParam("carrier_pitch", "Carrier: Pitch / Freq", 0.5f));
    layout.add(makeFloatParam("carrier_shape", "Carrier: Shape", 0.0f));         // Sine (0%)
    layout.add(makeFloatParam("carrier_drive", "Carrier: Drive", 0.5f));         // 0 dB (default)

    // --- 2. MODULATOR ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod_type", 1), "Modulator: Type",
        juce::StringArray{ "Fixed Osc", "Follow Osc", "FM Operator", "Fixed Sine*Noise",
                           "Follow Sine*Noise", "FM Op Sine*Noise", "S&H Noise" }, 0));
    layout.add(makeFloatParam("mod_shape", "Modulator: Shape", 0.0f));          // Sine (0%)
    layout.add(makeFloatParam("mod_depth", "Modulator: Depth", 0.5f));          // 0% Depth
    layout.add(makeFloatParam("mod_speed", "Modulator: Speed", 0.5286f));       // 55 Hz (0.5286)

    // --- 3. PITCH ENVELOPE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pitchenv_target", 1), "PitchEnv: Target",
        juce::StringArray{ "Off", "Carrier", "Modulator", "Both" }, 0));
    layout.add(makeFloatParam("pitchenv_slope", "PitchEnv: Slope", 0.0f));      // Exponential
    layout.add(makeFloatParam("pitchenv_depth", "PitchEnv: Depth", 0.5f));      // 0%
    layout.add(makeFloatParam("pitchenv_decay", "PitchEnv: Decay", 0.3806f));   // 333 ms

    // --- 4. DRIVE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("drive_type", 1), "Drive: Type",
        juce::StringArray{ "Off", "Saturation", "Wave Folder" }, 0));
    layout.add(makeFloatParam("drive_amount", "Drive: Amount", 0.5f));          // 0 dB / 0 folds
    layout.add(makeFloatParam("drive_bias", "Drive: Bias", 0.5f));              // 0 DC Bias
    layout.add(makeFloatParam("drive_filter", "Drive: Filter", 0.5f));          // Flat (50%)

    // --- 5. NOISE TRANSIENT ---
    layout.add(makeFloatParam("noise_sh_rate", "Noise: S&H Rate", 1.0f));       // 20 kHz
    layout.add(makeFloatParam("noise_filter", "Noise: Filter", 0.5f));          // Flat (50%)
    layout.add(makeFloatParam("noise_drive", "Noise: Drive", 0.5f));            // 0 dB (default)
    layout.add(makeFloatParam("noise_decay", "Noise: Decay", 0.3078f));         // 100 ms

    // --- 6. MIXER ---
    layout.add(makeFloatParam("mixer_carrier_level", "Mixer: Carrier Level", 0.5f)); // 100%
    layout.add(makeFloatParam("mixer_noise_level", "Mixer: Noise Level", 0.5f));     // 100%
    layout.add(makeFloatParam("mixer_drive", "Mixer: Drive", 0.5f));                 // 0 dB
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mixer_limiter", 1), "Mixer: Limiter",
        juce::StringArray{ "Off", "On" }, 1));                                      // On

    // --- 7. FILTER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter_type", 1), "Filter: Type",
        juce::StringArray{ "Off", "LPF", "BPF", "HPF", "Notch", "Comb", "Disperser" }, 0));
    layout.add(makeFloatParam("filter_style", "Filter: Style", 0.1667f));       // -12 dB/oct
    layout.add(makeFloatParam("filter_cutoff", "Filter: Cutoff", 1.0f));        // 20 kHz
    layout.add(makeFloatParam("filter_resonance", "Filter: Resonance", 0.0f));  // 0%

    // --- 8. FILTER ENVELOPE ---
    layout.add(makeFloatParam("filterenv_slope", "FilterEnv: Slope", 0.0f));    // Exponential
    layout.add(makeFloatParam("filterenv_depth", "FilterEnv: Depth", 0.5f));    // 0%
    layout.add(makeFloatParam("filterenv_decay", "FilterEnv: Decay", 0.3806f)); // 333 ms
    layout.add(makeFloatParam("filterenv_predrive", "FilterEnv: Pre-Drive", 0.5f)); // 0 dB

    // --- 9. RING MOD ---
    layout.add(makeFloatParam("ringmod_shape", "RingMod: Waveform", 0.0f));     // Sine
    layout.add(makeFloatParam("ringmod_rate", "RingMod: Rate", 0.5286f));       // 55 Hz
    layout.add(makeFloatParam("ringmod_amount", "RingMod: Amount", 0.0f));      // 0% (Dry)
    layout.add(makeFloatParam("ringmod_width", "RingMod: Width", 0.5f));        // Center (0%)

    // --- 10. FREQUENCY SHIFTER ---
    layout.add(makeFloatParam("freqshift_shift", "FreqShift: Shift", 0.5f));    // 0 Hz
    layout.add(makeFloatParam("freqshift_range", "FreqShift: Range", TbdAudio::rangeHzToNorm(3.0f))); // 3 Hz default
    layout.add(makeFloatParam("freqshift_blend", "FreqShift: Blend", 0.5f));    // Dry (0%)
    layout.add(makeFloatParam("freqshift_width", "FreqShift: Width", 0.5f));    // Center (0%)

    // --- 11. GRIT FX ---
    layout.add(makeFloatParam("grit_bits", "Grit: Bit Reduction", 1.0f));       // 16.0 bits
    layout.add(makeFloatParam("grit_rate", "Grit: Sample Rate", 1.0f));         // 20 kHz
    layout.add(makeFloatParam("grit_low_boost", "Grit: Low Boost", 0.0f));      // 0 dB
    layout.add(makeFloatParam("grit_high_boost", "Grit: High Boost", 0.0f));    // 0 dB

    // --- 12. AMP ---
    layout.add(makeFloatParam("amp_pan", "Amp: Pan", 0.5f));                    // Center
    layout.add(makeFloatParam("amp_level", "Amp: Level", 0.5f));                // 100%
    layout.add(makeFloatParam("amp_drive", "Amp: Drive", 0.5f));                // 0 dB
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("amp_limiter", 1), "Amp: Limiter",
        juce::StringArray{ "Off", "On" }, 1));                                  // On

    // --- 13. AMP ENVELOPE ---
    layout.add(makeFloatParam("ampenv_claps", "AmpEnv: Claps", 0.0f));          // 0 claps
    layout.add(makeFloatParam("ampenv_clapspeed", "AmpEnv: Clap Speed", 0.1429f)); // 3 ms
    layout.add(makeFloatParam("ampenv_slope", "AmpEnv: Slope", 0.0f));          // Exponential
    layout.add(makeFloatParam("ampenv_decay", "AmpEnv: Decay", 0.3806f));       // 333 ms

    return layout;
}

// JUCE plugin entry point factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BiaEr1AudioProcessor();
}