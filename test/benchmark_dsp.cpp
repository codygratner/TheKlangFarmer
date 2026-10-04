#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <cassert>
#include "FastMath.h"

using namespace TbdAudio::FastMath;

int main() {
    std::cout << "==================================================================\n";
    std::cout << "     The Klang Farmer / Klang Planter - FastMath DSP Benchmark    \n";
    std::cout << "==================================================================\n\n";

    // -------------------------------------------------------------
    // 1. Accuracy Verification (< 0.05% relative error)
    // -------------------------------------------------------------
    std::cout << "[1] Verifying Mathematical Accuracy against CRT Standard Math...\n";

    // Test fastPow2
    float maxPow2Error = 0.0f;
    for (float x = -10.0f; x <= 10.0f; x += 0.001f) {
        float expected = std::pow(2.0f, x);
        float actual = fastPow2(x);
        float relError = std::abs((actual - expected) / expected) * 100.0f;
        if (relError > maxPow2Error) maxPow2Error = relError;
    }
    std::cout << "  - fastPow2 max relative error: " << std::fixed << std::setprecision(4)
              << maxPow2Error << "% " << (maxPow2Error < 0.05f ? "[PASS]" : "[FAIL]") << "\n";
    assert(maxPow2Error < 0.05f);

    // Test fastExp
    float maxExpError = 0.0f;
    for (float x = -10.0f; x <= 10.0f; x += 0.001f) {
        float expected = std::exp(x);
        float actual = fastExp(x);
        float relError = std::abs((actual - expected) / expected) * 100.0f;
        if (relError > maxExpError) maxExpError = relError;
    }
    std::cout << "  - fastExp max relative error:  " << std::fixed << std::setprecision(4)
              << maxExpError << "% " << (maxExpError < 0.05f ? "[PASS]" : "[FAIL]") << "\n";
    assert(maxExpError < 0.05f);

    // Test fastSinNorm
    float maxSinError = 0.0f;
    for (float p = 0.0f; p < 1.0f; p += 0.0001f) {
        float expected = std::sin(p * TWO_PI);
        float actual = fastSinNorm(p);
        // Absolute error since sin crosses zero
        float absError = std::abs(actual - expected);
        if (absError > maxSinError) maxSinError = absError;
    }
    std::cout << "  - fastSinNorm max peak error:  " << std::fixed << std::setprecision(6)
              << maxSinError << " (" << (maxSinError * 100.0f) << "%) "
              << (maxSinError < 0.0005f ? "[PASS]" : "[FAIL]") << "\n";
    assert(maxSinError < 0.0005f);

    // Test fastTanhPrecise
    float maxTanhPreciseError = 0.0f;
    for (float x = -4.0f; x <= 4.0f; x += 0.001f) {
        float expected = std::tanh(x);
        float actual = fastTanhPrecise(x);
        float absError = std::abs(actual - expected);
        if (absError > maxTanhPreciseError) maxTanhPreciseError = absError;
    }
    std::cout << "  - fastTanhPrecise peak error:  " << std::fixed << std::setprecision(6)
              << maxTanhPreciseError << " (" << (maxTanhPreciseError * 100.0f) << "%) "
              << (maxTanhPreciseError < 0.0005f ? "[PASS]" : "[FAIL]") << "\n";
    assert(maxTanhPreciseError < 0.0005f);

    // Test fastDbToGain
    float maxDbError = 0.0f;
    for (float db = -96.0f; db <= 24.0f; db += 0.1f) {
        float expected = std::pow(10.0f, db / 20.0f);
        float actual = fastDbToGain(db);
        float relError = std::abs((actual - expected) / expected) * 100.0f;
        if (relError > maxDbError) maxDbError = relError;
    }
    std::cout << "  - fastDbToGain rel error:      " << std::fixed << std::setprecision(4)
              << maxDbError << "% " << (maxDbError < 0.05f ? "[PASS]" : "[FAIL]") << "\n\n";
    assert(maxDbError < 0.05f);

    // -------------------------------------------------------------
    // 2. Performance & Speedup Benchmark
    // -------------------------------------------------------------
    constexpr int NUM_ITERATIONS = 10000000;
    std::vector<float> input(NUM_ITERATIONS);
    std::vector<float> outCRT(NUM_ITERATIONS);
    std::vector<float> outFast(NUM_ITERATIONS);

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        input[i] = -5.0f + 10.0f * (static_cast<float>(i) / NUM_ITERATIONS);
    }

    std::cout << "[2] Running Throughput Benchmarks (" << NUM_ITERATIONS << " samples)...\n";

    // --- Benchmark pow(2, x) ---
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outCRT[i] = std::pow(2.0f, input[i]);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outFast[i] = fastPow2(input[i]);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double crtPowTime = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double fastPowTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double powSpeedup = crtPowTime / fastPowTime;
    std::cout << "  std::pow(2, x): " << crtPowTime << " ms | fastPow2: " << fastPowTime
              << " ms -> Speedup: " << std::fixed << std::setprecision(2) << powSpeedup << "x\n";

    // --- Benchmark sin(phase) ---
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        input[i] = static_cast<float>(i % 44100) / 44100.0f;
    }
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outCRT[i] = std::sin(input[i] * TWO_PI);
    }
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outFast[i] = fastSinNorm(input[i]);
    }
    t2 = std::chrono::high_resolution_clock::now();
    double crtSinTime = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double fastSinTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double sinSpeedup = crtSinTime / fastSinTime;
    std::cout << "  std::sin(2*pi*p): " << crtSinTime << " ms | fastSinNorm: " << fastSinTime
              << " ms -> Speedup: " << std::fixed << std::setprecision(2) << sinSpeedup << "x\n";

    // --- Benchmark tanh(x) ---
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        input[i] = -3.0f + 6.0f * (static_cast<float>(i) / NUM_ITERATIONS);
    }
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outCRT[i] = std::tanh(input[i]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        outFast[i] = fastTanh(input[i]);
    }
    t2 = std::chrono::high_resolution_clock::now();
    double crtTanhTime = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double fastTanhTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double tanhSpeedup = crtTanhTime / fastTanhTime;
    std::cout << "  std::tanh(x):   " << crtTanhTime << " ms | fastTanh: " << fastTanhTime
              << " ms -> Speedup: " << std::fixed << std::setprecision(2) << tanhSpeedup << "x\n";

    // --- Benchmark Full Oscillator Sample Loop ---
    float floatPhase = 0.0f;
    float freq = 130.81f; // C3
    float invSr = 1.0f / 44100.0f;

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        floatPhase += freq * invSr;
        if (floatPhase >= 1.0f) floatPhase -= std::floor(floatPhase);
        outCRT[i] = std::sin(floatPhase * TWO_PI);
    }
    t1 = std::chrono::high_resolution_clock::now();

    PhaseAccumulator32 accum;
    uint32_t inc = PhaseAccumulator32::calcInc(freq, invSr);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        accum.step(inc);
        outFast[i] = fastSinNorm(accum.getPhaseNorm());
    }
    t2 = std::chrono::high_resolution_clock::now();

    double crtOscTime = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double fastOscTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double oscSpeedup = crtOscTime / fastOscTime;
    std::cout << "  Float+sin Loop: " << crtOscTime << " ms | PhaseAccum32+fastSin: " << fastOscTime
              << " ms -> Speedup: " << std::fixed << std::setprecision(2) << oscSpeedup << "x\n";

    std::cout << "\n==================================================================\n";
    std::cout << ">>> BENCHMARK COMPLETE: All accuracy tests PASSED cleanly! <<<\n";
    std::cout << "==================================================================\n";

    return 0;
}
