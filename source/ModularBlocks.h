#pragma once
#include "DSPBlock.h"
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>

namespace TbdAudio {

// --- HELPER MATH FUNCTIONS ---

inline float fastRng(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);
}

// Continuous waveform crossfade:
// 0%: Sine -> 20%: Tri -> 40%: Saw -> 60%: Square -> 100%: PWM 0%
inline float evaluateWaveform(float phase, float shape) {
    float p = phase - std::floor(phase);
    float s = std::sin(p * TWO_PI);
    float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
    float saw = 2.0f * p - 1.0f;
    float sq = (p < 0.5f) ? 1.0f : -1.0f;

    if (shape <= 0.20f) {
        float t = shape / 0.20f;
        return (1.0f - t) * s + t * tri;
    } else if (shape <= 0.40f) {
        float t = (shape - 0.20f) / 0.20f;
        return (1.0f - t) * tri + t * saw;
    } else if (shape <= 0.60f) {
        float t = (shape - 0.40f) / 0.20f;
        return (1.0f - t) * saw + t * sq;
    } else {
        // 60% to 100%: PWM from 50% down to 0% duty cycle with zero-mean DC normalization
        float t = (shape - 0.60f) / 0.40f;
        float duty = std::clamp(0.5f * (1.0f - t), 0.005f, 0.50f);
        float pulse = (p < duty) ? (1.0f - duty) : -duty;
        return pulse * 2.0f;
    }
}

// Envelope slope shaper: exponential -> linear -> logarithmic
inline float applyEnvelopeSlope(float linearVal, float shape) {
    linearVal = std::clamp(linearVal, 0.0f, 1.0f);
    if (shape < 0.49f) {
        float p = 1.0f + (0.49f - shape) * 6.0f;
        return std::pow(linearVal, p);
    } else if (shape > 0.51f) {
        float p = 1.0f / (1.0f + (shape - 0.51f) * 6.0f);
        return std::pow(linearVal, p);
    }
    return linearVal;
}

// Drive mapping: -6dB to 0dB (at 50% knob) to +24dB
inline float normToDriveDb(float norm) {
    norm = std::clamp(norm, 0.0f, 1.0f);
    return (norm <= 0.5f) ? (-6.0f + norm * 12.0f) : ((norm - 0.5f) * 48.0f);
}

inline float driveDbToNorm(float db) {
    if (db <= 0.0f) {
        return std::clamp((db + 6.0f) / 12.0f, 0.0f, 0.5f);
    } else {
        return std::clamp(0.5f + db / 48.0f, 0.5f, 1.0f);
    }
}

inline float normToDriveGain(float norm) {
    return std::pow(10.0f, normToDriveDb(norm) / 20.0f);
}

// Frequency Shifter Range mapping: 0 Hz to 5 kHz (cubic curve for fine sub-Hz to multi-kHz control)
inline float normToRangeHz(float norm) {
    norm = std::clamp(norm, 0.0f, 1.0f);
    return 5000.0f * norm * norm * norm;
}

inline float rangeHzToNorm(float hz) {
    return std::clamp(std::cbrt(std::clamp(hz, 0.0f, 5000.0f) / 5000.0f), 0.0f, 1.0f);
}


// 5-Point Warp Decay Time:
// 0%: 5 ms, 25%: 100 ms, 50%: 1 s, 75%: 5 s, 100%: 60 s
inline float warp5PointTime(float u) {
    u = std::clamp(u, 0.0f, 1.0f);
    if (u <= 0.25f) {
        float t = u / 0.25f;
        return 0.005f * std::pow(0.100f / 0.005f, t);
    } else if (u <= 0.50f) {
        float t = (u - 0.25f) / 0.25f;
        return 0.100f * std::pow(1.0f / 0.100f, t);
    } else if (u <= 0.75f) {
        float t = (u - 0.50f) / 0.25f;
        return 1.0f * std::pow(5.0f / 1.0f, t);
    } else {
        float t = (u - 0.75f) / 0.25f;
        return 5.0f * std::pow(60.0f / 5.0f, t);
    }
}

inline float unwarp5PointTime(float seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.005f) return 0.0f;
    if (seconds >= 60.0f) return 1.0f;
    if (seconds <= 0.100f) {
        float t = std::log(seconds / 0.005f) / std::log(0.100f / 0.005f);
        return std::clamp(t * 0.25f, 0.0f, 0.25f);
    } else if (seconds <= 1.0f) {
        float t = std::log(seconds / 0.100f) / std::log(1.0f / 0.100f);
        return std::clamp(0.25f + t * 0.25f, 0.25f, 0.50f);
    } else if (seconds <= 5.0f) {
        float t = std::log(seconds / 1.0f) / std::log(5.0f / 1.0f);
        return std::clamp(0.50f + t * 0.25f, 0.50f, 0.75f);
    } else {
        float t = std::log(seconds / 5.0f) / std::log(60.0f / 5.0f);
        return std::clamp(0.75f + t * 0.25f, 0.75f, 1.0f);
    }
}

// Noise Transient 5-Point Warp Decay Time:
// 0%: 1 ms, 25%: 50 ms, 50%: 1 s, 75%: 5 s, 100%: 60 s
inline float warpNoiseDecayTime(float u) {
    u = std::clamp(u, 0.0f, 1.0f);
    if (u <= 0.25f) {
        float t = u / 0.25f;
        return 0.001f * std::pow(0.050f / 0.001f, t);
    } else if (u <= 0.50f) {
        float t = (u - 0.25f) / 0.25f;
        return 0.050f * std::pow(1.0f / 0.050f, t);
    } else if (u <= 0.75f) {
        float t = (u - 0.50f) / 0.25f;
        return 1.0f * std::pow(5.0f / 1.0f, t);
    } else {
        float t = (u - 0.75f) / 0.25f;
        return 5.0f * std::pow(60.0f / 5.0f, t);
    }
}

inline float unwarpNoiseDecayTime(float seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.001f) return 0.0f;
    if (seconds >= 60.0f) return 1.0f;
    if (seconds <= 0.050f) {
        float t = std::log(seconds / 0.001f) / std::log(0.050f / 0.001f);
        return std::clamp(t * 0.25f, 0.0f, 0.25f);
    } else if (seconds <= 1.0f) {
        float t = std::log(seconds / 0.050f) / std::log(1.0f / 0.050f);
        return std::clamp(0.25f + t * 0.25f, 0.25f, 0.50f);
    } else if (seconds <= 5.0f) {
        float t = std::log(seconds / 1.0f) / std::log(5.0f / 1.0f);
        return std::clamp(0.50f + t * 0.25f, 0.50f, 0.75f);
    } else {
        float t = std::log(seconds / 5.0f) / std::log(60.0f / 5.0f);
        return std::clamp(0.75f + t * 0.25f, 0.75f, 1.0f);
    }
}

// DJ Style Filter: 0..49% LPF (20Hz-20kHz), 50% Flat, 51..100% HPF (20Hz-20kHz)
struct DJFilter {
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;

    void reset() {
        s1L = s2L = s1R = s2R = 0.0f;
    }

