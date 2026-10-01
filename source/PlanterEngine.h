#pragma once
#include "ModularBlocks.h"

namespace TbdAudio {

// Dedicated Amp block for The Klang Planter
// Features: Level up to 200% before limiter, Drive before limiter, Limiter (bypass/limit), Pan post-limiter.
class PlanterAmpBlock : public DSPBlock {
public:
    void init(const BlockContext& c) override {
        sampleRate = c.sampleRate;
    }

    void trigger(float velocity) override {
        vel = velocity;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Level: 0% to 200% (normalized 0.0 to 1.0, where 0.5 = 100% / 0 dB, 1.0 = 200% / +6 dB)
        float level = params[0] * 2.0f;

        // 2. Pan: 100% L to Center to 100% R (def Center = 0.5)
        float pan = std::clamp(params[1], 0.0f, 1.0f);
        float gainL = std::cos(pan * 1.57079632679f);
        float gainR = std::sin(pan * 1.57079632679f);

        // 3. Drive: -6dB to +24dB (def 0.5 = +9dB) - comes before limiter
        float driveDb = normToDriveDb(params[2]);
        float driveGain = normToDriveGain(params[2]);
        bool hasDrive = (std::abs(driveDb) > 0.05f);

        // 4. Limiter: 0=Bypass, 1=Limit (def Limit = 1.0) - comes after Level & Drive
        bool hasLimiter = (params[3] >= 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = (i < static_cast<int>(ctx.ampEnvSignal.size())) ? ctx.ampEnvSignal[i] : 1.0f;
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Apply amp envelope, output gain (Level) and drive before limiter
            float curL = inL * envVal * level;
            float curR = inR * envVal * level;

            if (hasDrive) {
                curL *= driveGain;
                curR *= driveGain;
            }

            if (hasLimiter) {
                curL = std::tanh(curL);
                curR = std::tanh(curR);
            }

            // Pan post-limiter to preserve exact stereo balance
            if (left)  left[i]  = curL * gainL;
            if (right) right[i] = curR * gainR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float vel = 1.0f;
};

// --- THE KLANG PLANTER SYNTHESIS ENGINE ---
class PlanterDrumEngine {
public:
    enum BlockID {
        BLK_CARRIER = 0,
        BLK_MODULATOR,
        BLK_PITCHENV,
        BLK_NOISE,
        BLK_FILTER,
        BLK_FILTERENV,
        BLK_AMP,
        BLK_AMPENV,
        NUM_BLOCKS
    };

    PlanterDrumEngine() {
        init(44100.0f);
    }

    void init(float sampleRate) {
        ctx.sampleRate = sampleRate;
        ctx.invSr = 1.0f / sampleRate;

        carrier   = std::make_unique<CarrierBlock>(1);
        modulator = std::make_unique<ModulatorBlock>(1);
        pitchEnv  = std::make_unique<PitchEnvelopeBlock>(1);
        noise     = std::make_unique<NoiseTransientBlock>();
        filter    = std::make_unique<FilterBlock>(1);
        filterEnv = std::make_unique<FilterEnvelopeBlock>(1);
        amp       = std::make_unique<PlanterAmpBlock>();
        ampEnv    = std::make_unique<AmpEnvelopeBlock>();

        carrier->init(ctx);
        modulator->init(ctx);
        pitchEnv->init(ctx);
        noise->init(ctx);
        filter->init(ctx);
        filterEnv->init(ctx);
        amp->init(ctx);
        ampEnv->init(ctx);

        tempFML.resize(2048, 0.0f);
        tempFMR.resize(2048, 0.0f);
        tempNoiseL.resize(2048, 0.0f);
        tempNoiseR.resize(2048, 0.0f);
        tempMixL.resize(2048, 0.0f);
        tempMixR.resize(2048, 0.0f);

        ctx.mod1Signal.resize(2048, 0.0f);
        ctx.pitchEnv1Signal.resize(2048, 0.0f);
        ctx.filterEnv1Signal.resize(2048, 0.0f);
        ctx.ampEnvSignal.resize(2048, 0.0f);

        peakL.store(0.0f, std::memory_order_relaxed);
        peakR.store(0.0f, std::memory_order_relaxed);

        setDefaultParameters();
    }

    void setBpm(float bpm) { ctx.bpm = bpm; }

    void setDefaultParameters() {
        // Carrier (MIDI track, 0 st, Sine, 50% Depth)
        setBlockParameter(BLK_CARRIER, 0, 0.0f);
        setBlockParameter(BLK_CARRIER, 1, 0.5f);
        setBlockParameter(BLK_CARRIER, 2, 0.0f);
        setBlockParameter(BLK_CARRIER, 3, 0.5f);

        // Modulator (FM track, Osc type, Sine, 55Hz ratio)
        setBlockParameter(BLK_MODULATOR, 0, 2.0f / 2.0f);
        setBlockParameter(BLK_MODULATOR, 1, 0.0f);
        setBlockParameter(BLK_MODULATOR, 2, 0.0f);
        setBlockParameter(BLK_MODULATOR, 3, 0.50934f);

        // Pitch Envelope (Carrier target, Exp slope, 0 oct depth, 333ms decay)
        setBlockParameter(BLK_PITCHENV, 0, 0.5886f);
        setBlockParameter(BLK_PITCHENV, 1, 0.5f);
        setBlockParameter(BLK_PITCHENV, 2, 0.3806f);
        ctx.pitchEnv1Target = 1;

        // Noise Transient (24kHz S&H, 50% flat DJ, 0dB drive, 100ms decay)
        setBlockParameter(BLK_NOISE, 0, 1.0f);
        setBlockParameter(BLK_NOISE, 1, 0.5f);
        setBlockParameter(BLK_NOISE, 2, 0.5f);
        setBlockParameter(BLK_NOISE, 3, 0.3078f);

        // Filter (LPF, 12dB, 24kHz, 0% res)
        setBlockParameter(BLK_FILTER, 0, 0.0f);
        setBlockParameter(BLK_FILTER, 1, 1.0f / 4.0f);
        setBlockParameter(BLK_FILTER, 2, 1.0f);
        setBlockParameter(BLK_FILTER, 3, 0.0f);

        // Filter Envelope (Exp slope, 0 oct depth, 333ms decay, Crossfader 0% Center = 0.5)
        setBlockParameter(BLK_FILTERENV, 0, 0.5886f);
        setBlockParameter(BLK_FILTERENV, 1, 0.5f);
        setBlockParameter(BLK_FILTERENV, 2, 0.3806f);
        setBlockParameter(BLK_FILTERENV, 3, 0.5f); // 0% Both FM & Noise full volume

        // Amp (100% Level = 0.5, Center Pan = 0.5, 0dB Drive = 0.5, Limiter Limit = 1.0)
        setBlockParameter(BLK_AMP, 0, 0.5f);
        setBlockParameter(BLK_AMP, 1, 0.5f);
        setBlockParameter(BLK_AMP, 2, 0.5f);
        setBlockParameter(BLK_AMP, 3, 1.0f);

        // Amp Envelope (0 claps, 3ms speed, Exp slope, 333ms decay)
        setBlockParameter(BLK_AMPENV, 0, 0.0f);
        setBlockParameter(BLK_AMPENV, 1, 2.0f / 14.0f);
        setBlockParameter(BLK_AMPENV, 2, 0.5886f);
        setBlockParameter(BLK_AMPENV, 3, 0.3806f);
    }

    void setBlockParameter(BlockID block, int paramIndex, float value) {
        if (paramIndex < 0 || paramIndex >= 4) return;
        DSPBlock* b = getBlock(block);
        if (b) b->setParam(paramIndex, value);
    }

    float getBlockParameter(BlockID block, int paramIndex) const {
        if (paramIndex < 0 || paramIndex >= 4) return 0.0f;
        const DSPBlock* b = getBlock(block);
        return b ? b->getParam(paramIndex) : 0.0f;
    }

    void setPitchEnvTarget(int targetIndex) {
        ctx.pitchEnv1Target = std::clamp(targetIndex, 0, 3);
    }
    int getPitchEnvTarget() const { return ctx.pitchEnv1Target; }

    void trigger(float velocity = 1.0f) {
        noteOn(ctx.currentMidiNote > 0 ? ctx.currentMidiNote : 36, velocity);
    }

    void noteOn(int midiNote, float velocity = 1.0f) {
        ctx.isTriggered = true;
        ctx.currentMidiNote = midiNote;
        ctx.triggerVelocity = velocity;

        pitchEnv->trigger(velocity);
        modulator->trigger(velocity);
        carrier->trigger(velocity);
        noise->trigger(velocity);
        filterEnv->trigger(velocity);
        filter->trigger(velocity);
        ampEnv->trigger(velocity);
        amp->trigger(velocity);
    }

    void noteOff(int /*midiNote*/) {}

    void processStereo(float* left, float* right, int numSamples) {
        if (numSamples <= 0) return;
        ensureBufferSize(numSamples);

        // 1. Pitch Envelope
        pitchEnv->processStereo(nullptr, nullptr, numSamples, ctx);

        // 2. Modulator
        modulator->processStereo(nullptr, nullptr, numSamples, ctx);

        // 3. Carrier (FM synthesis pair)
        std::fill(tempFML.begin(), tempFML.begin() + numSamples, 0.0f);
        std::fill(tempFMR.begin(), tempFMR.begin() + numSamples, 0.0f);
        carrier->processStereo(tempFML.data(), tempFMR.data(), numSamples, ctx);

        // 4. Noise Transient
        std::fill(tempNoiseL.begin(), tempNoiseL.begin() + numSamples, 0.0f);
        std::fill(tempNoiseR.begin(), tempNoiseR.begin() + numSamples, 0.0f);
        noise->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);

        // 5. Filter Envelope
        filterEnv->processStereo(nullptr, nullptr, numSamples, ctx);

        // 6. Pre-Filter Crossfade (from Filter Envelope knob 4: params[3])
        // Bipolar: -100% (Noise only) to 0% (Both full volume) to +100% (FM pair only)
        float crossfadeNorm = filterEnv->getParam(3);
        float x = (crossfadeNorm - 0.5f) * 2.0f; // -1.0f to +1.0f
        float fmGain = 1.0f;
        float noiseGain = 1.0f;

        if (x <= 0.0f) {
            noiseGain = 1.0f;
            fmGain    = 1.0f + x;
        } else {
            noiseGain = 1.0f - x;
            fmGain    = 1.0f;
        }

        for (int i = 0; i < numSamples; ++i) {
            tempMixL[i] = tempFML[i] * fmGain + tempNoiseL[i] * noiseGain;
            tempMixR[i] = tempFMR[i] * fmGain + tempNoiseR[i] * noiseGain;
        }

        // 7. Filter (shapes both FM and Noise together!)
        filter->processStereo(tempMixL.data(), tempMixR.data(), numSamples, ctx);

        // 8. Amp Envelope & Amp (Level 0..200%, Drive, Limiter, Pan)
        ampEnv->processStereo(nullptr, nullptr, numSamples, ctx);
        amp->processStereo(tempMixL.data(), tempMixR.data(), numSamples, ctx);

        // Transfer to output buffers
        if (left)  std::copy(tempMixL.begin(), tempMixL.begin() + numSamples, left);
        if (right) std::copy(tempMixR.begin(), tempMixR.begin() + numSamples, right);

        // Master scope capture for live UI oscilloscope
        masterScope.pushBlock(left ? left : tempMixL.data(), numSamples);

        // Track peak levels for live header meters
        float maxL = 0.0f;
        float maxR = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            maxL = std::max(maxL, std::abs(tempMixL[i]));
            maxR = std::max(maxR, std::abs(tempMixR[i]));
        }

        float curL = peakL.load(std::memory_order_relaxed);
        float curR = peakR.load(std::memory_order_relaxed);
        float decay = 0.92f;
        peakL.store(std::max(maxL, curL * decay), std::memory_order_relaxed);
        peakR.store(std::max(maxR, curR * decay), std::memory_order_relaxed);

        ctx.isTriggered = false;
    }

