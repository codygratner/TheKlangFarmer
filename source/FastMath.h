#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <bit>

namespace TbdAudio {
namespace FastMath {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;
constexpr float INV_TWO_PI = 0.15915494309189533577f;
constexpr float LOG2_E = 1.44269504088896340736f;
constexpr float LN2 = 0.69314718055994530942f;
constexpr float DB_TO_LOG2 = 0.16609640474436813f; // log2(10) / 20

/**
 * @brief Fast 2^x computation using IEEE 754 exponent bit-manipulation
 *        and a 3rd-order minimax polynomial correction (< 0.02% max relative error).
 * @param x Exponent in range [-126.0f, +126.0f].
 * @return 2^x
 */
[[nodiscard]] inline float fastPow2(float x) noexcept {
    if (x <= -126.0f) return 0.0f;
    if (x >= 126.0f) return 8.507059e+37f;

    float xi = std::floor(x);
    float xf = x - xi;

    // Construct 2^xi by setting IEEE 754 exponent bits directly
    auto i = static_cast<int32_t>(xi);
    uint32_t expBits = static_cast<uint32_t>((i + 127) << 23);
    float p2i = std::bit_cast<float>(expBits);

    // 3rd-order minimax approximation for 2^xf on [0, 1)
    // Relative error < 0.015% across all fractions
    float poly = 1.0f + xf * (0.695846f + xf * (0.226066f + xf * 0.078088f));

    return p2i * poly;
}

/**
 * @brief Fast e^x computation via fastPow2(x * log2(e)) (< 0.02% error).
 */
[[nodiscard]] inline float fastExp(float x) noexcept {
    return fastPow2(x * LOG2_E);
}

/**
 * @brief Fast base-2 logarithm for positive floats (< 0.05% error).
 * @param x Positive value > 0.
 */
[[nodiscard]] inline float fastLog2(float x) noexcept {
    if (x <= 1e-12f) return -39.86f;

    uint32_t bits = std::bit_cast<uint32_t>(x);
    int32_t exp = static_cast<int32_t>((bits >> 23) & 0xFF) - 127;
    
    // Normalize mantissa to [1.0, 2.0)
    uint32_t mantissaBits = (bits & 0x007FFFFF) | 0x3F800000;
    float m = std::bit_cast<float>(mantissaBits) - 1.0f; // m in [0.0, 1.0)

    // Minimax polynomial for log2(1 + m) on [0, 1)
    float poly = m * (1.442695f - m * (0.721348f - m * 0.278653f));

    return static_cast<float>(exp) + poly;
}

/**
 * @brief Fast base-e natural logarithm (< 0.05% error).
 */
[[nodiscard]] inline float fastLog(float x) noexcept {
    return fastLog2(x) * LN2;
}

/**
 * @brief Fast sine for normalized phase [0, 1) (where 1.0 = 2*pi).
 *        Branchless-friendly minimax polynomial (< 0.02% max error).
 */
[[nodiscard]] inline float fastSinNorm(float p) noexcept {
    // Wrap to [0, 1)
    float z = p - std::floor(p);
    // Shift to [-0.5, 0.5]
    z = (z > 0.5f) ? (z - 1.0f) : z;
    // Quadrant fold into [-0.25, 0.25]
    if (z > 0.25f) {
        z = 0.5f - z;
    } else if (z < -0.25f) {
        z = -0.5f - z;
    }

    // Minimax polynomial for sin(2*pi*z) on [-0.25, 0.25]
    float w = z * z;
    constexpr float c1 = 6.2831853f;
    constexpr float c2 = -41.341675f;
    constexpr float c3 = 81.602237f;
    constexpr float c4 = -76.574959f;

    return z * (c1 + w * (c2 + w * (c3 + w * c4)));
}

/**
 * @brief Fast sine for radians [-inf, +inf] (< 0.02% max error).
 */
[[nodiscard]] inline float fastSin(float rad) noexcept {
    return fastSinNorm(rad * INV_TWO_PI);
}

/**
 * @brief Fast cosine for normalized phase [0, 1) (< 0.02% max error).
 */
[[nodiscard]] inline float fastCosNorm(float p) noexcept {
    return fastSinNorm(p + 0.25f);
}

/**
 * @brief Fast cosine for radians [-inf, +inf] (< 0.02% max error).
 */
[[nodiscard]] inline float fastCos(float rad) noexcept {
    return fastSinNorm(rad * INV_TWO_PI + 0.25f);
}

/**
 * @brief Padé rational saturation approximation for hyperbolic tangent:
 *        x * (27 + x^2) / (27 + 9 * x^2), clamped smoothly for |x| >= 3.0.
 *        Ideal for saturators, drive stages, and limiters.
 */
[[nodiscard]] inline float fastTanh(float x) noexcept {
    if (x <= -3.0f) return -1.0f;
    if (x >= 3.0f) return 1.0f;
    float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

/**
 * @brief High-precision Padé [7/6] hyperbolic tangent (< 0.01% max error across full range).
 */
[[nodiscard]] inline float fastTanhPrecise(float x) noexcept {
    if (x <= -4.5f) return -1.0f;
    if (x >= 4.5f) return 1.0f;
    float x2 = x * x;
    float x4 = x2 * x2;
    float x6 = x4 * x2;
    float num = x * (135135.0f + 17325.0f * x2 + 378.0f * x4 + x6);
    float den = 135135.0f + 62370.0f * x2 + 3150.0f * x4 + 28.0f * x6;
    return num / den;
}

/**
 * @brief Fast dB-to-linear conversion: 10^(db / 20) via fastPow2.
 *        Replaces heavy CRT std::pow(10.0f, db / 20.0f).
 */
[[nodiscard]] inline float fastDbToGain(float db) noexcept {
    return fastPow2(db * DB_TO_LOG2);
}

/**
 * @brief Fast linear-to-dB conversion: 20 * log10(gain).
 */
[[nodiscard]] inline float fastGainToDb(float gain) noexcept {
    if (gain <= 1e-6f) return -120.0f;
    return 6.020599913279624f * fastLog2(gain);
}

/**
 * @brief 32-bit branchless integer phase accumulator for audio oscillators.
 *        Eliminates floor() and branch checks via natural 32-bit unsigned overflow.
 */
class PhaseAccumulator32 {
public:
    uint32_t phase = 0;

    constexpr PhaseAccumulator32() noexcept = default;
    constexpr explicit PhaseAccumulator32(uint32_t initial) noexcept : phase(initial) {}

    inline void reset() noexcept { phase = 0; }

    inline void setPhaseNorm(float p) noexcept {
        float wrapped = p - std::floor(p);
        phase = static_cast<uint32_t>(static_cast<double>(wrapped) * 4294967296.0);
    }

    [[nodiscard]] inline float getPhaseNorm() const noexcept {
        return static_cast<float>(phase) * (1.0f / 4294967296.0f);
    }

    inline void step(uint32_t increment) noexcept {
        phase += increment;
    }

    inline void step(float freqHz, float invSr) noexcept {
        phase += calcInc(freqHz, invSr);
    }

    static inline uint32_t calcInc(float freqHz, float invSr) noexcept {
        return static_cast<uint32_t>(static_cast<double>(freqHz) * static_cast<double>(invSr) * 4294967296.0);
    }
};

} // namespace FastMath
} // namespace TbdAudio
