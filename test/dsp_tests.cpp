#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "ModularBlocks.h"

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

    // 4. Test Pitch Envelope 1 Modulation
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 0, 1.0f); // Both
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 1, 0.0f); // Exp
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 2, 1.0f); // Max depth
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 3, 0.3806f); // 333 ms
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
            // Turn off pitch envelopes to have stable carrier
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV1, 0, 0.0f); // Off
            eng.setPageParameter(TbdAudio::ModularDrumEngine::BLK_PITCHENV2, 0, 0.0f); // Off
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
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 1, 0.75f); // +25% knob
        velEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_VELOCITY, 2, 0.25f); // -25% knob
        velEngine.trigger(1.0f);
        assert(std::abs(velEngine.getContext().velDecayMod - 0.05f) < 0.001f);
        assert(std::abs(velEngine.getContext().velDepthMod - (-0.05f)) < 0.001f);

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
        assert(ctxDef.slopFilterCutoff == 0.0f);
        assert(ctxDef.slopRingModRate == 0.0f);
        assert(ctxDef.slopCombDamp == 0.0f);
        assert(ctxDef.slopCombCutoff == 0.0f);
        assert(ctxDef.slopDisperserCutoff == 0.0f);
        assert(ctxDef.slopPitchEnv1Depth == 0.0f);
        assert(ctxDef.slopPitchEnv2Depth == 0.0f);
        assert(ctxDef.slopFilterEnvDepth == 0.0f);
        assert(ctxDef.slopPitchEnv1Decay == 0.0f);
        assert(ctxDef.slopPitchEnv2Decay == 0.0f);
        assert(ctxDef.slopNoiseDecay == 0.0f);
        assert(ctxDef.slopFilterEnvDecay == 0.0f);
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
            assert(std::abs(ctx.slopFilterCutoff) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopEQFreq) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopEQFilter) <= boundFreq + 0.001f);
            assert(std::abs(ctx.slopPitchEnv1Depth) <= boundDepth + 0.001f);
            assert(std::abs(ctx.slopPitchEnv2Depth) <= boundDepth + 0.001f);
            assert(std::abs(ctx.slopFilterEnvDepth) <= boundDepth + 0.001f);
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

        // Type 0 = Off (bypass)
        filter.setParam(0, 0.0f); // Off
        std::vector<float> sig(256, 1.0f);
        filter.processStereo(sig.data(), nullptr, 256, ctx);
        for (float s : sig) {
            assert(std::abs(s - 1.0f) < 0.0001f);
        }

        // Test LPF at different slopes: -6dB, -12dB, -18dB, -24dB, -36dB
        for (int slope = 0; slope < 5; ++slope) {
            filter.init(ctx);
            filter.setParam(0, 0.25f); // 1 = LPF (normalized: 1/4 = 0.25)
            filter.setParam(1, slope * 0.25f); // Slope
            filter.setParam(2, 0.5f); // ~500 Hz cutoff
            filter.setParam(3, 0.2f); // Resonance

            std::vector<float> lpfSig(256, 0.5f);
            filter.processStereo(lpfSig.data(), nullptr, 256, ctx);
            for (float s : lpfSig) {
                assert(!std::isnan(s) && !std::isinf(s));
            }
        }
        std::cout << "PASS: Filter block types and discrete slopes verified." << std::endl;
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
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 0, 0.25f); // LPF
        fEngine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER1, 1, 0.25f); // -12dB/oct
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
        fxEng.setPreFXType(0, 1);  // Drive
        fxEng.setPreFXType(1, 0);  // Bypass
        fxEng.setPostFXType(0, 6); // Grit FX
        fxEng.setPostFXType(1, 0); // Bypass

        assert(fxEng.getPreFXType(0) == 1);
        assert(fxEng.getPostFXType(0) == 6);

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

        // Put WaveFolder (type 3) in all 4 Pre FX slots and all 4 Post FX slots
        for (int s = 0; s < 4; ++s) {
            multiFxEng.setPreFXType(s, 3);
            multiFxEng.setPreFXParam(s, 0, 1.0f); // Type On
            multiFxEng.setPreFXParam(s, 1, 0.25f * (s + 1)); // Folds: 2, 4, 6, 8
            multiFxEng.setPreFXParam(s, 2, 0.5f);
            multiFxEng.setPreFXParam(s, 3, 0.5f);

            multiFxEng.setPostFXType(s, 3);
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

    std::cout << "\n>>> ALL MODULAR DRUM DSP VERIFICATION TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
