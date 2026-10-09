STATUS: IDLE

# Subagent Dispatch Charter
> *Target Pipeline:* Subagent Delegation Protocol (The 20-Line & Heavy Task Gate)  
> *Active Branch:* `0.4.0-dev`  
> *Orchestrator:* Antigravity Main Chat (New Klang City / Klang Industries)

---

## 1. Assignment Header
- **STATUS**: `IDLE` <!-- IDLE | READY_FOR_SUBAGENT | IN_PROGRESS | COMPLETED -->
- **ROLE**: `research` <!-- research (Pro) | explore (Flash) | contrarian (Flash) | self (Subagent) -->
- **MODEL TIER**: `Flash (Default)` <!-- Flash (Thinking: High) | Pro (Thinking: High) -->
- **TASK TYPE**: `RESEARCH` <!-- CODE_IMPLEMENTATION | CONTRARIAN_AUDIT | RESEARCH | QA_VERIFICATION -->
- **TASK SLUG**: `idle`
- **TARGET REPOSITORIES**:
  - [ ] `TheKlangSuite` (`c:\Dev\TheKlangSuite`)
  - [ ] `ToadTracker` (`c:\Dev\ToadTracker`)
  - [ ] `nkai` (`c:\Dev\nkai`)
  - [ ] `TheKlangResearch` (`C:\Dev\Research`)

---

## 2. Scope & Target Files
- **Primary Objective**: 
- **Target Files / Boundaries**:
  - 

---

## 3. Mandatory Invariants & Constraints
- **Audio Thread Safety**: Zero allocations (`new`, `malloc`, `vector::push_back`), zero locks (`std::mutex`), zero blocking I/O on audio paths.
- **Context Protection**: Never output monolithic code blocks back to the main chat.
- **Boy Scout Cleanliness**: Zero AI slop, clean formatting, ruthlessly eliminate dead code.
- **Sidecar Asymmetry**: Rich markdown, HTML tables, and option cards belong in sidecar/artifacts, not main chat.

---

## 4. Mandatory Deliverables
- [ ] Direct file modifications (if write-authorized).
- [ ] Execution verification (tests / syntax validation / linter pass).
- [ ] Completion receipt logged to `.agents/pipeline/communique/subagent_receipt.md`.

---

## 5. Contrarian 80/20 Gate (Mandatory for Contrarian Role)
When executing in `contrarian` mode, the subagent MUST strictly address:
1. **Top 3 Failure Modes**: What will break under edge cases, scale, or user error?
2. **Real-World Precedent / Disaster**: Where has this failed historically in production?
3. **Cognitive & Token Maintenance Debt**: What is the ongoing cost in complexity and LLM allowance?
4. **The Pragmatic 80/20 Middle Ground**: What extracts 80% of the value with 20% of the complexity? (Eliminate pure obstructionism; derive actionable consensus).
