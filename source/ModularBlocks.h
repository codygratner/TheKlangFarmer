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

// Continuous waveform morph:
// 0.0: sine -> 0.25: tri -> 0.40: saw -> 0.50: square -> 1.0: pwm 0%
inline float evaluateWaveform(float phase, float shape) {
    float p = phase - std::floor(phase);
    float s = std::sin(p * TWO_PI);
    float tri = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
    float saw = 2.0f * p - 1.0f;
    float sq = (p < 0.5f) ? 1.0f : -1.0f;

    if (shape <= 0.25f) {
        float t = shape / 0.25f;
        return (1.0f - t) * s + t * tri;
    } else if (shape <= 0.40f) {
        float t = (shape - 0.25f) / 0.15f;
        return (1.0f - t) * tri + t * saw;
    } else if (shape <= 0.50f) {
        float t = (shape - 0.40f) / 0.10f;
        return (1.0f - t) * saw + t * sq;
    } else {
        // 50% to 100%: PWM from 50% down to 0% duty cycle with zero-mean DC normalization
        float t = (shape - 0.50f) / 0.50f;
        float duty = std::clamp(0.5f * (1.0f - t), 0.01f, 0.50f);
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

// DJ Style Filter helper: 0..49% LPF (20Hz-20kHz), 50% Flat, 51..100% HPF (20Hz-20kHz)
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
        shPhase = 0.0f;
        shVal = 0.0f;
        noiseLpfState = 0.0f;
    }

    void trigger(float) override {
        phase = 0.0f;
        shPhase = 0.0f;
    }

    float getBasePitch(const BlockContext& ctx) const {
        int style = std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);
        if (style == 0) {
            // Fixed freq: 20 Hz to 20 kHz
            return 20.0f * std::pow(20000.0f / 20.0f, params[1]);
        } else if (style == 1) {
            // Fixed pitch: MIDI note 0 to 127
            float note = params[1] * 127.0f;
            return 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
        } else if (style == 2) {
            // MIDI pitch: offset from -60 to +60 semitones
            float offset = std::round((params[1] - 0.5f) * 120.0f);
            return ctx.currentPitchHz * std::pow(2.0f, offset / 12.0f);
        } else {
            // Fixed noise: S&H rate 0.1 Hz to 5 kHz
            return 0.1f * std::pow(5000.0f / 0.1f, params[1]);
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. pitch tracking style: 0=fixed freq, 1=fixed pitch, 2=midi pitch, 3=fixed noise
        int style = std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);

        // 2. pitch / freq value
        float baseFreq = getBasePitch(ctx);
        float shRate = (style == 3) ? baseFreq : 1000.0f;

        // 3. shape: waveform morph or noise LPF (20 Hz to 20 kHz)
        float shape = params[2];
        float noiseLpfCoeff = 1.0f;
        if (style == 3) {
            float lpfCutoff = 20.0f * std::pow(20000.0f / 20.0f, shape);
            noiseLpfCoeff = std::clamp(TWO_PI * lpfCutoff * invSr, 0.001f, 0.999f);
        }

        // 4. level: 0% to 100% to 400%
        float gain = (params[3] <= 0.5f) ? (params[3] * 2.0f) : (1.0f + (params[3] - 0.5f) * 6.0f);

        for (int i = 0; i < numSamples; ++i) {
            float mod = (i < static_cast<int>(ctx.modSignal.size())) ? ctx.modSignal[i] : 0.0f;
            float oscOut = 0.0f;

            if (style == 3) {
                // Fixed Noise S&H
                shPhase += shRate * invSr;
                if (shPhase >= 1.0f) {
                    shPhase -= 1.0f;
                    shVal = fastRng(rngState);
                }
                noiseLpfState += noiseLpfCoeff * (shVal - noiseLpfState);
                oscOut = noiseLpfState * gain;
                currentFreq = shRate;
            } else {
                // Pitch modulated carrier
                float instFreq = baseFreq * std::pow(2.0f, mod * 4.0f);
                instFreq = std::clamp(instFreq, 2.0f, sampleRate * 0.48f);

                phase += instFreq * invSr;
                if (phase >= 1.0f) phase -= std::floor(phase);

                oscOut = evaluateWaveform(phase, shape) * gain;
                currentFreq = instFreq;
            }

            if (left) left[i] = oscOut;
            if (right) right[i] = oscOut;
        }
    }

    float getCurrentFreq() const { return currentFreq; }

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float shPhase = 0.0f;
    float shVal = 0.0f;
    float noiseLpfState = 0.0f;
    float currentFreq = 100.0f;
    uint32_t rngState = 0x13579BDF;
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
        linearProgress = 1.0f;
    }

    void trigger(float) override {
        phase = 0.0f;
        noisePhase = 0.0f;
        linearProgress = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Modulator type: 0..7
        int type = std::clamp(static_cast<int>(std::round(params[0] * 7.0f)), 0, 7);

        // 2. Shape
        float shape = params[1];
        float noiseShRate = 0.1f * std::pow(20000.0f / 0.1f, shape);

        // 3. Depth: -100% to 0 to +100%
        float depth = (params[2] - 0.5f) * 2.0f;

        // 4. Speed
        float speed = params[3];
        float oscFreq = 100.0f;
        float decayTime = 0.333f;

        float carrierPitch = std::max(ctx.carrierPitchHz, 10.0f);

        if (type == 0 || type == 3) {
            // Fixed frequency: 0.1 Hz to 5 kHz
            oscFreq = 0.1f * std::pow(5000.0f / 0.1f, speed);
        } else if (type == 1 || type == 4) {
            // Following offset: -64 to +64 semitones
            float noteOffset = -64.0f + speed * 128.0f;
            oscFreq = carrierPitch * std::pow(2.0f, noteOffset / 12.0f);
        } else if (type == 2) {
            // FM ratio: 1:32 to 1:1 to 32:1
            float ratio = 1.0f;
            if (speed <= 0.5f) {
                ratio = 1.0f / (32.0f - (speed * 2.0f) * 31.0f);
            } else {
                ratio = 1.0f + ((speed - 0.5f) * 2.0f) * 31.0f;
            }
            oscFreq = carrierPitch * ratio;
        } else if (type == 5) {
            // S&H noise clock
            oscFreq = 0.1f * std::pow(5000.0f / 0.1f, speed);
        } else if (type == 6) {
            // Fast decay: 10 ms to 333 ms to 5 s
            decayTime = (speed <= 0.5f) ? (0.010f * std::pow(0.333f / 0.010f, speed * 2.0f))
                                        : (0.333f * std::pow(5.0f / 0.333f, (speed - 0.5f) * 2.0f));
        } else if (type == 7) {
            // Slow decay: 100 ms to 5 s to 60 s
            decayTime = (speed <= 0.5f) ? (0.100f * std::pow(5.0f / 0.100f, speed * 2.0f))
                                        : (5.0f * std::pow(60.0f / 5.0f, (speed - 0.5f) * 2.0f));
        }

        oscFreq = std::clamp(oscFreq, 0.05f, sampleRate * 0.48f);
        decayTime = std::max(decayTime, 0.001f);

        ctx.modSignal.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float val = 0.0f;

            if (type <= 2) {
                // Pure oscillator morph
                phase += oscFreq * invSr;
                if (phase >= 1.0f) phase -= std::floor(phase);
                val = evaluateWaveform(phase, shape);
            } else if (type == 3 || type == 4) {
                // Sine * Noise ring-mod
                phase += oscFreq * invSr;
                if (phase >= 1.0f) phase -= std::floor(phase);
                float sinVal = std::sin(phase * TWO_PI);

                noisePhase += noiseShRate * invSr;
                if (noisePhase >= 1.0f) {
                    noisePhase -= 1.0f;
                    noiseVal = fastRng(rngState);
                }
                val = sinVal * noiseVal;
            } else if (type == 5) {
                // Fixed S&H Noise
                noisePhase += noiseShRate * invSr;
                if (noisePhase >= 1.0f) {
                    noisePhase -= 1.0f;
                    noiseVal = fastRng(rngState);
                }
                val = noiseVal;
            } else {
                // Decay envelope with slope
                linearProgress += invSr / decayTime;
                float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
                val = applyEnvelopeSlope(envLinear, shape);
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
    float linearProgress = 1.0f;
    uint32_t rngState = 0x98765432;
};

// --- BLOCK 3: DRIVE ---
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
        // 1. type: 0=off, 1=saturation, 2=clipper, 3=wave folder
        int type = std::clamp(static_cast<int>(std::round(params[0] * 3.0f)), 0, 3);
        if (type == 0) return; // bypass

        // 2. drive
        float driveGain = 1.0f;
        if (type == 1) {
            // Saturation: -6 dB to 0 dB to +24 dB
            float driveDb = (params[1] <= 0.5f) ? (-6.0f + params[1] * 12.0f) : ((params[1] - 0.5f) * 48.0f);
            driveGain = std::pow(10.0f, driveDb / 20.0f);
        } else if (type == 2) {
            // Clipper threshold: -96 dB to 0 dB
            float clipDb = -96.0f + params[1] * 96.0f;
            driveGain = 1.0f / std::max(std::pow(10.0f, clipDb / 20.0f), 0.0001f);
        } else {
            // Wavefolder gain: -96 dB to 0 dB
            float foldDb = -96.0f + params[1] * 96.0f;
            driveGain = std::max(std::pow(10.0f, foldDb / 20.0f), 0.0001f);
        }

        // 3. bias: -1.0 to 0 to +1.0
        float bias = (params[2] - 0.5f) * 2.0f;

        // 4. DJ Filter knob
        float filterKnob = params[3];

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Apply bias
            inL += bias;
            inR += bias;

            // Apply non-linearity
            if (type == 1) {
                // Soft saturation tanh
                inL = std::tanh(inL * driveGain);
                inR = std::tanh(inR * driveGain);
            } else if (type == 2) {
                // Hard clipper
                float scaledL = inL * driveGain;
                float scaledR = inR * driveGain;
                inL = std::clamp(scaledL, -1.0f, 1.0f) / std::max(driveGain, 1.0f);
                inR = std::clamp(scaledR, -1.0f, 1.0f) / std::max(driveGain, 1.0f);
            } else {
                // Sine wave folder
                inL = std::sin(inL * driveGain * PI * 2.0f);
                inR = std::sin(inR * driveGain * PI * 2.0f);
            }

            // Remove bias offset to preserve DC integrity
            inL -= bias * 0.5f;
            inR -= bias * 0.5f;

            // DJ Filter
            djFilter.process(inL, inR, filterKnob, sampleRate);

            if (left) left[i] = inL;
            if (right) right[i] = inR;
        }
    }

