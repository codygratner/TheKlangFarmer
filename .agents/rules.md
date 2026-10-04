# Audio Engineering, Toolchain & Execution Rules

## Real-Time Audio Thread Invariants (CRITICAL)
- **Zero Heap Allocations:** Never call `new`, `delete`, `malloc`, `free`, or resize dynamic containers (`std::vector`, `juce::Array`, etc.) inside `processBlock()`, render loops, or voice render callbacks. Pre-allocate all buffers and voices in `prepareToPlay()`.
- **Zero Locks:** Never acquire a `std::mutex`, `std::lock_guard`, or blocking synchronization primitives on the audio thread. Use lock-free atomics (`std::atomic`) or bounded lock-free FIFOs for cross-thread messaging.
- **Zero Blocking I/O:** Never call filesystem functions, network APIs, or logging/console output (`std::cout`, `DBG`) on the audio thread.

## Target Toolchain & Standards
- Standard: C++20.
- Framework: JUCE 9.x.
- Keep macOS deployment target at `CMAKE_OSX_DEPLOYMENT_TARGET=11.0`.
- Maintain clean compilation across Clang, GCC, and MSVC. Treat warnings as errors.

## Tool Permissions & Autonomy
- **Git:** Staging and committing verified changes for completed phases in `PLAN.md` is pre-approved.

## Git Branch Protection Guardrail (CRITICAL)
- **Protected Branches:** Never directly modify, stage, or commit files while the active Git branch is `master` or `main` unless the user explicitly commands it.
- **Pre-Execution Branch Check:** Before applying any edits, scaffolding files, or running plan execution loops:
  1. Inspect the active branch using `git branch --show-current` (or `git rev-parse --abbrev-ref HEAD`).
  2. If the active branch is `master` or `main`, **HALT IMMEDIATELY**. Do not touch source files.
  3. Formulate a suggested branch name derived from the task context or feature name (e.g., `feature/modulation-matrix` or `fix/alphabetize-fx-catalog`).
  4. Present the user with this exact decision gate:
     > ⚠️ **BRANCH GUARDRAIL ALERT** ⚠️  
     > You are currently on the **`master`** branch.
     >
     > How would you like to proceed?  
     > 1. **Make a new branch** (Recommended: `<suggested-branch-name>`)  
     > 2. **No, do this in master, I'm feeling fucking feisty**
  5. Wait for user input:
     - If the user selects option 1 (or confirms branch creation): run `git checkout -b <suggested-branch-name>` and proceed.
     - If the user selects option 2 (or says "feisty" / confirms master): proceed directly on `master`.

## Versioning & Build Hygiene Guardrail
- **Feature Branch Version Bumping:** Whenever beginning work on a feature branch (via `/pasteplan`), immediately bump the patch version (e.g. `0.1.8` -> `0.1.9`) and set `TKF_FEATURE_TAG` to `"-<slug>"` (e.g. `"-tooltips"`) in `CMakeLists.txt`. This forces DAWs to rescan the new VST3 and visually confirms the active build in the header.
- **Merge & Release Finalization:** Before finalizing a task in `/task-finish` and merging to `master`, always clear `TKF_FEATURE_TAG` (`set(TKF_FEATURE_TAG "")`), rebuild, and deploy the clean release version.
