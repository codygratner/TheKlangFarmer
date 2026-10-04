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