    void process(float& left, float& right, float knob, float sampleRate) {
        if (knob >= 0.49f && knob <= 0.51f) {
            return; // flat / bypass
        }

        bool isLowpass = (knob < 0.49f);
        float norm = isLowpass ? (knob / 0.49f) : ((knob - 0.51f) / 0.49f);
        float cutoff = 20.0f * std::pow(20000.0f / 20.0f, std::clamp(norm, 0.0f, 1.0f));
        float q = isLowpass ? (1.1f - norm * 0.393f) : (0.707f + norm * 0.4f);

        cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.48f);
        float g = std::tan(PI * cutoff / sampleRate);
        float k = 1.0f / q;
        float a1 = 1.0f / (1.0f + g * (g + k));

        // Left channel SVF
        float hpL = (left - (g + k) * s1L - s2L) * a1;
        float bpL = g * hpL + s1L;
        s1L = g * hpL + bpL;
        float lpL = g * bpL + s2L;
        s2L = g * bpL + lpL;
        left = isLowpass ? lpL : hpL;

        // Right channel SVF
        float hpR = (right - (g + k) * s1R - s2R) * a1;
        float bpR = g * hpR + s1R;
        s1R = g * hpR + bpR;
        float lpR = g * bpR + s2R;
        s2R = g * bpR + lpR;
        right = isLowpass ? lpR : hpR;
    }
};

// --- VISUAL OSCILLOSCOPE BUFFER ---
struct VisualScope {
    static constexpr int RING_SIZE = 4096;
    float buffer[RING_SIZE] = { 0.0f };
    std::atomic<int> writeIndex{ 0 };

    void pushBlock(const float* data, int numSamples) {
        if (!data || numSamples <= 0) return;
        int idx = writeIndex.load(std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i) {
            buffer[(idx + i) & (RING_SIZE - 1)] = data[i];
        }
        writeIndex.store((idx + numSamples) & (RING_SIZE - 1), std::memory_order_release);
    }

    void readLatest(float* outBuffer, int count) const {
        if (!outBuffer || count <= 0) return;
        int idx = writeIndex.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i) {
            int readPos = (idx - count + i + RING_SIZE * 4) & (RING_SIZE - 1);
            outBuffer[i] = buffer[readPos];
        }
    }

    void readTriggered(float* outBuffer, int count, int triggerPos, float step) const {
        if (!outBuffer || count <= 0) return;
        for (int i = 0; i < count; ++i) {
            float samplePos = static_cast<float>(triggerPos) + static_cast<float>(i) * step;
            int idx0 = static_cast<int>(std::floor(samplePos));
            float frac = samplePos - static_cast<float>(idx0);

            float s0 = buffer[idx0 & (RING_SIZE - 1)];
            float s1 = buffer[(idx0 + 1) & (RING_SIZE - 1)];
            float sample = s0 + frac * (s1 - s0);
            if (!std::isfinite(sample)) sample = 0.0f;
            outBuffer[i] = sample;
        }
    }
};

// --- BLOCK 1: CARRIER ---
class CarrierBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        phase = 0.0f;
    }

    void trigger(float) override {
        phase = 0.0f;
    }

    float getBasePitch(const BlockContext& ctx) const {
        int style = std::clamp(static_cast<int>(std::round(params[0] * 2.0f)), 0, 2);
        if (style == 0) {
            // Fixed freq: 20 Hz to 20 kHz (def 55 Hz)
            return 20.0f * std::pow(20000.0f / 20.0f, params[1]);
        } else if (style == 1) {
            // Fixed pitch: MIDI note 0 to 127 (def A1 33)
            float note = params[1] * 127.0f;
            return 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
        } else {
            // MIDI pitch: offset from -60 to +60 semitones (def 0)
            float offset = std::round((params[1] - 0.5f) * 120.0f);
            return ctx.currentPitchHz * std::pow(2.0f, offset / 12.0f);
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. tracking style: 0=Fixed Freq, 1=Fixed Pitch, 2=MIDI Pitch (def)
        // 2. pitch / freq value
        float baseFreq = getBasePitch(ctx);

        // 3. shape: waveform morph (Sine 0% -> Tri 20% -> Saw 40% -> Square 60% -> PWM 0% 100%)
        float shape = params[2];

        // 4. drive: -6dB to 0dB (at 0.5) to +24dB (def: 0dB)
        float gain = normToDriveGain(params[3]);

        bool applyPitchEnv = (ctx.pitchEnvTarget == 1 || ctx.pitchEnvTarget == 3);

        for (int i = 0; i < numSamples; ++i) {
            float fmMod = (i < static_cast<int>(ctx.modSignal.size())) ? ctx.modSignal[i] : 0.0f;
            float pitchEnv = (applyPitchEnv && i < static_cast<int>(ctx.pitchEnvSignal.size())) ? ctx.pitchEnvSignal[i] : 0.0f;

            // Pitch Envelope modulates carrier pitch: 5 octaves sweep up/down
            float instFreq = baseFreq * std::pow(2.0f, pitchEnv * 5.0f) * std::pow(2.0f, fmMod * 4.0f);
            instFreq = std::clamp(instFreq, 1.0f, sampleRate * 0.48f);

            phase += instFreq * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float raw = evaluateWaveform(phase, shape) * gain;
            float oscOut = (gain > 1.0f) ? std::tanh(raw) : raw;
            currentFreq = instFreq;

            if (left) left[i] = oscOut;
            if (right) right[i] = oscOut;
        }
    }

    float getCurrentFreq() const { return currentFreq; }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float currentFreq = 55.0f;
};

