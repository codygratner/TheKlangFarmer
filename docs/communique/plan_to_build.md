# Master Blueprint & Dispatch Contract
**Origin:** New Klang City (Planning Headquarters)  
**Destination:** Klang Industries (The Factory Floor)  
**Date:** 2026-10-06  
**Active Milestone:** v0.3.1 (Pre-Release Regression Hotfix)  
**Task Name:** Fix Release Teardown Crash (0xC0000005) in VersionChecker  
**Recommended Model Tier:** Tier 2 (`Gemini 3.8 Flash (Thinking: High)`)  
**Status:** READY_FOR_EXECUTION  

---

## Strategic Objective
Eliminate the process exit access violation (`0xC0000005` in `ntdll.dll!RtlFreeHeap`) when running `gui_tests.exe` in the Release configuration. The crash occurs because `VersionChecker` was allocated as a static local and destroyed after `juce::shutdownJuce_GUI()` tore down the GUI/thread subsystem. Inheriting from `juce::DeletedAtShutdown` and implementing an explicit `VersionChecker::teardown()` before `guiContext` is destroyed resolves the lifecycle conflict completely.

---

## Directives for Klang Industries
1. **Model Check**: Please verify your model setting is **Tier 2: Gemini 3.8 Flash (Thinking: High)**.
2. **Branch**: Maintain execution on `0.3.1-dev`.
3. **Execution Plan**: Follow [`PLAN.md`](file:///c:/Dev/TheKlangSuite/PLAN.md) strictly across Phases 1 through 4:
   - Phase 1: In [`source/VersionChecker.h`](file:///c:/Dev/TheKlangSuite/source/VersionChecker.h), inherit from `juce::DeletedAtShutdown` and add `static void teardown();`.
   - Phase 2: In [`source/VersionChecker.cpp`](file:///c:/Dev/TheKlangSuite/source/VersionChecker.cpp), implement `getInstance()` with a managed heap pointer and `teardown()`.
   - Phase 3: In [`test/gui_tests.cpp`](file:///c:/Dev/TheKlangSuite/test/gui_tests.cpp), invoke `VersionChecker::teardown();` before `guiContext` destructs.
   - Phase 4: Compile Release `gui_tests` and verify `.\build\Release\gui_tests.exe` exits cleanly with `EXIT CODE: 0`.
4. **Validation**: Execute `build/Release/gui_tests.exe` (215/215 passed, code 0) and `build/Release/dsp_tests.exe` (100% passed).
5. **Deployment**: Run `deploy.ps1` to keep all artifacts in sync.
6. **Communique**: Report results in `docs/communique/build_to_plan.md` upon completion.
