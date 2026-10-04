#pragma once
#include "ModularBlocks.h"

namespace TbdAudio {

// Dedicated Amp block for The Klang Planter
// Features: Drive (-inf..0dB..+24dB), Pan, Velocity Slope, Velocity Floor (1%..100%), permanent tanh Limiter.
class PlanterAmpBlock : public DSPBlock {
public:
    void init(const BlockContext& c) override {
        sampleRate = c.sampleRate;
        velGain = 1.0f;
        limiterActivity.store(0.0f, std::memory_order_relaxed);
    }

    void trigger(float velocity) override {
        float vNorm = std::clamp(velocity, 0.0f, 1.0f);
        float s = params[2]; // Vel Slope (0.75 = LIN, < 0.70 = EXP, > 0.80 = LOG)
        float vCurved = vNorm;
        if (s >= 0.73f && s <= 0.77f) {
            vCurved = vNorm;
        } else if (s < 0.75f) {
            float k = 1.0f + (0.75f - s) * 4.0f;
            vCurved = std::pow(vNorm, k);
        } else {
            float k = 1.0f / (1.0f + (s - 0.75f) * 4.0f);
            vCurved = std::pow(vNorm, k);
        }

        // Vel Floor: 1% (at 0.0) to 50% (at 0.5) to 100% (at 1.0)
        float floorVal = (params[3] <= 0.5f)
            ? (0.01f + params[3] * 0.98f)
            : (0.50f + (params[3] - 0.5f) * 1.0f);
        floorVal = std::clamp(floorVal, 0.01f, 1.0f);

        velGain = floorVal + (1.0f - floorVal) * vCurved;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Amp Drive: -inf dB to 0dB (at 0.5) to +24dB (at 1.0)
        float v = params[0];
        float driveGain = 1.0f;
        if (v < 0.001f) {
            driveGain = 0.0f;
        } else if (v <= 0.5f) {
            float db = (v / 0.5f - 1.0f) * 60.0f;
            driveGain = FastMath::fastDbToGain(db);
        } else {
            float db = ((v - 0.5f) / 0.5f) * 24.0f;
            driveGain = FastMath::fastDbToGain(db);
        }

        // 2. Pan: 100% L to Center to 100% R (def Center = 0.5)
        float pan = std::clamp(params[1], 0.0f, 1.0f);
        float gainL = FastMath::fastCos(pan * 1.57079632679f);
        float gainR = FastMath::fastSin(pan * 1.57079632679f);

        float blockMaxReduction = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            float envVal = (i < static_cast<int>(ctx.ampEnvSignal.size())) ? ctx.ampEnvSignal[i] : 1.0f;
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Apply envelope, velocity scaling, and amp drive
            float curL = inL * envVal * velGain * driveGain;
            float curR = inR * envVal * velGain * driveGain;

            // Limiter is permanently enabled: smooth tanh soft-saturation
            float limL = FastMath::fastTanh(curL);
            float limR = FastMath::fastTanh(curR);

            float redL = std::max(0.0f, std::abs(curL) - std::abs(limL));
            float redR = std::max(0.0f, std::abs(curR) - std::abs(limR));
            blockMaxReduction = std::max(blockMaxReduction, std::max(redL, redR));

            // Pan post-limiter to preserve exact stereo balance
            if (left)  left[i]  = limL * gainL;
            if (right) right[i] = limR * gainR;
        }

        // Smooth limiter reduction activity with ~60ms decay
        float curAct = limiterActivity.load(std::memory_order_relaxed);
        float newAct = std::max(blockMaxReduction, curAct * 0.90f);
        limiterActivity.store(newAct, std::memory_order_relaxed);
    }

    float getLimiterActivity() const {
        return limiterActivity.load(std::memory_order_relaxed);
    }

    float getVelGain() const { return velGain; }

private:
    float sampleRate = 44100.0f;
    float velGain = 1.0f;
    std::atomic<float> limiterActivity { 0.0f };
};

