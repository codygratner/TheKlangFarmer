# [THE-6] Fix FX Catalog Alphabetical Ordering (Phase Smear Slot 4 Fix)
**Linear URL:** https://linear.app/the-klang-farmer/issue/THE-6/fix-fx-catalog-alphabetical-ordering-phase-smear-slot-4-fix
**Source:** Comment

---

# Implementation Plan & Technical Audit: Fix FX Catalog Alphabetical Ordering (`THE-6`)

**Repository:** `https://github.com/codygratner/TheKlangFarmer.git`
**Base Commit:** `master` (`d04353c`) — JUCE `9.0.3` (`be29c81`)
**Target Branch:** `codygratner/the-6-fix-fx-catalog-alphabetical-ordering-phase-smear-slot-4-fix`
**Linear Issue:** `THE-6`

---

## 1. Executive Summary & Canonical Index Mapping

When **PhaseSmear** was renamed to **Phase Smear**, its catalog position remained at index `4` (between *Comb Filter* and *Drive*). Re-ordering the 13-processor FX catalog alphabetically places **Phase Smear** at index `9` (immediately before **Phaser** at index `10`, since ASCII space `0x20` precedes `'r'` `0x72`).

Because `Phaser` (`10`), `RingMod` (`11`), `Tempo Delay` (`12`), and `Wave Folder` (`13`) are already in alphabetical order after `Phase Smear`, **only indices `4` through `9` change** (`0..3` and `10..13` are invariant):

| Index (`type`) | Previous FX (`d04353c`) | New Alphabetical FX (`THE-6`) | Delta |
| :---: | :--- | :--- | :--- |
| **0** | `None` (`nullptr`) | `None` (`nullptr`) | `0` (Invariant) |
| **1** | `Bell EQ` (`EQBlock`) | `Bell EQ` (`EQBlock`) | `0` (Invariant) |
| **2** | `Chorus` (`ChorusBlock`) | `Chorus` (`ChorusBlock`) | `0` (Invariant) |
| **3** | `Comb Filter` (`CombFilterBlock`) | `Comb Filter` (`CombFilterBlock`) | `0` (Invariant) |
| **4** | `Phase Smear` (`PhaseSmearBlock`) | **`Drive`** (`DriveBlock`) | Was `5 -> 4` (`-1`) |
| **5** | `Drive` (`DriveBlock`) | **`Filter`** (`FilterBlock(0)`) | Was `6 -> 5` (`-1`) |
| **6** | `Filter` (`FilterBlock(0)`) | **`Flanger`** (`FlangerBlock`) | Was `7 -> 6` (`-1`) |
| **7** | `Flanger` (`FlangerBlock`) | **`Frequency Shifter`** (`FrequencyShifterBlock`) | Was `8 -> 7` (`-1`) |
| **8** | `Frequency Shifter` (`FrequencyShifterBlock`) | **`Grit FX`** (`GritBlock`) | Was `9 -> 8` (`-1`) |
| **9** | `Grit FX` (`GritBlock`) | **`Phase Smear`** (`PhaseSmearBlock`) | **Was `4 -> 9` (`+5`)** |
| **10** | `Phaser` (`PhaserBlock`) | `Phaser` (`PhaserBlock`) | `0` (Invariant) |
| **11** | `RingMod` (`RingModBlock`) | `RingMod` (`RingModBlock`) | `0` (Invariant) |
| **12** | `Tempo Delay` (`DelayBlock`) | `Tempo Delay` (`DelayBlock`) | `0` (Invariant) |
| **13** | `Wave Folder` (`WaveFolderBlock`) | `Wave Folder` (`WaveFolderBlock`) | `0` (Invariant) |

### Default Patch Pre/Post FX Slot Remapping
To keep the factory default patch audio output 100% identical:

