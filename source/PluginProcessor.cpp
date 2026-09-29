#include "PluginProcessor.h"
#include "PluginEditor.h"

BiaEr1AudioProcessor::BiaEr1AudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Retrieve direct raw parameter pointers
    carrierTrackingParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("carrier_tracking"));
    carrierPitchParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_pitch"));
    carrierShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_shape"));
    carrierLevelParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("carrier_level"));

    modTypeParam         = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("mod_type"));
    modShapeParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_shape"));
    modDepthParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_depth"));
    modSpeedParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("mod_speed"));

    driveTypeParam       = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("drive_type"));
    driveAmountParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_amount"));
    driveBiasParam       = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_bias"));
    driveFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("drive_filter"));

    noiseShRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_sh_rate"));
    noiseFilterParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_filter"));
    noiseLevelParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_level"));
    noiseDecayParam      = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("noise_decay"));

    filterTypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filter_type"));
    filterCutoffParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_cutoff"));
    filterDepthParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_depth"));
    filterDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filter_decay"));

    ringModShapeParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_shape"));
    ringModRateParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_rate"));
    ringModAmountParam   = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_amount"));
    ringModWidthParam    = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("ringmod_width"));

    gritBitsParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_bits"));
    gritRateParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("grit_rate"));

    freqShiftShiftParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_shift"));
    freqShiftRangeParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_range"));
    freqShiftBlendParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_blend"));
    freqShiftWidthParam  = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqshift_width"));

    ampPanParam          = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_pan"));
    ampLevelParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_level"));
    ampDriveParam        = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_drive"));
    ampLowBoostParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_low_boost"));

    ampEnvTypeParam      = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("amp_env_type"));
    ampEnvClapsParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_env_claps"));
    ampEnvShapeParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_env_shape"));
    ampEnvDecayParam     = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("amp_env_decay"));
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

    // Update block parameters using normalized values [0.0, 1.0]
    if (carrierTrackingParam) engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 0, static_cast<float>(carrierTrackingParam->getIndex()) / 3.0f);
    if (carrierPitchParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 1, getNorm(carrierPitchParam));
    if (carrierShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 2, getNorm(carrierShapeParam));
    if (carrierLevelParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER, 3, getNorm(carrierLevelParam));

    if (modTypeParam)         engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 0, static_cast<float>(modTypeParam->getIndex()) / 7.0f);
    if (modShapeParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 1, getNorm(modShapeParam));
    if (modDepthParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 2, getNorm(modDepthParam));
    if (modSpeedParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODULATOR, 3, getNorm(modSpeedParam));

    if (driveTypeParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 0, static_cast<float>(driveTypeParam->getIndex()) / 3.0f);
    if (driveAmountParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 1, getNorm(driveAmountParam));
    if (driveBiasParam)       engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 2, getNorm(driveBiasParam));
    if (driveFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DRIVE, 3, getNorm(driveFilterParam));

    if (noiseShRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 0, getNorm(noiseShRateParam));
    if (noiseFilterParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 1, getNorm(noiseFilterParam));
    if (noiseLevelParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 2, getNorm(noiseLevelParam));
    if (noiseDecayParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_NOISE, 3, getNorm(noiseDecayParam));

    if (filterTypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, static_cast<float>(filterTypeParam->getIndex()) / 9.0f);
    if (filterCutoffParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, getNorm(filterCutoffParam));
    if (filterDepthParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, getNorm(filterDepthParam));
    if (filterDecayParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, getNorm(filterDecayParam));

    if (ringModShapeParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 0, getNorm(ringModShapeParam));
    if (ringModRateParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 1, getNorm(ringModRateParam));
    if (ringModAmountParam)   engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 2, getNorm(ringModAmountParam));
    if (ringModWidthParam)    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_RINGMOD, 3, getNorm(ringModWidthParam));

    if (gritBitsParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 0, getNorm(gritBitsParam));
    if (gritRateParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_GRIT, 1, getNorm(gritRateParam));

    if (freqShiftShiftParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 0, getNorm(freqShiftShiftParam));
    if (freqShiftRangeParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 1, getNorm(freqShiftRangeParam));
    if (freqShiftBlendParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 2, getNorm(freqShiftBlendParam));
    if (freqShiftWidthParam)  engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 3, getNorm(freqShiftWidthParam));

    if (ampPanParam)          engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 0, getNorm(ampPanParam));
    if (ampLevelParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 1, getNorm(ampLevelParam));
    if (ampDriveParam)        engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 2, getNorm(ampDriveParam));
    if (ampLowBoostParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMP, 3, getNorm(ampLowBoostParam));

    if (ampEnvTypeParam)      engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, static_cast<float>(ampEnvTypeParam->getIndex()));
    if (ampEnvClapsParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, getNorm(ampEnvClapsParam));
    if (ampEnvShapeParam)     engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, getNorm(ampEnvShapeParam));
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
        juce::StringArray{ "Fixed Freq", "Fixed Pitch", "MIDI Pitch", "Fixed Noise" }, 0));
    layout.add(makeFloatParam("carrier_pitch", "Carrier: Pitch / Freq", 0.173f)); // ~65 Hz
    layout.add(makeFloatParam("carrier_shape", "Carrier: Shape", 0.0f));         // Sine
    layout.add(makeFloatParam("carrier_level", "Carrier: Level", 0.5f));         // 100%

    // --- 2. MODULATOR ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mod_type", 1), "Modulator: Type",
        juce::StringArray{ "Fixed Osc", "Follow Osc", "FM Ratio", "Sine x Noise",
                           "Follow x Noise", "S&H Noise", "Fast Decay", "Slow Decay" }, 0));
    layout.add(makeFloatParam("mod_shape", "Modulator: Shape", 0.0f));          // Sine
    layout.add(makeFloatParam("mod_depth", "Modulator: Depth", 0.5f));          // 0% Depth
    layout.add(makeFloatParam("mod_speed", "Modulator: Speed", 0.5f));

    // --- 3. DRIVE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("drive_type", 1), "Drive: Type",
        juce::StringArray{ "Off", "Saturation", "Clipper", "Wave Folder" }, 0));
    layout.add(makeFloatParam("drive_amount", "Drive: Amount", 0.20f));         // 0 dB Saturation
    layout.add(makeFloatParam("drive_bias", "Drive: Bias", 0.5f));              // 0 DC Bias
    layout.add(makeFloatParam("drive_filter", "Drive: Filter", 0.5f));          // Flat

    // --- 4. NOISE TRANSIENT ---
    layout.add(makeFloatParam("noise_sh_rate", "Noise: S&H Rate", 0.755f));     // 1000 Hz
    layout.add(makeFloatParam("noise_filter", "Noise: Filter", 0.5f));          // Flat
    layout.add(makeFloatParam("noise_level", "Noise: Level", 0.0f));            // 0%
    layout.add(makeFloatParam("noise_decay", "Noise: Decay", 0.297f));          // 100 ms

    // --- 5. FILTER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter_type", 1), "Filter: Type",
        juce::StringArray{ "NoRez LPF", "NoRez BPF", "NoRez HPF", "NoRez Notch",
                           "Rezzy LPF", "Rezzy BPF", "Rezzy HPF", "Rezzy Notch", "Comb", "APF Disperser" }, 0));
    layout.add(makeFloatParam("filter_cutoff", "Filter: Cutoff", 0.90f));       // Open ~10 kHz
    layout.add(makeFloatParam("filter_depth", "Filter: Depth / Res", 0.5f));    // 0%
    layout.add(makeFloatParam("filter_decay", "Filter: Decay / Damp / Stages", 0.5f));

    // --- 6. RING MOD ---
    layout.add(makeFloatParam("ringmod_shape", "RingMod: Waveform", 0.0f));     // Sine
    layout.add(makeFloatParam("ringmod_rate", "RingMod: Rate", 0.638f));        // 100 Hz
    layout.add(makeFloatParam("ringmod_amount", "RingMod: Amount", 0.0f));      // 0% (Dry)
    layout.add(makeFloatParam("ringmod_width", "RingMod: Width", 0.5f));        // Center

    // --- 7. GRIT FX ---
    layout.add(makeFloatParam("grit_bits", "Grit: Bit Reduction", 1.0f));       // 16 bits (Clean)
    layout.add(makeFloatParam("grit_rate", "Grit: Sample Rate", 1.0f));         // 20 kHz (Clean)

    // --- 8. FREQUENCY SHIFTER ---
    layout.add(makeFloatParam("freqshift_shift", "FreqShift: Shift", 0.5f));    // 0 Hz
    layout.add(makeFloatParam("freqshift_range", "FreqShift: Range", 0.20f));   // 1000 Hz
    layout.add(makeFloatParam("freqshift_blend", "FreqShift: Blend", 0.5f));    // Dry
    layout.add(makeFloatParam("freqshift_width", "FreqShift: Width", 0.5f));    // Center

    // --- 9. AMP ---
    layout.add(makeFloatParam("amp_pan", "Amp: Pan", 0.5f));                    // Center
    layout.add(makeFloatParam("amp_level", "Amp: Level", 0.5f));                // 100%
    layout.add(makeFloatParam("amp_drive", "Amp: Drive", 0.20f));               // 0 dB
    layout.add(makeFloatParam("amp_low_boost", "Amp: Low Boost", 0.0f));        // 0 dB

    // --- 10. AMP ENVELOPE ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("amp_env_type", 1), "AmpEnv: Type",
        juce::StringArray{ "Fast Decay", "Slow Decay" }, 0));
    layout.add(makeFloatParam("amp_env_claps", "AmpEnv: Claps", 0.0f));         // 1 clap
    layout.add(makeFloatParam("amp_env_shape", "AmpEnv: Shape", 0.5f));         // Linear slope
    layout.add(makeFloatParam("amp_env_decay", "AmpEnv: Decay", 0.5f));         // ~333 ms

    return layout;
}

// JUCE plugin entry point factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new BiaEr1AudioProcessor();
}