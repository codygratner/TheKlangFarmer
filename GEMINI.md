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


## Strict Background Task Etiquette & Watchdog Timer Policy
- **No Polling or Pinging**: When a long-running command (like a build, test suite, or script) goes to the background, you must NEVER use `manage_task` to poll its status in a loop or spam the chat with progress checks.
- **No Task Log Peeking**: You must NEVER inspect running task logs via `cat`, `Get-Content`, `type`, `head`, `tail`, or `view_file` on `.system_generated/tasks/task-*.log`. Reading logs while a background task is actively running is strictly prohibited.
- **Two-Stage Watchdog Timer Protocol (Target-Aware Adaptive Timing)**:
  - **Quick Tasks & Single Targets (5 min / 15 min)**: For incremental builds, running single test executables (`dsp_tests`, `capture_screenshot`), git operations, or quick CLI scripts:
    1. **T = 0 (Launch & Yield)**: Schedule a 5-minute watchdog timer:
       `schedule(DurationSeconds=300, TimerCondition="<task-id>", Prompt="5-minute build watchdog: check if task is actively making progress or stuck.")`
       Output a concise one-sentence notification and yield turn immediately.
    2. **T = 5 Min (Stage 1 Health Check)**: Check status once silently via `manage_task(Action="status", TaskId="<task-id>")`. If actively progressing, schedule the final 10-minute timeout watchdog (`DurationSeconds=600`, Prompt="15-minute hard timeout: task has stalled or hung; terminate and investigate.") and yield silently.
    3. **T = 15 Min (Stage 2 Hard Timeout & Auto-Kill)**: If the 15-minute timer expires, terminate via `manage_task(Action="kill", TaskId="<task-id>")`, inspect logs, and alert user.
  - **Full Rebuilds & Multi-Target Test Suites (10 min / 20 min)**: For full multi-target builds (`TheKlangFarmer_VST3` + `TheKlangPlanter_VST3` + `TheKlangEditor` + test suites, full `/build-validate` pipeline, or `--clean-first`):
    1. **T = 0 (Launch & Yield)**: Schedule a 10-minute watchdog timer:
       `schedule(DurationSeconds=600, TimerCondition="<task-id>", Prompt="10-minute full build watchdog: check if compilation/tests are progressing or stuck.")`
       Output a concise one-sentence notification and yield turn immediately.
    2. **T = 10 Min (Stage 1 Health Check)**: Check status once silently via `manage_task(Action="status", TaskId="<task-id>")`. If actively progressing, schedule the final 10-minute timeout watchdog (`DurationSeconds=600`, Prompt="20-minute hard timeout: task has stalled or hung; terminate and investigate.") and yield silently.
    3. **T = 20 Min (Stage 2 Hard Timeout & Auto-Kill)**: If the 20-minute timer expires, terminate via `manage_task(Action="kill", TaskId="<task-id>")`, inspect logs, and alert user.
  - **Remote GitHub CI & Cloud Workflows (10 min / 20 min / 30 min / 50 min)**: For remote multi-platform GitHub Actions runs (Linux, macOS, Windows matrix, auval, pluginval, release packaging):
    1. **T = 0 (Push/Trigger & Yield)**: Schedule a 10-minute watchdog timer:
       `schedule(DurationSeconds=600, Prompt="10-minute CI watchdog: check GitHub Actions run status across Linux, macOS, and Windows.")`
       Output a concise one-sentence notification with the run URL and active branch, then yield turn immediately.
    2. **T = 10 Min (Checkpoint 1)**: Query GitHub API once. Output a concise markdown table showing runner statuses and active steps. If progressing normally, schedule Checkpoint 2 (`DurationSeconds=600`, Prompt="20-minute CI watchdog: check if compilation has finished and validation tests started.") and yield turn. If any job failed, fetch failure logs immediately and alert user.
    3. **T = 20 Min (Checkpoint 2)**: Query GitHub API once. Output concise table. If validation tests (`auval`, `pluginval`) are in progress, schedule Checkpoint 3 (`DurationSeconds=600`, Prompt="30-minute CI watchdog: check if validation tests and artifact packaging completed.") and yield turn. If all passed, notify user with summary. If failed, fetch failure logs.
    4. **T = 30 Min (Checkpoint 3)**: Query GitHub API once. Output concise table. If close to finishing, schedule the final 20-minute hard timeout (`DurationSeconds=1200`, Prompt="50-minute hard timeout: remote CI has hung or deadlocked; terminate and investigate.") and yield turn. If all passed, notify user.
    5. **T = 50 Min (Hard Timeout & Auto-Cancel)**: If the 50-minute timer expires and jobs are still running, auto-cancel the remote run via GitHub API, fetch logs from the stalled runner/step, present root-cause analysis, and alert user.


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

## Strict Git Branch Safety Gate (NEVER Work Directly on Master/Main)
- **Branch Required for All Work**: Under no circumstances should new features, refactors, audits, or bug fixes be developed or committed directly on the `master` or `main` branch.
- **Mandatory Branch Creation**: Before modifying any source files, assets, or beginning execution of a plan/task, you MUST check the active branch (`git branch --show-current`). If on `master` or `main`, you MUST immediately create and switch to a descriptive branch (`git checkout -b feat/<slug>` or `feature/<slug>`).
- **Explicit Override Only**: You may ONLY work or commit directly on `master`/`main` if the user explicitly orders you to do so in the chat (e.g., "commit to master" or "do this on master"). Never assume permission.