private:
    float sampleRate = 44100.0f;
    DJFilter djFilter;
};

// --- BLOCK 4: NOISE TRANSIENT ---
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

    void trigger(float /*velocity*/) override {
        timeSinceTrigger = 0.0f;
        shPhase = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. S&H rate: 0.1 Hz to 20 kHz
        float shRate = 0.1f * std::pow(20000.0f / 0.1f, params[0]);

        // 2. DJ filter knob
        float filterKnob = params[1];

        // 3. Level: 0% to 100%
        float level = params[2];

        // 4. Decay: 10 ms to 333 ms to 5 seconds
        float decayTime = (params[3] <= 0.5f) ? (0.010f * std::pow(0.333f / 0.010f, params[3] * 2.0f))
                                              : (0.333f * std::pow(5.0f / 0.333f, (params[3] - 0.5f) * 2.0f));
        decayTime = std::max(decayTime, 0.001f);

        for (int i = 0; i < numSamples; ++i) {
            float noiseOut = 0.0f;

            shPhase += shRate * invSr;
            if (shPhase >= 1.0f) {
                shPhase -= 1.0f;
                shVal = fastRng(rngState);
            }

            // Exponential transient decay envelope
            float env = std::exp(-timeSinceTrigger / decayTime);
            timeSinceTrigger += invSr;

            noiseOut = shVal * env * level;

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

// --- BLOCK 5: FILTER ---
class FilterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        sampleRate = ctx.sampleRate;
        s1L = s2L = s1R = s2R = 0.0f;
        env = 0.0f;
        combBufferL.assign(maxDelaySamples, 0.0f);
        combBufferR.assign(maxDelaySamples, 0.0f);
        combWriteIdx = 0;
        combDampL = combDampR = 0.0f;
        apfStateL.assign(32, 0.0f);
        apfStateR.assign(32, 0.0f);
    }

    void trigger(float /*velocity*/) override {
        env = 1.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. type: 0..3 No Rez (LP, BP, HP, Notch), 4..7 Rezzy (LP, BP, HP, Notch), 8 Comb, 9 APF Disperser
        int type = std::clamp(static_cast<int>(std::round(params[0] * 9.0f)), 0, 9);

        // 2. Cutoff: 20 Hz to 20 kHz
        float baseCutoff = 20.0f * std::pow(20000.0f / 20.0f, params[1]);

        // 3. Depth:
        // No Rez & Rezzy: decay envelope depth -100% to +100%
        // Comb: resonance -100% to +100%
        // APF: resonance -100% to +100%
        float depth = (params[2] - 0.5f) * 2.0f;

        // 4. Decay:
        // No Rez: 100 ms to 5 s to 60 s
        // Rezzy: 10 ms to 333 ms to 5 s
        // Comb: dampening 0.1 Hz to 20 kHz
        // APF: simultaneous APFs 0 to 32
        float decayTime = 0.333f;
        float combDampCoeff = 0.5f;
        int apfStages = 0;

        if (type <= 3) {
            // No Rez
            decayTime = (params[3] <= 0.5f) ? (0.100f * std::pow(5.0f / 0.100f, params[3] * 2.0f))
                                            : (5.0f * std::pow(60.0f / 5.0f, (params[3] - 0.5f) * 2.0f));
        } else if (type <= 7) {
            // Rezzy
            decayTime = (params[3] <= 0.5f) ? (0.010f * std::pow(0.333f / 0.010f, params[3] * 2.0f))
                                            : (0.333f * std::pow(5.0f / 0.333f, (params[3] - 0.5f) * 2.0f));
        } else if (type == 8) {
            // Comb dampening: 0.1 Hz to 20 kHz
            float dampHz = 0.1f * std::pow(20000.0f / 0.1f, params[3]);
            combDampCoeff = std::clamp(TWO_PI * dampHz * invSr, 0.0001f, 0.999f);
        } else {
            // APF stages: integer 0 to 32
            apfStages = std::clamp(static_cast<int>(std::round(params[3] * 32.0f)), 0, 32);
        }

        decayTime = std::max(decayTime, 0.001f);
        float decayCoeff = std::exp(-invSr / decayTime);
        float q = (type >= 4 && type <= 7) ? 8.0f : 0.707f;
        float combFb = depth * 0.96f; // Comb resonance
        float apfRes = depth;          // APF resonance

        for (int i = 0; i < numSamples; ++i) {
            env *= decayCoeff;

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            if (type <= 7) {
                // SVF Lowpass/Bandpass/Highpass/Notch
                float cutoff = baseCutoff * std::pow(2.0f, env * depth * 5.0f);
                cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.48f);

                float g = std::tan(PI * cutoff * invSr);
                float k = 1.0f / q;
                float a1 = 1.0f / (1.0f + g * (g + k));

                int svfMode = type % 4; // 0=LP, 1=BP, 2=HP, 3=Notch

                // Left SVF
                float hpL = (inL - (g + k) * s1L - s2L) * a1;
                float bpL = g * hpL + s1L;
                s1L = g * hpL + bpL;
                float lpL = g * bpL + s2L;
                s2L = g * bpL + lpL;

                // Right SVF
                float hpR = (inR - (g + k) * s1R - s2R) * a1;
                float bpR = g * hpR + s1R;
                s1R = g * hpR + bpR;
                float lpR = g * bpR + s2R;
                s2R = g * bpR + lpR;

                if (svfMode == 0) { inL = lpL; inR = lpR; }
                else if (svfMode == 1) { inL = bpL; inR = bpR; }
                else if (svfMode == 2) { inL = hpL; inR = hpR; }
                else { inL = hpL + lpL; inR = hpR + lpR; }
            } else if (type == 8) {
                // Comb Filter
                float delayLen = std::clamp(sampleRate / std::clamp(baseCutoff, 20.0f, 15000.0f), 2.0f, static_cast<float>(maxDelaySamples - 2));
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
                // APF (Disperser cascade)
                if (apfStages > 0) {
                    float apfCutoff = std::clamp(baseCutoff, 20.0f, sampleRate * 0.48f);
                    float tanVal = std::tan(PI * apfCutoff * invSr);
                    float a = (tanVal - 1.0f) / (tanVal + 1.0f);
                    // resonance shapes allpass pole slightly
                    a = std::clamp(a + apfRes * 0.15f, -0.99f, 0.99f);

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

private:
    float invSr = 1.0f / 44100.0f;
    float sampleRate = 44100.0f;
    float s1L = 0.0f, s2L = 0.0f;
    float s1R = 0.0f, s2R = 0.0f;
    float env = 0.0f;

    static constexpr int maxDelaySamples = 4096;
    std::vector<float> combBufferL;
    std::vector<float> combBufferR;
    int combWriteIdx = 0;
    float combDampL = 0.0f, combDampR = 0.0f;

    std::vector<float> apfStateL;
    std::vector<float> apfStateR;
};

// --- BLOCK 6: RING MODULATOR ---
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
        // 1. Waveform morph: sine, tri, saw, sq, pwm 0%
        float shape = params[0];

        // 2. Rate: 0.1 Hz to 5 kHz
        float rate = 0.1f * std::pow(5000.0f / 0.1f, params[1]);

        // 3. Amount: 0% to 100%
        float amount = params[2];

        // 4. Width: -100% to 0% to +100%
        float width = (params[3] - 0.5f) * 2.0f;

        for (int i = 0; i < numSamples; ++i) {
            phase += rate * invSr;
            if (phase >= 1.0f) phase -= std::floor(phase);

            float phaseL = phase;
            float phaseR = phase + width * 0.25f;
            if (phaseR >= 1.0f) phaseR -= std::floor(phaseR);
            if (phaseR < 0.0f) phaseR += 1.0f - std::floor(phaseR);

            float carrierL = evaluateWaveform(phaseL, shape);
            float carrierR = evaluateWaveform(phaseR, shape);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            float modL = inL * carrierL;
            float modR = inR * carrierR;

            if (left) left[i] = inL * (1.0f - amount) + modL * amount;
            if (right) right[i] = inR * (1.0f - amount) + modR * amount;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phase = 0.0f;
};

// --- BLOCK 7: GRIT FX ---
class GritBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        holdL = holdR = 0.0f;
        acc = 0.0f;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Bit reduction: 1.0 bit to 16.0 bit
        float bits = 1.0f + params[0] * 15.0f;
        float steps = std::pow(2.0f, bits);
        bool hasBitCrush = (bits < 15.9f);

        // 2. Sample rate reduction: 20 Hz to 20 kHz
        float targetSr = 20.0f * std::pow(20000.0f / 20.0f, params[1]);
        float phaseInc = targetSr / sampleRate;
        bool hasDownsample = (targetSr < sampleRate * 0.48f);

        for (int i = 0; i < numSamples; ++i) {
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Sample rate reduction (zero-order hold)
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

            // Bit reduction (linear quantization)
            if (hasBitCrush) {
                curL = std::round(curL * steps) / steps;
                curR = std::round(curR * steps) / steps;
            }

            if (left) left[i] = curL;
            if (right) right[i] = curR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float holdL = 0.0f, holdR = 0.0f;
    float acc = 0.0f;
};

// --- BLOCK 8: FREQUENCY SHIFTER ---
class FrequencyShifterBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        phaseL = phaseR = 0.0f;
        for (int k = 0; k < 4; ++k) {
            apI1L[k] = apI2L[k] = apQ1L[k] = apQ2L[k] = 0.0f;
            apI1R[k] = apI2R[k] = apQ1R[k] = apQ2R[k] = 0.0f;
        }
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& /*ctx*/) override {
        // 1. Shift: -X to 0 to +X where X is range
        float shiftNorm = (params[0] - 0.5f) * 2.0f;

        // 2. Range: 0 Hz to 5 kHz
        float rangeHz = params[1] * 5000.0f;
        float totalShift = shiftNorm * rangeHz;

        // 3. Blend: -100% (LSB) to 0% (Dry) to +100% (USB)
        float blend = (params[2] - 0.5f) * 2.0f;

        // 4. Width: -100% to 0% to +100%
        float width = (params[3] - 0.5f) * 2.0f;

        float shiftL = totalShift * (1.0f - width * 0.35f);
        float shiftR = totalShift * (1.0f + width * 0.35f);

        // 4-stage Weaver/allpass Hilbert 90-degree phase difference coefficients
        constexpr float polesI[4] = { 0.161758f, 0.733029f, 0.945350f, 0.990598f };
        constexpr float polesQ[4] = { 0.479401f, 0.876218f, 0.976598f, 0.997500f };

        for (int i = 0; i < numSamples; ++i) {
            phaseL += shiftL * invSr;
            if (phaseL >= 1.0f) phaseL -= std::floor(phaseL);
            if (phaseL < 0.0f) phaseL += 1.0f - std::floor(phaseL);

            phaseR += shiftR * invSr;
            if (phaseR >= 1.0f) phaseR -= std::floor(phaseR);
            if (phaseR < 0.0f) phaseR += 1.0f - std::floor(phaseR);

            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Hilbert 90-degree phase split for Left
            float iL = inL;
            for (int k = 0; k < 4; ++k) {
                float y = polesI[k] * iL + apI1L[k];
                apI1L[k] = iL - polesI[k] * y;
                iL = y;
            }
            float qL = inL;
            for (int k = 0; k < 4; ++k) {
                float y = polesQ[k] * qL + apQ1L[k];
                apQ1L[k] = qL - polesQ[k] * y;
                qL = y;
            }

            // Hilbert 90-degree phase split for Right
            float iR = inR;
            for (int k = 0; k < 4; ++k) {
                float y = polesI[k] * iR + apI1R[k];
                apI1R[k] = iR - polesI[k] * y;
                iR = y;
            }
            float qR = inR;
            for (int k = 0; k < 4; ++k) {
                float y = polesQ[k] * qR + apQ1R[k];
                apQ1R[k] = qR - polesQ[k] * y;
                qR = y;
            }

            // Quadrature mixing
            float cosL = std::cos(phaseL * TWO_PI);
            float sinL = std::sin(phaseL * TWO_PI);
            float usbL = iL * cosL - qL * sinL;
            float lsbL = iL * cosL + qL * sinL;

            float cosR = std::cos(phaseR * TWO_PI);
            float sinR = std::sin(phaseR * TWO_PI);
            float usbR = iR * cosR - qR * sinR;
            float lsbR = iR * cosR + qR * sinR;

            float outL = inL;
            float outR = inR;

            if (blend > 0.0f) {
                outL = inL * (1.0f - blend) + usbL * blend;
                outR = inR * (1.0f - blend) + usbR * blend;
            } else {
                float absB = -blend;
                outL = inL * (1.0f - absB) + lsbL * absB;
                outR = inR * (1.0f - absB) + lsbR * absB;
            }

            if (left) left[i] = outL;
            if (right) right[i] = outR;
        }
    }

private:
    float invSr = 1.0f / 44100.0f;
    float phaseL = 0.0f, phaseR = 0.0f;
    float apI1L[4] = {0.0f}, apI2L[4] = {0.0f}, apQ1L[4] = {0.0f}, apQ2L[4] = {0.0f};
    float apI1R[4] = {0.0f}, apI2R[4] = {0.0f}, apQ1R[4] = {0.0f}, apQ2R[4] = {0.0f};
};

// --- BLOCK 9: AMP (Pan, Level, Drive, Low Boost) ---
class AmpBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        sampleRate = ctx.sampleRate;
        invSr = ctx.invSr;
        shelfS1L = shelfS2L = shelfS1R = shelfS2R = 0.0f;
    }

    void trigger(float velocity) override {
        vel = velocity;
    }

    void process(float* buffer, int numSamples, BlockContext& ctx) override {
        processStereo(buffer, nullptr, numSamples, ctx);
    }

    void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) override {
        // 1. Pan: 100% L to C to 100% R
        float pan = params[0];
        float gainL = std::cos(pan * 1.57079632679f);
        float gainR = std::sin(pan * 1.57079632679f);

        // 2. Level: 0% to 100% to 400%
        float level = (params[1] <= 0.5f) ? (params[1] * 2.0f) : (1.0f + (params[1] - 0.5f) * 6.0f);

        // 3. Drive: -6 dB to 0 dB to +24 dB
        float driveDb = (params[2] <= 0.5f) ? (-6.0f + params[2] * 12.0f) : ((params[2] - 0.5f) * 48.0f);
        float driveGain = std::pow(10.0f, driveDb / 20.0f);
        bool hasDrive = (std::abs(driveDb) > 0.05f);

        // 4. Low Boost: 0 dB to +24 dB low shelf at 120 Hz
        float boostDb = params[3] * 24.0f;
        bool hasBoost = (boostDb > 0.05f);

        // Biquad low shelf coeffs at 120 Hz
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        if (hasBoost) {
            float f0 = 120.0f;
            float w0 = TWO_PI * f0 * invSr;
            float cosw0 = std::cos(w0);
            float sinw0 = std::sin(w0);
            float A = std::pow(10.0f, boostDb / 40.0f);
            float alpha = sinw0 * 0.70710678f;

            float a0 = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;
            b0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / a0;
            b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / a0;
            b2 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha)) / a0;
            a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosw0)) / a0;
            a2 = ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / a0;
        }

        for (int i = 0; i < numSamples; ++i) {
            float envVal = (i < static_cast<int>(ctx.ampEnvSignal.size())) ? ctx.ampEnvSignal[i] : 1.0f;
            float inL = left ? left[i] : 0.0f;
            float inR = right ? right[i] : inL;

            // Apply Amp envelope & velocity & level
            float curL = inL * envVal * vel * level;
            float curR = inR * envVal * vel * level;

            // Apply Drive saturation
            if (hasDrive) {
                curL = std::tanh(curL * driveGain);
                curR = std::tanh(curR * driveGain);
            }

            // Apply Low Shelf Boost
            if (hasBoost) {
                float yL = b0 * curL + shelfS1L;
                shelfS1L = b1 * curL - a1 * yL + shelfS2L;
                shelfS2L = b2 * curL - a2 * yL;
                curL = yL;

                float yR = b0 * curR + shelfS1R;
                shelfS1R = b1 * curR - a1 * yR + shelfS2R;
                shelfS2R = b2 * curR - a2 * yR;
                curR = yR;
            }

            // Apply Pan
            if (left)  left[i]  = curL * gainL;
            if (right) right[i] = curR * gainR;
        }
    }

