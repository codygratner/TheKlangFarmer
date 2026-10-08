#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "FastMath.h"

namespace TbdAudio {

// --- 1. MODULATION SOURCES (16+ assignable sources) ---
enum class ModSource : int {
    LFO1 = 0,
    LFO2,
    LFO3,
    LFO4,
    Env1,
    Env2,
    Env3,
    Env4,
    Random1,
    Random2,
    Velocity,
    KeyTrack,
    Slop,
    Macro1,
    Macro2,
    Macro3,
    Macro4,
    Hydra1,
    Hydra2,
    Hydra3,
    Hydra4,
    Count // 21 total assignable sources
};

inline const char* getModSourceName(ModSource src) {
    switch (src) {
        case ModSource::LFO1:     return "LFO 1";
        case ModSource::LFO2:     return "LFO 2";
        case ModSource::LFO3:     return "LFO 3";
        case ModSource::LFO4:     return "LFO 4";
        case ModSource::Env1:     return "Env 1";
        case ModSource::Env2:     return "Env 2";
        case ModSource::Env3:     return "Env 3";
        case ModSource::Env4:     return "Env 4";
        case ModSource::Random1:  return "Random 1";
        case ModSource::Random2:  return "Random 2";
        case ModSource::Velocity: return "Velocity";
        case ModSource::KeyTrack: return "KeyTrack";
        case ModSource::Slop:     return "Slop";
        case ModSource::Macro1:   return "Macro 1";
        case ModSource::Macro2:   return "Macro 2";
        case ModSource::Macro3:   return "Macro 3";
        case ModSource::Macro4:   return "Macro 4";
        case ModSource::Hydra1:   return "Hydra 1";
        case ModSource::Hydra2:   return "Hydra 2";
        case ModSource::Hydra3:   return "Hydra 3";
        case ModSource::Hydra4:   return "Hydra 4";
        default:                  return "None";
    }
}

// --- 2. MODULATION CURVES ---
enum class ModCurve : int {
    Linear = 0,
    Exponential,
    Logarithmic,
    SCurve
};

inline float evaluateModCurve(float val, ModCurve curve, bool isBipolar) {
    if (isBipolar) {
        float sign = (val >= 0.0f) ? 1.0f : -1.0f;
        float mag = std::clamp(std::abs(val), 0.0f, 1.0f);
        switch (curve) {
            case ModCurve::Linear:
                return val;
            case ModCurve::Exponential:
                return sign * (mag * mag);
            case ModCurve::Logarithmic:
                return sign * std::sqrt(mag);
            case ModCurve::SCurve:
                return sign * (mag * mag * (3.0f - 2.0f * mag));
            default:
                return val;
        }
    } else {
        float u = std::clamp(val, 0.0f, 1.0f);
        switch (curve) {
            case ModCurve::Linear:
                return u;
            case ModCurve::Exponential:
                return u * u;
            case ModCurve::Logarithmic:
                return std::sqrt(u);
            case ModCurve::SCurve:
                return u * u * (3.0f - 2.0f * u);
            default:
                return u;
        }
    }
}

// --- 3. MODULATION ROUTE (Bounded, pre-allocated, zero-allocation) ---
struct ModRoute {
    int sourceId { -1 };        // ModSource as int, or -1 for inactive
    int targetParamId { -1 };   // Destination parameter ID / index, or -1
    float depth { 0.0f };       // Normalized modulation depth (-1.0f to +1.0f)
    bool isBipolar { true };    // Bipolar (-1..+1) vs Unipolar (0..+1)
    int viaSourceId { -1 };     // Secondary "via" modulator source ID, or -1
    float viaDepth { 0.0f };    // Secondary depth multiplier
    ModCurve curve { ModCurve::Linear };
    bool active { false };

    // Evaluates route output given source and via raw values
    float evaluate(float sourceVal, float viaVal = 1.0f) const {
        if (!active || sourceId < 0) return 0.0f;

        float effectiveDepth = depth;
        if (viaSourceId >= 0) {
            effectiveDepth = depth * (viaVal * viaDepth);
        }

        float shapedSource = evaluateModCurve(sourceVal, curve, isBipolar);
        return shapedSource * effectiveDepth;
    }
};

// --- 4. HYDRA META-MODULATOR DSP STRUCTURES ---
enum class HydraInputSource : int {
    None = 0,
    LFO1, LFO2, LFO3, LFO4,
    Env1, Env2, Env3, Env4,
    Random1, Random2,
    Velocity,
    KeyTrack,
    Slop,
    Macro1, Macro2, Macro3, Macro4,
    // Audio-Rate Oscillator Inputs (for authentic gritty FM textures):
    OscCarrier1,
    OscModulator1,
    OscCarrier2,
    OscModulator2
};

inline bool isHydraAudioRate(HydraInputSource s) {
    return s >= HydraInputSource::OscCarrier1 && s <= HydraInputSource::OscModulator2;
}

struct HydraDestination {
    int targetParamId { -1 }; // Continuous parameter index or -1
    float minVal { 0.0f };     // Output value when input is at minimum (0.0)
    float maxVal { 1.0f };     // Output value when input is at maximum (1.0)
    ModCurve curve { ModCurve::Linear };
    bool active { false };

