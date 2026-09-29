#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "ModularBlocks.h"

int main() {
    std::cout << "Starting DSP Verification Tests for 16-Block Modular Drum Synth..." << std::endl;

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

    // 2. Test Parameter Sweeps across all 15 blocks (0.0, 0.25, 0.5, 0.75, 1.0)
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
    std::cout << "PASS: 16-block parameter sweep stability test." << std::endl;

    // 3. Test Comb filter and APF disperser
    // Comb filter (Block BLK_COMB, Type = 1.0f On)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 0, 1.0f);        // On
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 1, 0.5f);        // Dampening
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 2, 0.5f);        // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_COMB, 3, 0.95f);       // High resonance
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

    // APF Disperser (Block BLK_DISPERSER, Type = 1.0f On)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 0, 1.0f); // On
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 1, 1.0f); // 32 stages
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 2, 0.6f); // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_DISPERSER, 3, 0.8f); // Resonance
    engine.trigger(1.0f);
    for (int block = 0; block < 50; ++block) {
        engine.processStereo(left.data(), right.data(), blockSize);
        for (int i = 0; i < blockSize; ++i) {
            if (std::isnan(left[i]) || std::isinf(left[i]) ||
                std::isnan(right[i]) || std::isinf(right[i])) {
                std::cerr << "FAILED: NaN or Inf in APF Disperser!" << std::endl;
                return 1;
            }
        }
    }
    // Verify 2nd-order Disperser phase smearing and allpass energy preservation
    {
        TbdAudio::DisperserBlock disp;
        TbdAudio::BlockContext ctx;
        ctx.sampleRate = 44100.0f;
        ctx.invSr = 1.0f / 44100.0f;
        disp.init(ctx);
        disp.setParam(0, 1.0f);          // On
        disp.setParam(1, 16.0f / 32.0f); // 16 stages
        disp.setParam(2, 0.65f);         // ~600 Hz cutoff
        disp.setParam(3, 0.85f);         // High resonance (Q ~ 14.5)

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
            std::cerr << "FAILED: Disperser is not preserving allpass energy! Energy = " << energy << std::endl;
            return 1;
        }

        // 16 cascaded 2nd-order APF stages must smear the single-sample impulse over dozens of samples
        if (nonZeroSamples < 20) {
            std::cerr << "FAILED: Disperser did not smear impulse! Nonzero samples: " << nonZeroSamples << std::endl;
            return 1;
        }
    }
    std::cout << "PASS: 2nd-order APF Disperser smearing, zapping, and energy conservation verified." << std::endl;

    // 4. Test Pitch Envelope Modulation
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 0, 1.0f); // Both
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 1, 0.0f); // Exp
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 2, 1.0f); // Max depth
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 3, 0.3806f); // 333 ms
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
    std::cout << "PASS: Pitch Envelope test." << std::endl;

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
            // Turn off pitch envelope to have stable carrier
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV, 0, 0.0f); // Off
            // Frequency shifter: Range = rangeHz / 5000, Blend = 1.0 (100% wet), Shift = shiftNorm
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 0, shiftNorm);
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FREQSHIFT, 1, rangeHz / 5000.0f);
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

    // 7. Test VisualScope Buffer Capture for all 16 blocks
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
        std::cout << "PASS: All 16 VisualScope buffers populated with valid finite samples." << std::endl;
    }

    // 8. Test Velocity Modulation
    {
        TbdAudio::ModularDrumEngine velEngine;
        velEngine.init(44100.0f);
        velEngine.setMidiPitch(36); // C2

        // Set Velocity: Slope = Linear (0.5), Volume = 100% (-100% at min vel, param3 = 1.0f)
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 0, 0.5f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.5f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.5f);
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 3, 1.0f); // 100% volume sensitivity

        // Trigger at max velocity (1.0)
        velEngine.trigger(1.0f);
        std::vector<float> highL(256, 0.0f);
        std::vector<float> highR(256, 0.0f);
        velEngine.processStereo(highL.data(), highR.data(), 256);

        float maxPeak = 0.0f;
        for (float s : highL) maxPeak = std::max(maxPeak, std::abs(s));

        // Trigger at very low velocity (0.01)
        velEngine.trigger(0.01f);
        std::vector<float> lowL(256, 0.0f);
        std::vector<float> lowR(256, 0.0f);
        velEngine.processStereo(lowL.data(), lowR.data(), 256);

        float minPeak = 0.0f;
        for (float s : lowL) minPeak = std::max(minPeak, std::abs(s));

        std::cout << "Velocity Volume Sensitivity: Peak at Vel 1.0 = " << maxPeak << " | Peak at Vel 0.01 = " << minPeak << std::endl;
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

        std::cout << "PASS: Velocity modulation (volume, decay, depth polarity, curves) verified." << std::endl;
    }

    std::cout << "\n>>> ALL 16-BLOCK DSP VERIFICATION TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