private:
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    float vel = 1.0f;
    float shelfS1L = 0.0f, shelfS2L = 0.0f;
    float shelfS1R = 0.0f, shelfS2R = 0.0f;
};

// --- BLOCK 10: AMP ENVELOPE (Type, Claps, Shape, Decay) ---
class AmpEnvelopeBlock : public DSPBlock {
public:
    void init(const BlockContext& ctx) override {
        invSr = ctx.invSr;
        timeSinceTrigger = 1000.0f;
    }

    void trigger(float /*velocity*/) override {
        timeSinceTrigger = 0.0f;
    }

    void process(float* /*buffer*/, int numSamples, BlockContext& ctx) override {
        processStereo(nullptr, nullptr, numSamples, ctx);
    }

    void processStereo(float* /*left*/, float* /*right*/, int numSamples, BlockContext& ctx) override {
        // 1. Type: 0=Fast decay, 1=Slow decay
        int type = (params[0] >= 0.5f) ? 1 : 0;

        // 2. Claps: 1 to 16
        int numClaps = 1 + static_cast<int>(std::round(params[1] * 15.0f));

        // 3. Shape: slope (exp -> lin -> log)
        float shape = params[2];

        // 4. Decay:
        // Fast time: 10 ms to 333 ms to 5 seconds
        // Slow time: 100 ms to 5 seconds to 60 seconds
        float decayTime = 0.333f;
        if (type == 0) {
            decayTime = (params[3] <= 0.5f) ? (0.010f * std::pow(0.333f / 0.010f, params[3] * 2.0f))
                                            : (0.333f * std::pow(5.0f / 0.333f, (params[3] - 0.5f) * 2.0f));
        } else {
            decayTime = (params[3] <= 0.5f) ? (0.100f * std::pow(5.0f / 0.100f, params[3] * 2.0f))
                                            : (5.0f * std::pow(60.0f / 5.0f, (params[3] - 0.5f) * 2.0f));
        }
        decayTime = std::max(decayTime, 0.001f);

        constexpr float clapInterval = 0.018f; // 18 ms between claps
        float clapBurstsDuration = (numClaps > 1) ? ((numClaps - 1) * clapInterval) : 0.0f;

        ctx.ampEnvSignal.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float envVal = 0.0f;
            if (numClaps > 1 && timeSinceTrigger < clapBurstsDuration) {
                float burstIdx = std::floor(timeSinceTrigger / clapInterval);
                float burstTime = timeSinceTrigger - burstIdx * clapInterval;
                float burstEnv = (burstTime < 0.001f) ? (burstTime / 0.001f)
                                                      : std::exp(-(burstTime - 0.001f) / 0.004f);
                envVal = burstEnv * 0.9f;
            } else {
                float tailTime = (numClaps > 1) ? (timeSinceTrigger - clapBurstsDuration) : timeSinceTrigger;
                float linearProgress = tailTime / decayTime;
                float envLinear = std::clamp(1.0f - linearProgress, 0.0f, 1.0f);
                envVal = applyEnvelopeSlope(envLinear, shape);
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
        BLK_DRIVE,
        BLK_NOISE,
        BLK_FILTER,
        BLK_RINGMOD,
        BLK_GRIT,
        BLK_FREQSHIFT,
        BLK_AMP,
        BLK_AMPENV,
        NUM_BLOCKS
    };

    void init(float sampleRate) {
        ctx.sampleRate = sampleRate;
        ctx.invSr = 1.0f / sampleRate;

        // Instantiate all 10 blocks
        allBlocks.resize(NUM_BLOCKS);
        allBlocks[BLK_CARRIER]   = std::make_unique<CarrierBlock>();
        allBlocks[BLK_MODULATOR] = std::make_unique<ModulatorBlock>();
        allBlocks[BLK_DRIVE]     = std::make_unique<DriveBlock>();
        allBlocks[BLK_NOISE]     = std::make_unique<NoiseTransientBlock>();
        allBlocks[BLK_FILTER]    = std::make_unique<FilterBlock>();
        allBlocks[BLK_RINGMOD]   = std::make_unique<RingModBlock>();
        allBlocks[BLK_GRIT]      = std::make_unique<GritBlock>();
        allBlocks[BLK_FREQSHIFT] = std::make_unique<FrequencyShifterBlock>();
        allBlocks[BLK_AMP]       = std::make_unique<AmpBlock>();
        allBlocks[BLK_AMPENV]    = std::make_unique<AmpEnvelopeBlock>();

        for (auto& b : allBlocks) b->init(ctx);

        // Musical default parameters
        setPageParameter(BLK_CARRIER, 0, 0.0f); // Fixed Frequency
        setPageParameter(BLK_CARRIER, 1, 0.173f); // ~65 Hz
        setPageParameter(BLK_CARRIER, 2, 0.0f); // Sine
        setPageParameter(BLK_CARRIER, 3, 0.5f); // 100% Level

        setPageParameter(BLK_MODULATOR, 0, 0.0f); // Fixed Osc
        setPageParameter(BLK_MODULATOR, 1, 0.0f); // Sine
        setPageParameter(BLK_MODULATOR, 2, 0.5f); // 0 Depth
        setPageParameter(BLK_MODULATOR, 3, 0.5f); // Speed

        setPageParameter(BLK_DRIVE, 0, 0.0f); // Off
        setPageParameter(BLK_DRIVE, 1, 0.20f); // 0 dB
        setPageParameter(BLK_DRIVE, 2, 0.5f); // 0 Bias
        setPageParameter(BLK_DRIVE, 3, 0.5f); // Flat DJ filter

        setPageParameter(BLK_NOISE, 0, 0.755f); // 1000 Hz S&H
        setPageParameter(BLK_NOISE, 1, 0.5f); // Flat DJ filter
        setPageParameter(BLK_NOISE, 2, 0.0f); // 0 Level
        setPageParameter(BLK_NOISE, 3, 0.297f); // 100 ms Decay

        setPageParameter(BLK_FILTER, 0, 0.0f); // No rez LPF
        setPageParameter(BLK_FILTER, 1, 0.90f); // Cutoff high (open)
        setPageParameter(BLK_FILTER, 2, 0.5f); // 0 Depth
        setPageParameter(BLK_FILTER, 3, 0.5f); // Decay

        setPageParameter(BLK_RINGMOD, 0, 0.0f); // Sine
        setPageParameter(BLK_RINGMOD, 1, 0.638f); // 100 Hz
        setPageParameter(BLK_RINGMOD, 2, 0.0f); // 0 Amount (dry)
        setPageParameter(BLK_RINGMOD, 3, 0.5f); // Width center

        setPageParameter(BLK_GRIT, 0, 1.0f); // 16 bits (clean)
        setPageParameter(BLK_GRIT, 1, 1.0f); // 20 kHz (clean)
        setPageParameter(BLK_GRIT, 2, 0.0f);
        setPageParameter(BLK_GRIT, 3, 0.0f);

        setPageParameter(BLK_FREQSHIFT, 0, 0.5f); // 0 Shift
        setPageParameter(BLK_FREQSHIFT, 1, 0.20f); // 1000 Hz Range
        setPageParameter(BLK_FREQSHIFT, 2, 0.5f); // Dry
        setPageParameter(BLK_FREQSHIFT, 3, 0.5f); // Width center

        setPageParameter(BLK_AMP, 0, 0.5f); // Center pan
        setPageParameter(BLK_AMP, 1, 0.5f); // 100% Level
        setPageParameter(BLK_AMP, 2, 0.20f); // 0 dB Master Drive
        setPageParameter(BLK_AMP, 3, 0.0f); // 0 dB Low Boost

        setPageParameter(BLK_AMPENV, 0, 0.0f); // Fast decay
        setPageParameter(BLK_AMPENV, 1, 0.0f); // 1 clap
        setPageParameter(BLK_AMPENV, 2, 0.5f); // Linear slope
        setPageParameter(BLK_AMPENV, 3, 0.5f); // ~333 ms decay
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
        if (!std::isfinite(f0) || f0 < 20.0f) f0 = 100.0f;
        if (f0 > 4000.0f) f0 = 4000.0f;

        float period = ctx.sampleRate / f0;
        // Lock oscilloscope to 2 complete cycles of the carrier wave for a rock-solid static view
        float totalSpan = 2.0f * period;
        totalSpan = std::clamp(totalSpan, 8.0f, static_cast<float>(VisualScope::RING_SIZE / 2));
        float step = totalSpan / static_cast<float>(count - 1);

        int head = scopes[BLK_CARRIER].writeIndex.load(std::memory_order_acquire);
        int searchStart = (head - static_cast<int>(totalSpan) - 4 + VisualScope::RING_SIZE * 4) & (VisualScope::RING_SIZE - 1);
        int triggerPos = searchStart;

        // Search backward up to 1.5 periods in Carrier for a rising zero crossing
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
        // Ensure modulation buffers are correctly sized
        ctx.modSignal.assign(numSamples, 0.0f);
        ctx.ampEnvSignal.assign(numSamples, 1.0f);

        // Update carrier base pitch so modulator can track it
        if (auto* carrier = dynamic_cast<CarrierBlock*>(allBlocks[BLK_CARRIER].get())) {
            ctx.carrierPitchHz = carrier->getBasePitch(ctx);
            lastCarrierFreq.store(carrier->getCurrentFreq(), std::memory_order_relaxed);
        }

        // 1. Modulator generates frequency modulation signal
        allBlocks[BLK_MODULATOR]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_MODULATOR].pushBlock(ctx.modSignal.data(), numSamples);

        // 2. Amp Envelope generates envelope signal
        allBlocks[BLK_AMPENV]->processStereo(nullptr, nullptr, numSamples, ctx);
        scopes[BLK_AMPENV].pushBlock(ctx.ampEnvSignal.data(), numSamples);

        // Clear audio buffers
        if (left) std::fill(left, left + numSamples, 0.0f);
        if (right) std::fill(right, right + numSamples, 0.0f);

        // 3. Carrier generates audio (into left/right)
        allBlocks[BLK_CARRIER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_CARRIER].pushBlock(left, numSamples);

        // 4. Carrier into Drive
        allBlocks[BLK_DRIVE]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_DRIVE].pushBlock(left, numSamples);

