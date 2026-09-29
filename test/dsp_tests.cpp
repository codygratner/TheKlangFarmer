#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "ModularBlocks.h"

int main() {
    std::cout << "Starting DSP Verification Tests for 13-Block Modular Drum Synth..." << std::endl;

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

    // 2. Test Parameter Sweeps across all 13 blocks (0.0, 0.25, 0.5, 0.75, 1.0)
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
    std::cout << "PASS: 13-block parameter sweep stability test." << std::endl;

    // 3. Test Comb filter and APF disperser
    // Comb filter (Type 5 = 5/6.0f)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, 5.0f / 6.0f); // Comb
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, 0.5f);        // Dampening
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, 0.5f);        // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, 0.95f);       // High resonance
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

    // APF Disperser (Type 6 = 1.0f)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, 1.0f); // Disperser
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, 1.0f); // 32 stages
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, 0.6f); // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, 0.8f); // Resonance
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
    std::cout << "PASS: APF Disperser test." << std::endl;

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

        // Helper to count zero crossings of shifted 440 Hz sine wave
        auto measureFreq = [&](float shiftHz) -> float {
            fs.init(ctx);
            // range = 5000 Hz, so param[1] = 1.0f
            // totalShift = (param[0] - 0.5f) * 2.0f * 5000.0f
            float param0 = 0.5f + (shiftHz / 5000.0f) * 0.5f;
            fs.setParam(0, param0); // Shift
            fs.setParam(1, 1.0f);   // Range = 5000 Hz
            fs.setParam(2, 1.0f);   // Blend = 100% wet
            fs.setParam(3, 0.5f);   // Width = 0

            // Generate 440 Hz sine wave for 1 second
            constexpr int N = 44100;
            std::vector<float> testBuf(N);
            for (int i = 0; i < N; ++i) {
                testBuf[i] = std::sin(2.0f * 3.14159265358979323846f * 440.0f * i / 44100.0f);
            }
            fs.processStereo(testBuf.data(), nullptr, N, ctx);

            // Count positive zero crossings in the stable region (after 4000 samples)
            int crossings = 0;
            int startIdx = 4000;
            int endIdx = N - 1;
            for (int i = startIdx; i < endIdx; ++i) {
                if (testBuf[i] <= 0.0f && testBuf[i + 1] > 0.0f) {
                    crossings++;
                }
            }
            float duration = static_cast<float>(endIdx - startIdx) / 44100.0f;
            return static_cast<float>(crossings) / duration;
        };

        float freqUp = measureFreq(100.0f);
        float freqDown = measureFreq(-100.0f);
        std::cout << "Frequency Shifter: Input 440 Hz | Shift +100 Hz -> " << freqUp
                  << " Hz | Shift -100 Hz -> " << freqDown << " Hz" << std::endl;

        if (freqUp < 530.0f || freqUp > 550.0f) {
            std::cerr << "FAILED: Shift +100 Hz did not shift up to ~540 Hz (got " << freqUp << ")" << std::endl;
            return 1;
        }
        if (freqDown < 330.0f || freqDown > 350.0f) {
            std::cerr << "FAILED: Shift -100 Hz did not shift down to ~340 Hz (got " << freqDown << ")" << std::endl;
            return 1;
        }
        std::cout << "PASS: Frequency Shifter directional accuracy (shift up/down verified)." << std::endl;
    }

    // 7. Test VisualScope Buffer Capture for all 13 blocks
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
        std::cout << "PASS: All 13 VisualScope buffers populated with valid finite samples." << std::endl;
    }

    std::cout << "\n>>> ALL 13-BLOCK DSP VERIFICATION TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
