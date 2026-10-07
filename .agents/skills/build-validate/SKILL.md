---
name: build-validate
description: Automates CMake builds, filters noisy compiler and linker outputs down to clean actionable errors, and executes native test suites (dsp_tests, pluginval). Triggers on `/buildvalidate`, `/build-validate`, "validate build", "build and test", or "run tests".
---

# Build Validator & Test Runner

## Goal
Execute a clean, targeted build of the project or specific target, filter away hundreds of lines of build noise to surface only the exact error lines and compiler diagnostics, run the automated test suite (`dsp_tests` / `pluginval`), and present a concise health report.

## Operational Constraints
- **Preserve Terminal Context:** Never dump thousands of lines of raw compiler output into the chat. Extract and format only errors and warnings.
- **Fail Fast:** If compilation fails, do not attempt to run unit tests or launch host validation.
- **Strict VST3 Installation Path:** If a full plugin install build is performed on Windows, only deploy to `C:\Program Files\Common Files\VST3\The Klang Farmer.vst3`. Never use local AppData directories.

## Workflow

### 1. Identify Target & Configuration
- If a target is specified in the invocation (e.g. `/buildvalidate dsp_tests`), build that target.
- Default targets:
  - Quick test run: `dsp_tests` and `gui_tests` (unless `--skip-gui` is passed)
  - Full plugin build: `TheKlangFarmer_VST3`, `TheKlangPlanter_VST3`, `dsp_tests`, `gui_tests`
- Configuration: Default to `Release` unless explicitly asked for `Debug`.


### 1.5 Parse Test Suite Flags
- Check if the user invoked the command with `--skip-gui` (or requested to bypass GUI tests).
- If `--skip-gui` is active, remove `gui_tests` from the target execution list.

### 2. Execute CMake Build
Before building, determine if any `assets/*.json` files or `CMakeLists.txt` were added or modified in this phase.
- **If YES (Smart Cache-Busting):** Run a fast reconfigure first to ensure the BinaryData cache catches the changes:
  ```powershell
  cmake -B build
  ```
- **Then, execute the build command:**
  ```powershell
  cmake --build build --config Release --parallel --target <TARGET>
  ```
*(Never use `--clean-first` globally, as it destroys fast C++ iteration times. Only use it as a last-resort recovery step if the linker explicitly fails).*

Capture stdout and stderr.

### 2.5 Execution Guardrail & Watchdog Timer Policy
When running CMake builds or tests using `run_command`:
1. **Never Poll or Peek Logs**: If the build task goes to the background, DO NOT loop, poll, or repeatedly call `manage_task(Action="status")`. Never inspect running task logs via `cat`, `Get-Content`, or `view_file`.
2. **Two-Stage Watchdog Timer Protocol (Target-Aware Adaptive Timing)**:
   - **Quick Tasks & Single Targets (5 min / 15 min)** (e.g. `dsp_tests`, `capture_screenshot`, fast syntax checks):
     - **T = 0**: `schedule(DurationSeconds=300, TimerCondition="<task-id>", Prompt="5-minute build watchdog: check if task is actively making progress or stuck.")`
     - **T = 5 Min (Stage 1 Health Check)**: If timer fires, check status once via `manage_task(Action="status")`. If progressing, set 10-minute timeout watchdog: `schedule(DurationSeconds=600, TimerCondition="<task-id>", Prompt="15-minute hard timeout: task has stalled or hung; terminate and investigate.")` and yield silently.
     - **T = 15 Min (Stage 2 Hard Timeout & Auto-Kill)**: Terminate via `manage_task(Action="kill", TaskId="<task-id>")`, inspect logs, and alert user.
   - **Full Rebuilds & Multi-Target Test Suites (10 min / 20 min)** (e.g. full `/build-validate` pipeline, `TheKlangFarmer_VST3` + `TheKlangPlanter_VST3` + `TheKlangEditor` + test suites, or `--clean-first`):
     - **T = 0**: `schedule(DurationSeconds=600, TimerCondition="<task-id>", Prompt="10-minute full build watchdog: check if compilation/tests are progressing or stuck.")`
     - **T = 10 Min (Stage 1 Health Check)**: If timer fires, check status once via `manage_task(Action="status")`. If progressing, set 10-minute timeout watchdog: `schedule(DurationSeconds=600, TimerCondition="<task-id>", Prompt="20-minute hard timeout: task has stalled or hung; terminate and investigate.")` and yield silently.
     - **T = 20 Min (Stage 2 Hard Timeout & Auto-Kill)**: Terminate via `manage_task(Action="kill", TaskId="<task-id>")`, inspect logs, and alert user.

### 3. Parse & Filter Compiler Output
Filter output using regex patterns to isolate relevant diagnostic lines:
- **MSVC Errors/Warnings**: `.*\\([0-9]+\\): (error|warning) [A-Z0-9]+: .*`
- **GCC / Clang Errors**: `.*:[0-9]+:[0-9]+: (fatal error|error|warning): .*`
- **Linker Errors**: `(LNK[0-9]+|undefined reference to).*`

If the build succeeds with 0 errors, output:
`✅ Build Succeeded: Target '<TARGET>' compiled cleanly.`

If the build fails, output an actionable error table:
| File | Line | Severity | Code | Message |
| :--- | :--- | :--- | :--- | :--- |
| `path/to/file.cpp` | 142 | `error` | `C2065` | `'fastPow2': undeclared identifier` |

### 4. Execute Native Tests
If compilation succeeded and the built target includes `dsp_tests` or testing was requested:
1. Run `./build/Release/dsp_tests.exe` (or `./build/dsp_tests` on macOS/Linux).
2. Check exit code:
   - Exit code `0`: All assertions passed.
   - Non-zero: Parse the test failure output to extract the exact test suite and assertion that failed.

### 5. Execute Host Validation (Optional / When Requested)
If `pluginval` is available locally and VST3 bundles are built:
```powershell
.\pluginval.exe --strictness-level 5 --validate-in-process --timeout-ms 60000 "build\TheKlangFarmer_artefacts\Release\VST3\The Klang Farmer.vst3"
```

### 6. Output Diagnostic Summary
Present a clear, high-level summary:

```markdown
### 🔨 Build & Validation Report

- **Target:** `<target>` (`Release`)
- **Compilation:** ✅ PASSED (0 errors, 0 warnings)
- **DSP Unit Tests (`dsp_tests`):** ✅ 32/32 tests passed (0 failures)
- **Host Validation (`pluginval`):** ✅ In-process validation passed
```

### 7. 2-Strike Factory Floor Escalation Protocol
If compilation or unit test execution fails during an automated phase:
1. Klang Industries executing on **Tier 2 (Flash High)** is allowed up to **2 iterative attempts** to resolve the compiler or test failure.
2. If unresolved after **2 attempts**:
   - **Hard Pause:** Do NOT make further speculative edits.
   - **Issue Tier Upgrade Advisory:** Present an advisory recommending escalation to **Tier 1: Gemini 3.1 Pro (Thinking: High)**.
   - **Pause for Confirmation:** Stop and wait for user confirmation before proceeding.

