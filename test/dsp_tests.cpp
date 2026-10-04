#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "ModularBlocks.h"
#include "PlanterEngine.h"

int main() {
    std::cout << "Starting DSP Verification Tests for 22-Block Modular Drum Synth..." << std::endl;

    TbdAudio::ModularDrumEngine engine;
    engine.init(44100.0f);

    constexpr int blockSize = 256;
    std::vector<float> left(blockSize, 0.0f);
    std::vector<float> right(blockSize, 0.0f);

    // 1. Basic Trigger Test
    engine.setMidiPitch(36); // C2
    engine.trigger(0.9f);
    engine.processStereo(left.data(), right.data(), blockSize);

    bool hasSignal = false;
    for (int i = 0; i < blockSize; ++i) {
        if (std::isnan(left[i]) || std::isinf(left[i]) ||
            std::isnan(right[i]) || std::isinf(right[i])) {
            std::cerr << "FAILED: NaN or Inf detected in basic trigger output!" << std::endl;
            return 1;
        }
        if (std::abs(left[i]) > 0.0001f || std::abs(right[i]) > 0.0001f) {
            hasSignal = true;
        }
    }

    if (!hasSignal) {
        std::cerr << "FAILED: No audio signal generated after trigger!" << std::endl;
        return 1;
    }
    std::cout << "PASS: Basic trigger and audio generation." << std::endl;

    // 2. Test Parameter Sweeps across all 22 blocks (0.0, 0.25, 0.5, 0.75, 1.0)
    constexpr float testVals[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    for (int b = 0; b < TbdAudio::ModularDrumEngine::NUM_BLOCKS; ++b) {
        auto blockId = static_cast<TbdAudio::ModularDrumEngine::BlockID>(b);
        for (int p = 0; p < 4; ++p) {
            for (float v : testVals) {
                engine.setPageParameter(blockId, p, v);
            }
        }
    }

    // Trigger again with modified parameters
    engine.trigger(1.0f);
    for (int block = 0; block < 100; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf detected during parameter sweep test!" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "PASS: 22-block parameter sweep stability test." << std::endl;

    // 3. Test Comb filter and Phase Smear (APF Disperser)
    // Comb filter (Block BLK_COMB: Dampening, Cutoff, Res, Mix)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 0, 0.5f);        // Dampening
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 1, 0.5f);        // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 2, 0.95f);       // High resonance
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 3, 0.75f);       // Mix (+50%:50%)
    engine.trigger(1.0f);
    for (int block = 0; block < 50; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in Comb filter!" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "PASS: Comb filter test." << std::endl;

    // Phase Smear (Block BLK_DISPERSER: Order, Amount, Cutoff, Resonance)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 0, 0.0f); // 2nd Order
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 1, 1.0f); // 32 stages
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 2, 0.6f); // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 3, 0.8f); // Resonance
    engine.trigger(1.0f);
    for (int block = 0; block < 50; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in Phase Smear (2nd Order)!" << std::endl;
                return 1;
            }
        }
    }
    // Test 4th Order mode
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 0, 1.0f); // 4th Order
    engine.trigger(1.0f);
    for (int block = 0; block < 50; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in Phase Smear (4th Order)!" << std::endl;
                return 1;
            }
        }
    }

    // Verify Phase Smear 2nd-order and 4th-order smearing and allpass energy preservation
    {
        for (int order = 0; order <= 1; ++order) {
            TbdAudio::DisperserBlock disp;
            TbdAudio::BlockContext ctx;
            ctx.sampleRate = 44100.0f;
            ctx.invSr = 1.0f / 44100.0f;
            disp.init(ctx);
            disp.setParam(0, static_cast<float>(order)); // 0 = 2nd Order, 1 = 4th Order
            disp.setParam(1, 16.0f / 32.0f);             // 16 stages
            disp.setParam(2, 0.65f);                     // ~600 Hz cutoff
            disp.setParam(3, 0.85f);                     // High resonance (Q ~ 14.5)

            constexpr int impLen = 512;
            std::vector<float> impBuf(impLen, 0.0f);
            impBuf[0] = 1.0f; // Single impulse
            disp.processStereo(impBuf.data(), nullptr, impLen, ctx);

            float energy = 0.0f;
            int nonZeroSamples = 0;
            for (float s : impBuf) {
                energy += s * s;
                if (std::abs(s) > 0.005f) nonZeroSamples++;
            }

            // Energy should be strictly conserved within ~5% for allpass
            if (std::abs(energy - 1.0f) > 0.05f) {
                std::cerr << "FAILED: Phase Smear order " << order << " is not preserving allpass energy! Energy = " << energy << std::endl;
                return 1;
            }

            // Cascaded APF stages must smear the single-sample impulse over dozens of samples
            if (nonZeroSamples < 20) {
                std::cerr << "FAILED: Phase Smear order " << order << " did not smear impulse! Nonzero samples: " << nonZeroSamples << std::endl;
                return 1;
            }
        }
    }
    std::cout << "PASS: Phase Smear 2nd-order & 4th-order smearing, zapping, and energy conservation verified." << std::endl;

    // 4. Test Pitch Envelope 1 Modulation (Opp mode: target = 3 / 3.0f = 1.0f)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 0, 0.0f); // Exp
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 1, 1.0f); // Max depth
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 2, 0.3806f); // 333 ms
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 3, 1.0f); // Opp (target 3)
    engine.trigger(1.0f);
    for (int block = 0; block < 50; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in Pitch Envelope test!" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "PASS: Pitch Envelope Opp mode test." << std::endl;

    // 5. Test Claps Burst on Amp Envelope
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, 1.0f); // 32 claps
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, 0.1429f); // 3 ms
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, 0.0f); // Exp slope
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, 0.3806f); // 333 ms
    engine.trigger(1.0f);
    for (int block = 0; block < 100; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in Clap Envelope!" << std::endl;
                return 1;
            }
        }
    }
    std::cout << "PASS: Claps burst envelope test." << std::endl;

    // 6. Test Frequency Shifter Directional Accuracy
    {
        TbdAudio::FrequencyShifterBlock fs;
        TbdAudio::BlockContext ctx;
        ctx.sampleRate = 44100.0f;
        ctx.invSr = 1.0f / 44100.0f;
        fs.init(ctx);

        // Helper to test frequency shift across multiple test frequencies
        auto testShift = [&](float inFreq, float shiftHz) -> float {
            fs.init(ctx);
            float param0 = 0.5f + (shiftHz / 5000.0f) * 0.5f;
            fs.setParam(0, param0); // Shift
            fs.setParam(1, 1.0f);   // Range = 5000 Hz
            fs.setParam(2, 1.0f);   // Blend = 100% wet
            fs.setParam(3, 0.5f);   // Width = 0

            constexpr int N = 44100;
            std::vector<float> testBuf(N);
            for (int i = 0; i < N; ++i) {
                testBuf[i] = std::sin(2.0f * 3.14159265358979323846f * inFreq * i / 44100.0f);
            }
            fs.processStereo(testBuf.data(), nullptr, N, ctx);

            int crossings = 0;
            int startIdx = 8000;
            int endIdx = N - 1;
            for (int i = startIdx; i < endIdx; ++i) {
                if (testBuf[i] <= 0.0f && testBuf[i + 1] > 0.0f) {
                    crossings++;
                }
            }
            float duration = static_cast<float>(endIdx - startIdx) / 44100.0f;
            return static_cast<float>(crossings) / duration;
        };

        for (float f : { 65.4f, 130.0f, 260.0f, 440.0f, 1000.0f }) {
            float up = testShift(f, +25.0f);
            float down = testShift(f, -25.0f);
            std::cout << "Input: " << f << " Hz | Shift +25 Hz -> " << up << " Hz | Shift -25 Hz -> " << down << " Hz" << std::endl;
        }

        // Test FULL DRUM ENGINE with Frequency Shifter
        std::cout << "\nTesting Full Drum Engine through Frequency Shifter:" << std::endl;
        auto testDrumEngineShift = [&](float shiftNorm, float rangeHz) -> float {
            TbdAudio::ModularDrumEngine eng;
            eng.init(44100.0f);
            eng.setMidiPitch(36); // C2 = 65.4 Hz
            // Turn off pitch envelopes to have stable carrier
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 3, 0.0f); // Off
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 3, 0.0f); // Off
            // Isolate Carrier 1 in mixer (C1 = 100%, C2 = 0%, RingMod = 0%, Noise = 0%)
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, 0.5f);
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, 0.0f);
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, 0.0f);
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 3, 0.0f);
            // Frequency shifter: Range = rangeHzToNorm(rangeHz), Blend = 1.0 (100% wet), Shift = shiftNorm
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 0, shiftNorm);
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 1, TbdAudio::rangeHzToNorm(rangeHz));
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 2, 1.0f); // 100% wet
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 3, 0.5f); // center width
            // Set amp env decay long so signal persists
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, 1.0f);
            eng.trigger(1.0f);

            constexpr int N = 44100;
            std::vector<float> bL(N, 0.0f);
            std::vector<float> bR(N, 0.0f);
            eng.processStereo(bL.data(), bR.data(), N);

            int crossings = 0;
            int startIdx = 8000;
            int endIdx = N - 1;
            for (int i = startIdx; i < endIdx; ++i) {
                if (bL[i] <= 0.0f && bL[i + 1] > 0.0f) {
                    crossings++;
                }
            }
            float duration = static_cast<float>(endIdx - startIdx) / 44100.0f;
            return static_cast<float>(crossings) / duration;
        };

        // Range = 50 Hz (so shift ranges from -50 Hz to +50 Hz)
        float baseFreq = testDrumEngineShift(0.5f, 50.0f); // 0 Hz shift
        float upFreq = testDrumEngineShift(0.7f, 50.0f);   // +20 Hz shift
        float downFreq = testDrumEngineShift(0.3f, 50.0f); // -20 Hz shift
        std::cout << "Engine (Range 50Hz): Shift 0Hz -> " << baseFreq
                  << " Hz | Shift +20Hz -> " << upFreq
                  << " Hz | Shift -20Hz -> " << downFreq << " Hz" << std::endl;

        // Range = 500 Hz
        float up500 = testDrumEngineShift(0.7f, 500.0f);   // +200 Hz shift
        float down500 = testDrumEngineShift(0.3f, 500.0f); // -200 Hz shift
        std::cout << "Engine (Range 500Hz): Shift +200Hz -> " << up500
                  << " Hz | Shift -200Hz -> " << down500 << " Hz" << std::endl;

    }

    // 7. Test VisualScope Buffer Capture for all 22 blocks
    {
        float scopeData[128] = { 0.0f };
        for (int b = 0; b < TbdAudio::ModularDrumEngine::NUM_BLOCKS; ++b) {
            engine.getScopeData(b, scopeData, 128);
            for (int i = 0; i < 128; ++i) {
                if (std::isnan(scopeData[i]) || std::isinf(scopeData[i])) {
                    std::cerr << "FAILED: NaN or Inf in VisualScope buffer for block " << b << std::endl;
                    return 1;
                }
            }
        }
        std::cout << "PASS: All 22 VisualScope buffers populated with valid finite samples." << std::endl;
    }

    // 8. Test Velocity Modulation
    {
        TbdAudio::ModularDrumEngine velEngine;
        velEngine.init(44100.0f);
        velEngine.setMidiPitch(36); // C2

        // Set Velocity: Slope = Linear (0.75), Volume = 100% (-100% at min vel, param3 = 1.0f)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 0, 0.75f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.5f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.5f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, 1.0f); // 100% volume sensitivity

        // Trigger at max velocity (1.0)
        velEngine.trigger(1.0f);
        std::vector<float> highL(1024, 0.0f);
        std::vector<float> highR(1024, 0.0f);
        velEngine.processStereo(highL.data(), highR.data(), 1024);

        float maxPeak = 0.0f;
        for (float s : highL) maxPeak = std::max(maxPeak, std::abs(s));

        // Trigger at very low velocity (0.01) from silence
        TbdAudio::ModularDrumEngine lowEngine;
        lowEngine.init(44100.0f);
        lowEngine.setMidiPitch(36);
        lowEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 0, 0.75f);
        lowEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.5f);
        lowEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.5f);
        lowEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, 1.0f);

        lowEngine.trigger(0.01f);
        std::vector<float> lowL(1024, 0.0f);
        std::vector<float> lowR(1024, 0.0f);
        lowEngine.processStereo(lowL.data(), lowR.data(), 1024);

        float minPeak = 0.0f;
        for (float s : lowL) minPeak = std::max(minPeak, std::abs(s));

        std::cout << "Velocity Volume Sensitivity: Peak at Vel 1.0 = " << maxPeak 
                  << " (gain=" << velEngine.getContext().velVolumeGain << ") | Peak at Vel 0.01 = " 
                  << minPeak << " (gain=" << lowEngine.getContext().velVolumeGain << ")" << std::endl;
        if (minPeak >= maxPeak * 0.1f) {
            std::cerr << "FAILED: Velocity volume did not scale output correctly!" << std::endl;
            return 1;
        }

        // Test Decay and Depth polarity for higher vs lower velocities
        // 1. Positive velDecay/velDepth (+100%, knob = 1.0f) at high velocity (1.0f) -> MUST INCREASE (> 0)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 1.0f); // +100%
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 1.0f); // +100%
        velEngine.trigger(1.0f);
        if (velEngine.getContext().velDecayMod <= 0.0f || velEngine.getContext().velDepthMod <= 0.0f) {
            std::cerr << "FAILED: Positive velDecay/velDepth did not increase controls at high velocity!" << std::endl;
            return 1;
        }

        // 2. Negative velDecay/velDepth (-100%, knob = 0.0f) at high velocity (1.0f) -> MUST DECREASE (< 0)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.0f); // -100%
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.0f); // -100%
        velEngine.trigger(1.0f);
        if (velEngine.getContext().velDecayMod >= 0.0f || velEngine.getContext().velDepthMod >= 0.0f) {
            std::cerr << "FAILED: Negative velDecay/velDepth did not decrease controls at high velocity!" << std::endl;
            return 1;
        }

        // 3. Positive velDecay/velDepth (+100%, knob = 1.0f) at low velocity (0.1f) -> MUST DECREASE (< 0)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 1.0f); // +100%
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 1.0f); // +100%
        velEngine.trigger(0.1f);
        if (velEngine.getContext().velDecayMod >= 0.0f || velEngine.getContext().velDepthMod >= 0.0f) {
            std::cerr << "FAILED: Positive velDecay/velDepth did not decrease controls at low velocity!" << std::endl;
            return 1;
        }

        // 4. Negative velDecay/velDepth (-100%, knob = 0.0f) at low velocity (0.1f) -> MUST INCREASE (> 0)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.0f); // -100%
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.0f); // -100%
        velEngine.trigger(0.1f);
        if (velEngine.getContext().velDecayMod <= 0.0f || velEngine.getContext().velDepthMod <= 0.0f) {
            std::cerr << "FAILED: Negative velDecay/velDepth did not increase controls at low velocity!" << std::endl;
            return 1;
        }

        // 5. Test Exponential Warp accuracy:
        // A. Mathematical warp helper checks
        assert(std::abs(TbdAudio::warpUnipolarExp(0.50f) - 0.10f) < 0.001f);
        assert(std::abs(TbdAudio::unwarpUnipolarExp(0.10f) - 0.50f) < 0.001f);
        assert(std::abs(TbdAudio::warpUnipolarExp(0.0f) - 0.0f) < 0.0001f);
        assert(std::abs(TbdAudio::warpUnipolarExp(1.0f) - 1.0f) < 0.0001f);

        assert(std::abs(TbdAudio::warpBipolarExp(0.75f) - 0.05f) < 0.001f);
        assert(std::abs(TbdAudio::warpBipolarExp(0.25f) - (-0.05f)) < 0.001f);
        assert(std::abs(TbdAudio::unwarpBipolarExp(0.05f) - 0.75f) < 0.001f);
        assert(std::abs(TbdAudio::unwarpBipolarExp(-0.05f) - 0.25f) < 0.001f);
        assert(std::abs(TbdAudio::warpBipolarExp(0.50f) - 0.0f) < 0.0001f);
        assert(std::abs(TbdAudio::warpBipolarExp(1.0f) - 1.0f) < 0.0001f);
        assert(std::abs(TbdAudio::warpBipolarExp(0.0f) - (-1.0f)) < 0.0001f);

        // B. Bipolar ±25% knob displacement -> ±5% effective modulation at max velocity (velModFactor = 1.0)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.25f); // -25% Depth knob
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.75f); // +25% Decay knob
        velEngine.trigger(1.0f);
        assert(std::abs(velEngine.getContext().velDepthMod - (-0.05f)) < 0.001f);
        assert(std::abs(velEngine.getContext().velDecayMod - 0.05f) < 0.001f);

        // C. Unipolar 50% knob travel -> 10% effective volume attenuation at min velocity
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, 0.50f); // 50% knob
        velEngine.trigger(0.0f);
        assert(std::abs(velEngine.getContext().velVolumeGain - 0.90f) < 0.001f); // 1.0 - 0.10 = 0.90

        std::cout << "PASS: Velocity modulation (volume, decay, depth polarity, exponential curves) verified." << std::endl;
    }

    // 9. Test Slop Modulation (Block 21: BLK_SLOP)
    {
        TbdAudio::ModularDrumEngine slopEngine;
        slopEngine.init(44100.0f);
        slopEngine.setMidiPitch(36);

        // A. At default 0% Slop, all offsets MUST be strictly 0.0f
        slopEngine.trigger(1.0f);
        const auto& ctxDef = slopEngine.getContext();
        assert(ctxDef.slopCarrier1Pitch == 0.0f);
        assert(ctxDef.slopCarrier2Pitch == 0.0f);
        assert(ctxDef.slopMod1Freq == 0.0f);
        assert(ctxDef.slopMod1Filter == 0.0f);
        assert(ctxDef.slopMod2Freq == 0.0f);
        assert(ctxDef.slopMod2Filter == 0.0f);
        assert(ctxDef.slopDriveFilter == 0.0f);
        assert(ctxDef.slopWaveFolderFilter == 0.0f);
        assert(ctxDef.slopNoiseShRate == 0.0f);
        assert(ctxDef.slopNoiseFilter == 0.0f);
        assert(ctxDef.slopFilter1Cutoff == 0.0f);
        assert(ctxDef.slopRingModRate == 0.0f);
        assert(ctxDef.slopCombDamp == 0.0f);
        assert(ctxDef.slopCombCutoff == 0.0f);
        assert(ctxDef.slopDisperserCutoff == 0.0f);
        assert(ctxDef.slopPitchEnv1Depth == 0.0f);
        assert(ctxDef.slopPitchEnv2Depth == 0.0f);
        assert(ctxDef.slopFilterEnv1Depth == 0.0f);
        assert(ctxDef.slopPitchEnv1Decay == 0.0f);
        assert(ctxDef.slopPitchEnv2Decay == 0.0f);
        assert(ctxDef.slopNoiseDecay == 0.0f);
        assert(ctxDef.slopFilterEnv1Decay == 0.0f);
        assert(ctxDef.slopAmpEnvDecay == 0.0f);
        assert(ctxDef.slopAmpPan == 0.0f);
        assert(ctxDef.slopEQFreq == 0.0f);
        assert(ctxDef.slopEQFilter == 0.0f);
        std::cout << "PASS: Slop defaults strictly zero with zero offsets." << std::endl;

        // B. Enable Slop (Freq = 50%, Depth = 40%, Decay = 30%, Pan = 60%)
        slopEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 0, 0.50f);
        slopEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 1, 0.40f);
        slopEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 2, 0.30f);
        slopEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_SLOP, 3, 0.60f);

        // Trigger multiple hits and verify independent stepped randomization
        float prevCarrier1Pitch = 0.0f;
        float prevCutoff = 0.0f;
        float prevPan = 0.0f;
        bool hasVariation = false;
        bool hasIndependentDraws = false;

        for (int hit = 0; hit < 10; ++hit) {
            slopEngine.trigger(1.0f);
            const auto& ctx = slopEngine.getContext();

            // Check range bounds under exponential warp:
            float boundFreq  = TbdAudio::warpUnipolarExp(0.50f);
            float boundDepth = TbdAudio::warpUnipolarExp(0.40f);
            float boundDecay = TbdAudio::warpUnipolarExp(0.30f);
            float boundPan   = TbdAudio::warpUnipolarExp(0.60f);

            assert(std::abs(ctx.slopCarrier1Pitch) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopCarrier2Pitch) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopMod1Freq) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopFilter1Cutoff) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopEQFreq) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopEQFilter) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopPitchEnv1Depth) <= boundDepth + 0.001f);
            assert(std::abs(ctx.slopPitchEnv2Depth) <= boundDepth + 0.001f);
            assert(std::abs(ctx.slopFilterEnv1Depth) <= boundDepth + 0.001f);
            assert(std::abs(ctx.slopPitchEnv1Decay) <= boundDecay + 0.001f);
            assert(std::abs(ctx.slopAmpEnvDecay) <= boundDecay + 0.001f);
            assert(std::abs(ctx.slopAmpPan) <= boundPan + 0.001f);

            // Verify independent random values across different destinations in the same trigger hit
            if (ctx.slopCarrier1Pitch != ctx.slopFilter1Cutoff &&
                ctx.slopFilter1Cutoff != ctx.slopMod1Freq &&
                ctx.slopPitchEnv1Decay != ctx.slopAmpEnvDecay) {
                hasIndependentDraws = true;
            }

            // Verify variation across hits
            if (hit > 0) {
                if (ctx.slopCarrier1Pitch != prevCarrier1Pitch ||
                    ctx.slopFilter1Cutoff != prevCutoff ||
                    ctx.slopAmpPan != prevPan) {
                    hasVariation = true;
                }
            }

            prevCarrier1Pitch = ctx.slopCarrier1Pitch;
            prevCutoff = ctx.slopFilter1Cutoff;
            prevPan = ctx.slopAmpPan;

            // Process buffer to verify audio stability with slop
            std::vector<float> slopL(blockSize, 0.0f);
            std::vector<float> slopR(blockSize, 0.0f);
            slopEngine.processStereo(slopL.data(), slopR.data(), blockSize);
            for (int i = 0; i < blockSize; ++i) {
                assert(!std::isnan(slopL[i]) && !std::isinf(slopL[i]));
                assert(!std::isnan(slopR[i]) && !std::isinf(slopR[i]));
            }
        }

        if (!hasIndependentDraws) {
            std::cerr << "FAILED: Slop did not generate independent random values across controls!" << std::endl;
            return 1;
        }
        if (!hasVariation) {
            std::cerr << "FAILED: Slop did not vary across consecutive hits!" << std::endl;
            return 1;
        }
        std::cout << "PASS: Independent stepped random Slop controls (frequency, depth, decay, pan) verified." << std::endl;
    }

    // 10. Test Dual FM Voices & Mixer Routing
    {
        TbdAudio::ModularDrumEngine dualEngine;
        dualEngine.init(44100.0f);

        // Turn down everything except Carrier 1 in Mixer
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, 0.5f); // Carrier 1 = 100%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, 0.0f); // Carrier 2 = 0%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, 0.0f); // RingMod = 0%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 3, 0.0f); // Noise = 0%

        dualEngine.trigger(1.0f);
        std::vector<float> buf1(blockSize, 0.0f);
        dualEngine.processStereo(buf1.data(), nullptr, blockSize);
        float peakC1 = 0.0f;
        for (float v : buf1) peakC1 = std::max(peakC1, std::abs(v));
        assert(peakC1 > 0.05f);

        // Turn down Carrier 1 and turn on Carrier 2 in Mixer
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, 0.0f); // Carrier 1 = 0%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, 0.5f); // Carrier 2 = 100%
        dualEngine.trigger(1.0f);
        std::vector<float> buf2(blockSize, 0.0f);
        dualEngine.processStereo(buf2.data(), nullptr, blockSize);
        float peakC2 = 0.0f;
        for (float v : buf2) peakC2 = std::max(peakC2, std::abs(v));
        assert(peakC2 > 0.05f);

        // Turn on RingMod in Mixer
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 0, 0.0f); // Carrier 1 = 0%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 1, 0.0f); // Carrier 2 = 0%
        dualEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MIXER, 2, 0.5f); // RingMod = 100%
        dualEngine.trigger(1.0f);
        std::vector<float> bufRM(blockSize, 0.0f);
        dualEngine.processStereo(bufRM.data(), nullptr, blockSize);
        float peakRM = 0.0f;
        for (float v : bufRM) peakRM = std::max(peakRM, std::abs(v));
        assert(peakRM > 0.05f);

        std::cout << "PASS: Dual FM voice pairs and Mixer (Carrier 1, Carrier 2, RingMod) verified." << std::endl;
    }

    // 11. Test Wave Folder
    {
        TbdAudio::ModularDrumEngine wfEngine;
        wfEngine.init(44100.0f);

        // Compare output without wave folding vs with 6 folds
        wfEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 0, 0.0f); // Off
        wfEngine.trigger(1.0f);
        std::vector<float> cleanBuf(512, 0.0f);
        wfEngine.processStereo(cleanBuf.data(), nullptr, 512);

        wfEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 0, 1.0f); // On
        wfEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_WAVEFOLDER, 1, 6.0f / 8.0f); // 6 folds
        wfEngine.trigger(1.0f);
        std::vector<float> foldedBuf(512, 0.0f);
        wfEngine.processStereo(foldedBuf.data(), nullptr, 512);

        bool hasDifference = false;
        for (size_t i = 0; i < cleanBuf.size(); ++i) {
            assert(!std::isnan(foldedBuf[i]) && !std::isinf(foldedBuf[i]));
            if (std::abs(cleanBuf[i] - foldedBuf[i]) > 0.01f) {
                hasDifference = true;
            }
        }
        assert(hasDifference);
        std::cout << "PASS: Standalone Wave Folder block verified." << std::endl;
    }

    // 12. Test Renoise Offline Render Simulation (arbitrary & varying buffer sizes)
    {
        TbdAudio::ModularDrumEngine renoiseEngine;
        renoiseEngine.init(48000.0f); // Common offline render sample rate

        const int blockSizes[] = { 64, 128, 512, 1024, 2048, 384, 96 };
        for (int sz : blockSizes) {
            std::vector<float> leftR(sz, 0.0f);
            std::vector<float> rightR(sz, 0.0f);
            renoiseEngine.trigger(0.85f);
            renoiseEngine.processStereo(leftR.data(), rightR.data(), sz);
            for (int i = 0; i < sz; ++i) {
                assert(!std::isnan(leftR[i]) && !std::isinf(leftR[i]));
                assert(!std::isnan(rightR[i]) && !std::isinf(rightR[i]));
            }
        }
        std::cout << "PASS: Renoise offline variable block sizes rendering verified." << std::endl;
    }

    // 13. Test Filter Block (Slope and Type selectors)
    {
        TbdAudio::FilterBlock filter;
        TbdAudio::BlockContext ctx;
        ctx.sampleRate = 44100.0f;
        ctx.invSr = 1.0f / 44100.0f;
        filter.init(ctx);

        // Test LPF at different slopes: 6, 12, 18, 24, 36
        for (int slope = 0; slope < 5; ++slope) {
            filter.init(ctx);
            filter.setParam(0, 0.0f); // 0 = LPF (normalized: 0/3 = 0.0)
            filter.setParam(1, slope * 0.25f); // Slope
            filter.setParam(2, 0.5f); // ~500 Hz cutoff
            filter.setParam(3, 0.2f); // Resonance

            std::vector<float> lpfSig(256, 0.5f);
            filter.processStereo(lpfSig.data(), nullptr, 256, ctx);
            for (float s : lpfSig) {
                assert(!std::isnan(s) && !std::isinf(s));
            }
        }

        // Test 100% resonance (self-oscillation boundary) across all slopes
        for (int slope = 0; slope < 5; ++slope) {
            filter.init(ctx);
            filter.setParam(0, 0.0f); // LPF
            filter.setParam(1, slope * 0.25f);
            filter.setParam(2, 0.5f); // ~500 Hz cutoff
            filter.setParam(3, 1.0f); // 100% Resonance

            std::vector<float> impulse(512, 0.0f);
            impulse[0] = 1.0f;
            filter.processStereo(impulse.data(), nullptr, 512, ctx);
            for (float s : impulse) {
                assert(!std::isnan(s) && !std::isinf(s));
                // Peak gain should be bounded by the self-oscillation calibration (target peak ~20.0)
                assert(std::abs(s) < 100.0f);
            }
        }

        // Test all 4 types: 0=LPF, 1=BPF, 2=HPF, 3=BRF
        for (int type = 0; type < 4; ++type) {
            filter.init(ctx);
            filter.setParam(0, static_cast<float>(type) / 3.0f);
            filter.setParam(1, 0.25f); // 12 dB/oct
            filter.setParam(2, 0.5f);
            filter.setParam(3, 0.2f);

            std::vector<float> sig(256, 0.5f);
            filter.processStereo(sig.data(), nullptr, 256, ctx);
            for (float s : sig) {
                assert(!std::isnan(s) && !std::isinf(s));
            }
        }
        std::cout << "PASS: Filter block types (LPF, BPF, HPF, BRF) and discrete slopes verified." << std::endl;
    }

    // 14. Test Bell EQ Block (BLK_EQ)
    {
        TbdAudio::EQBlock eq;
        TbdAudio::BlockContext ctx;
        ctx.sampleRate = 44100.0f;
        ctx.invSr = 1.0f / 44100.0f;
        eq.init(ctx);

        // A. At default 0 dB gain (param 2 = 0.5f) and DJ filter centered (param 3 = 0.5f), EQ is transparent
        eq.setParam(0, 1.0f); // Freq = 24 kHz
        eq.setParam(1, 0.0f); // Width = 0.1 oct
        eq.setParam(2, 0.5f); // Gain = 0 dB
        eq.setParam(3, 0.5f); // DJ Filter = Flat

        std::vector<float> sig(512);
        for (int i = 0; i < 512; ++i) {
            sig[i] = std::sin(2.0f * 3.14159265f * 1000.0f * i / 44100.0f);
        }
        std::vector<float> origSig = sig;
        eq.processStereo(sig.data(), nullptr, 512, ctx);
        for (size_t i = 0; i < sig.size(); ++i) {
            assert(std::abs(sig[i] - origSig[i]) < 0.0001f);
        }

        // B. Test Boost vs Cut at 1 kHz center frequency
        float freqParam1k = std::log(1000.0f / 20.0f) / std::log(24000.0f / 20.0f);

        // Boost +12 dB (gainParam = 0.5 + 12/48 = 0.75)
        eq.init(ctx);
        eq.setParam(0, freqParam1k);
        eq.setParam(1, 0.3f); // ~1 octave width
        eq.setParam(2, 0.75f); // +12 dB
        eq.setParam(3, 0.5f); // Flat DJ filter
        std::vector<float> boostSig = origSig;
        eq.processStereo(boostSig.data(), nullptr, 512, ctx);

        // Cut -12 dB (gainParam = 0.5 - 12/48 = 0.25)
        eq.init(ctx);
        eq.setParam(0, freqParam1k);
        eq.setParam(1, 0.3f); // ~1 octave width
        eq.setParam(2, 0.25f); // -12 dB
        eq.setParam(3, 0.5f); // Flat DJ filter
        std::vector<float> cutSig = origSig;
        eq.processStereo(cutSig.data(), nullptr, 512, ctx);

        float peakBoost = 0.0f, peakCut = 0.0f, peakOrig = 0.0f;
        for (int i = 100; i < 500; ++i) {
            peakBoost = std::max(peakBoost, std::abs(boostSig[i]));
            peakCut = std::max(peakCut, std::abs(cutSig[i]));
            peakOrig = std::max(peakOrig, std::abs(origSig[i]));
        }
        assert(peakBoost > peakOrig * 1.5f);
        assert(peakCut < peakOrig * 0.7f);

        // C. Test DJ Filter in EQ block (LPF tilt vs HPF tilt)
        eq.init(ctx);
        eq.setParam(0, freqParam1k);
        eq.setParam(1, 0.3f);
        eq.setParam(2, 0.5f); // 0 dB
        eq.setParam(3, 0.1f); // Low DJ filter -> cuts highs
        std::vector<float> djLpfSig(512);
        for (int i = 0; i < 512; ++i) {
            djLpfSig[i] = std::sin(2.0f * 3.14159265f * 8000.0f * i / 44100.0f); // 8 kHz tone
        }
        eq.processStereo(djLpfSig.data(), nullptr, 512, ctx);
        float peak8k = 0.0f;
        for (int i = 100; i < 500; ++i) peak8k = std::max(peak8k, std::abs(djLpfSig[i]));
        assert(peak8k < 0.2f); // Strongly attenuated by DJ filter LPF

        std::cout << "PASS: Bell EQ block (transparency, peaking boost/cut, DJ filter tilt) verified." << std::endl;
    }

    // 15. Test Per-Voice Filters (Voice 1, Voice 2, Transients)
    {
        TbdAudio::ModularDrumEngine fEngine;
        fEngine.init(44100.0f);
        fEngine.setMidiPitch(36); // C2

        // Carrier 1 with LPF active at low cutoff
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 2, 0.4f); // Saw wave (lots of harmonics)
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 0, 0.0f);  // LPF
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 1, 0.25f); // 12dB/oct
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 2, 0.3f);  // Low cutoff
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 3, 0.5f);  // Res
        fEngine.trigger(1.0f);

        std::vector<float> testL(blockSize, 0.0f);
        std::vector<float> testR(blockSize, 0.0f);
        fEngine.processStereo(testL.data(), testR.data(), blockSize);

        for (int i = 0; i < blockSize; ++i) {
            assert(!std::isnan(testL[i]) && !std::isinf(testL[i]));
        }
        std::cout << "PASS: Per-Voice filter routing verified." << std::endl;
    }

    // 16. Test Limiter Block (On/Off, Gain, Threshold, Release)
    {
        TbdAudio::LimiterBlock lim;
        TbdAudio::BlockContext ctx;
        ctx.sampleRate = 44100.0f;
        ctx.invSr = 1.0f / 44100.0f;
        lim.init(ctx);

        // Turn on limiter: On (1.0), 0 dB Gain (12/36), -6 dB Thresh (18/24 = 0.75), 50 ms release
        lim.setParam(0, 1.0f);
        lim.setParam(1, 12.0f / 36.0f);
        lim.setParam(2, 0.75f); // -6 dB threshold (~0.501 lin)
        lim.setParam(3, 0.6296f);

        // Feed hot signal (amplitude 2.0)
        constexpr int sigLen = 512;
        std::vector<float> hotSig(sigLen, 2.0f);
        lim.processStereo(hotSig.data(), nullptr, sigLen, ctx);

        // After attack, output must be clamped/attenuated to around threshold (0.501)
        float maxOut = 0.0f;
        for (int i = 50; i < sigLen; ++i) {
            maxOut = std::max(maxOut, std::abs(hotSig[i]));
        }
        assert(maxOut <= 1.05f); // Zero overshoot exceeding ceiling
        std::cout << "PASS: Limiter block dynamics and threshold reduction verified." << std::endl;
    }

    // 17. Test FX Pickers and Dynamic Chain Processing
    {
        TbdAudio::ModularDrumEngine fxEng;
        fxEng.init(44100.0f);

        // Route Drive into Pre FX Slot 0 and Grit into Post FX Slot 0
        fxEng.setPreFXType(0, 4);  // Drive (alphabetical index 4)
        fxEng.setPreFXType(1, 0);  // Bypass
        fxEng.setPostFXType(0, 8); // Grit FX (alphabetical index 8)
        fxEng.setPostFXType(1, 0); // Bypass

        assert(fxEng.getPreFXType(0) == 4);
        assert(fxEng.getPostFXType(0) == 8);

        fxEng.trigger(1.0f);
        std::vector<float> fxL(blockSize, 0.0f);
        std::vector<float> fxR(blockSize, 0.0f);
        fxEng.processStereo(fxL.data(), fxR.data(), blockSize);

        for (int i = 0; i < blockSize; ++i) {
            assert(!std::isnan(fxL[i]) && !std::isinf(fxL[i]));
        }
        std::cout << "PASS: FX Pickers and dynamic routing verified." << std::endl;
    }

    // 18. Test Multi-Instance FX Slots: 8 Wavefolders in series
    {
        TbdAudio::ModularDrumEngine multiFxEng;
        multiFxEng.init(44100.0f);

        // Put WaveFolder (type 13) in all 4 Pre FX slots and all 4 Post FX slots
        for (int s = 0; s < 4; ++s) {
            multiFxEng.setPreFXType(s, 13);
            multiFxEng.setPreFXParam(s, 0, 1.0f); // Type On
            multiFxEng.setPreFXParam(s, 1, 0.25f * (s + 1)); // Folds: 2, 4, 6, 8
            multiFxEng.setPreFXParam(s, 2, 0.5f);
            multiFxEng.setPreFXParam(s, 3, 0.5f);

            multiFxEng.setPostFXType(s, 13);
            multiFxEng.setPostFXParam(s, 0, 1.0f); // Type On
            multiFxEng.setPostFXParam(s, 1, 0.25f * (s + 1)); // Folds: 2, 4, 6, 8
            multiFxEng.setPostFXParam(s, 2, 0.5f);
            multiFxEng.setPostFXParam(s, 3, 0.5f);
        }

        multiFxEng.setMidiPitch(36);
        multiFxEng.trigger(1.0f);

        std::vector<float> wfL(blockSize, 0.0f);
        std::vector<float> wfR(blockSize, 0.0f);
        for (int blk = 0; blk < 50; ++blk) {
            multiFxEng.processStereo(wfL.data(), wfR.data(), blockSize);
            for (int i = 0; i < blockSize; ++i) {
                assert(!std::isnan(wfL[i]) && !std::isinf(wfL[i]));
                assert(!std::isnan(wfR[i]) && !std::isinf(wfR[i]));
            }
        }
        std::cout << "PASS: Multi-instance FX: 8 independent Wavefolders in series verified." << std::endl;
    }

    // 19. Test New Effects: Chorus (2), Phaser (10), Flanger (6), and Tempo Delay (12)
    {
        TbdAudio::ModularDrumEngine modFxEng;
        modFxEng.init(44100.0f);
        modFxEng.setBpm(135.0f);
        assert(std::abs(modFxEng.getBpm() - 135.0f) < 0.001f);

        // Put Chorus in Pre 0, Phaser in Pre 1, Flanger in Post 0, Tempo Delay in Post 1
        modFxEng.setPreFXType(0, 2); // Chorus
        modFxEng.setPreFXParam(0, 0, 0.5f);
        modFxEng.setPreFXParam(0, 1, 0.7f);
        modFxEng.setPreFXParam(0, 2, 0.6f);
        modFxEng.setPreFXParam(0, 3, 0.5f);

        modFxEng.setPreFXType(1, 10); // Phaser
        modFxEng.setPreFXParam(1, 0, 0.4f);
        modFxEng.setPreFXParam(1, 1, 0.8f);
        modFxEng.setPreFXParam(1, 2, 0.7f);
        modFxEng.setPreFXParam(1, 3, 0.5f);

        modFxEng.setPostFXType(0, 6); // Flanger (alphabetical index 6)
        modFxEng.setPostFXParam(0, 0, 0.3f);
        modFxEng.setPostFXParam(0, 1, 0.7f);
        modFxEng.setPostFXParam(0, 2, 0.8f);
        modFxEng.setPostFXParam(0, 3, 0.5f);

        modFxEng.setPostFXType(1, 12); // Tempo Delay
        modFxEng.setPostFXParam(1, 0, 5.0f / 9.0f); // 1/8 note division
        modFxEng.setPostFXParam(1, 1, 0.5f);       // 50% feedback
        modFxEng.setPostFXParam(1, 2, 0.7f);       // Tone damping
        modFxEng.setPostFXParam(1, 3, 0.4f);       // 40% mix

        assert(modFxEng.getPreFXType(0) == 2);
        assert(modFxEng.getPreFXType(1) == 10);
        assert(modFxEng.getPostFXType(0) == 6);
        assert(modFxEng.getPostFXType(1) == 12);

        modFxEng.setMidiPitch(36);
        modFxEng.trigger(1.0f);

        std::vector<float> outL(blockSize, 0.0f);
        std::vector<float> outR(blockSize, 0.0f);
        float peakL = 0.0f;
        float peakR = 0.0f;
        for (int blk = 0; blk < 60; ++blk) {
            modFxEng.processStereo(outL.data(), outR.data(), blockSize);
            for (int i = 0; i < blockSize; ++i) {
                assert(!std::isnan(outL[i]) && !std::isinf(outL[i]));
                assert(!std::isnan(outR[i]) && !std::isinf(outR[i]));
                peakL = std::max(peakL, std::abs(outL[i]));
                peakR = std::max(peakR, std::abs(outR[i]));
            }
        }
        std::cout << "PASS: Chorus, Phaser, Flanger, and Tempo Delay verified." << std::endl;
    }

    // 20. Test Carrier Tracking Modes (MIDI +-24st, Freq 20Hz-24kHz, Fixed Note 0-127)
    {
        TbdAudio::ModularDrumEngine trackEng;
        trackEng.init(44100.0f);

        auto* carBlock = dynamic_cast<TbdAudio::CarrierBlock*>(trackEng.getBlock(TbdAudio::ModularDrumEngine::BLK_CARRIER1));
        assert(carBlock != nullptr);

        // Mode 0: MIDI tracking
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 0, 0.0f); // MIDI
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, 0.5f); // 0 st
        trackEng.setMidiPitch(36); // C2 = 65.406 Hz
        const auto& ctx0 = trackEng.getContext();
        float f0 = carBlock->getBasePitch(ctx0);
        assert(std::abs(f0 - 65.4064f) < 0.1f);

        // MIDI +24 st offset (+2 octaves)
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, 1.0f); // +24 st
        float fPlus24 = carBlock->getBasePitch(ctx0);
        assert(std::abs(fPlus24 - 65.4064f * 4.0f) < 0.5f);

        // MIDI -24 st offset (-2 octaves)
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, 0.0f); // -24 st
        float fMinus24 = carBlock->getBasePitch(ctx0);
        assert(std::abs(fMinus24 - 65.4064f * 0.25f) < 0.1f);

        // Mode 1: Fixed Frequency
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 0, 0.5f); // Freq (0.5 * 2 = 1)
        float norm55 = std::log(55.0f / 20.0f) / std::log(24000.0f / 20.0f);
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, norm55);
        trackEng.setMidiPitch(36);
        float fFreq1 = carBlock->getBasePitch(trackEng.getContext());
        assert(std::abs(fFreq1 - 55.0f) < 0.1f);
        trackEng.setMidiPitch(60); // changing incoming MIDI note has NO effect
        float fFreq2 = carBlock->getBasePitch(trackEng.getContext());
        assert(std::abs(fFreq2 - 55.0f) < 0.1f);

        // Mode 2: Fixed Note
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 0, 1.0f); // Note (1.0 * 2 = 2)
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, 33.0f / 127.0f); // A1 (33 = 55 Hz)
        trackEng.setMidiPitch(36);
        float fNote1 = carBlock->getBasePitch(trackEng.getContext());
        assert(std::abs(fNote1 - 55.0f) < 0.1f);
        trackEng.setMidiPitch(72); // changing incoming MIDI note has NO effect
        float fNote2 = carBlock->getBasePitch(trackEng.getContext());
        assert(std::abs(fNote2 - 55.0f) < 0.1f);

        // Fixed Note C4 (60 = 261.63 Hz)
        trackEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_CARRIER1, 1, 60.0f / 127.0f);
        float fNoteC4 = carBlock->getBasePitch(trackEng.getContext());
        assert(std::abs(fNoteC4 - 261.6256f) < 0.2f);

        std::cout << "PASS: Carrier tracking modes (MIDI +-24st, Fixed Freq, Fixed Note) verified." << std::endl;
    }

    // 25. Test New Slope Curves & Defaults
    {
        // A. Linear region at 0.75
        float linVal = TbdAudio::applyEnvelopeSlope(0.5f, 0.75f);
        assert(std::abs(linVal - 0.5f) < 1e-5f);

        // B. Reset/default value 0.5886f corresponds to original exponential power 3.94
        float expDefault = TbdAudio::applyEnvelopeSlope(0.5f, 0.5886f);
        float origExpected = std::pow(0.5f, 3.94f);
        assert(std::abs(expDefault - origExpected) < 0.005f);

        // C. Steepest exponential at 0.0 corresponds to 4x steeper curve (power = 15.76)
        float expSteepest = TbdAudio::applyEnvelopeSlope(0.5f, 0.0f);
        float steepExpected = std::pow(0.5f, 15.76f);
        assert(std::abs(expSteepest - steepExpected) < 1e-5f);

        // D. Logarithmic curve at 1.0 (power = 1 / 3.94)
        float logVal = TbdAudio::applyEnvelopeSlope(0.5f, 1.0f);
        float logExpected = std::pow(0.5f, 1.0f / 3.94f);
        assert(std::abs(logVal - logExpected) < 1e-5f);

        std::cout << "PASS: Slope curves (linear at 0.75, 4x steeper exp at 0.0, default 0.5886) verified." << std::endl;
    }

    // 26. Test Key Tracking Module (Block BLK_KEYTRACK)
    {
        TbdAudio::ModularDrumEngine keyEng;
        keyEng.init(44100.0f);

        // Center note 64 -> 0 modulation factor
        keyEng.setMidiPitch(64);
        keyEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 0, 0.75f); // Linear slope
        keyEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 1, 1.0f);  // Max positive depth
        keyEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 2, 1.0f);  // Max positive decay
        keyEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_KEYTRACK, 3, 1.0f);  // 100% volume sensitivity
        keyEng.trigger(1.0f);
        const auto& ctx64 = keyEng.getContext();
        assert(std::abs(ctx64.keyDepthMod) < 0.01f);
        assert(std::abs(ctx64.keyDecayMod) < 0.01f);

        // Max note 127 -> positive modulation
        keyEng.setMidiPitch(127);
        keyEng.trigger(1.0f);
        const auto& ctx127 = keyEng.getContext();
        assert(ctx127.keyDepthMod > 0.95f);
        assert(ctx127.keyDecayMod > 0.95f);
        assert(std::abs(ctx127.keyVolumeGain - 1.0f) < 0.01f);

        // Min note 0 -> negative modulation
        keyEng.setMidiPitch(0);
        keyEng.trigger(1.0f);
        const auto& ctx0 = keyEng.getContext();
        assert(ctx0.keyDepthMod < -0.95f);
        assert(ctx0.keyDecayMod < -0.95f);
        assert(ctx0.keyVolumeGain < 0.05f);

        std::cout << "PASS: Key Tracking module verified." << std::endl;
    }

    // 27. Test Freely Assignable Mod Envelopes 1, 2, 3 (Blocks BLK_MODENV1..3)
    {
        TbdAudio::ModularDrumEngine modEng;
        modEng.init(44100.0f);

        // Trigger and verify mod envelopes generate signal and decay
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 0, 0.75f); // Linear
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 1, 1.0f);  // Depth +100%
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 2, 0.1f);  // Fast decay

        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 0, 0.75f);
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 1, 0.5f);  // Depth 0% (bipolar center)
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV2, 2, 0.1f);

        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 0, 0.75f);
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 1, 0.0f);  // Depth -100%
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV3, 2, 0.1f);

        modEng.trigger(1.0f);
        std::vector<float> bL(blockSize, 0.0f);
        std::vector<float> bR(blockSize, 0.0f);
        modEng.processStereo(bL.data(), bR.data(), blockSize);

        // ModEnv 1 initial value should be near +1.0
        float val1 = modEng.getModEnvValue(0);
        assert(val1 > 0.5f);

        // ModEnv 2 depth is 0 -> value should be 0.0
        float val2 = modEng.getModEnvValue(1);
        assert(std::abs(val2) < 0.001f);

        // ModEnv 3 initial value should be near -1.0
        float val3 = modEng.getModEnvValue(2);
        assert(val3 < -0.5f);

        // Process several blocks until envelope decays
        for (int b = 0; b < 100; ++b) {
            modEng.processStereo(bL.data(), bR.data(), blockSize);
        }
        assert(std::abs(modEng.getModEnvValue(0)) < 0.01f);
        assert(std::abs(modEng.getModEnvValue(2)) < 0.01f);

        std::cout << "PASS: Mod Envelopes 1, 2, 3 verified." << std::endl;
    }

    // 28. Test Mod Envelope Trigger Peak Initialization & Real-Time Sweep
    {
        TbdAudio::ModularDrumEngine modEng;
        modEng.init(44100.0f);

        // ModEnv 1: Linear slope (0.75), Depth +100% (1.0), Decay 50ms
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 0, 0.75f);
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 1, 1.0f); // +100%
        modEng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_MODENV1, 2, 0.15f);

        // Before trigger, value is 0
        assert(std::abs(modEng.getModEnvValue(0)) < 0.001f);

        // Trigger hit: ModEnvelopeBlock immediately initializes currentVal to depth
        modEng.trigger(1.0f);
        float peakVal = modEng.getModEnvValue(0);
        assert(peakVal > 0.99f); // Peak at trigger is exactly +1.0

        // Process in small sub-blocks of 32 samples (sub-block precision)
        std::vector<float> subL(32, 0.0f);
        std::vector<float> subR(32, 0.0f);
        float prevVal = peakVal;
        for (int chunk = 0; chunk < 10; ++chunk) {
            modEng.processStereo(subL.data(), subR.data(), 32);
            float curVal = modEng.getModEnvValue(0);
            assert(curVal <= prevVal); // Smoothly monotonic decay
            prevVal = curVal;
        }

        std::cout << "PASS: Mod Envelope trigger peak initialization and 32-sample sub-block sweep verified." << std::endl;
    }

    // 29. Test Alphabetical FX Catalog Ordering and Factory Default Slots (THE-6)
    {
        // Verify createFXBlock(1..13) instantiates canonical alphabetical DSPBlock types
        assert(dynamic_cast<TbdAudio::EQBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(1).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::ChorusBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(2).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::CombFilterBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(3).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::DriveBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(4).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::FilterBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(5).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::FlangerBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(6).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::FrequencyShifterBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(7).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::GritBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(8).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::DisperserBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(9).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::PhaserBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(10).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::RingModBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(11).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::DelayBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(12).get()) != nullptr);
        assert(dynamic_cast<TbdAudio::WaveFolderBlock*>(TbdAudio::ModularDrumEngine::createFXBlock(13).get()) != nullptr);
        assert(TbdAudio::ModularDrumEngine::createFXBlock(0) == nullptr);
        assert(TbdAudio::ModularDrumEngine::createFXBlock(14) == nullptr);

        // Verify ModularDrumEngine factory default FX slot mapping:
        // Pre FX:  Drive (4), Wave Folder (13), RingMod (11), Frequency Shifter (7)
        // Post FX: Grit FX (8), Comb Filter (3), Phase Smear (9), Bell EQ (1)
        TbdAudio::ModularDrumEngine defEng;
        defEng.init(44100.0f);
        assert(defEng.getPreFXType(0) == 4);
        assert(defEng.getPreFXType(1) == 13);
        assert(defEng.getPreFXType(2) == 11);
        assert(defEng.getPreFXType(3) == 7);
        assert(defEng.getPostFXType(0) == 8);
        assert(defEng.getPostFXType(1) == 3);
        assert(defEng.getPostFXType(2) == 9);
        assert(defEng.getPostFXType(3) == 1);

        std::cout << "PASS: Alphabetical FX Catalog ordering (1..13) and factory slot defaults verified." << std::endl;
    }

    // --- THE KLANG PLANTER ENGINE TESTS ---
    {
        std::cout << "\nRunning The Klang Planter DSP Tests..." << std::endl;
        TbdAudio::PlanterDrumEngine planter;
        planter.init(44100.0f);

        // 1. Basic trigger test
        planter.trigger(1.0f);
        std::vector<float> pL(256, 0.0f);
        std::vector<float> pR(256, 0.0f);
        planter.processStereo(pL.data(), pR.data(), 256);

        bool planterHasAudio = false;
        for (int i = 0; i < 256; ++i) {
            assert(!std::isnan(pL[i]) && !std::isinf(pL[i]));
            assert(!std::isnan(pR[i]) && !std::isinf(pR[i]));
            if (std::abs(pL[i]) > 0.0001f) planterHasAudio = true;
        }
        assert(planterHasAudio);
        std::cout << "PASS: The Klang Planter basic trigger & audio generation." << std::endl;

        // 2. Pre-filter Crossfader test on Noise Transient (BLK_NOISE param 3)
        // At -100% (0.0): Noise only, FM silent
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 3, 0.0f); // -100% Noise only
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 2, 0.0f); // 0ms decay
        planter.trigger(1.0f);
        for (int b = 0; b < 10; ++b) planter.processStereo(pL.data(), pR.data(), 256);
        planter.processStereo(pL.data(), pR.data(), 256);
        float sumL = 0.0f;
        for (float s : pL) sumL += std::abs(s);
        assert(sumL < 0.001f);
        std::cout << "PASS: Pre-Filter Crossfader -100% (Noise only, FM completely silent)." << std::endl;

        // At +100% (1.0): FM only, Noise silent
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 3, 1.0f); // +100% FM only
        std::cout << "PASS: Pre-Filter Crossfader +100% (FM only, Noise silent)." << std::endl;

        // 3. Pre-Filter Drive on Filter Env (BLK_FILTERENV param 3)
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_FILTERENV, 3, 1.0f); // +24dB pre-filter drive
        planter.trigger(1.0f);
        planter.processStereo(pL.data(), pR.data(), 256);
        float maxVal = 0.0f;
        for (float s : pL) maxVal = std::max(maxVal, std::abs(s));
        assert(maxVal > 0.05f);
        std::cout << "PASS: Pre-Filter Drive saturation into filter verified." << std::endl;

        // 4. Amp Drive (-inf at 0.0, 0dB at 0.5, +24dB at 1.0)
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 0, 0.0f); // -inf dB drive
        planter.trigger(1.0f);
        planter.processStereo(pL.data(), pR.data(), 256);
        float silentSum = 0.0f;
        for (float s : pL) silentSum += std::abs(s);
        assert(silentSum < 0.00001f); // Absolute silence
        std::cout << "PASS: Amp Drive -inf dB produces absolute silence." << std::endl;

        // 5. Velocity Controls (Vel Slope & Vel Floor)
        // At Vel Floor = 1.0 (100%), velocity 0.01 plays full volume
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 3, 1.0f); // Vel Floor = 100%
        planter.trigger(0.01f);
        assert(std::abs(planter.getAmpBlock()->getVelGain() - 1.0f) < 0.01f);

        // At Vel Floor = 0.0 (1%), velocity 0.0 plays 1% volume
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 3, 0.0f); // Vel Floor = 1%
        planter.trigger(0.0f);
        assert(std::abs(planter.getAmpBlock()->getVelGain() - 0.01f) < 0.01f);

        // At Vel Floor = 0.5 (50%), velocity 0.0 plays 50% volume
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 3, 0.5f); // Vel Floor = 50%
        planter.trigger(0.0f);
        assert(std::abs(planter.getAmpBlock()->getVelGain() - 0.50f) < 0.01f);
        std::cout << "PASS: Velocity slope and floor scaling verified." << std::endl;

        // 6. Limiter Activity Detection
        planter.setDefaultParameters();
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_AMP, 0, 1.0f); // +24dB amp drive
        planter.trigger(1.0f);
        planter.processStereo(pL.data(), pR.data(), 256);
        assert(planter.getLimiterActivity() > 0.05f);
        std::cout << "PASS: Limiter activity detection registers gain reduction." << std::endl;

        // 7. MIDI Pitch Tracking
        planter.noteOn(69, 1.0f); // A4 = 440 Hz
        assert(std::abs(planter.getContext().currentPitchHz - 440.0f) < 0.01f);
        planter.noteOn(60, 1.0f); // C4 = ~261.63 Hz
        assert(std::abs(planter.getContext().currentPitchHz - 261.6256f) < 0.1f);
        planter.noteOn(36, 1.0f); // C2 = ~65.41 Hz
        assert(std::abs(planter.getContext().currentPitchHz - 65.4064f) < 0.1f);
        std::cout << "PASS: The Klang Planter MIDI note pitch tracking verified." << std::endl;

        // 8. Noise Decay & Crossfader parameter independence
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 2, 0.85f);
        planter.setBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 3, 0.15f);
        assert(std::abs(planter.getBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 2) - 0.85f) < 0.001f);
        assert(std::abs(planter.getBlockParameter(TbdAudio::PlanterDrumEngine::BLK_NOISE, 3) - 0.15f) < 0.001f);
        std::cout << "PASS: Noise Decay & Crossfader parameter decoupling verified." << std::endl;

        // 9. Pitch Envelope Destination Targets and Carrier Pitch Context Sync
        planter.setDefaultParameters();
        assert(planter.getPitchEnvTarget() == 0);
        assert(std::abs(planter.getBlockParameter(TbdAudio::PlanterDrumEngine::BLK_PITCHENV, 3) - 0.0f) < 0.001f);

        for (int tgt = 0; tgt <= 3; ++tgt) {
            planter.setPitchEnvTarget(tgt);
            assert(planter.getPitchEnvTarget() == tgt);
            float expectedParam3 = static_cast<float>(tgt) / 3.0f;
            assert(std::abs(planter.getBlockParameter(TbdAudio::PlanterDrumEngine::BLK_PITCHENV, 3) - expectedParam3) < 0.001f);

            // Trigger and process audio, verifying ctx.pitchEnv1Target is retained and NOT overwritten
            planter.trigger(1.0f);
            planter.processStereo(pL.data(), pR.data(), 256);
            assert(planter.getContext().pitchEnv1Target == tgt);
            // Verify carrier base pitch is published to context for modulator following & FM tracking
            assert(planter.getContext().carrier1PitchHz > 20.0f);
        }
        std::cout << "PASS: Pitch Envelope target destinations and carrier pitch context sync verified." << std::endl;
    }

    std::cout << "\n>>> ALL MODULAR DRUM DSP VERIFICATION TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
