#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "ModularBlocks.h"

int main() {
    std::cout << "Starting DSP Verification Tests for 10-Block Modular Drum Synth..." << std::endl;

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

    // 2. Test Parameter Sweeps across all 10 blocks (0.0, 0.25, 0.5, 0.75, 1.0)
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
    std::cout << "PASS: 10-block parameter sweep stability test." << std::endl;

    // 3. Test Comb filter and APF disperser
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, 8.0f / 9.0f); // Comb
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, 0.5f);        // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, 0.95f);       // High resonance
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, 0.5f);        // Dampening
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

    // Test APF Disperser (Stages on param 3, Resonance on param 2)
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 0, 1.0f); // APF Disperser
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 1, 0.6f); // Cutoff
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 2, 0.7f); // Resonance
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_FILTER, 3, 1.0f); // 32 stages
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

    // 4. Test Clean Tone (Ensure clean musical sound at default, no 20 Hz decimation clicks)
    {
        TbdAudio::ModularDrumEngine cleanEngine;
        cleanEngine.init(44100.0f);
        cleanEngine.trigger(1.0f);
        std::vector<float> cleanBufL(4096, 0.0f);
        std::vector<float> cleanBufR(4096, 0.0f);
        cleanEngine.processStereo(cleanBufL.data(), cleanBufR.data(), 4096);

        int maxIdentical = 0;
        int currentIdentical = 0;
        for (size_t i = 1; i < cleanBufL.size(); ++i) {
            if (std::abs(cleanBufL[i]) > 0.001f && cleanBufL[i] == cleanBufL[i - 1]) {
                currentIdentical++;
                if (currentIdentical > maxIdentical) maxIdentical = currentIdentical;
            } else {
                currentIdentical = 0;
            }
        }
        if (maxIdentical > 10) {
            std::cerr << "FAILED: Audio appears decimated/sample-held! Max identical samples = " << maxIdentical << std::endl;
            return 1;
        }
        std::cout << "PASS: Clean tone at defaults with continuous non-clicking output." << std::endl;
    }

    // 5. Test Claps Burst on Amp Envelope
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 0, 0.0f); // Fast decay
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 1, 1.0f); // 16 claps
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 2, 0.5f); // Linear slope
    engine.setPageParameter(TbdAudio::ModularDrumEngine::BLK_AMPENV, 3, 0.5f); // ~333 ms
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

    // 6. Test VisualScope Buffer Capture
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
        std::cout << "PASS: All 10 VisualScope buffers populated with valid finite samples." << std::endl;
    }

    std::cout << "\n>>> ALL 10-BLOCK DSP VERIFICATION TESTS PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
