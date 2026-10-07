# The Klang Suite: Developer History & Institutional Memory

## Section A: Executive Institutional Memory & Lessons Learned

### Audio Thread & DSP Rules
- **Zero Allocations:** Never call 
ew, malloc, ree, or resize dynamic containers (std::vector::push_back, juce::Array::add, std::string concatenation) inside processBlock(), processStereo(), or per-sample render loops. Pre-allocate in prepareToPlay().
- **Zero Locks:** Never acquire a std::mutex, std::lock_guard, juce::CriticalSection, or wait on thread synchronization primitives in the audio processing path. Use lock-free atomics (std::atomic) or bounded FIFO queues for audio-to-UI messaging.
- **Zero Blocking I/O:** Never call filesystem operations, network APIs, or logging/console output (std::cout, printf, DBG(), juce::Logger) on the audio thread.
- **SIMD & Fast Math:** Prefer TbdAudio::FastMath over standard CRT transcendentals (std::pow, std::sin, std::tanh) in hot audio loops.

### JUCE 9.0.3 Hygiene
- **Timer Destructor Safety:** Always call stopTimer() as the first line of destructors in all juce::Timer subclasses to prevent JUCE issue #1696 unload crashes.

### Data-Driven Architecture
- **JSON First Priority:** The data-driven JSON architecture is the primary design pattern. 
- **JSON Single Source of Truth:** All parameter definitions, layout schemas, and UI metadata must be strictly authored in and parsed from the modular JSON files within ssets/controls/.
- **Parameter Registration:** ParameterManager.cpp dynamically registers APVTS parameters at runtime directly from ssets/controls/*.json.

### Cross-Platform Compilation Quirks
- **macOS Deployment Target:** Set CMAKE_OSX_DEPLOYMENT_TARGET="11.0" and CMAKE_OSX_ARCHITECTURES="arm64;x86_64" before project() in CMakeLists.txt.
- **Linux Portability:** Use -static-libstdc++ -static-libgcc and set VST3_AUTO_MANIFEST FALSE.
- **Windows VST3 Paths:** Only install to C:\Program Files\Common Files\VST3\The Klang Farmer.vst3. Never use local AppData directories.

### Agent Watchdog Protocols
- **Two-Stage Target-Aware Watchdog Timers:**
  - Quick Tasks & Single Targets (5 min / 15 min): DurationSeconds=300 / 600.
  - Full Rebuilds & Multi-Target Test Suites (10 min / 20 min): DurationSeconds=600 / 600.

## Section B: Chronological Session Harvest


### Session: 2026-10-06 13:04 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Test the new `/refresh-context` skill and verify context clues generation.
- **Files Modified/Created:** `CMakeLists.txt`, `PLAN.md`, `.gemini\config\skills\refresh-context\SKILL.md`, `docs/DEV_HISTORY.md`, `context_clues.md`
- **Key Decisions:** Switched to `0.3.1-dev` branch and completed Phase 1 of the refresh-context plan.

### Session: 2026-10-06 13:14 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Finalize the `/refresh-context` task, fix the UI instructions, and commit the feature.
- **Files Modified/Created:** `GEMINI.md`, `PLAN.md`, `docs/DEV_HISTORY.md`, `docs/BACKLOG.md`
- **Key Decisions:** Replaced the hallucinated `/clear` command with "Replace with New" in all rules and documentation. Committed the completed feature to the `0.3.1-dev` branch.

### Session: 2026-10-06 13:16 (172f4706-f36e-4cbe-aff3-aafefc715464)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Realized "Replace with New" spawns a new chat window; user will delete this one.
- **Files Modified/Created:** None
- **Key Decisions:** Discard this session.

### Session: 2026-10-06 22:10 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Optimize The Klang Planter GUI rendering performance and eliminate UI thread latency.
- **Files Modified/Created:** `source/PlanterEditor.h`, `source/PlanterEditor.cpp`, `docs/completed_plans/2026-10-06_optimize_planter_gui_rendering.md`, `docs/communique/build_to_plan.md`
- **Key Decisions:** Made `PlanterHeaderVisualizer` opaque with solid chassis fill to eliminate parent component background repaint cascades. Reduced oscilloscope path points from 128 to 64 with smooth rounded stroking. Decreased editor timer frequency from 60 Hz to 30 Hz standard. Implemented idle throttling to skip repainting when audio is silent. Cached static header text strings and pre-computed font glyph widths in constructor.

### Session: 2026-10-07 05:00 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Eradicate the Release teardown crash (`0xC0000005`) in `gui_tests.exe` and recast backlog item 0.5 into an interactive parameter audit feature inside The Klang Editor.
- **Files Modified/Created:** `source/VersionChecker.h`, `source/VersionChecker.cpp`, `test/gui_tests.cpp`, `test/PluginIntensiveTestSuite.h`, `docs/completed_plans/2026-10-06_fix_release_teardown_crash.md`, `docs/communique/build_to_plan.md`, `docs/BACKLOG.md`
- **Key Decisions:** Inherited `juce::DeletedAtShutdown` in `VersionChecker` and added static `teardown()` before `guiContext` destruction. Isolated and resolved asynchronous modal double-free in `test/PluginIntensiveTestSuite.h` by instantiating `CallOutBox` directly via `std::make_unique`. Verified 215 / 215 GUI unit tests and 100% DSP tests with exit code 0. Recast backlog item 0.5 into an editor feature with mandatory planning discussion directive for New Klang City.


