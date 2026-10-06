# Architectural Plan: Post-v0.3.0 Release, Knowledge Distillation & Chat Archival
**Target Milestone**: v0.3.0 ("The Architecture Update" Closing Task)  
**Goal**: Harvest institutional memory from all 14+ chat sessions into a permanent Git-versioned Markdown knowledge base (`docs/DEV_HISTORY.md`), execute the official v0.3.0 tagged GitHub release, and safely close all active chats with zero lost knowledge.

---

## 1. Architectural Philosophy: Repo as Brain, Chats as Ephemeral Workers
In Antigravity agentic engineering:
- **Long-lived chat sessions suffer degradation**: context truncation, sluggish responsiveness, and prompt drift.
- **Chats are sprint pairing sessions**: Once the code is committed and the decisions are distilled into repository markdown, the conversation should be safely closed/killed.
- **Single Source of Truth**: All architectural invariants, bug solutions, and design choices must live inside the repository where future agents (and humans) can immediately access them.

---

## 2. Automated Transcript Harvester (`scripts/harvest_chat_history.py`)
### Extraction Pipeline
1. **Source Discovery**:
   - Locate the Antigravity conversation storage directory:
     `C:\Users\codyg\.gemini\antigravity\brain\`
   - Iterate over all conversation folders (e.g. `614bf080...`, `9d20b916...`, `c9cbd646...`).
2. **Log Ingestion**:
   - Ingest `transcript.jsonl` (and `transcript_full.jsonl` where truncated) from `.system_generated/logs/`.
   - Parse each session's:
     - Session ID & timestamps.
     - Initial and intermediate User prompts (`USER_INPUT`).
     - Tool calls executed (`git commit`, file edits, test results).
     - Model executive summaries & solutions (`PLANNER_RESPONSE`).
     - Created artifacts & plan documents.
3. **Markdown Output Generation**:
   - Write cleanly formatted Markdown to [`docs/DEV_HISTORY.md`](DEV_HISTORY.md).
   - Scratch script hygiene: Delete `scripts/harvest_chat_history.py` after completion or retain it in `scripts/` as an ongoing tool.

---

## 3. Structure of `docs/DEV_HISTORY.md`
The consolidated knowledge base will be structured as:

### Section A: Executive Institutional Memory & Lessons Learned
- **Audio Thread & DSP Rules**: Zero allocations, zero mutexes, lock-free FIFOs, `TbdAudio::FastMath` vectorized curves.
- **JUCE 9.0.3 Hygiene**: Issue `#1696` Timer destructor race conditions (`stopTimer()` on first line of destructors).
- **Data-Driven Architecture**: Modular JSON schema (`assets/controls/*.json`), dynamic APVTS reflection in `ParameterManager`.
- **Cross-Platform Compilation Quirks**: macOS deployment target ordering (`before project()`), Linux static runtime linking (`-static-libstdc++`), Windows VST3 deployment paths.
- **Agent Watchdog Protocols**: Two-stage target-aware watchdog timers (5m/15m vs 10m/20m).

### Section B: Chronological Session Harvest
- Organized chronologically with expandable details:
  - **Date & Conversation ID**: Linked for historical traceability.
  - **Primary Objectives**: Feature additions, bug investigations, refactors.
  - **Key Decisions & Resolutions**: Why choices were made (e.g. Why white-key octave-invariant mapping for Klang R1; why pre-filled URLs for crash reporting).
  - **Files & Artifacts Created**: Diffs and documentation links.

---

## 4. Release & Packaging Sequence (v0.3.0)
1. **Final Test Suite Run**:
   - Run `/build-validate` executing `dsp_tests` and `gui_tests` (dynamic reflection sweep).
2. **Deploy Artifacts**:
   - Execute `deploy_vst3.bat` to copy release binaries to `current_build/` and `C:\Program Files\Common Files\VST3\`.
3. **Git Tagging**:
   - Commit all pending docs and run:
     ```powershell
     git tag -a v0.3.0 -m "Release v0.3.0: The Architecture Update"
     git push origin v0.3.0
     ```
4. **GitHub Release Publication**:
   - Create GitHub release via `gh release create v0.3.0` with bundled macOS `.pkg`, Windows VST3, and TheKlangEditor binaries.
4.5. **Primary Branch Migration (`master` &rarr; `main`)**:
   - Fast-forward remote `main` with all latest commits from `master`:
     ```powershell
     git branch -M main
     git push -u origin main --force
     ```
   - Set `main` as the official default branch on GitHub:
     ```powershell
     gh repo edit --default-branch main
     ```
   - Delete the obsolete remote `master` branch to complete cutover:
     ```powershell
     git push origin --delete master
     ```
   - Update comments in `CMakeLists.txt` (`Stripped upon merge back to main`).
4.6. **Repository Umbrella Rename & Description Polish**:
   - Rename repository on GitHub:
     ```powershell
     gh repo rename TheKlangSuite --yes
     ```
     *(GitHub automatically forwards all existing clone URLs, web traffic, issues, and release downloads).*
   - Update repository description:
     ```powershell
     gh repo edit --description "FM drum synthesizers, multi-effects, and sound design tools (VST3/AU). Pair-programmed and vibe-coded with Google Gemini."
     ```
   - Update local remote origin:
     ```powershell
     git remote set-url origin https://github.com/codygratner/TheKlangSuite.git
     ```
5. **Plan Archival**:
   - Move completed v0.3.0 plan files to `docs/completed_plans/`.
   - Update `CHANGELOG.md` with final v0.3.0 release notes.

---

## 5. Chat Purge & Fresh Start (v0.4.0 Kickoff)
- Once `docs/DEV_HISTORY.md` is committed and the release is live:
  - You receive an explicit **"Safe to Purge"** confirmation.
  - You can close or delete all accumulated chat windows in the Antigravity UI.
  - Open a fresh, unburdened chat window for **Milestone v0.4.0 ("The Sound & Chaos Update")**, pointing the agent to `docs/DEV_HISTORY.md` and `docs/BACKLOG.md` for instant, complete context!