## Strict Debugging Heuristics
- **Initialization Order First**: When encountering garbage data, parse failures, or unexplained crashes during asset loading, you must ALWAYS verify the C++ object lifecycle and static initialization order *before* investigating file encoding or unicode issues. Memory corruption masquerades as unicode errors.
- **JSON Merge Priority (The Overwrite Trap)**: When loading multiple JSON files into a central manager, strictly verify the logical sequence. Base/Master JSON files MUST be loaded *first*, followed by specific modules/effects. If loaded out of order, the master file will silently overwrite the specific module's data. Always verify the file iteration sequence before debugging "missing" parameters.

## Strict Backlog Location (Single Source of Truth)
- **Use the Existing File**: The *only* valid backlog file for this project is `docs/BACKLOG.md` (which serves as the public roadmap on GitHub). 
- **Alias Mapping**: Whenever the user mentions adding something to the "backlog", "back log", "back burner", "todolist", "todo list", "the todo", or similar phrases, you must strictly map that request to `docs/BACKLOG.md`.
- **No Rogue Files or Artifacts**: You must NEVER create new files named `backlog.md`, `BACKLOG.md`, `TODO.md`, `future_backlog.md`, etc., in the root directory or as an artifact in your brain directory. 
- **Modification Only**: Always search for and append to the existing `docs/BACKLOG.md` file in the repository.

## Smart /grill-me Wrap-Up & Mandatory Interactive Decision Modals
- **Interactive Modal Required (Zero Plain-Text Number Menus)**: Whenever presenting decision options to the user—whether at the end of `/grill-me`, `/plan`, `/strict-plan`, `/paste-plan`, `/read-plan`, or any workflow fork—you must NEVER output raw numbered text lists (e.g. `1. Option A, 2. Option B, 3. Both`) in the chat forcing the user to type "3". You MUST ALWAYS invoke the `ask_question` tool so the user gets an interactive clickable modal.
- **Proactive Housekeeping**: Whenever you complete a /grill-me interactive interview, you must document the final design conclusion (Backlog, Plan, or Both).
- **Contextual Bypass**: If the user's answers during the interview *explicitly* stated where the item should go (e.g., "put this in v0.4 of the backlog"), you are authorized to bypass the formal 3-option menu and immediately execute the documentation.
- **When in Doubt, Ask via `ask_question`**: If the destination is ambiguous, you must call `ask_question` with the options formatted as user actions (e.g., `(Recommended) Both: Add to Backlog and draft Plan`, `Backlog Only`, `Draft Plan Only`).

## Strict Scratch Script Hygiene
- **Instant Cleanup**: When you create temporary Python scripts (`update_file.py`, `fix_code.py`, `script.py`) to execute refactors, tests, or file modifications, you must **ALWAYS** delete the script immediately upon completion.
- **Execution Chain**: Always chain the removal directly in your terminal command. For example: `python script.py ; rm script.py`.
- **No Clutter**: Under no circumstances should you leave temporary scripts tracked or untracked in the root project directory.

## Git is the Archive (Ruthless Deletion)
- **No Comment Graveyards**: When refactoring or removing obsolete C++ or JSON source code, you must ruthlessly and completely delete the dead code. Never comment out large blocks of obsolete code "just to be safe."
- **No Legacy Files**: Never rename files to `old_Component.cpp` or move them to a `legacy/` directory. 
- **Rely on Git**: Git is the only acceptable time machine. If we need to see how an old feature worked, we will look at the Git history.
- **Scope**: This applies strictly to Source Code. Documentation files (like `BACKLOG_ARCHIVE.md` or PLAN artifacts) are exempt and should be preserved as requested.
## Strict Build Validation & Deployment
- **Always Deploy**: Whenever you successfully build the project and fix a bug or add a feature, you MUST ensure you run the uild-validate skill or execute deploy_vst3.bat to copy the generated .exe and .vst3 artifacts into the current_build/ directory and C:\Program Files\Common Files\VST3\. Do not leave the user looking at stale builds.
- **Dual Pipeline (Fast Iterate -> Validate)**: You may use raw cmake --build for your own rapid iteration and syntax checking. However, before presenting a finished feature or bugfix to the user, you MUST use the uild-validate skill to run the full test suite and trigger automated deployment.

## Strict Test-Driven Guardrail (Mandatory Test Parity)
- **Zero Orphaned Features**: Under no circumstances should a new DSP algorithm, audio parameter, UI card, page, modal, or editor tool be merged without corresponding test coverage in `test/dsp_tests.cpp` and `test/gui_tests.cpp`.
- **Dynamic Reflection Compliance**: `gui_tests` dynamically sweeps 100% of all registered APVTS parameters and JSON assets. If any parameter lacks a UI binding or test case, `gui_tests` will hard-fail the build.
- **Mandatory Planning Test Phase**: All architectural plans (`/plan`, `/strict-plan`, `/pasteplan`) MUST include an explicit Test Suite Update phase as a prerequisite for task completion.
- **Pre-Merge Validation**: No feature may be considered done or deployed until both `dsp_tests` and `gui_tests` pass with zero failures.
