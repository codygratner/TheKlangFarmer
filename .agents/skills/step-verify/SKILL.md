---
name: step-verify
description: Enforces incremental engineering discipline between phases of PLAN.md. Audits code changes against active phase acceptance criteria, updates checklist checkmarks in PLAN.md, stages phase modifications in git, and pauses for user review before advancing. Triggers on `/stepverify`, `/step-verify`, "verify step", "verify phase", "next step", "progress check", or "audit phase".
---

# Step Verifier & Milestone Progression Check

## Goal
Audit changes made during the active phase of an implementation plan against its documented acceptance criteria in `PLAN.md`. Upon successful verification, update `PLAN.md` checkmarks, stage git changes for the completed milestone, and present a concise summary before proceeding to the subsequent phase.

## Operational Constraints
- **Do Not Skip Phases:** Phases must be executed and verified sequentially unless explicitly directed by the user.
- **Strict Verification Before Checkoff:** Never check off `[x]` in `PLAN.md` without verifying that the corresponding code changes and tests exist on disk.
- **Stage Progress Incrementally:** Stage changes milestone-by-milestone to maintain clean git diff boundaries.

## Workflow

### 1. Identify Active Phase in `PLAN.md`
1. Open and parse `PLAN.md` at the project root.
2. Locate the first phase containing unchecked boxes (`- [ ]`).
3. Extract:
   - Phase Title and Scope.
   - List of acceptance criteria and deliverables.
   - Target files and test requirements for this phase.

### 2. Audit Code Changes Against Phase Criteria
1. Inspect git modifications:
   ```powershell
   git status --short
   git diff --name-only
   ```
2. For each deliverable in the active phase:
   - Check if the targeted files contain the required implementation (classes, functions, macros, or configurations).
   - If audio code was touched, check for audio thread safety (or invoke `/audiothread-guard`).
   - If unit tests were specified, check that test cases exist in `test/dsp_tests.cpp` or equivalent test runners.

### 3. Run Targeted Phase Tests (When Build is Authorized)
If compilation/testing has been authorized by the user for this step:
- Run targeted tests for the phase (e.g. `dsp_tests` or specific test cases).
- If tests fail, halt progression and report the failure details. Do not mark the phase as complete.

### 4. Update `PLAN.md` on Disk
1. Use `replace_file_content` to update the active phase checkboxes from `- [ ]` to `- [x]`.
2. Ensure no unrelated sections of `PLAN.md` are altered.

### 5. Stage Git Changes for Completed Milestone
Stage the files modified during this phase:
```powershell
git add <file1> <file2> ...
```
*Note: Do not commit yet unless requested; staging establishes clean milestone checkpoints.*

### 6. Present Milestone Verification Card
Output a structured progression report to the user:

```markdown
### 🏁 Milestone Verified: Phase <N> — <Phase Name>

**Deliverables Checked:**
- [x] <Deliverable 1 summary>
- [x] <Deliverable 2 summary>

**Verification Findings:**
- **Code Audit:** Verified required symbols and data structures in [`source/FileName.cpp`](file:///path/to/source/FileName.cpp).
- **Tests:** <Test execution status or test case addition confirmation>.
- **Git Status:** Staged <N> modified files for Phase <N>.

---

#### ⏭️ Next Up: Phase <N+1> — <Next Phase Name>
- **Objective:** <Brief summary of what Phase N+1 covers>.
- **Target Files:** [`source/NextFile.h`](file:///path/to/source/NextFile.h)

*Ready to proceed to Phase <N+1>? Say "proceed" or provide any adjustments.*
```

### 7. Tier Transition Advisory Protocol
Before advancing to Phase <N+1>, evaluate whether Phase <N+1> requires a different complexity tier than Phase <N>:
- **Same Tier**: Do NOT display the Model Advisory banner. Simply pause for user to reply "proceed".
- **Tier UPGRADE (e.g. Tier 3 ➔ 2, Tier 2 ➔ 1, Tier 3 ➔ 1)**:
  - Display the Model Advisory banner at the very end.
  - **Hard Pause**: Wait for the user to explicitly reply "proceed" before executing Phase N+1.
- **Tier DOWNGRADE by 1 Level (e.g. Tier 1 ➔ 2, or Tier 2 ➔ 3)**:
  - Display the Model Advisory banner recommending the downgrade to save quota.
  - **5-Minute Grace Timer**: Schedule a 5-minute timer (`schedule(DurationSeconds=300, Prompt="5-minute downgrade timer expired: proceeding to Phase <N+1> on existing higher tier.")`) and pause in chat text.
  - If user replies "proceed" (or switches model), proceed on the lower tier.
  - If the timer fires without user input, auto-proceed using the existing (one-level higher) tier.
- **Tier DOWNGRADE by 2 Levels (Tier 1 ➔ Tier 3)**:
  - Display the Model Advisory banner.
  - **Hard Pause Without Timer**: Stop and wait indefinitely for the user to reply "proceed" (never auto-proceed or burn expensive Tier 1 Pro quota on trivial tasks).