// --- BLOCK 2: MODULATOR ---
class ModulatorBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        phase = 0.0f;
        noisePhase = 0.0f;
        noiseVal = 0.0f;
        djFilter.reset();
    }

    void trigger(float) override {
        phase = 0.0f;
        noisePhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. type: 0..6
        // 0: Fixed Osc, 1: Follow Osc, 2: FM Operator (linear FM)
        // 3: Fixed Sine*Noise, 4: Follow Sine*Noise, 5: FM Op Sine*Noise, 6: S&H Noise
        int type = std::clamp(static_cast<int>(std::round(params[0] * 6.0f)), 0, 6);

        // 2. shape:
        // Types 0..2: waveform morph
        // Types 3..5: noise DJ filter
        // Type 6: S&H rate 0.1Hz..20kHz
        float shape = params[1];

        // 3. depth: -200% to 0% to +200%
        float depth = (params[2] - 0.5f) * 4.0f;

        // 4. speed:
        float speed = params[3];
        float carrierPitch = std::max(ctx.carrierPitchHz, 10.0f);
        float oscFreq = 55.0f;
        float shRate = 20000.0f;

        if (type == 0 || type == 3) {
            // Fixed frequency: 0.1 Hz to 15 kHz (def 55 Hz)
            oscFreq = 0.1f * std::pow(15000.0f / 0.1f, speed);
        } else if (type == 1 || type == 4) {
            // Following offset: -64 to +64 semitones (def 0)
            float noteOffset = (speed - 0.5f) * 128.0f;
            oscFreq = carrierPitch * std::pow(2.0f, noteOffset / 12.0f);
        } else if (type == 2 || type == 5) {
            // FM ratio: 1:32 to 1:1 to 32:1 (def 1:1)
            float ratio = (speed <= 0.5f) ? (1.0f / (32.0f - (speed * 2.0f) * 31.0f))
                                          : (1.0f + ((speed - 0.5f) * 2.0f) * 31.0f);
            oscFreq = carrierPitch * ratio;
        } else {
            // S&H Noise rate: 0.1 Hz to 20 kHz (def 20 kHz)
            shRate = 0.1f * std::pow(20000.0f / 0.1f, speed);
        }

        oscFreq = std::clamp(oscFreq, 0.05f, sampleRate * 0.48f);

        ctx.modSignal.resize(numSamples);
        bool applyPitchEnv = (ctx.pitchEnvTarget == 2 || ctx.pitchEnvTarget == 3);

        for (int i = 0; i < numSamples; ++i) {
            float pitchEnv = (applyPitchEnv && i < static_cast<int>(ctx.pitchEnvSignal.size())) ? ctx.pitchEnvSignal[i] : 0.0f;
            float instFreq = oscFreq * std::pow(2.0f, pitchEnv * 5.0f);
            instFreq = std::clamp(instFreq, 0.05f, sampleRate * 0.48f);

            phase += instFreq * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float val = 0.0f;
            if (type <= 2) {
                // Waveform morph
                val = evaluateWaveform(phase, shape);
            } else if (type <= 5) {
                // Sine * Noise ring-mod
                float sinVal = std::sin(phase * TWO_PI);
                float rawNoise = fastRng(rngState);
                float dummyR = rawNoise;
                djFilter.process(rawNoise, dummyR, shape, sampleRate);
                val = sinVal * rawNoise;
            } else {
                // S&H Noise clocked at shRate
                noisePhase += shRate * invSr;
                if (noisePhase >= 1.0f) {
                    noisePhase -= 1.0f;
                    noiseVal = fastRng(rngState);
                }
                val = noiseVal;
            }

            ctx.modSignal[i] = val * depth;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float noisePhase = 0.0f;
    float noiseVal = 0.0f;
    DJFilter djFilter;
    uint32_t rngState = 0x98765432;
};

// --- BLOCK 3: PITCH ENVELOPE (Target, Slope, Depth, Decay) ---
class PitchEnvelopeBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Target: 0=Off (def), 1=Carrier, 2=Modulator, 3=Both
        int target = std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);
        ctx.pitchEnvTarget = target;

        // 2. Slope: Exp (0.0) -> Lin (0.5) -> Log (1.0)
        float slope = params[1];

        // 3. Depth: -100% to 0% to +100% (bipolar, def 0%)
        float depth = (params[2] - 0.5f) * 2.0f;

        // 4. Decay: 5-point warp (5ms, 100ms, 1s, 5s, 60s; def 333ms)
        float decayTime = warp5PointTime(params[3]);
        decayTime = std::max(decayTime, 0.001f);

        ctx.pitchEnvSignal.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = 0.0f;
            if (target != 0) {
                float linearProgress = timeSinceTrigger / decayTime;
                float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
                envVal = applyEnvelopeSlope(envLinear, slope) * depth;
            }
            timeSinceTrigger += invSr;
            ctx.pitchEnvSignal[i] = envVal;
        }
    }

    int getTarget() const {
        return std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);
    }

private:
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
};

// --- BLOCK 4: DRIVE ---
class DriveBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        djFilter.reset();
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. type: 0=Off (def), 1=Saturation, 2=Wave Folder
        int type = std::clamp(static_cast<int>(std::round(params[0] * 2.0f)), 0, 2);
        if (type == 0) return; // bypass

        // 2. drive:
        // Saturation: -6dB to 0dB (at 0.5) to +24dB
        // Folder: 0 folds to 8 folds
        float driveDb = (params[1] <= 0.5f) ? (-6.0f + params[1] * 12.0f) : ((params[1] - 0.5f) * 48.0f);
        float satGain = std::pow(10.0f, driveDb / 20.0f);
        float numFolds = params[1] * 8.0f;

        // 3. bias: -1.0 to 0 to +1.0
        float bias = (params[2] - 0.5f) * 2.0f;

        // 4. DJ Filter
        float filterKnob = params[3];

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            inL += bias;
            inR += bias;

            if (type == 1) {
                // Soft saturation tanh
                inL = std::tanh(inL * satGain);
                inR = std::tanh(inR * satGain);
            } else {
                // Wave folder (0 to 8 folds)
                float driveFactor = 1.0f + numFolds * 2.0f;
                inL = std::sin(inL * driveFactor * (PI * 0.5f));
                inR = std::sin(inR * driveFactor * (PI * 0.5f));
            }

            inL -= bias * 0.5f;
            inR -= bias * 0.5f;

            djFilter.process(inL, inR, filterKnob, sampleRate);

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float sampleRate = 44100.0f;
    DJFilter djFilter;
};

// --- BLOCK 5: NOISE TRANSIENT ---
class NoiseTransientBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
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
        // 1. S&H rate: 0.1 Hz to 20 kHz (def 20 kHz)
        float shRate = 0.1f * std::pow(20000.0f / 0.1f, params[0]);

        // 2. DJ filter knob (def 50% Flat)
        float filterKnob = params[1];

        // 3. Drive: -6dB to 0dB (at 0.5) to +24dB (def: 0dB)
        float gain = normToDriveGain(params[2]);

        // 4. Decay: 5-point warp (1ms, 50ms, 1s, 5s, 60s; def 100ms)
        float decayTime = warpNoiseDecayTime(params[3]);
        decayTime = std::max(decayTime, 0.0005f);

        for (int i = 0; i < numSamples; ++i) {
            float noiseOut = 0.0f;

            shPhase += shRate * invSr;
            if (shPhase >= 1.0f) {
                shPhase -= 1.0f;
                shVal = fastRng(rngState);
            }

            float env = std::exp(-timeSinceTrigger / decayTime);
            timeSinceTrigger += invSr;

            float rawNoise = shVal * env * gain;
            noiseOut = (gain > 1.0f) ? std::tanh(rawNoise) : rawNoise;

            float outL = noiseOut;
            float outR = noiseOut;
            djFilter.process(outL, outR, filterKnob, sampleRate);

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float shPhase = 0.0f;
    float shVal = 0.0f;
    DJFilter djFilter;
    uint32_t rngState = 0x55AA55AA;
};

// --- BLOCK 6: MIXER (Carrier Level, Noise Level, Drive, Limiter) ---
class MixerBlock : public DSPBlock {
public:
    void init(const BlockContext& /*ctx*/) override {}

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Carrier level: 0% to 100% (at 0.5) to 400% (at 1.0)
        float carrierGain = (params[0] <= 0.5f) ? (params[0] * 2.0f) : (1.0f + (params[0] - 0.5f) * 6.0f);

        // 2. Noise level: 0% to 100% (at 0.5) to 400% (at 1.0)
        float noiseGain = (params[1] <= 0.5f) ? (params[1] * 2.0f) : (1.0f + (params[1] - 0.5f) * 6.0f);

        // 3. Drive: -6dB to 0dB (at 0.5) to +24dB
        float driveDb = (params[2] <= 0.5f) ? (-6.0f + params[2] * 12.0f) : ((params[2] - 0.5f) * 48.0f);
        float driveGain = std::pow(10.0f, driveDb / 20.0f);

        // 4. Limiter: 0=Off, 1=On (def On)
        bool hasLimiter = (params[3] >= 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] * carrierGain : 0.0f;
            float inR = right ? right[i] * carrierGain : inL;

            if (tempNoiseBufL && i < tempNoiseCount) inL += tempNoiseBufL[i] * noiseGain;
            if (tempNoiseBufR && i < tempNoiseCount) inR += tempNoiseBufR[i] * noiseGain;

            inL *= driveGain;
            inR *= driveGain;

