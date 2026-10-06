# Execution Plan: The Klang Editor Updates & Linting

## Phase 1: Automated Build Tracking
**Goal:** Display the CMake Git Commit Count and Hash in the Editor's title bar (e.g., 0.3.0 (Build 171 - 155333f)).
- **Step 1.1: CMake Integration**
  - Add a custom command to CMake to grab the current git rev-parse --short HEAD and git rev-list --count HEAD.
- **Step 1.2: Title Bar Integration**
  - Pass the git hash and commit count to the JUCE preprocessor via CMake 	arget_compile_definitions.
  - Update 	ools/editor/Main.cpp's MainWindow to display the formatted string.

## Phase 2: Consolidated JSON Snapshot & Factory Restore
**Goal:** Add Export, Import, and Factory Restore functionality to The Klang Editor.
- **Step 2.1: Snapshot UI Buttons**
  - Add Export Snapshot, Import Snapshot, and Restore Factory Defaults buttons to the MainComponent header (next to Save/Reload).
- **Step 2.2: Export Snapshot Logic**
  - Implement logic to bundle all ssets/controls/*.json and ssets/layouts/*.json files into a single timestamped .json archive.
- **Step 2.3: Import Snapshot Logic**
  - Implement juce::FileChooser to load an external snapshot JSON and overwrite the local ssets/ files.
- **Step 2.4: Factory Restore Logic**
  - Implement logic to revert all on-disk JSONs to a clean ssets/factory_defaults_snapshot.json if a setting is borked.

## Phase 3: Automated C++ Linting & Formatting
**Goal:** Enforce the project's A+ code quality standards automatically.
- **Step 3.1: Create .clang-format**
  - Generate a .clang-format file matching the existing 4-space indent and camelCase style rules.
- **Step 3.2: Build Process Integration**
  - Add a custom target in CMake to run clang-format on all source files.

## Verification Plan
- Use step-verify between phases to ensure code compiles and tests pass.
- Run uild-validate before completing the task.