        // 5. Noise Transient into temporary buffer
        tempNoiseL.assign(numSamples, 0.0f);
        tempNoiseR.assign(numSamples, 0.0f);
        allBlocks[BLK_NOISE]->processStereo(tempNoiseL.data(), tempNoiseR.data(), numSamples, ctx);
        scopes[BLK_NOISE].pushBlock(tempNoiseL.data(), numSamples);

        // 6. Drive output and Noise Transient mixed together
        for (int i = 0; i < numSamples; ++i) {
            if (left)  left[i]  += tempNoiseL[i];
            if (right) right[i] += tempNoiseR[i];
        }

        // 7. Mixer output into Filter
        allBlocks[BLK_FILTER]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_FILTER].pushBlock(left, numSamples);

        // 8. Filter into RingMod
        allBlocks[BLK_RINGMOD]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_RINGMOD].pushBlock(left, numSamples);

        // 9. RingMod into Grit FX
        allBlocks[BLK_GRIT]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_GRIT].pushBlock(left, numSamples);

        // 10. Grit FX into Frequency Shifter
        allBlocks[BLK_FREQSHIFT]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_FREQSHIFT].pushBlock(left, numSamples);

        // 11. Frequency Shifter into Amp (applying amp envelope, master drive, low boost, pan, level)
        allBlocks[BLK_AMP]->processStereo(left, right, numSamples, ctx);
        scopes[BLK_AMP].pushBlock(left, numSamples);
    }

private:
    BlockContext ctx;
    std::vector<std::unique_ptr<DSPBlock>> allBlocks;
    VisualScope scopes[NUM_BLOCKS];
    mutable std::atomic<float> lastCarrierFreq{ 100.0f };
    std::vector<float> tempNoiseL;
    std::vector<float> tempNoiseR;
};

} // namespace TbdAudio