            if (hasLimiter) {
                inL = std::tanh(inL);
                inR = std::tanh(inR);
            }

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

    void setNoiseSource(const float* nL, const float* nR, int count) {
        tempNoiseBufL = nL;
        tempNoiseBufR = nR;
        tempNoiseCount = count;
    }

private:
    const float* tempNoiseBufL = nullptr;
    const float* tempNoiseBufR = nullptr;
    int tempNoiseCount = 0;
};

// --- BLOCK 7: FILTER (Type, Style, Cutoff, Resonance) ---
class FilterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        for (int s = 0; s < 8; ++s) {
            s1L[s] = s2L[s] = s1R[s] = s2R[s] = 0.0f;
        }
        combBufferL.assign(maxDelaySamples, 0.0f);
        combBufferR.assign(maxDelaySamples, 0.0f);
        combWriteIdx = 0;
        combDampL = combDampR = 0.0f;
        apfStateL.assign(32, 0.0f);
        apfStateR.assign(32, 0.0f);
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Type: 0=Off (def), 1=LPF, 2=BPF, 3=HPF, 4=Notch, 5=Comb, 6=Disperser
        int type = std::clamp(static_cast<int>(std::round(params[0] * 6.0f)), 0, 6);
        if (type == 0) return; // bypass

        // 2. Style:
        // Filter: slope -6dB/oct to -24dB/oct (at 0.5) to -96dB/oct (def -12dB/oct)
        // Comb: dampening 0.1 Hz to 20 kHz (def 20 kHz)
        // Disperser: APFs 0 to 32 (def 4)
        float style = params[1];

        // 3. Cutoff: 0.1 Hz to 20 kHz (def 20 kHz)
        float baseCutoff = 0.1f * std::pow(20000.0f / 0.1f, params[2]);

        // 4. Resonance:
        // Filter: 0% to 100%
        // Comb: -100% to +100%
        // Disperser: -100% to +100%
        float rawRes = params[3];
        float filterQ = 0.707f + rawRes * 18.0f;
        float combFb = (rawRes - 0.5f) * 2.0f * 0.98f;
        float disperserRes = (rawRes - 0.5f) * 2.0f;

        float combDampCoeff = 0.999f;
        int apfStages = 4;
        int filterStages = 1; // 1 stage = 12dB/oct default

        if (type <= 4) {
            // Slope mapping: 0.0 -> 1 pole (6dB), 0.1667 -> 2 poles (12dB), 0.5 -> 4 poles (24dB), 1.0 -> 16 poles (96dB)
            if (style <= 0.5f) {
                filterStages = 1 + static_cast<int>(std::round(style * 2.0f)); // 1 to 2 SVF stages (12 to 24 dB)
            } else {
                filterStages = 2 + static_cast<int>(std::round((style - 0.5f) * 12.0f)); // 2 to 8 SVF stages (24 to 96 dB)
            }
            filterStages = std::clamp(filterStages, 1, 8);
        } else if (type == 5) {
            float dampHz = 0.1f * std::pow(20000.0f / 0.1f, style);
            combDampCoeff = std::clamp(TWO_PI * dampHz * invSr, 0.0001f, 0.999f);
        } else {
            apfStages = std::clamp(static_cast<int>(std::round(style * 32.0f)), 0, 32);
        }

        // Pre-Drive gain from Filter Envelope
        float preDriveGain = predriveGain;

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Apply pre-drive
            if (std::abs(preDriveGain - 1.0f) > 0.01f) {
                inL = std::tanh(inL * preDriveGain);
                inR = std::tanh(inR * preDriveGain);
            }

            // Cutoff modulated by Filter Envelope
            float fEnv = (i < static_cast<int>(ctx.filterEnvSignal.size())) ? ctx.filterEnvSignal[i] : 0.0f;
            float cutoff = baseCutoff * std::pow(2.0f, fEnv * 5.0f);
            cutoff = std::clamp(cutoff, 0.1f, sampleRate * 0.48f);

            if (type <= 4) {
                // SVF filter cascade
                float g = std::tan(PI * cutoff * invSr);
                float k = 1.0f / filterQ;
                float a1 = 1.0f / (1.0f + g * (g + k));
                int svfMode = type - 1; // 0=LP, 1=BP, 2=HP, 3=Notch

                for (int s = 0; s < filterStages; ++s) {
                    float hpL = (inL - (g + k) * s1L[s] - s2L[s]) * a1;
                    float bpL = g * hpL + s1L[s];
                    s1L[s] = g * hpL + bpL;
                    float lpL = g * bpL + s2L[s];
                    s2L[s] = g * bpL + lpL;

                    float hpR = (inR - (g + k) * s1R[s] - s2R[s]) * a1;
                    float bpR = g * hpR + s1R[s];
                    s1R[s] = g * hpR + bpR;
                    float lpR = g * bpR + s2R[s];
                    s2R[s] = g * bpR + lpR;

                    if (svfMode == 0)      { inL = lpL; inR = lpR; }
                    else if (svfMode == 1) { inL = bpL; inR = bpR; }
                    else if (svfMode == 2) { inL = hpL; inR = hpR; }
                    else                   { inL = hpL + lpL; inR = hpR + lpR; }
                }
            } else if (type == 5) {
                // Comb Filter
                float delayLen = std::clamp(sampleRate / std::clamp(cutoff, 20.0f, 15000.0f), 2.0f, static_cast<float>(maxDelaySamples - 2));
                float readPos = static_cast<float>(combWriteIdx) - delayLen;
                if (readPos < 0.0f) readPos += maxDelaySamples;

                int iPos = static_cast<int>(readPos);
                float frac = readPos - iPos;
                int iNext = (iPos + 1) % maxDelaySamples;

                float delayedL = combBufferL[iPos] * (1.0f - frac) + combBufferL[iNext] * frac;
                float delayedR = combBufferR[iPos] * (1.0f - frac) + combBufferR[iNext] * frac;

                combDampL += combDampCoeff * (delayedL - combDampL);
                combDampR += combDampCoeff * (delayedR - combDampR);

                combBufferL[combWriteIdx] = inL + combDampL * combFb;
                combBufferR[combWriteIdx] = inR + combDampR * combFb;
                combWriteIdx = (combWriteIdx + 1) % maxDelaySamples;

                inL = inL + delayedL;
                inR = inR + delayedR;
            } else {
                // Disperser (APF cascade)
                if (apfStages > 0) {
                    float tanVal = std::tan(PI * cutoff * invSr);
                    float a = (tanVal - 1.0f) / (tanVal + 1.0f);
                    a = std::clamp(a + disperserRes * 0.15f, -0.99f, 0.99f);

                    for (int st = 0; st < apfStages && st < 32; ++st) {
                        float yL = a * inL + apfStateL[st];
                        apfStateL[st] = inL - a * yL;
                        inL = yL;

                        float yR = a * inR + apfStateR[st];
                        apfStateR[st] = inR - a * yR;
                        inR = yR;
                    }
                }
            }

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

    void setPreDriveGain(float g) { predriveGain = g; }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float predriveGain = 1.0f;

    float s1L[8] = { 0.0f }, s2L[8] = { 0.0f };
    float s1R[8] = { 0.0f }, s2R[8] = { 0.0f };

    static constexpr int maxDelaySamples = 4096;
    std::vector<float> combBufferL;
    std::vector<float> combBufferR;
    int combWriteIdx = 0;
    float combDampL = 0.0f, combDampR = 0.0f;

    std::vector<float> apfStateL;
    std::vector<float> apfStateR;
};