    float mapValue(float normalizedIn) const {
        if (!active) return 0.0f;
        float shaped = evaluateModCurve(std::clamp(normalizedIn, 0.0f, 1.0f), curve, false);
        return minVal + shaped * (maxVal - minVal);
    }
};

struct HydraHub {
    HydraInputSource inputSource { HydraInputSource::None };
    std::array<HydraDestination, 8> destinations;
    float currentOutputValue { 0.0f };

    bool isAudioRate() const {
        return isHydraAudioRate(inputSource);
    }

    void reset() {
        currentOutputValue = 0.0f;
    }

    void setDestination(int slot, int targetParamId, float minVal, float maxVal, ModCurve curve = ModCurve::Linear) {
        if (slot >= 0 && slot < 8) {
            destinations[slot].targetParamId = targetParamId;
            destinations[slot].minVal = minVal;
            destinations[slot].maxVal = maxVal;
            destinations[slot].curve = curve;
            destinations[slot].active = (targetParamId >= 0);
        }
    }

    void clearDestination(int slot) {
        if (slot >= 0 && slot < 8) {
            destinations[slot] = HydraDestination{};
        }
    }
};

// --- 5. MODULATION MATRIX (Bounded 64 routes, 4 Hydras, 4 Macros) ---
class ModulationMatrix {
public:
    static constexpr int MAX_ROUTES = 64;
    static constexpr int NUM_SOURCES = static_cast<int>(ModSource::Count);

    ModulationMatrix() {
        reset();
    }

    void reset() {
        sourceValues.fill(0.0f);
        macroValues.fill(0.0f);
        for (auto& h : hydras) {
            h.reset();
        }
    }

    void setSourceValue(ModSource src, float val) {
        int idx = static_cast<int>(src);
        if (idx >= 0 && idx < NUM_SOURCES) {
            sourceValues[idx] = val;
        }
    }

    float getSourceValue(ModSource src) const {
        int idx = static_cast<int>(src);
        return (idx >= 0 && idx < NUM_SOURCES) ? sourceValues[idx] : 0.0f;
    }

    void setMacro(int index, float val) {
        if (index >= 0 && index < 4) {
            macroValues[index] = std::clamp(val, 0.0f, 1.0f);
            setSourceValue(static_cast<ModSource>(static_cast<int>(ModSource::Macro1) + index), macroValues[index]);
        }
    }

    float getMacro(int index) const {
        return (index >= 0 && index < 4) ? macroValues[index] : 0.0f;
    }

    bool setRoute(int routeIndex, const ModRoute& route) {
        if (routeIndex >= 0 && routeIndex < MAX_ROUTES) {
            routes[routeIndex] = route;
            return true;
        }
        return false;
    }

    const ModRoute& getRoute(int routeIndex) const {
        static const ModRoute invalidRoute{};
        if (routeIndex >= 0 && routeIndex < MAX_ROUTES) {
            return routes[routeIndex];
        }
        return invalidRoute;
    }

    ModRoute& getRouteRef(int routeIndex) {
        return routes[std::clamp(routeIndex, 0, MAX_ROUTES - 1)];
    }

    void clearRoute(int routeIndex) {
        if (routeIndex >= 0 && routeIndex < MAX_ROUTES) {
            routes[routeIndex] = ModRoute{};
        }
    }

    void clearAllRoutes() {
        for (auto& r : routes) {
            r = ModRoute{};
        }
    }

    int getNumActiveRoutes() const {
        int count = 0;
        for (const auto& r : routes) {
            if (r.active) ++count;
        }
        return count;
    }

    HydraHub& getHydra(int index) {
        return hydras[std::clamp(index, 0, 3)];
    }

    const HydraHub& getHydra(int index) const {
        return hydras[std::clamp(index, 0, 3)];
    }

    // Evaluate single route using current internal source values
    float evaluateRoute(int routeIndex) const {
        if (routeIndex < 0 || routeIndex >= MAX_ROUTES) return 0.0f;
        const auto& r = routes[routeIndex];
        if (!r.active || r.sourceId < 0 || r.sourceId >= NUM_SOURCES) return 0.0f;

        float sVal = sourceValues[r.sourceId];
        float vVal = (r.viaSourceId >= 0 && r.viaSourceId < NUM_SOURCES) ? sourceValues[r.viaSourceId] : 1.0f;
        return r.evaluate(sVal, vVal);
    }

    // Sums all active modulation offsets into destAccumulators (pre-allocated array)
    void evaluateBlockModulations(float* destAccumulators, int numDests) const {
        if (!destAccumulators || numDests <= 0) return;

        for (int i = 0; i < MAX_ROUTES; ++i) {
            const auto& r = routes[i];
            if (r.active && r.targetParamId >= 0 && r.targetParamId < numDests) {
                destAccumulators[r.targetParamId] += evaluateRoute(i);
            }
        }

        // Fan out Hydra block-rate modulations if not audio-rate
        for (int h = 0; h < 4; ++h) {
            const auto& hub = hydras[h];
            if (!hub.isAudioRate()) {
                float inVal = hub.currentOutputValue;
                for (int d = 0; d < 8; ++d) {
                    const auto& dest = hub.destinations[d];
                    if (dest.active && dest.targetParamId >= 0 && dest.targetParamId < numDests) {
                        destAccumulators[dest.targetParamId] += dest.mapValue(inVal);
                    }
                }
            }
        }
    }

private:
    std::array<ModRoute, MAX_ROUTES> routes;
    std::array<HydraHub, 4> hydras;
    std::array<float, NUM_SOURCES> sourceValues;
    std::array<float, 4> macroValues;
};

} // namespace TbdAudio
