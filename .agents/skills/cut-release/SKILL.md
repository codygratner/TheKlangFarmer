---
name: cut-release
description: Executes the rigorous pre-release regression gauntlet, safe cruft sweep, dual-configuration verification, automated version bump, git tagging, artifact deployment, and institutional memory harvesting. Triggers on `/cutrelease`, `/cut-release`, "cut release", "release version", or "tag release".
---

# Cut Release & Regression Gauntlet

## Goal
Safely and deterministically cut an official Git release tag for **The Klang Suite**, enforcing the 5-stage pre-release regression gauntlet, auto-cleaning safe temporary cruft, rolling back to Bug Squashing Mode if any defect is detected, updating `CMakeLists.txt`, tagging the commit, and optionally summoning the transcript harvester to wipe context cleanly for the next milestone.

---

## Operational Guardrails
- **Zero Premature Tagging**: Never tag a release or bump version strings until 100% of tests pass across both Debug and Release configurations.
- **Rollback to Bug Squashing Mode**: If a compiler error, linker failure, unit test assertion failure, or uncommitted source edit is detected, immediately halt, revert any partial version edits, and output an actionable bug triage report.
- **Safe Cruft Policy**: Automatically delete known temporary files (`*.tmp`, `temp_*.txt`, `update_*.py`, `*.dump`), but halt if uncommitted edits to tracked C++/JSON files exist.
- **Strict VST3 Destination**: Binaries must deploy strictly to `C:\Program Files\Common Files\VST3\` and `current_build\`. Never use AppData.

---

## Workflow Phases

### Phase 1: Hardened Pre-Flight Cruft Sweep & Workspace Hygiene
1. **Tier 1: Recursive Ephemeral Purge**:
   - Recursively scan the entire repository (not just root) for disposable scratch, update, and planning backup files:
     ```powershell
     Get-ChildItem -Path . -Recurse -File -Include "temp_*.*", "update_*.py", "fix_*.py", "scratch_*.py", "*.dump", "context_snapshot*.md", "PLAN_BACKUP*.md" | Remove-Item -Force
     ```
2. **Tier 2: Pipeline, Inbox & Communiqué Cleanliness**:
   - **Active Plan**: Verify `PLAN.md` is strictly in an idle state (`# No Active Plan` or `# Implementation Plan` with zero active unchecked tasks). If an active plan is in progress, halt release.
   - **Communiqués**: Verify `.agents/pipeline/communique/plan_to_build.md` is marked `COMPLETED` and `build_to_plan.md` is marked `COMPLETE ✅`.
   - **Vault Inbox Zero**: Run a sync pass with `tools/sync_obsidian_vault.ps1` to ensure all completed mobile notes are archived to `TheKlangVault/Inbox/Archive/` and `docs/inbox/` only contains `README.md`.
   - **Backlog Integrity Lockout**: Scan `docs/BACKLOG.md` for the target release milestone. Verify that 100% of items are marked `— ✅ COMPLETED` and zero stale `[`PLAN.md`]` links exist. If any item is uncompleted or points to an unarchived plan, **HALT RELEASE IMMEDIATELY** and report the unreconciled backlog items.
3. **Tier 3: Diagnostic Disk Hygiene**:
   - Clear out stale test failure artifacts so release gauntlet screenshots are 100% fresh:
     ```powershell
     if (Test-Path "test_screenshots") { Get-ChildItem -Path "test_screenshots" -File | Remove-Item -Force }
     ```
4. **Tier 4: Static Audio-Safety & Debug Leak Audit**:
   - Fast static scan across modified C++ source files (`source/`, `test/`):
     - Scan for leftover console output: `std::cout`, `printf`, or unvectorized `DBG(` in audio processing loops.
     - Check timer hygiene: verify any `juce::Timer` subclasses call `stopTimer()` as the first line of their destructors.
5. **Git Workspace Audit**:
   - Execute `git status --porcelain`.
   - If untracked temporary files were removed, verify status is now clean.
   - If there are uncommitted edits to tracked files (`source/`, `assets/`, `CMakeLists.txt`) or untracked source files:
     - **HALT IMMEDIATELY**.
     - Output triage message:
       ```
       ⚠️ RELEASE HALTED: Uncommitted source changes or untracked source files detected.
       Please commit or stash changes before initiating /cut-release.
       ```
     - Enter Bug Squashing Mode.

---

### Phase 2: Dual-Configuration Regression Gauntlet
1. **Debug Build & Assertions Verification**:
   - Reconfigure and build debug test targets:
     ```powershell
     cmake -B build -DCMAKE_BUILD_TYPE=Debug
     cmake --build build --config Debug --target dsp_tests gui_tests
     ```
   - Execute `build/bin/Debug/dsp_tests.exe` and `build/bin/Debug/gui_tests.exe`.
   - If any assertion fails or MSVC debug runtime throws:
     - **HALT IMMEDIATELY**.
     - Extract the exact file and line number of the failure.
     - Output Bug Triage Report in chat and yield turn.
