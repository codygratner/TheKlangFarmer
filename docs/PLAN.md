# Cruft Purge Execution Plan

## Phase 1: Identify & Remove Dead APVTS Parameters (C++)
The transition to dynamic Multi-Instance FX slots (`pre_fx_1` to `4` and `post_fx_1` to `4`) fully replaced the original standalone hardcoded FX modules. However, their APVTS parameter bindings and layout definitions were never removed from the C++ processor. 
**Target Files:** `source/FarmerProcessor.h`, `source/FarmerProcessor.cpp`
- **Actions:**
  - Delete `juce::AudioParameter*` declarations for 9 dead FX modules: Drive, FX Filter, Wave Folder, RingMod, Frequency Shifter, Grit FX, Comb Filter, Phase Smear, EQ.
  - Delete their `dynamic_cast<juce::AudioParameter*>` initialization lines in the constructor.
  - Delete their `layout.add(...)` definitions (e.g. `"wavefolder_fold"`, `"ringmod_shape"`).
  - Delete their block mapping lines in `applyBaseParameters()` (e.g., `engine.setPageParameter(BLK_WAVEFOLDER, ...)`).

## Phase 2: Purge Dead DSP Engine Routing (C++)
The standalone DSP blocks corresponding to the dead parameters are instantiated but bypassed; the audio engine exclusively routes audio through the dynamic `preFXBlocks[s]` and `postFXBlocks[s]` arrays instead.
**Target Files:** `source/ModularBlocks.h`
- **Actions:**
  - Remove dead block indices from the `BlockID` enum (`BLK_DRIVE`, `BLK_FXFILTER`, `BLK_WAVEFOLDER`, `BLK_RINGMOD`, `BLK_FREQSHIFT`, `BLK_GRIT`, `BLK_COMB`, `BLK_PHASE_SMEAR`, `BLK_EQ`).
  - Delete their instantiation inside the `ModularDrumEngine` constructor (`allBlocks[BLK_WAVEFOLDER] = std::make_unique<WaveFolderBlock>();`, etc.).
  - Delete default parameter settings for these blocks (`setPageParameter(BLK_WAVEFOLDER, ...)`, etc.).
  - Delete the entirely unused `void processFX(int fxType, ...)` method which previously facilitated processing these static blocks.
  - Verify that `processStereo` is completely clean of references to these blocks.

## Phase 3: Validate Safe Compilation
- **Actions:**
  - Run the `build-validate` skill to compile the project.
  - Run `audiothread-guard` to ensure no safety violations.
  - Assert that all automated tests (DSP and GUI) still pass, proving the deleted code was structurally isolated and effectively orphaned.

## Definition of Done
- The C++ codebase contains zero explicit APVTS definitions or DSP routings for the 9 legacy standalone FX blocks.
- The project successfully builds, deploys, and passes all validation steps without missing symbols or segfaults.