| Slot | Processor | Old Index | New Index |
| :--- | :--- | :---: | :---: |
| **Pre FX 1** | Drive | `5` | **`4`** |
| **Pre FX 2** | Wave Folder | `13` | `13` (unchanged) |
| **Pre FX 3** | RingMod | `11` | `11` (unchanged) |
| **Pre FX 4** | Frequency Shifter | `8` | **`7`** |
| **Post FX 1** | Grit FX | `9` | **`8`** |
| **Post FX 2** | Comb Filter | `3` | `3` (unchanged) |
| **Post FX 3** | Phase Smear | `4` | **`9`** |
| **Post FX 4** | Bell EQ | `1` | `1` (unchanged) |

---

## 2. Step-by-Step Implementation Plan

### Phase 1: DSP Factory & Engine Defaults (`source/ModularBlocks.h`) — [x] COMPLETED

- [x] **Step 1.1: Re-order `createFXBlock(int type)` (lines ~2412–2429)**
Update `switch (type)` cases `4` through `9`:

```cpp
static std::unique_ptr<DSPBlock> createFXBlock(int type) {
    switch (type) {
        case 1:  return std::make_unique<EQBlock>();               // Bell EQ
        case 2:  return std::make_unique<ChorusBlock>();           // Chorus
        case 3:  return std::make_unique<CombFilterBlock>();       // Comb Filter
        case 4:  return std::make_unique<DriveBlock>();            // Drive
        case 5:  return std::make_unique<FilterBlock>(0);          // Filter
        case 6:  return std::make_unique<FlangerBlock>();          // Flanger
        case 7:  return std::make_unique<FrequencyShifterBlock>(); // Frequency Shifter
        case 8:  return std::make_unique<GritBlock>();             // Grit FX
        case 9:  return std::make_unique<PhaseSmearBlock>();        // Phase Smear
        case 10: return std::make_unique<PhaserBlock>();           // Phaser
        case 11: return std::make_unique<RingModBlock>();          // RingMod
        case 12: return std::make_unique<DelayBlock>();            // Tempo Delay
        case 13: return std::make_unique<WaveFolderBlock>();       // Wave Folder
        default: return nullptr;
    }
}
```

- [x] **Step 1.2: Update `ModularDrumEngine` Default FX Type Arrays (lines ~2680–2695)**
Update both `preFXTypes` and `postFXTypes` initialization defaults in `ModularDrumEngine` so standalone engine instances (including `dsp_tests`) match the parameter layout defaults:

```cpp
// Pre FX default types: Drive (4), Wave Folder (13), RingMod (11), Frequency Shifter (7)
preFXTypes[0] = 4;
preFXTypes[1] = 13;
preFXTypes[2] = 11;
preFXTypes[3] = 7;

// Post FX default types: Grit FX (8), Comb Filter (3), Phase Smear (9), Bell EQ (1)
postFXTypes[0] = 8;
postFXTypes[1] = 3;
postFXTypes[2] = 9;
postFXTypes[3] = 1;
```

*(Also audit `ModularBlocks.h` for any conditional checks comparing `preFXTypes[i]` or `postFXTypes[i]` against hardcoded integer literals `4..9`.)*

---

### Phase 2: Processor Parameter Layout, Presets & State Migration (`source/PluginProcessor.cpp`) — [x] COMPLETED

- [x] **Step 2.1: Update `fxChoices` & Default Slot Indices in `createParameterLayout()` (lines ~977–1010)**
```cpp
const juce::StringArray fxChoices {
    "None",              // 0
    "Bell EQ",           // 1
    "Chorus",            // 2
    "Comb Filter",       // 3
    "Drive",             // 4
    "Filter",            // 5
    "Flanger",           // 6
    "Frequency Shifter", // 7
    "Grit FX",           // 8
    "Phase Smear",       // 9
    "Phaser",            // 10
    "RingMod",           // 11
    "Tempo Delay",       // 12
    "Wave Folder"        // 13
};
```
Update default indices passed to `juce::AudioParameterChoice` for the 8 Pre/Post FX slots:
- Pre FX 1 Type: `5` $\to$ `4` (Drive)
- Pre FX 2 Type: `13` $\to$ `13` (Wave Folder)
- Pre FX 3 Type: `11` $\to$ `11` (RingMod)
- Pre FX 4 Type: `8` $\to$ `7` (Frequency Shifter)
- Post FX 1 Type: `9` $\to$ `8` (Grit FX)
- Post FX 2 Type: `3` $\to$ `3` (Comb Filter)
- Post FX 3 Type: `4` $\to$ `9` (Phase Smear)
- Post FX 4 Type: `1` $\to$ `1` (Bell EQ)

