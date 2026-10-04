---
name: context-extract
description: Extracts a lightweight Markdown snapshot of target files, schemas, directory layout, spec doc, and Linear issue context for external LLM planning. Automatically handles feature discovery, compiles context_snapshot.md, manages browser launching and clipboard copying, and reminds the user to select the Pro model in Gemini Web. Explicitly instructs Gemini Web not to invoke Linear.app extensions. Supports flags --clipboard (-c) and --open (-o). Triggers on `/contextextract`.
---

# Context Extractor for External Planning

## Goal
Assemble a clean, condensed snapshot of relevant code, interfaces, directory layout, primary project spec, and Linear issue details into `context_snapshot.md`, explicitly instruct external LLMs to avoid connecting Linear extensions, then manage clipboard copying and launching the external planning workspace.

## Operational Constraints
- **Strictly No Implementation:** Do not propose architectures or write feature code. Act strictly as an extraction compiler.
- **Single Process:** Do not launch parallel sub-agents or background test runs.
- **Token-Efficient Discovery:** Never perform unbounded whole-repo reads. Use fast indexing (`rg`, `find`, glob) with hard match caps.
- **Suppress External Linear Extensions:** Explicitly forbid external LLMs from attempting to connect to or query Linear.app, as all Linear actions are strictly managed by Antigravity in the local IDE.

## Workflow

### 1. Flag Detection & Command Parsing
Inspect the invocation:
- Flag `--clipboard` or `-c`: Auto-copy markdown to OS clipboard.
- Flag `--open` or `-o`: Auto-launch `https://gemini.google.com` in default browser.
- Detect any Linear issue key pattern (`[A-Z]+-[0-9]+`, e.g. `THE-9`). If found:
  1. Call Linear MCP or `linear_sync.py` to fetch the title, description, and status.
  2. Reserve this context to prepend in Section 1 of the snapshot under `## 0. Linear Task Context`.
  3. Strip the issue ID and flags from the remaining search query.

### 2. Locate Project Spec
Search root and `docs/` for `*SPEC*.md`, `*ARCHITECTURE*.md`, `*DESIGN*.md`, or `README.md`. Read and reserve this document.

### 3. Scope Determination & Discovery
- **Case A: Explicit Paths Provided:** Use specified files.
- **Case B: Feature or Concept Described:**
  1. Identify 2–4 search keywords.
  2. Use file and content search (`rg`) to find candidates.
  3. Select the top 3 to 7 most relevant files (prioritize headers, parameter layouts, interfaces, DSP blocks).
- **Case C: Blank Trigger:** Prompt the user for target files, feature description, or Linear issue ID.

### 4. Focused Directory Map
Run a shallow file tree (depth 2–3) restricted to the primary source tree (`Source/`, `src/`, `include/`), excluding `.git`, `build`, `cmake-build-*`, `JuceLibraryCode`, `node_modules`, and binary assets.

### 5. Read Target Files
Read the identified files, stripping boilerplate license headers to preserve tokens.

### 6. Write `context_snapshot.md`
Create or overwrite `context_snapshot.md` at project root:

# Planning Context Snapshot
**Timestamp:** <YYYY-MM-DD HH:MM>  
**Linear Issue:** <Issue Key & Title if present, else None>

> [!IMPORTANT]
> **PLANNING INSTRUCTIONS FOR GEMINI:**
> - **DO NOT connect to or use Linear.app tools or extensions.**
> - All Linear issue tracking, comments, and status updates are managed automatically by Antigravity in the local IDE.
> - The task context below is 100% self-contained. Focus solely on producing a clean, phased implementation plan for Antigravity's `/pasteplan` skill.

## 0. Linear Task Context
<Linear issue description and acceptance criteria if fetched>

## 1. Project Specification & Architecture
*(Source: `<path/to/SPEC.md>`)*
```markdown
<Contents document of primary spec>
```

## 2. Directory Tree
```text
<Shallow source tree>
```

## 3. Core Schemas, Structs & Interfaces
```cpp
<Extracted enums, layout parameter structs, typedefs>
```

## 4. Source Files
### `<path/to/file.ext>`
```<language>
<File content>
```

### 7. Handoff & Action Execution

Helper commands:
- **Clipboard:**
  - Windows: `powershell -Command "Get-Content context_snapshot.md -Raw | Set-Clipboard"`
  - macOS: `pbcopy < context_snapshot.md`
  - Linux: `xclip -selection clipboard < context_snapshot.md` (or `wl-copy < context_snapshot.md`)
- **Browser:**
  - Windows: `cmd /c start https://gemini.google.com`
  - macOS: `open https://gemini.google.com`
  - Linux: `xdg-open https://gemini.google.com`

Always display this visual banner:

# ⚠️ **REMINDER: VERIFY / SELECT THE PRO MODEL IN GEMINI WEB** ⚠️

- If `--clipboard` and `--open` were supplied: Execute both, display banner, confirm.
- If only `--clipboard`: Execute copy, display banner, confirm.
- If only `--open`: Launch browser, display banner, confirm.
- If neither flag was supplied: Display banner and prompt user for clipboard/browser actions.
