# Implementation Plan: FastMath SIMD Core & Mac Compatibility Fixes (THE-8 & THE-5)

**Branch:** `feature/THE-8-THE-5-fastmath-and-mac-fixes`  
**Linear Issues:** [THE-8](https://linear.app/the-klang-farmer/issue/THE-8/fastmathh-and-simd-accelerated-dsp-core-desktop-and-embedded), [THE-5](https://linear.app/the-klang-farmer/issue/THE-5/mac-and-linux-compatibility-dynamic-host-loading-ci-and-dockerization)

---

## 1. Overview & Objectives
1. **Mac & Linux Compatibility Hardening (THE-5)**:
   - Configure `CMakeLists.txt` with `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` before `project()`.
   - Add Linux static runtime linking `-static-libstdc++ -static-libgcc` and set `VST3_AUTO_MANIFEST FALSE`.
   - Enforce headless compile definitions `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`, `JUCE_VST3_CAN_REPLACE_VST2=0`.
   - Update `.github/workflows/build_and_release.yml` with ad-hoc codesigning (`codesign --force --deep -s -`) and bundle `Unlock_and_Install_Mac.command` + `README_MAC_INSTALL.txt`.
   - Create `.github/workflows/ci.yml` for automated CI testing across macOS, Linux, and Windows with `pluginval` and `auval`.
   - Provide `docker/Dockerfile.linux` and `docker/docker-compose.yml` for reproducible Linux builds.

2. **FastMath.h SIMD Acceleration Core (THE-8)**:
   - Implement `source/FastMath.h` (`TbdAudio::FastMath`):
     - `fastPow2(float x)`: IEEE 754 bit-manipulation with 3rd-order minimax polynomial (< 0.05% max error).
     - `fastExp(float x)`: Fast percussive decay curve generator.
     - `fastSin(float rad)` & `fastSinNorm(float phase)`: Branchless polynomial approximation.
     - `fastCos(float rad)` & `fastCosNorm(float phase)`: Branchless cosine.
     - `fastTanh(float x)`: Padé rational saturation approximation.
     - `fastDbToGain(float db)`: 2-cycle dB-to-linear conversion.
     - `PhaseAccumulator32`: Branchless 32-bit fixed-point phase accumulator.
   - Implement `test/benchmark_dsp.cpp` (`benchmark_dsp.exe`) to measure cycle counts, speedup, and verify < 0.05% error.
   - Integrate `FastMath` into `source/ModularBlocks.h` and `source/PlanterEngine.h` hot loops.
   - Verify real-time audio-thread safety (`audiothread-guard`) and run all test suites (`dsp_tests.exe`).

---

## 2. Step-by-Step Implementation Phases

### Phase 1: Cross-Platform Build & CI Hardening (THE-5) — [x] COMPLETED
- [x] **Step 1.1**: Update `CMakeLists.txt`:
  - Place `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` before `project()`.
  - Add Linux `-static-libstdc++ -static-libgcc` linking options.
  - Set `VST3_AUTO_MANIFEST FALSE` on `TheKlangFarmer` and `TheKlangPlanter`.
  - Set `JUCE_WEB_BROWSER=0` and `JUCE_USE_CURL=0` compile definitions.
- [x] **Step 1.2**: Create `scripts/mac/Unlock_and_Install_Mac.command` and `scripts/mac/README_MAC_INSTALL.txt`.
- [x] **Step 1.3**: Update `.github/workflows/build_and_release.yml` with ad-hoc codesigning and macOS installer bundling.
- [x] **Step 1.4**: Create `.github/workflows/ci.yml` for multi-platform CI verification with `pluginval`.
- [x] **Step 1.5**: Create `docker/Dockerfile.linux` and `docker/docker-compose.yml`.
- [x] **Step 1.6**: Run `/compat-check` audit to verify all compatibility requirements.

---

### Phase 2: Core `FastMath.h` Engine & Mathematical Accuracy (THE-8) — [x] COMPLETED
- [x] **Step 2.1**: Author `source/FastMath.h` under `TbdAudio::FastMath`.
- [x] **Step 2.2**: Implement `test/benchmark_dsp.cpp` to benchmark throughput and verify < 0.05% error against CRT.
- [x] **Step 2.3**: Add `benchmark_dsp` executable target to `CMakeLists.txt`.
- [x] **Step 2.4**: Add `FastMath` accuracy and edge-case test cases to `test/dsp_tests.cpp`.
- [x] **Step 2.5**: Compile and execute `dsp_tests.exe` and `benchmark_dsp.exe`.

---

### Phase 3: Hot-Path DSP Integration in Modular Synth Engine (THE-8) — [x] COMPLETED
- [x] **Step 3.1**: Include `FastMath.h` in `source/DSPBlock.h`.
- [x] **Step 3.2**: Optimize `evaluateWaveform` in `source/ModularBlocks.h` using `fastSinNorm`.
- [x] **Step 3.3**: Optimize `CarrierBlock` and `ModulatorBlock` inner sample loops (`fastPow2`, `fastSinNorm`).
- [x] **Step 3.4**: Optimize `SaturationBlock`, `LimiterBlock`, and drive stages using `fastTanh` and `fastDbToGain`.
- [x] **Step 3.5**: Optimize filter frequency sweeps and modulation envelopes in `source/ModularBlocks.h` using `fastPow2`.
- [x] **Step 3.6**: Optimize `source/PlanterEngine.h` oscillators and saturation using `FastMath`.
- [x] **Step 3.7**: Run `/audiothread-guard` on modified files.
- [x] **Step 3.8**: Run `dsp_tests.exe` and verify all 33+ unit tests pass with zero regressions.

---

### Phase 4: Benchmarking, Validation & Documentation Sync — [x] COMPLETED
- [x] **Step 4.1**: Execute `benchmark_dsp.exe` and record CPU cycle reduction and timing metrics.
- [x] **Step 4.2**: Build full plugin targets `TheKlangFarmer_VST3` and `TheKlangPlanter_VST3`.
- [x] **Step 4.3**: Update `docs/BACKLOG.md` and `docs/BACKLOG_ARCHIVE.md` logging completion of THE-8 and THE-5.
- [x] **Step 4.4**: Stage and commit changes.