- [x] **Step 2.2: Audit Any Built-In Presets or Reset Helpers in `source/PluginProcessor.cpp`**
Audited: No raw FX indices used in factory preset tables; all use dynamic choice indices.

- [x] **Step 2.3: Backward-Compatible DAW State Migration (`getStateInformation` / `setStateInformation`)**
To ensure existing DAW sessions saved under the legacy ordering (`v0.1.5` / `v0.1.6` without `fxCatalogVersion`) load with the exact same FX processors while `pluginval` round-trip state tests remain 100% deterministic:
1. In `getStateInformation()`, stamp `xml->setAttribute("fxCatalogVersion", 2);` before serializing to binary.
2. In `setStateInformation()`, if `xmlState->getIntAttribute("fxCatalogVersion", 1) < 2`, iterate `<PARAM>` child elements matching the 8 FX slot type parameter IDs and remap legacy values `4..9`:

```cpp
static int migrateLegacyFXTypeIndex(int oldIndex) noexcept {
    if (oldIndex == 4) return 9; // Phase Smear: 4 -> 9
    if (oldIndex >= 5 && oldIndex <= 9) return oldIndex - 1; // Drive..Grit FX: 5..9 -> 4..8
    return oldIndex;
}
```
After migrating, set `xmlState->setAttribute("fxCatalogVersion", 2);` prior to `apvts.replaceState(...)`.

---

### Phase 3: Editor ComboBox List & UI Switch Statements (`source/PluginEditor.cpp`) — [x] COMPLETED

- [x] **Step 3.1: Update `fxChoices` `StringArray` in `PluginEditor.cpp` (lines ~1689–1705)**
Match the exact alphabetical `fxChoices` list from `PluginProcessor.cpp`.

- [x] **Step 3.2: Update `getFXParamTitle` & UI Dynamic Control / Switch Blocks in `PluginEditor.cpp`**
Updated every `switch (fxType)` and `currentType` dynamic control check in `source/PluginEditor.cpp`:
- `case 4:` $\to$ **Drive** (previously `case 5`)
- `case 5:` $\to$ **Filter** (previously `case 6`)
- `case 6:` $\to$ **Flanger** (previously `case 7`)
- `case 7:` $\to$ **Frequency Shifter** (previously `case 8`)
- `case 8:` $\to$ **Grit FX** (previously `case 9`)
- `case 9:` $\to$ **Phase Smear** (previously `case 4`)
- Real-time oscilloscope filter/EQ XY plot mode checks updated (`fxType == 5` for FilterXY, `fxType == 1` for EqXY).

---

### Phase 4: Unit Tests & Documentation (`test/dsp_tests.cpp`, `spec.md`, `future_backlog_and_reminders.md`) — [x] COMPLETED

- [x] **`test/dsp_tests.cpp`**:
   - Updated existing `setPreFXType` / `setPostFXType` calls referencing indices `4..9` (Drive 4, Grit FX 8, Flanger 6).
   - Added test 29 verifying `createFXBlock(1..13)` instantiates the exact alphabetical `DSPBlock` subclasses (`EQBlock`, `ChorusBlock`, `CombFilterBlock`, `DriveBlock`, `FilterBlock`, `FlangerBlock`, `FrequencyShifterBlock`, `GritBlock`, `PhaseSmearBlock`, `PhaserBlock`, `RingModBlock`, `DelayBlock`, `WaveFolderBlock`).
   - Verified default FX slot assignments on `ModularDrumEngine` initialization match alphabetical ordering.
- [x] **`spec.md` & `future_backlog_and_reminders.md`**:
   - Updated the FX catalog table and default slot index documentation in `spec.md`.
   - Updated backlog status for `THE-6`.
- [x] **Verification**:
   - Build and run `dsp_tests` to confirm all unit tests pass.
   - Stage all phase changes in git.
