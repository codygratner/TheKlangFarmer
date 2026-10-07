# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-07  
**Active Milestone:** v0.3.2 "Agent Infrastructure & Editor Upgrades"  
**Task Name:** Developer Logging Subsystem (`TKS_LOG`) & Diagnostics Engine  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** COMPLETED ✅  
**Source Plan:** [`docs/completed_plans/2026-10-07_dev_logger_subsystem.md`](file:///c:/Dev/TheKlangSuite/docs/completed_plans/2026-10-07_dev_logger_subsystem.md)  

---

## Strategic Objective
Implement a structured, leveled developer logging subsystem (`TKS_LOG_INFO`, `TKS_LOG_WARN`, `TKS_LOG_ERROR`) in `source/DevLogger.h` that routes simultaneously to the system debugger (`OutputDebugString` / `DBG`) and a 5 MB rotating log file (`%LOCALAPPDATA%/TheKlangSuite/dev.log`), strictly protects real-time audio threads via runtime assertions and static guardrails, compiles to zero-cost no-ops in Release builds, and replaces raw logger calls across the suite.

---

## Acceptance Criteria & Execution Guardrails
1. **Core Logger Implementation (`source/DevLogger.h`)**:
   - `RlyehSound::DevLogger` singleton wrapping `juce::FileLogger::createDefaultAppLogger("TheKlangSuite", "dev.log", ...)`.
   - Macros `TKS_LOG_INFO`, `TKS_LOG_WARN`, `TKS_LOG_ERROR`, and `TKS_LOG`.
   - In Release (`!JUCE_DEBUG`), macros expand to `do {} while (false)` for absolute zero runtime/binary cost.
2. **Audio Thread Safety Invariant**:
   - Audio thread ID tracking via `registerAudioThread()` in `prepareToPlay()`.
   - `jassert(!isAudioThread())` and safe abort (`if (isAudioThread()) return;`) inside `DevLogger::log()`.
   - Static guardrail: `audiothread-guard` updated to flag `TKS_LOG*` in Category C.
3. **Integration Points**:
   - `FarmerProcessor.cpp` / `PlanterProcessor.cpp`: `prepareToPlay` and `releaseResources`.
   - `ParameterManager.cpp`: Asset loading counts and warnings.
   - Clean up raw `juce::Logger::writeToLog` calls in `tools/editor/Main.cpp`, `MainComponent.cpp`, and `source/UIComponents.cpp`.
4. **Test Suite Expansion**:
   - Add `test/DevLoggerTest.h` to `gui_tests.cpp`.
   - 100% test pass across `dsp_tests.exe` and `gui_tests.exe`.
   - Run `deploy.ps1` to sync artifacts.
