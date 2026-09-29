#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace TbdAudio {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;

struct BlockContext {
    float sampleRate = 44100.0f;
    float invSr = 1.0f / 44100.0f;
    // Shared contextual signals passed down the chain
    float triggerVelocity = 1.0f;
    bool isTriggered = false;
    float currentPitchHz = 65.4064f; // C2 default
    int currentMidiNote = 36;
    float carrierPitchHz = 65.4064f;

    // Inter-block modulation buffers
    std::vector<float> modSignal;
    std::vector<float> pitchEnvSignal;
    std::vector<float> filterEnvSignal;
    std::vector<float> ampEnvSignal;
};

class DSPBlock {
public:
    virtual ~DSPBlock() = default;
    virtual void init(const BlockContext& ctx) = 0;
    virtual void trigger(float /*velocity*/) {}
    
    // Set parameter 0..3 (normalized 0.0 to 1.0)
    virtual void setParam(int index, float value) {
        if (index >= 0 && index < 4) params[index] = std::clamp(value, 0.0f, 1.0f);
    }
    virtual float getParam(int index) const {
        return (index >= 0 && index < 4) ? params[index] : 0.0f;
    }

    // Process mono buffer in-place
    virtual void process(float* buffer, int numSamples, BlockContext& ctx) = 0;

    // Process stereo buffer in-place
    virtual void processStereo(float* left, float* right, int numSamples, BlockContext& ctx) {
        process(left, numSamples, ctx);
        if (right != nullptr && right != left) {
            std::copy(left, left + numSamples, right);
        }
    }

protected:
    float params[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

} // namespace TbdAudio