2. **Release Build & 100% Test Pass**:
   - Build all targets in Release mode:
     ```powershell
     cmake --build build --config Release --target TheKlangFarmer_VST3 TheKlangPlanter_VST3 TheKlangEditor dsp_tests gui_tests
     ```
   - Execute `build/bin/Release/dsp_tests.exe` (100% audio thread invariants, SIMD, FastMath).
   - Execute `build/bin/Release/gui_tests.exe` (207+ UI bindings, 6-pillar VST3 parameter normalization, smoke paint passes).
   - If any test fails, halt and output Bug Triage Report.
3. **Headless Host Compliance (`pluginval`)**:
   - If `tools/run_pluginval.ps1` exists, execute:
     ```powershell
     powershell -ExecutionPolicy Bypass -File tools/run_pluginval.ps1 -Strictness 5
     ```
   - If failures occur, report them and halt.

---

### Phase 2.5: Pro High Documentation Polish & Wiki Sync
1. **Model Advisory Verification (Tier 1 Pro High)**:
   - Verify active model is `Gemini 3.1 Pro (Thinking: High)`. If not, render the Model Advisory banner and pause for user model swap.
2. **Release Documentation Audit**:
   - Update `CHANGELOG.md` with structured user-facing release notes.
   - Update `docs/history/DEV_HISTORY.md` with key milestone accomplishments.
   - Archive completed specs in `docs/specs/` to `docs/history/archives/`.
   - Normalize cross-document links across `docs/` (ensure standard Markdown links `[Title](path.md)`).
3. **Obsidian Vault Mirror Sync**:
   - Run `powershell -ExecutionPolicy Bypass -File .\tools\sync_obsidian_vault.ps1`.
   - Verify `TheKlangVault/Docs/` receives clean documentation with zero errors.

---

### Phase 3: Automated Version Bump, Tagging & Deployment
1. **Target Version Determination**:
   - Read current version from `CMakeLists.txt` (`project(TheKlangSuite VERSION X.X.X)`).
   - Determine target version from user argument (e.g. `/cut-release 0.3.1`) or prompt user.
2. **Update Version Strings**:
   - In `CMakeLists.txt`: update `project(TheKlangSuite VERSION <NEW_VERSION>)`.
   - In `source/VersionChecker.h`: verify version constant matches `<NEW_VERSION>`.
3. **Artifact Deployment**:
   - Run `deploy.ps1` to place fresh binaries into `current_build/` and system VST3 directory.
4. **Git Commit & Annotated Tag**:
   ```powershell
   git add CMakeLists.txt source/VersionChecker.h CHANGELOG.md docs/
   git commit -m "chore(release): bump version to v<NEW_VERSION>"
   git tag -a "v<NEW_VERSION>" -m "Release v<NEW_VERSION>"
   ```

---

### Phase 4: Desktop Sanity Gate & GitHub Remote Push
1. **Desktop Verification Pause**:
   - Provide direct links to the fresh artifacts in `current_build\Standalone\The Klang Farmer.exe`, `current_build\Standalone\The Klang Planter.exe`, and `current_build\Editor\The Klang Editor.exe`.
   - Wait for the user to confirm their visual and auditory sanity check.
2. **Interactive Push Prompt via `ask_question`**:
   - Prompt the user to ensure the release is synchronized to GitHub:
     - Question: `"Release v<NEW_VERSION> is tagged and verified locally! Would you like to push the branch and release tag to GitHub?"`
     - Options:
       - `(Recommended) Push Branch & Tag to GitHub (git push origin <branch> --tags)`
       - `Push Release Tag Only (git push origin v<NEW_VERSION>)`
       - `Keep Local Only (Do not push to remote yet)`
3. **Remote Push Execution**:
   - If selected, execute the appropriate `git push` command and confirm remote publication on GitHub.

---

### Phase 5: Clean Slate & Knowledge Harvester
1. **Interactive Prompt via `ask_question`**:
   - Present modal:
     - Question: `"Release v<NEW_VERSION> successfully finalized! Would you like to summon the transcript harvester to archive institutional memory and wipe context for the next milestone?"`
     - Options:
       - `(Recommended) Yes: Harvest all chat memory into docs/history/DEV_HISTORY.md and reset context`
       - `No: Keep active chat transcripts intact`
2. **If Harvest Confirmed**:
   - Archive previous `docs/history/DEV_HISTORY.md` to `docs/history/archives/DEV_HISTORY_v<OLD_VERSION>.md`.
   - Append distilled summary of this release to `docs/history/DEV_HISTORY.md`.
   - Overwrite `context_clues_build.md` and `context_clues_plan.md` with clean slate message pointing to the next milestone in `docs/BACKLOG.md`.
   - Notify user that context is wiped and ready for a fresh start!

