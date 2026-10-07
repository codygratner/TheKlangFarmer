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

### Session: 2026-10-07 05:54 (9241988e-1227-466e-a0a5-446d61a69d77)
- **Chat Role:** Strategic Planning & Architecture (New Klang City) + Klang Industries
- **Primary Objectives:** Enforce strict schema separation of concerns, resolve missing Master Limiter parameters in The Klang Planter tree, introduce pre-indexed Cross-Reference ("Where Used") inspector, populate top-level callout parameter controls, and cut/publish the official v0.3.1 release.
- **Files Modified/Created:** `assets/themes/theme.json`, `assets/themes/callouts.json`, `assets/controls/*.json` (stripped `ui_colors`), `tools/editor/MainComponent.cpp`, `test/EditorTestSuite.h`, `test/ParameterSchemaAuditTest.h`, `GEMINI.md`, `docs/BACKLOG.md`
- **Key Decisions:**
  - Enforced strict schema separation: visual styling and palettes moved to `assets/themes/theme.json` and `callouts.json`, leaving `assets/controls/` strictly for DSP parameter contracts.
  - Sourced Master Limiter popover parameters from `assets/themes/callouts.json` and dynamically mounted `[Callout] Master Limiter` under The Klang Planter in the tree with popover preview rendering, keeping `assets/layouts/tkp_layout.json` strictly to the physical 8-card 4x2 grid.
  - Implemented pre-indexed bidirectional Where-Used & Associations panel in <2ms with double-click tree navigation.
  - Enabled top-level callout parameter property population in `formEditor` (matching Cards 1–8).
  - Codified the Strict Release Authority Gate in `GEMINI.md`, restricting `/cut-release` and release tagging strictly to New Klang City (Ivory Tower).
  - Codified Strict Clipboard & External Link Ingestion Guardrail in `GEMINI.md`.
  - Passed dual Debug and Release regression gauntlets (257/257 `gui_tests` and 100% `dsp_tests`).
  - Stamped local and remote Git tag `v0.3.1` and pushed branch `0.3.1-dev` and tag `v0.3.1` to GitHub.

### Session: 2026-10-07 07:59 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat (Klang Industries)
- **Primary Objectives:** Implement unified filterable master tree with 3-button filter bar, dedicated text schema layer (`assets/text/strings.json`), and DSP block control file renaming.
- **Files Modified/Created:** `CMakeLists.txt`, `assets/controls/*`, `assets/text/strings.json`, `assets/themes/theme.json`, `source/ParameterManager.h`, `source/ParameterManager.cpp`, `tools/editor/MainComponent.h`, `tools/editor/MainComponent.cpp`, `test/EditorTestSuite.h`, `test/ParameterSchemaAuditTest.h`, `docs/completed_plans/2026-10-07_unified_tree_text_schema.md`
- **Key Decisions:**
  - Renamed plural control files (`modulator.json`, `filter.json`, `envelope.json`) to singular names for consistency.
  - Extracted 100% of descriptions, choice tooltips, and localized UI copy out of control JSONs into `assets/text/strings.json` under modular namespaces (`"shared"`, `"farmer"`, `"planter"`).
  - Extended `ParameterManager` to load and merge `strings.json` into `ControlDef` at startup and support dynamic reload on disk modifications.
  - Unified `masterTree` in The Klang Editor with a 3-button filter bar (`[Controls]`, `[Layout]`, `[Theme]`) and Smart Minimum sizing.
  - Enabled simultaneous property panel editing of DSP mathematical limits and user-facing text descriptions.
  - Hardened dynamic JSON parsing against `0xC0000005` access violations by replacing unsafe `prop.value.isObject()` calls with `prop.value.getDynamicObject() != nullptr` checks for non-DynamicObject values.
  - Added explicit `stopTimer()` to `MainComponent::~MainComponent()` to satisfy JUCE 9.0.3 timer hygiene.
  - All 275 GUI unit tests and DSP tests passed; binaries deployed via `deploy.ps1`.

### Session: 2026-10-07 12:21 (651a7dba-f353-4c1f-942c-e4e75285057e)
- **Chat Role:** Planning (New Klang City)
- **Primary Objectives:** Conducted `/grill-me` architectural interview and generated a comprehensive, Simplenote-optimized SQA Briefing & Discussion Guide (`docs/SQA_MEETING_BRIEFING.md`) for user's meeting with SQA colleague. Integrated 4-layer schema leak audits, pre-release regression gauntlet, architectural glossary governance, and planned `TKS_LOG` developer logging & diagnostics subsystem.
- **Files Modified/Created:** `docs/SQA_MEETING_BRIEFING.md`, `context_clues_plan.md`, `docs/DEV_HISTORY.md`
- **Key Decisions:**
  - Kept briefing domain-agnostic (desktop application with real-time computational engine, JSON data contracts, and interactive UI) so enterprise multi-tenant QA experience maps directly without audio/DAW domain friction.
  - Formatted for Simplenote compatibility: replaced Mermaid flowchart with clean monospace ASCII testing pyramid and replaced markdown tables with structured cards.
  - Codified the 5-tier testing pyramid: Engine Stability, Schema Contracts, Headless UI Sweeps, Hardening/Stress, and CI/CD with Host Fuzzing.
  - Enhanced with recent architectural milestones: 4-layer schema separation, automated schema leak testing, 5-point pre-release regression gauntlet, and `docs/GLOSSARY.md` taxonomy.

### Session: 2026-10-07 14:44 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat (Klang Industries)
- **Primary Objectives:** Implement Developer Logging Subsystem (`TKS_LOG`) and diagnostics engine across plugins and editor with real-time audio thread safety invariant.
- **Files Modified/Created:** `CMakeLists.txt`, `source/DevLogger.h`, `source/FarmerProcessor.cpp`, `source/PlanterProcessor.cpp`, `source/ParameterManager.cpp`, `source/UIComponents.cpp`, `tools/editor/Main.cpp`, `tools/editor/MainComponent.cpp`, `test/DevLoggerTest.h`, `test/gui_tests.cpp`, `docs/completed_plans/2026-10-07_dev_logger_subsystem.md`
- **Key Decisions:**
  - Implemented header-only `RlyehSound::DevLogger` singleton wrapping `juce::FileLogger::createDefaultAppLogger("TheKlangSuite", "dev.log", ...)` targeting `%LOCALAPPDATA%/TheKlangSuite/dev.log`.
  - Added atomic audio thread ID registration (`registerAudioThread()`) and zero-allocation, zero-lock safe early exit (`if (isAudioThread()) return;`) with `jassert(!isAudioThread())` in Debug.
  - Defined preprocessor macros `TKS_LOG_INFO`, `TKS_LOG_WARN`, `TKS_LOG_ERROR`, `TKS_LOG` compiling to zero-cost `do {} while (false)` in Release (`!JUCE_DEBUG`) for zero binary strings and zero runtime overhead.
  - Replaced legacy `juce::Logger::writeToLog` calls across `Main.cpp`, `MainComponent.cpp`, and `UIComponents.cpp`.
  - Authored `DevLoggerTest.h` test suite; 294 / 294 GUI tests and 100% DSP tests passed cleanly in both Debug and Release configurations.