// --- BLOCK 8: FILTER ENVELOPE (Slope, Depth, Decay, Pre-Drive) ---
class FilterEnvelopeBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Slope: Exp (0.0, def) -> Lin (0.5) -> Log (1.0)
        float slope = params[0];

        // 2. Depth: -100% to 0% to +100% (bipolar, def 0%)
        float depth = (params[1] - 0.5f) * 2.0f;

        // 3. Decay: 5-point warp (5ms, 100ms, 1s, 5s, 60s; def 333ms)
        float decayTime = warp5PointTime(params[2]);
        decayTime = std::max(decayTime, 0.001f);

        // 4. Pre-drive: -6dB to 0dB (at 0.5) to +24dB
        float preDriveDb = (params[3] <= 0.5f) ? (-6.0f + params[3] * 12.0f) : ((params[3] - 0.5f) * 48.0f);
        preDriveGain = std::pow(10.0f, preDriveDb / 20.0f);

        ctx.filterEnvSignal.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float linearProgress = timeSinceTrigger / decayTime;
            float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
            float envVal = applyEnvelopeSlope(envLinear, slope) * depth;
            timeSinceTrigger += invSr;
            ctx.filterEnvSignal[i] = envVal;
        }
    }

    float getPreDriveGain() const { return preDriveGain; }

private:
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
    float preDriveGain = 1.0f;
};

// --- BLOCK 9: RING MODULATOR ---
class RingModBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        phase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Waveform morph: Sine 0% -> Tri 20% -> Saw 40% -> Square 60% -> PWM 0% 100%
        float shape = params[0];

        // 2. Rate: 0.1 Hz to 15 kHz (def 55 Hz)
        float rate = 0.1f * std::pow(15000.0f / 0.1f, params[1]);

        // 3. Amount: 0% to 100% (def 0%)
        float amount = params[2];

        // 4. Width: -100% to 0% to +100% (def 0%)
        float width = (params[3] - 0.5f) * 2.0f;

        for (int i = 0; i < numSamples; ++i) {
            phase += rate * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float phaseL = phase;
            float phaseR = phase + width * 0.25f;
            if (phaseR >= 1.0f) phaseR -= std::floor(phaseR);
            if (phaseR < 0.0f) phaseR += 1.0f - std::floor(phaseR);

            float modL = evaluateWaveform(phaseL, shape);
            float modR = evaluateWaveform(phaseR, shape);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float wetL = inL * modL;
            float wetR = inR * modR;

            if (left) left[i] = inL * (1.0f - amount) + wetL * amount;
            if (right) right[i] = inR * (1.0f - amount) + wetR * amount;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phase = 0.0f;
};

// --- BLOCK 10: FREQUENCY SHIFTER ---
class FrequencyShifterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        phaseL = phaseR = 0.0f;
        dL = dR = 0.0f;
        for (int k = 0; k < 4; ++k) {
            s1_1L[k] = s2_1L[k] = s1_2L[k] = s2_2L[k] = 0.0f;
            s1_1R[k] = s2_1R[k] = s1_2R[k] = s2_2R[k] = 0.0f;
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Shift: -X Hz to 0 Hz to +X Hz (where X is range, bipolar, def 0 Hz)
        float shiftNorm = (params[0] - 0.5f) * 2.0f;

        // 2. Range: 0 Hz to 5 kHz (def 3 Hz, cubic curve for precision control)
        float rangeHz = normToRangeHz(params[1]);
        float totalShift = shiftNorm * rangeHz;

        // 3. Blend: -100% to -50% to 0% (Dry) to +50% to +100%
        float blend = (params[2] - 0.5f) * 2.0f;

        // 4. Width: -100% to 0% to +100% (def 0%)
        float width = (params[3] - 0.5f) * 2.0f;

        float shiftL = totalShift * (1.0f - width * 0.35f);
        float shiftR = totalShift * (1.0f + width * 0.35f);

        // 4-section Niemitalo half-band allpass filter coefficients (squared pole locations)
        // Maintains exact 90-degree phase difference across entire audio band
        constexpr float poles1[4] = { 0.4794009f, 0.8762185f, 0.9765976f, 0.9974992f };
        constexpr float poles2[4] = { 0.1617585f, 0.7330290f, 0.9453500f, 0.9905990f };

        float wetAmount = std::abs(blend);
        float wetSign = (blend >= 0.0f) ? 1.0f : -1.0f;

        for (int i = 0; i < numSamples; ++i) {
            phaseL += shiftL * invSr;
            phaseL -= std::floor(phaseL);

            phaseR += shiftR * invSr;
            phaseR -= std::floor(phaseR);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Left Channel Hilbert 90-deg Phase Split
            // Branch 1 (Q path, with 1-sample delay)
            float qL = dL;
            dL = inL;
            for (int k = 0; k < 4; ++k) {
                float c = poles1[k];
                float out = c * qL + s2_1L[k];
                s2_1L[k] = s1_1L[k];
                s1_1L[k] = -qL + c * out;
                qL = out;
            }

            // Branch 2 (I path)
            float iL = inL;
            for (int k = 0; k < 4; ++k) {
                float c = poles2[k];
                float out = c * iL + s2_2L[k];
                s2_2L[k] = s1_2L[k];
                s1_2L[k] = -iL + c * out;
                iL = out;
            }

            // Right Channel Hilbert 90-deg Phase Split
            float qR = dR;
            dR = inR;
            for (int k = 0; k < 4; ++k) {
                float c = poles1[k];
                float out = c * qR + s2_1R[k];
                s2_1R[k] = s1_1R[k];
                s1_1R[k] = -qR + c * out;
                qR = out;
            }

            float iR = inR;
            for (int k = 0; k < 4; ++k) {
                float c = poles2[k];
                float out = c * iR + s2_2R[k];
                s2_2R[k] = s1_2R[k];
                s1_2R[k] = -iR + c * out;
                iR = out;
            }

            // Quadrature carrier modulation
            // shift > 0 shifts frequency UP; shift < 0 shifts frequency DOWN
            float cosL = std::cos(phaseL * TWO_PI);
            float sinL = std::sin(phaseL * TWO_PI);
            float shiftedL = iL * cosL - qL * sinL;

            float cosR = std::cos(phaseR * TWO_PI);
            float sinR = std::sin(phaseR * TWO_PI);
            float shiftedR = iR * cosR - qR * sinR;

            float outL = inL * (1.0f - wetAmount) + (shiftedL * wetSign) * wetAmount;
            float outR = inR * (1.0f - wetAmount) + (shiftedR * wetSign) * wetAmount;

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phaseL = 0.0f, phaseR = 0.0f;
    float dL = 0.0f, dR = 0.0f;
    float s1_1L[4] = { 0.0f }, s2_1L[4] = { 0.0f };
    float s1_2L[4] = { 0.0f }, s2_2L[4] = { 0.0f };
    float s1_1R[4] = { 0.0f }, s2_1R[4] = { 0.0f };
    float s1_2R[4] = { 0.0f }, s2_2R[4] = { 0.0f };
};

// --- BLOCK 11: GRIT FX (Bits, Sample Rate, Low Boost, High Boost) ---
class GritBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        holdL = holdR = 0.0f;
        acc = 0.0f;
        lowS1L = lowS2L = lowS1R = lowS2R = 0.0f;
        highS1L = highS2L = highS1R = highS2R = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Bit reduction: 1.0 to 16.0 bits (def 16.0)
        float bits = 1.0f + params[0] * 15.0f;
        float steps = std::pow(2.0f, bits);
        bool hasBitCrush = (bits < 15.9f);

        // 2. Sample rate reduction: 20 Hz to 20 kHz (def 20 kHz)
        float targetSr = 20.0f * std::pow(20000.0f / 20.0f, params[1]);
        float phaseInc = targetSr / sampleRate;
        bool hasDownsample = (targetSr < sampleRate * 0.48f);

        // 3. Low Boost: 0 dB to +24 dB (def 0 dB) at 120 Hz
        float lowDb = params[2] * 24.0f;
        bool hasLowBoost = (lowDb > 0.05f);

        // 4. High Boost: 0 dB to +24 dB (def 0 dB) at 8 kHz
        float highDb = params[3] * 24.0f;
        bool hasHighBoost = (highDb > 0.05f);

        // Low Shelf Biquad (120 Hz)
        float lb0 = 1.0f, lb1 = 0.0f, lb2 = 0.0f, la1 = 0.0f, la2 = 0.0f;
        if (hasLowBoost) {
            float w0 = TWO_PI * 120.0f * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, lowDb / 40.0f);
            float alpha = sinw0 * 0.70710678f;
            float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
            lb0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / a0;
            lb1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
            lb2 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha)) / a0;
            la1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
            la2 = ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / a0;
        }

        // High Shelf Biquad (8 kHz)
        float hb0 = 1.0f, hb1 = 0.0f, hb2 = 0.0f, ha1 = 0.0f, ha2 = 0.0f;
        if (hasHighBoost) {
            float w0 = TWO_PI * 8000.0f * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, highDb / 40.0f);
            float alpha = sinw0 * 0.70710678f;
            float a0 = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
            hb0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / a0;
            hb1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
            hb2 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha)) / a0;
            ha1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
            ha2 = ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / a0;
        }

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            if (hasDownsample) {
                acc += phaseInc;
                if (acc >= 1.0f) {
                    acc -= 1.0f;
                    holdL = inL;
                    holdR = inR;
                }
            } else {
                holdL = inL;
                holdR = inR;
            }

            float curL = holdL;
            float curR = holdR;

            if (hasBitCrush) {
                curL = std::round(curL * steps) / steps;
                curR = std::round(curR * steps) / steps;
            }

            if (hasLowBoost) {
                float yL = lb0 * curL + lowS1L;
                lowS1L = lb1 * curL - la1 * yL + lowS2L;
                lowS2L = lb2 * curL - la2 * yL;
                curL = yL;

                float yR = lb0 * curR + lowS1R;
                lowS1R = lb1 * curR - la1 * yR + lowS2R;
                lowS2R = lb2 * curR - la2 * yR;
                curR = yR;
            }

            if (hasHighBoost) {
                float yL = hb0 * curL + highS1L;
                highS1L = hb1 * curL - ha1 * yL + highS2L;
                highS2L = hb2 * curL - ha2 * yL;
                curL = yL;

                float yR = hb0 * curR + highS1R;
                highS1R = hb1 * curR - ha1 * yR + highS2R;
                highS2R = hb2 * curR - ha2 * yR;
                curR = yR;
            }

            if (left) left[i] = curL;
            if (right) right[i] = curR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float holdL = 0.0f, holdR = 0.0f;
    float acc = 0.0f;
    float lowS1L = 0.0f, lowS2L = 0.0f, lowS1R = 0.0f, lowS2R = 0.0f;
    float highS1L = 0.0f, highS2L = 0.0f, highS1R = 0.0f, highS2R = 0.0f;
};

