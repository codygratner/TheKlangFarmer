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

## Strict Data-Driven Architecture (CRITICAL)
- **JSON First Priority**: The data-driven JSON architecture is the primary design pattern for this project. Always default to data-driven solutions for new features, UI layouts, colors, and DSP parameters.
- **No Hardcoded Values (Unless Mandatory)**: Under no circumstances should APVTS parameter IDs, default values, min/max ranges, string labels, quick-snap intervals, or tooltips be hardcoded in C++ source files *unless it is absolutely technically mandatory* (e.g., due to strict real-time DSP constraints or third-party API requirements).
- **JSON Single Source of Truth**: All parameter definitions, layout schemas, and UI metadata must be strictly authored in and parsed from the modular JSON files within `assets/controls/`.

## Strict Planning Guardrails
- **No Spontaneous Implementation**: When the user invokes planning commands (/plan, /strict-plan, /pasteplan, /readplan), you must NEVER automatically start writing C++ code, compiling, or modifying source files.
- **Mandatory Confirmation**: Always stop, summarize the loaded plan, and explicitly ask the user for permission to begin implementation, or if they prefer to defer it to the backlog.


## Strict Background Task Etiquette
- **No Polling or Pinging**: When a long-running command (like a build, test suite, or script) goes to the background, you must NEVER use `manage_task` to poll its status.
- **Yield and Wait**: Stop calling tools and yield your turn. The system will automatically wake you up with the task's final output when it completes. Do not spam the chat with "checking status" updates.


## Strict C++ Formatting & Style
- **Indentation**: Exactly 4 spaces. No tabs. No 2-space indents.
- **Naming**: `camelCase` for variables and methods. `PascalCase` for classes and structs.
- **Braces**: 
  - Functions and Classes must use Allman style (opening brace on a new line).
  - Control flow (`if`, `for`, `while`) must use K&R style (opening brace on the same line).
  - Omit braces for single-line `if` statements.
- **Pointers/References**: Attach the asterisk/ampersand to the type, not the variable (e.g., `float* myPtr`, not `float *myPtr`).
- **Modern C++**: Use `auto` where types are obvious from the right-hand side, and use `const` generously.

## Strict Memory Management
- **No Raw Ownership**: Never use raw `new` or `delete`. 
- **Smart Pointers**: Always use `std::unique_ptr` and `std::make_unique` for dynamic object ownership.
- **Observation Only**: Raw pointers (`T*`) and references (`T&`) may ONLY be used for non-owning observation and passing objects to functions.

## UI & Audio Thread Separation
- **No UI in DSP**: Never call UI methods (e.g., `repaint()`, `setValue()`, component constructors) from `processBlock()` or `processStereo()`.
- **Async Communication**: If the audio thread needs to update the UI (like a visualizer or meter), it must use lock-free FIFOs or `juce::AsyncUpdater`.

## Strict Git Commit Etiquette
- **Conventional Commits**: All git commits must strictly follow the conventional commit format: `<type>(<optional scope>): <description>`.
- **Allowed Types**: `feat:` (new features), `fix:` (bug fixes), `refactor:` (code restructuring), `chore:` (tooling, dependencies), `docs:` (documentation/plans).
- **Example**: `feat(presets): add JSON preset browser modal`

## Strict Debugging Heuristics
- **Initialization Order First**: When encountering garbage data, parse failures, or unexplained crashes during asset loading, you must ALWAYS verify the C++ object lifecycle and static initialization order *before* investigating file encoding or unicode issues. Memory corruption masquerades as unicode errors.
- **JSON Merge Priority (The Overwrite Trap)**: When loading multiple JSON files into a central manager, strictly verify the logical sequence. Base/Master JSON files MUST be loaded *first*, followed by specific modules/effects. If loaded out of order, the master file will silently overwrite the specific module's data. Always verify the file iteration sequence before debugging "missing" parameters.