    void getScopeData(float* dest, int count) const {
        masterScope.readLatest(dest, count);
    }

    float getPeakL() const { return peakL.load(std::memory_order_relaxed); }
    float getPeakR() const { return peakR.load(std::memory_order_relaxed); }

    DSPBlock* getBlock(BlockID id) {
        switch (id) {
            case BLK_CARRIER:   return carrier.get();
            case BLK_MODULATOR: return modulator.get();
            case BLK_PITCHENV:  return pitchEnv.get();
            case BLK_NOISE:     return noise.get();
            case BLK_FILTER:    return filter.get();
            case BLK_FILTERENV: return filterEnv.get();
            case BLK_AMP:       return amp.get();
            case BLK_AMPENV:    return ampEnv.get();
            default:            return nullptr;
        }
    }

    const DSPBlock* getBlock(BlockID id) const {
        switch (id) {
            case BLK_CARRIER:   return carrier.get();
            case BLK_MODULATOR: return modulator.get();
            case BLK_PITCHENV:  return pitchEnv.get();
            case BLK_NOISE:     return noise.get();
            case BLK_FILTER:    return filter.get();
            case BLK_FILTERENV: return filterEnv.get();
            case BLK_AMP:       return amp.get();
            case BLK_AMPENV:    return ampEnv.get();
            default:            return nullptr;
        }
    }

private:
    void ensureBufferSize(int numSamples) {
        if (static_cast<int>(tempFML.size()) < numSamples) {
            tempFML.resize(numSamples);
            tempFMR.resize(numSamples);
            tempNoiseL.resize(numSamples);
            tempNoiseR.resize(numSamples);
            tempMixL.resize(numSamples);
            tempMixR.resize(numSamples);

            ctx.mod1Signal.resize(numSamples);
            ctx.pitchEnv1Signal.resize(numSamples);
            ctx.filterEnv1Signal.resize(numSamples);
            ctx.ampEnvSignal.resize(numSamples);
        }
    }

    BlockContext ctx;

    std::unique_ptr<CarrierBlock> carrier;
    std::unique_ptr<ModulatorBlock> modulator;
    std::unique_ptr<PitchEnvelopeBlock> pitchEnv;
    std::unique_ptr<NoiseTransientBlock> noise;
    std::unique_ptr<FilterBlock> filter;
    std::unique_ptr<FilterEnvelopeBlock> filterEnv;
    std::unique_ptr<PlanterAmpBlock> amp;
    std::unique_ptr<AmpEnvelopeBlock> ampEnv;

    std::vector<float> tempFML, tempFMR;
    std::vector<float> tempNoiseL, tempNoiseR;
    std::vector<float> tempMixL, tempMixR;

    VisualScope masterScope;
    std::atomic<float> peakL{ 0.0f };
    std::atomic<float> peakR{ 0.0f };
};

} // namespace TbdAudio
