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


### Session: 2026-09-30 17:01 (614bf080-6ee4-42fc-83b0-1a1cb89a9c66)
- **Primary Objectives:** please import from the DrumVST project, I couldn't "open  folder" and had to create a new project. the final thing it was trying to do is rename the D yes, please delete it. also, did you import the conversation history first? I'm assuming that's NOT kept in THAT directory, but I'd like to first make
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 15:43 (3883592a-f4a6-4e2a-a11c-8e822ecfc51c)
- **Primary Objectives:** I have placed the complete architectural specification in `TRACKER_SPEC.md`. I'd also like to target ARM for a RaspberryPi running arm. OH! and, I'd like to target Steam Deck as well!!
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 15:54 (73e2be84-01c5-4312-923a-d081c3e26894)
- **Primary Objectives:** ok, I just got the $99 gemini plan, so lets go ahead and do the fast math and mac build fixes
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 16:38 (681d6024-b52b-4b46-b3d2-d089715702e9)
- **Primary Objectives:** /paste-plan ```cpp if you haven't already, bump the version number. then, merge back to master and build locally
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 17:55 (dabeb9df-15f0-4212-a1ca-c723aa498ce5)
- **Primary Objectives:** /paste-plan --build
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 18:42 (07efbd57-52e8-4661-afbf-191bb84f97db)
- **Primary Objectives:** /paste-plan --build maknig 100% sure, this is also updating the filter in TKP as well, yeah?
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 19:13 (d8807f82-f5e6-4475-a82e-996a9ed95884)
- **Primary Objectives:** create a new branch and update the version number to 0.2.0. update all the docs, including the spec, readme and quickstart. then, merge back to master looks like you didn't update the docs like I asked?
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-04 20:48 (534dc2f4-ad3a-48cc-8451-d345c088ef38)
- **Primary Objectives:** /paste-plan put this as the top item on the todo list
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-05 11:48 (538c8906-de7c-4b3d-9b31-aea120b7ea6d)
- **Primary Objectives:** /paste-plan proceedproceed
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-05 16:58 (50373ee0-abb8-49ee-802a-344c921b90df)
- **Primary Objectives:** /read-plan the theme plan from teh backlog /read-plan the edit modal with quick snaps
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 00:28 (50f8619e-6c46-4433-981b-48e31f012fd2)
- **Primary Objectives:** let's do the parameter audit! /plan can we audit what values are in what json file? is there a reason for duplicated entries?
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 12:03 (03df1a26-8539-4b56-a494-ff1e0cf9034a)
- **Primary Objectives:** /grill-me alright! let's build the gui test tools from the backlog! this should be for every program/plugin, I suppose? that way ALL guis can be teste /grill-me and, the tests for the editer? is that comprehensive as well?
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 13:56 (c9cbd646-1a31-4fc7-b77d-c6f103f951e9)
- **Primary Objectives:** alright! let's get started on the cruft purge! execute it!
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 15:25 (9d20b916-fcfd-4eb8-ab01-4c8d78f546d1)
- **Primary Objectives:** alright! let's tackle the last three tasks from the 0.3.0 roadmap in order that makes the most sense what's left on the tasks for 0.3.0?
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 17:33 (9241988e-1227-466e-a0a5-446d61a69d77)
- **Primary Objectives:** you are the new klang planning folder! alright, let's harvest the bodies of the old chats and make sure the harvest is good!! that way I can dispose of the corpses (old chats). there's a ta
- **Files Modified/Created:** None
- **Key Decisions:** (Auto-harvested chat session)

### Session: 2026-10-06 13:04 (8fe6b97b-f027-4cc9-8cc2-d5f67ccfb0c9)
- **Chat Role:** Implementation & Build Chat
- **Primary Objectives:** Test the new `/refresh-context` skill and verify context clues generation.
- **Files Modified/Created:** `CMakeLists.txt`, `PLAN.md`, `.gemini\config\skills\refresh-context\SKILL.md`, `docs/DEV_HISTORY.md`, `context_clues.md`
- **Key Decisions:** Switched to `0.3.1-dev` branch and completed Phase 1 of the refresh-context plan.
