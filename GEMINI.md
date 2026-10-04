# Agent Guidelines & Guardrails

## Audio Thread Invariants (CRITICAL)
- **Zero Allocations**: Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::Array::add`, `std::string` concatenation) inside `processBlock()`, `processStereo()`, or per-sample render loops. Pre-allocate in `prepareToPlay()`.
- **Zero Locks**: Never acquire a `std::mutex`, `std::lock_guard`, `juce::CriticalSection`, or wait on thread synchronization primitives in the audio processing path. Use lock-free atomics (`std::atomic`) or bounded FIFO queues for audio-to-UI messaging.
- **Zero Blocking I/O**: Never call filesystem operations, network APIs, or logging/console output (`std::cout`, `printf`, `DBG()`, `juce::Logger`) on the audio thread.
- **SIMD & Fast Math**: Prefer `TbdAudio::FastMath` over standard CRT transcendentals (`std::pow`, `std::sin`, `std::tanh`) in hot audio loops.

## Target Toolchain & Standards
- **C++ Standard**: C++20 (`CMAKE_CXX_STANDARD 20`).
- **Framework**: JUCE 9.0.3.
- **JUCE 9.0.3 Timer Hygiene**: Always call `stopTimer()` as the first line of destructors in all `juce::Timer` subclasses to prevent `#1696` unload crashes.
- **macOS Deployment Target**: Set `CMAKE_OSX_DEPLOYMENT_TARGET="11.0"` and `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` before `project()` in `CMakeLists.txt`.
- **Linux Portability**: Use `-static-libstdc++ -static-libgcc` and set `VST3_AUTO_MANIFEST FALSE`.

## Strict TODO & Backlog Handling
- **Record Only, Do Not Implement**: Whenever the user mentions items "for the todo", "on the todo list", "add to the todo", "for later", or lists future backlog features/fixes, you must **ONLY** document and record them into the project backlog / TODO list artifact.
- **No Premature Implementation**: Under no circumstances should you edit source code, run compilers, trigger builds, execute test suites, or generate releases for items marked for the TODO list or future work.
- **Wait for Explicit Execution Authorization**: Always stop after notating the items and confirm what was logged. Do not begin implementation until the user explicitly directs you to start working on them (e.g., "go ahead and implement X now" or "start working on X").

## Strict VST3 Deployment Directory
- **Program Files Only**: When deploying or copying the built VST3 plugin on Windows, you must **ONLY** install to `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3`.
- **Never Use AppData**: Under no circumstances should you copy, deploy, or fallback to `%LOCALAPPDATA%\Programs\Common\VST3` or any other local user directories.