// Dedicated Noise Transient block for The Klang Planter
// Parameters:
//   [0]: S&H Rate (0.1 Hz to 24 kHz)
//   [1]: DJ Filter (-100% LPF to 0% Flat to +100% HPF)
//   [2]: Decay (5-point warp: 1ms, 50ms, 1s, 5s, 60s)
//   [3]: Crossfade (FM vs Noise, processed in PlanterDrumEngine::processStereo)
class PlanterNoiseTransientBlock : public DSPBlock {
public:
    void init(const BlockContext& c) override {
        invSr = c.invSr;
        sampleRate = c.sampleRate;
        timeSinceTrigger = 1000.0f;
        shPhase = 0.0f;
        shVal = 0.0f;
        djFilter.reset();
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
        shPhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        float shParam = std::clamp(params[0], 0.0f, 1.0f);
        float shRate = 0.1f * std::pow(24000.0f / 0.1f, shParam);

        float filterKnob = std::clamp(params[1], 0.0f, 1.0f);

        float decayParam = std::clamp(params[2], 0.0f, 1.0f);
        float decayTime = warpNoiseDecayTime(decayParam);
        decayTime = std::max(decayTime, 0.0005f);

        for (int i = 0; i < numSamples; ++i) {
            shPhase += shRate * invSr;
            if (shPhase >= 1.0f) {
                shPhase -= 1.0f;
                shVal = fastRng(rngState);
            }

            float env = FastMath::fastExp(-timeSinceTrigger / decayTime);
            timeSinceTrigger += invSr;

            float sample = shVal * env;
            float outL = sample;
            float outR = sample;
            djFilter.process(outL, outR, filterKnob, sampleRate);

            if (left)  left[i]  = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float shPhase = 0.0f;
    float shVal = 0.0f;
    uint32_t rngState = 123456789;
    DJFilter djFilter;
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
        noise     = std::make_unique<PlanterNoiseTransientBlock>();
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

        // Modulator (FM track, Osc type, Sine, 1:1 Ratio = 0.5)
        setBlockParameter(BLK_MODULATOR, 0, 2.0f / 2.0f);
        setBlockParameter(BLK_MODULATOR, 1, 0.0f);
        setBlockParameter(BLK_MODULATOR, 2, 0.0f);
        setBlockParameter(BLK_MODULATOR, 3, 0.5f);

        // Pitch Envelope (Carrier target, Exp slope, 0 oct depth, 177ms decay)
        setBlockParameter(BLK_PITCHENV, 0, 0.5886f);
        setBlockParameter(BLK_PITCHENV, 1, 0.5f);
        setBlockParameter(BLK_PITCHENV, 2, 0.3806f);
        setBlockParameter(BLK_PITCHENV, 3, 0.0f);
        ctx.pitchEnv1Target = 0;

        // Noise Transient (24kHz S&H, 50% flat DJ, 30ms decay, Crossfader +100% FM = 1.0)
        setBlockParameter(BLK_NOISE, 0, 1.0f);
        setBlockParameter(BLK_NOISE, 1, 0.5f);
        setBlockParameter(BLK_NOISE, 2, 0.3078f); // Decay
        setBlockParameter(BLK_NOISE, 3, 1.0f);    // Crossfade: 100% FM

        // Filter (LPF, 12dB, 24kHz, 0% res)
        setBlockParameter(BLK_FILTER, 0, 0.0f);
        setBlockParameter(BLK_FILTER, 1, 1.0f / 4.0f);
        setBlockParameter(BLK_FILTER, 2, 1.0f);
        setBlockParameter(BLK_FILTER, 3, 0.0f);

        // Filter Envelope (Exp slope, 0 oct depth, 177ms decay, Pre-Filter Drive 0dB = 0.5)
        setBlockParameter(BLK_FILTERENV, 0, 0.5886f);
        setBlockParameter(BLK_FILTERENV, 1, 0.5f);
        setBlockParameter(BLK_FILTERENV, 2, 0.3806f);
        setBlockParameter(BLK_FILTERENV, 3, 0.5f); // Pre-Filter Drive: 0.0 dB

        // Amp (0dB Drive = 0.5, Center Pan = 0.5, LIN Vel Slope = 0.75, 50% Vel Floor = 0.5)
        setBlockParameter(BLK_AMP, 0, 0.5f);  // Drive: 0.0 dB
        setBlockParameter(BLK_AMP, 1, 0.5f);  // Pan: Center
        setBlockParameter(BLK_AMP, 2, 0.75f); // Vel Slope: Linear
        setBlockParameter(BLK_AMP, 3, 0.5f);  // Velocity Floor: 50%

        // Amp Envelope (0 claps, 3ms speed, Exp slope, 177ms decay)
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
        setBlockParameter(BLK_PITCHENV, 3, static_cast<float>(ctx.pitchEnv1Target) / 3.0f);
    }
    int getPitchEnvTarget() const { return ctx.pitchEnv1Target; }

    void trigger(float velocity = 1.0f) {
        noteOn(ctx.currentMidiNote > 0 ? ctx.currentMidiNote : 36, velocity);
    }

    void noteOn(int midiNote, float velocity = 1.0f) {
        ctx.isTriggered = true;
        ctx.currentMidiNote = midiNote;
        ctx.currentPitchHz = 440.0f * FastMath::fastPow2((static_cast<float>(midiNote) - 69.0f) / 12.0f);
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

        // Update carrier base pitch for modulator tracking (Following and FM ratio)
        ctx.carrier1PitchHz = carrier->getBasePitch(ctx);

        // 1. Pitch Envelope
        pitchEnv->processStereo(nullptr, nullptr, numSamples, ctx);

        // 2. Modulator
        modulator->processStereo(nullptr, nullptr, numSamples, ctx);

        // 3. Carrier (FM synthesis pair)
        std::fill(tempFML.begin(), tempFML.begin() + numSamples, 0.0f);
        std::fill(tempFMR.begin(), tempFMR.begin() + numSamples, 0.0f);
        carrier->processStereo(tempFML.data(), tempFMR.data(), numSamples, ctx);

        // 4. Noise Transient (S&H Rate, DJ Filter, Decay)
        std::fill(tempNoiseL.begin(), tempNoiseL.begin() + numSamples, 0.0f);
        std::fill(tempNoiseR.begin(), tempNoiseR.begin() + numSamples, 0.0f);
        noise->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);

        // 5. Filter Envelope
        filterEnv->processStereo(nullptr, nullptr, numSamples, ctx);

        // 6. Pre-Filter Crossfade (from Noise Transient knob 4: params[3])
        // Bipolar: -100% (Noise only) to 0% (Both full volume) to +100% (FM pair only)
        float crossfadeNorm = noise->getParam(3);
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

        // 7. Pre-Filter Drive (from Filter Envelope knob 4: params[3])
        // -6dB to 0dB (at 0.5) to +24dB
        float pDrive = filterEnv->getParam(3);
        float preDriveDb = (pDrive <= 0.5f) ? (-6.0f + pDrive * 12.0f) : ((pDrive - 0.5f) * 48.0f);
        float preDriveGain = FastMath::fastDbToGain(preDriveDb);
        if (std::abs(preDriveGain - 1.0f) > 0.01f) {
            for (int i = 0; i < numSamples; ++i) {
                tempMixL[i] = FastMath::fastTanh(tempMixL[i] * preDriveGain);
                tempMixR[i] = FastMath::fastTanh(tempMixR[i] * preDriveGain);
            }
        }

        // 8. Filter (shapes both FM and Noise together!)
        filter->processStereo(tempMixL.data(), tempMixR.data(), numSamples, ctx);

        // 9. Amp Envelope & Amp (Drive, Pan, Vel Slope, Velocity Floor, Limiter)
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
    float getLimiterActivity() const {
        return amp ? amp->getLimiterActivity() : 0.0f;
    }

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

    PlanterAmpBlock* getAmpBlock() { return amp.get(); }

private:
    void ensureBufferSize(int numSamples) {
        if (static_cast<int>(tempFML.size()) < numSamples) {
            tempFML.resize(numSamples, 0.0f);
            tempFMR.resize(numSamples, 0.0f);
            tempNoiseL.resize(numSamples, 0.0f);
            tempNoiseR.resize(numSamples, 0.0f);
            tempMixL.resize(numSamples, 0.0f);
            tempMixR.resize(numSamples, 0.0f);
            ctx.mod1Signal.resize(numSamples, 0.0f);
            ctx.pitchEnv1Signal.resize(numSamples, 0.0f);
            ctx.filterEnv1Signal.resize(numSamples, 0.0f);
            ctx.ampEnvSignal.resize(numSamples, 0.0f);
        }
    }

    BlockContext ctx;
    std::unique_ptr<CarrierBlock> carrier;
    std::unique_ptr<ModulatorBlock> modulator;
    std::unique_ptr<PitchEnvelopeBlock> pitchEnv;
    std::unique_ptr<PlanterNoiseTransientBlock> noise;
    std::unique_ptr<FilterBlock> filter;
    std::unique_ptr<FilterEnvelopeBlock> filterEnv;
    std::unique_ptr<PlanterAmpBlock> amp;
    std::unique_ptr<AmpEnvelopeBlock> ampEnv;

    std::vector<float> tempFML, tempFMR;
    std::vector<float> tempNoiseL, tempNoiseR;
    std::vector<float> tempMixL, tempMixR;

    VisualScope masterScope;
    std::atomic<float> peakL { 0.0f };
    std::atomic<float> peakR { 0.0f };
};

} // namespace TbdAudio