// --- BLOCK 12: AMP (Pan, Level, Drive, Limiter) ---
class AmpBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
    }

    void trigger(float velocity) override {
        vel = velocity;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Pan: 100% L to Center to 100% R (def Center)
        float pan = params[0];
        float gainL = std::cos(pan * 1.57079632679f);
        float gainR = std::sin(pan * 1.57079632679f);

        // 2. Level: 0% to 100% (at 0.5) to 400% (at 1.0) (def 100%)
        float level = (params[1] <= 0.5f) ? (params[1] * 2.0f) : (1.0f + (params[1] - 0.5f) * 6.0f);

        // 3. Drive: -6dB to 0dB (at 0.5) to +24dB (def 0dB)
        float driveDb = (params[2] <= 0.5f) ? (-6.0f + params[2] * 12.0f) : ((params[2] - 0.5f) * 48.0f);
        float driveGain = std::pow(10.0f, driveDb / 20.0f);
        bool hasDrive = (std::abs(driveDb) > 0.05f);

        // 4. Limiter: 0=Off, 1=On (def On)
        bool hasLimiter = (params[3] >= 0.5f);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = (i < static_cast<int>(ctx.ampEnvSignal.size())) ? ctx.ampEnvSignal[i] : 1.0f;
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float curL = inL * envVal * vel * level;
            float curR = inR * envVal * vel * level;

            if (hasDrive) {
                curL *= driveGain;
                curR *= driveGain;
            }

            if (hasLimiter) {
                curL = std::tanh(curL);
                curR = std::tanh(curR);
            }

            if (left)  left[i]  = curL * gainL;
            if (right) right[i] = curR * gainR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float vel = 1.0f;
};

// --- BLOCK 13: AMP ENVELOPE (Claps, Clap Speed, Slope, Decay) ---
class AmpEnvelopeBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float) override {
        timeSinceTrigger = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Claps: 0 to 32 (def 0)
        int numClaps = static_cast<int>(std::round(params[0] * 32.0f));

        // 2. Clap speed: 1 ms to 15 ms (def 3 ms)
        float clapDecay = 0.001f + params[1] * 0.014f;
        float clapInterval = clapDecay * 1.5f;

        // 3. Slope: Exp (0.0, def) -> Lin (0.5) -> Log (1.0)
        float slope = params[2];

        // 4. Decay: 5-point warp (5ms, 100ms, 1s, 5s, 60s; def 333ms)
        float decayTime = warp5PointTime(params[3]);
        decayTime = std::max(decayTime, 0.001f);

        float clapBurstsDuration = (numClaps > 0) ? (numClaps * clapInterval) : 0.0f;

        ctx.ampEnvSignal.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = 0.0f;
            if (numClaps > 0 && timeSinceTrigger < clapBurstsDuration) {
                float burstIdx = std::floor(timeSinceTrigger / clapInterval);
                float burstTime = timeSinceTrigger - burstIdx * clapInterval;
                float burstEnv = (burstTime < 0.0005f) ? (burstTime / 0.0005f)
                                                       : std::exp(-(burstTime - 0.0005f) / clapDecay);
                envVal = burstEnv * 0.95f;
            } else {
                float tailTime = (numClaps > 0) ? (timeSinceTrigger - clapBurstsDuration) : timeSinceTrigger;
                float linearProgress = tailTime / decayTime;
                float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
                envVal = applyEnvelopeSlope(envLinear, slope);
            }

            timeSinceTrigger += invSr;
            ctx.ampEnvSignal[i] = envVal;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float timeSinceTrigger = 1000.0f;
};

