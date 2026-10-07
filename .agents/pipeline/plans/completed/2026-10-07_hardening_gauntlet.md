# Plan: v0.3.3 Hardening Gauntlet

## Overview
This task implements a comprehensive "Hardening Gauntlet" for the C++ DSP architecture and the CI/CD pipeline, focusing on runtime audio safety, concurrency memory safety, and schema fuzzing.

## Goals
1. **CI Toolchain Lockdown**: Enforce `-Werror` (warnings as errors) in CI via a CMake flag, catching tech debt early.
2. **Sanitizer Gauntlet (ASan/TSan)**: Add CMake presets/flags to compile test suites with Clang's AddressSanitizer and ThreadSanitizer to mathematically prove thread safety.
3. **The "Monitor Saver" Protocol (NaN/Inf Guard)**: Add FTZ/DAZ macros and SIMD-accelerated NaN/Inf detection at the tail end of `processBlock` to prevent DAW explosions.
4. **Data-Driven "Poison Pill" Fuzzing**: Extend `gui_tests` with an invalid/corrupted JSON schema test to verify graceful fallback instead of fatal exceptions.
5. **Asynchronous DAW Automation Defense**: Add `AutomationStressTest` in `HardeningSuites.h` to bombard the APVTS with parameter updates at sample-rate speed while the DSP engine runs.

## Phases

### Phase 1: CI Toolchain Lockdown & Sanitizer Integration
- [x] In `CMakeLists.txt`, add an `option(TK_STRICT_WARNINGS "Treat warnings as errors (for CI)" OFF)`.
- [x] If `TK_STRICT_WARNINGS` is ON, add `/WX` (MSVC) or `-Werror` (GCC/Clang) to `CMAKE_CXX_FLAGS`.
- [x] Add `option(TK_USE_ASAN "Enable Address/Thread Sanitizer" OFF)`. If ON, append `-fsanitize=address,undefined` (or thread) to compiler and linker flags (UNIX/Apple only, as MSVC ASan has different semantics).

### Phase 2: The "Monitor Saver" Protocol (DSP NaN/Inf Failsafe)
- [x] In `source/FastMath.h`, add inline SIMD utility methods `enableFTZDAZ()` and `disableFTZDAZ()`.
- [x] In `source/FastMath.h`, add `sanitizeBuffer(float* buffer, int numSamples)` that iterates and checks for `std::isnan` and `std::isinf`. If found, it zeroes the buffer.
- [x] In `source/KlangCoreProcessor.cpp` and/or `FarmerProcessor.cpp`, invoke `enableFTZDAZ()` at the start of `processBlock`.
- [x] At the end of `processBlock`, call `FastMath::sanitizeBuffer()` on all active channels.

### Phase 3: Asynchronous DAW Automation Defense
- [x] In `test/HardeningSuites.h`, add a new suite `AutomationStressTest`.
- [x] Instantiate `TheKlangFarmerAudioProcessor`.
- [x] Spawn a `std::thread` that runs a `for` loop (e.g., 50,000 iterations) randomly updating APVTS parameters using `setParameterNotifyingHost`.
- [x] Concurrently, in the main thread, allocate a `juce::AudioBuffer<float>` and repeatedly call `processBlock` (simulating the audio thread).
- [x] Join the background thread and `reporter.expect` that it completed without deadlocking or crashing.

### Phase 4: Data-Driven "Poison Pill" Fuzzing
- [x] In `test/ChaosMonkeySuite.h` or `HardeningSuites.h`, add a `PoisonPillSchemaSuite`.
- [x] Generate an invalid JSON string (e.g., malformed syntax `{"broken": true, }`, or completely empty `""`).
- [x] Intercept the `ParameterManager` or `juce::JSON::parse` logic to force it to read the bad data, ensuring it returns a fallback schema or handles the exception cleanly without a hard crash.
- [x] Ensure `gui_tests.exe` runs and passes this new gauntlet.

## Acceptance Criteria
- [x] CMake configures correctly with and without `-DTK_STRICT_WARNINGS=ON`.
- [x] The audio buffer is mathematically protected from NaNs/Infs escaping.
- [x] `dsp_tests.exe` and `gui_tests.exe` compile and pass 100%.
- [x] The DAW automation stress test correctly pounds the APVTS without data races.
- [x] AudioThread invariants (zero allocations, zero locks) are verified by `audiothread-guard` during `step-verify`.
