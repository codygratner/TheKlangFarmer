# 🎛️ The Audio DSP & C++ Field Guide

> **Author:** The Klang Suite Project Journal  
> **Audience:** Developers transitioning into C++ audio development, AI-assisted software engineers.  
> **Status:** Living Field Manual (Milestone v0.4.0)

*Note: For AI pair-programming workflows and pipeline protocols, see the [Vibe-Coder's Field Guide](vibe_coding_field_guide.md).*

## 1. The Hard Invariants: Why Audio Programming Rejects "Lazy Vibes"

In web or backend development, a slow database query or a 50ms garbage collection pause is a minor latency blip. In real-time audio DSP, **it is a catastrophic failure**.

The audio card hardware requests a new buffer of samples every 1.3ms (at 64 samples @ 48kHz). If your code misses that deadline by even one microsecond, the audio stream drops out, resulting in an audible click, pop, or harsh digital glitch.

Therefore, our AI pair-programming rules had to codify non-negotiable **Audio Thread Invariants**:
1. **Zero Heap Allocations:** Never call `new`, `malloc`, `free`, or resize dynamic containers (`std::vector::push_back`, `juce::String` concatenation) on the audio path. Everything must be pre-allocated in `prepareToPlay()`.
2. **Zero Locks:** Never acquire a `std::mutex`, `juce::CriticalSection`, or wait on thread primitives. Audio-to-UI communication must use lock-free atomics (`std::atomic<float>`) or single-reader single-writer FIFOs.
3. **Zero Blocking I/O:** Never call filesystem operations, network sockets, or console output (`std::cout`, `DBG()`, `printf`) on the audio thread.
4. **SIMD & FastMath:** Standard math functions (`std::pow`, `std::sin`, `std::tanh`) take 50–120 CPU cycles. In hot voice loops, you must use rational Padé or polynomial approximations (`TbdAudio::FastMath`).

## 2. Advice for Fellow Developers (From Python/JS to C++)

If you are a developer with experience in dynamic languages (Python, JavaScript, Ruby) or garbage-collected frameworks and want to build high-performance C++ software with an AI assistant:

1. **Don't Let the AI Guess the Architecture:** AI agents are brilliant code-completion engines, but they will default to whatever pattern is easiest in the moment (which is usually monolithic, hardcoded C++). Enforce clean architectural patterns (like data schemas and interface boundaries) from Day 1.
2. **Invest Heavily in Test Harnesses:** Writing a functional test harness (`gui_tests`) feels like a detour when you just want to build your app. In reality, it is the single best investment you will make. It allows you to accept large AI refactors with complete confidence.
3. **Embrace "Living Documentation":** Keep your taxonomy codified in a glossary (`GLOSSARY.md`). If you and the AI agree that a container is a "Card" and a rotary control is a "Knob", you eliminate 90% of naming bugs and mismatched variables.
4. **Treat Failure as Data:** When a bug slips through, don't just fix the code. Ask: *"What guardrail was missing that allowed this to happen?"* Update your `GEMINI.md` or test suite so the exact same mistake can never be made again.

---

*“Code is ephemeral; test harnesses, data contracts, and architectural guardrails are permanent.”*