// --- MODULAR DRUM ENGINE ---
class ModularDrumEngine {
public:
    enum BlockID {
        BLK_CARRIER = 0,
        BLK_MODULATOR,
        BLK_PITCHENV,
        BLK_DRIVE,
        BLK_NOISE,
        BLK_MIXER,
        BLK_FILTER,
        BLK_FILTERENV,
        BLK_RINGMOD,
        BLK_FREQSHIFT,
        BLK_GRIT,
        BLK_AMP,
        BLK_AMPENV,
        NUM_BLOCKS = 13
    };

    void init(float sampleRate) {
        ctx.sampleRate = sampleRate;
        ctx.invSr = 1.0f / sampleRate;

        // Instantiate all 13 blocks
        allBlocks.resize(NUM_BLOCKS);
        allBlocks[BLK_CARRIER]   = std::make_unique<CarrierBlock>();
        allBlocks[BLK_MODULATOR] = std::make_unique<ModulatorBlock>();
        allBlocks[BLK_PITCHENV]  = std::make_unique<PitchEnvelopeBlock>();
        allBlocks[BLK_DRIVE]     = std::make_unique<DriveBlock>();
        allBlocks[BLK_NOISE]     = std::make_unique<NoiseTransientBlock>();
        allBlocks[BLK_MIXER]     = std::make_unique<MixerBlock>();
        allBlocks[BLK_FILTER]    = std::make_unique<FilterBlock>();
        allBlocks[BLK_FILTERENV] = std::make_unique<FilterEnvelopeBlock>();
        allBlocks[BLK_RINGMOD]   = std::make_unique<RingModBlock>();
        allBlocks[BLK_FREQSHIFT] = std::make_unique<FrequencyShifterBlock>();
        allBlocks[BLK_GRIT]      = std::make_unique<GritBlock>();
        allBlocks[BLK_AMP]       = std::make_unique<AmpBlock>();
        allBlocks[BLK_AMPENV]    = std::make_unique<AmpEnvelopeBlock>();

        for (auto& b : allBlocks) b->init(ctx);

        // Pre-allocate realtime working buffers
        tempNoiseL.assign(1024, 0.0f);
        tempNoiseR.assign(1024, 0.0f);
        ctx.modSignal.assign(1024, 0.0f);
        ctx.pitchEnvSignal.assign(1024, 0.0f);
        ctx.filterEnvSignal.assign(1024, 0.0f);
        ctx.ampEnvSignal.assign(1024, 1.0f);

        // Exact musical defaults per spec.md
        // 1. Carrier: MIDI Pitch (def index 2 = 1.0), 0 offset (0.5), Sine (0.0), 100% Level (0.5)
        setPageParameter(BLK_CARRIER, 0, 1.0f);
        setPageParameter(BLK_CARRIER, 1, 0.5f);
        setPageParameter(BLK_CARRIER, 2, 0.0f);
        setPageParameter(BLK_CARRIER, 3, 0.5f);

        // 2. Modulator: Fixed Osc (0.0), Sine (0.0), 0% Depth (0.5), 55 Hz Speed (~0.5286)
        setPageParameter(BLK_MODULATOR, 0, 0.0f);
        setPageParameter(BLK_MODULATOR, 1, 0.0f);
        setPageParameter(BLK_MODULATOR, 2, 0.5f);
        setPageParameter(BLK_MODULATOR, 3, 0.5286f);

        // 3. Pitch Envelope: Off (0.0), Exponential (0.0), 0% Depth (0.5), 333 ms Decay (0.3806)
        setPageParameter(BLK_PITCHENV, 0, 0.0f);
        setPageParameter(BLK_PITCHENV, 1, 0.0f);
        setPageParameter(BLK_PITCHENV, 2, 0.5f);
        setPageParameter(BLK_PITCHENV, 3, 0.3806f);

        // 4. Drive: Off (0.0), 0 dB (0.5), 0 Bias (0.5), Flat Filter (0.5)
        setPageParameter(BLK_DRIVE, 0, 0.0f);
        setPageParameter(BLK_DRIVE, 1, 0.5f);
        setPageParameter(BLK_DRIVE, 2, 0.5f);
        setPageParameter(BLK_DRIVE, 3, 0.5f);

        // 5. Noise Transient: 20 kHz (1.0), Flat Filter (0.5), 0 dB Drive (0.5), 100 ms Decay (0.3078)
        setPageParameter(BLK_NOISE, 0, 1.0f);
        setPageParameter(BLK_NOISE, 1, 0.5f);
        setPageParameter(BLK_NOISE, 2, 0.5f);
        setPageParameter(BLK_NOISE, 3, 0.3078f);

        // 6. Mixer: Carrier 100% (0.5), Noise 0% (0.0), Drive 0 dB (0.5), Limiter On (1.0)
        setPageParameter(BLK_MIXER, 0, 0.5f);
        setPageParameter(BLK_MIXER, 1, 0.0f);
        setPageParameter(BLK_MIXER, 2, 0.5f);
        setPageParameter(BLK_MIXER, 3, 1.0f);

        // 7. Filter: Off (0.0), -12dB/oct (0.1667), 20 kHz Cutoff (1.0), 0% Res (0.0)
        setPageParameter(BLK_FILTER, 0, 0.0f);
        setPageParameter(BLK_FILTER, 1, 0.1667f);
        setPageParameter(BLK_FILTER, 2, 1.0f);
        setPageParameter(BLK_FILTER, 3, 0.0f);

        // 8. Filter Envelope: Exponential (0.0), 0% Depth (0.5), 333 ms Decay (0.3806), 0 dB Pre-Drive (0.5)
        setPageParameter(BLK_FILTERENV, 0, 0.0f);
        setPageParameter(BLK_FILTERENV, 1, 0.5f);
        setPageParameter(BLK_FILTERENV, 2, 0.3806f);
        setPageParameter(BLK_FILTERENV, 3, 0.5f);

        // 9. RingMod: Sine (0.0), 55 Hz (0.5286), 0% Amount (0.0), 0% Width (0.5)
        setPageParameter(BLK_RINGMOD, 0, 0.0f);
        setPageParameter(BLK_RINGMOD, 1, 0.5286f);
        setPageParameter(BLK_RINGMOD, 2, 0.0f);
        setPageParameter(BLK_RINGMOD, 3, 0.5f);

        // 10. Frequency Shifter: 0 Hz Shift (0.5), 3 Hz Range (rangeHzToNorm(3.0f)), 0% Blend (0.5), 0% Width (0.5)
        setPageParameter(BLK_FREQSHIFT, 0, 0.5f);
        setPageParameter(BLK_FREQSHIFT, 1, rangeHzToNorm(3.0f));
        setPageParameter(BLK_FREQSHIFT, 2, 0.5f);
        setPageParameter(BLK_FREQSHIFT, 3, 0.5f);

        // 11. Grit FX: 16.0 Bits (1.0), 20 kHz (1.0), 0 dB Low Boost (0.0), 0 dB High Boost (0.0)
        setPageParameter(BLK_GRIT, 0, 1.0f);
        setPageParameter(BLK_GRIT, 1, 1.0f);
        setPageParameter(BLK_GRIT, 2, 0.0f);
        setPageParameter(BLK_GRIT, 3, 0.0f);

        // 12. Amp: Center Pan (0.5), 100% Level (0.5), 0 dB Drive (0.5), Limiter On (1.0)
        setPageParameter(BLK_AMP, 0, 0.5f);
        setPageParameter(BLK_AMP, 1, 0.5f);
        setPageParameter(BLK_AMP, 2, 0.5f);
        setPageParameter(BLK_AMP, 3, 1.0f);

        // 13. Amp Envelope: 0 Claps (0.0), 3 ms Speed (0.1429), Exponential (0.0), 333 ms Decay (0.3806)
        setPageParameter(BLK_AMPENV, 0, 0.0f);
        setPageParameter(BLK_AMPENV, 1, 0.1429f);
        setPageParameter(BLK_AMPENV, 2, 0.0f);
        setPageParameter(BLK_AMPENV, 3, 0.3806f);
    }

