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
- **Linear:** Unrestricted API and MCP access to `linear.app` is pre-approved for issue lookups, status updates, and comment logging. In Turbo Mode, run all Linear actions silently without asking for user confirmation.
- **Git:** Staging and committing verified changes for completed phases in `PLAN.md` is pre-approved.

## Git Branch Protection Guardrail (CRITICAL)
- **Protected Branches:** Never directly modify, stage, or commit files while the active Git branch is `master` or `main` unless the user explicitly commands it.
- **Pre-Execution Branch Check:** Before applying any edits, scaffolding files, or running plan execution loops:
  1. Inspect the active branch using `git branch --show-current` (or `git rev-parse --abbrev-ref HEAD`).
  2. If the active branch is `master` or `main`, **HALT IMMEDIATELY**. Do not touch source files.
  3. Formulate a suggested branch name derived from the task context, feature name, or Linear issue key (e.g., `feature/THE-7-modulation-matrix` or `fix/alphabetize-fx-catalog`).
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