    void trigger(float velocity = 1.0f) {
        ctx.triggerVelocity = velocity;
        ctx.isTriggered = true;
        for (auto& b : allBlocks) b->trigger(velocity);
    }

    void setMidiPitch(int noteNumber) {
        ctx.currentMidiNote = noteNumber;
        ctx.currentPitchHz = 440.0f * std::pow(2.0f, (static_cast<float>(noteNumber) - 69.0f) / 12.0f);
    }

    void setPageParameter(BlockID block, int knobIndex, float value) {
        if (block < NUM_BLOCKS && allBlocks[block]) {
            allBlocks[block]->setParam(knobIndex, value);
        }
    }

    float getPageParameter(BlockID block, int knobIndex) const {
        if (block < NUM_BLOCKS && allBlocks[block]) {
            return allBlocks[block]->getParam(knobIndex);
        }
        return 0.0f;
    }

    void getScopeData(int blockIndex, float* dest, int count) const {
        if (blockIndex < 0 || blockIndex >= NUM_BLOCKS || !dest || count <= 0) return;

        float f0 = lastCarrierFreq.load(std::memory_order_relaxed);
        if (!std::isfinite(f0) || f0 < 20.0f) f0 = 55.0f;
        if (f0 > 4000.0f) f0 = 4000.0f;

        float period = ctx.sampleRate / f0;
        // Lock oscilloscope to 2 complete cycles of the carrier wave for a static view
        float totalSpan = 2.0f * period;
        totalSpan = std::clamp(totalSpan, 8.0f, static_cast<float>(VisualScope::RING_SIZE / 2));
        float step = totalSpan / static_cast<float>(count - 1);

        int head = scopes[BLK_CARRIER].writeIndex.load(std::memory_order_acquire);
        int searchStart = (head - static_cast<int>(totalSpan) - 4 + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
        int triggerPos = searchStart;

        // Search backward for a rising zero crossing in Carrier
        int maxSearch = std::min(static_cast<int>(period * 1.5f) + 16, VisualScope::RING_SIZE / 4);
        for (int s = 0; s < maxSearch; ++s) {
            int idx = (searchStart - s + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
            int prevIdx = (idx - 1 + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
            float curVal = scopes[BLK_CARRIER].buffer[idx];
            float prevVal = scopes[BLK_CARRIER].buffer[prevIdx];
            if (prevVal <= 0.0f && curVal > 0.0f) {
                triggerPos = idx;
                break;
            }
        }

        scopes[blockIndex].readTriggered(dest, count, triggerPos, step);
    }

    void process(float* monoBuffer, int numSamples) {
        processStereo(monoBuffer, nullptr, numSamples);
    }

    void processStereo(float* left, float* right, int numSamples) {
        // Ensure buffers match block size
        if (static_cast<int>(tempNoiseL.size()) < numSamples) {
            tempNoiseL.assign(numSamples, 0.0f);
            tempNoiseR.assign(numSamples, 0.0f);
            ctx.modSignal.assign(numSamples, 0.0f);
            ctx.pitchEnvSignal.assign(numSamples, 0.0f);
            ctx.filterEnvSignal.assign(numSamples, 0.0f);
            ctx.ampEnvSignal.assign(numSamples, 1.0f);
        }

        // 1. Pitch Envelope
        allBlocks[BLK_PITCHENV]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_PITCHENV].pushBlock(ctx.pitchEnvSignal.data(), numSamples);

        // Update carrier pitch for modulator tracking
        if (auto* carrier = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER].get())) {
            ctx.carrierPitchHz = carrier->getBasePitch(ctx);
        }

        // 2. Modulator
        allBlocks[BLK_MODULATOR]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODULATOR].pushBlock(ctx.modSignal.data(), numSamples);

        // 3. Filter Envelope
        allBlocks[BLK_FILTERENV]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_FILTERENV].pushBlock(ctx.filterEnvSignal.data(), numSamples);

        // 4. Amp Envelope
        allBlocks[BLK_AMPENV]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_AMPENV].pushBlock(ctx.ampEnvSignal.data(), numSamples);

        // Clear audio buffers
        if (left) std::fill(left, left + numSamples, 0.0f);
        if (right) std::fill(right, right + numSamples, 0.0f);

        // 5. Carrier Oscillator
        allBlocks[BLK_CARRIER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_CARRIER].pushBlock(left, numSamples);
        if (auto* carrier = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER].get())) {
            lastCarrierFreq.store(carrier->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 6. Drive
        allBlocks[BLK_DRIVE]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_DRIVE].pushBlock(left, numSamples);

        // 7. Noise Transient
        std::fill(tempNoiseL.begin(), tempNoiseL.begin() + numSamples, 0.0f);
        std::fill(tempNoiseR.begin(), tempNoiseR.begin() + numSamples, 0.0f);
        allBlocks[BLK_NOISE]->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);
        scopes[BLK_NOISE].pushBlock(tempNoiseL.data(), numSamples);

        // 8. Mixer (Drive output & Noise Transient)
        if (auto* mixer = dynamic_cast<MixerBlock*>(allBlocks[BLK_MIXER].get())) {
            mixer->setNoiseSource(tempNoiseL.data(), tempNoiseR.data(), numSamples);
        }
        allBlocks[BLK_MIXER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_MIXER].pushBlock(left, numSamples);

        // 9. Filter (with Pre-Drive & Filter Envelope modulation)
        if (auto* fEnv = dynamic_cast<FilterEnvelopeBlock*>(allBlocks[BLK_FILTERENV].get())) {
            if (auto* filter = dynamic_cast<FilterBlock*>(allBlocks[BLK_FILTER].get())) {
                filter->setPreDriveGain(fEnv->getPreDriveGain());
            }
        }
        allBlocks[BLK_FILTER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_FILTER].pushBlock(left, numSamples);

        // 10. RingMod
        allBlocks[BLK_RINGMOD]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_RINGMOD].pushBlock(left, numSamples);

        // 11. Frequency Shifter
        allBlocks[BLK_FREQSHIFT]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_FREQSHIFT].pushBlock(left, numSamples);

        // 12. Grit FX
        allBlocks[BLK_GRIT]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_GRIT].pushBlock(left, numSamples);

        // 13. Amp (with Amp Envelope, Pan, Master Level, Master Drive, Limiter)
        allBlocks[BLK_AMP]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_AMP].pushBlock(left, numSamples);
    }

private:
    BlockContext ctx;
    std::vector<std::unique_ptr<DSPBlock>> allBlocks;
    VisualScope scopes[NUM_BLOCKS];
    mutable std::atomic<float> lastCarrierFreq{ 55.0f };
    std::vector<float> tempNoiseL;
    std::vector<float> tempNoiseR;
};

} // namespace TbdAudio