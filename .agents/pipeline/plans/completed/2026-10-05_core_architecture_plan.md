# Core Architecture Refactor (Base Classes)

## Goal Description
Refactor the codebase to use a "Common Core" base-class architecture. Currently, `TheKlangFarmer` and `TheKlangPlanter` (and eventually the hardware plugin) duplicate standard JUCE boilerplate, UI LookAndFeel, preset management, and APVTS loading/saving. 

By extracting this shared logic into `KlangCoreProcessor` and `KlangCoreEditor`, we achieve DRY (Don't Repeat Yourself) code. Any new feature (like the JSON Preset Browser or Data-Driven UI Layouts) only needs to be written once in the Core class, and all derivative plugins (Farmer, Planter, Hardware) inherit it automatically.

## Proposed Changes

### 1. The Core DSP Processor
Extract APVTS, JSON Preset Management, and basic MIDI/Audio bus setup into a shared base class.

#### [NEW] `source/KlangCoreProcessor.h` & `.cpp`
```cpp
class KlangCoreProcessor : public juce::AudioProcessor {
public:
    KlangCoreProcessor(const BusesProperties& ioLayouts);
    
    // Shared State Management (Written once, used by all)
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // The APVTS (Shared across all derivatives)
    juce::AudioProcessorValueTreeState apvts;
    
    // Pure virtual function forces derived classes to define their own parameters
    virtual juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() = 0;
};
```

#### [MODIFY] `source/PluginProcessor.h` & `source/PlanterProcessor.h`
Change inheritance from `juce::AudioProcessor` to `KlangCoreProcessor`. Strip out the duplicated `getStateInformation` and `setStateInformation` methods, routing them to the base class.

### 2. The Core UI Editor
Extract standard UI elements (Header bar, Preset LCD, Save buttons, LookAndFeel theme application, and Layout JSON parsing) into a shared base editor.

#### [NEW] `source/KlangCoreEditor.h` & `.cpp`
```cpp
class KlangCoreEditor : public juce::AudioProcessorEditor {
public:
    KlangCoreEditor(KlangCoreProcessor& p);
    
    // Shared Header UI components
    PresetBrowserComponent presetBrowser;
    juce::TextButton saveButton;
    juce::Label presetLcd;

    // Shared layout parser
    void loadJsonLayout(const juce::String& layoutId);
};
```

#### [MODIFY] `source/PluginEditor.h` & `source/PlanterEditor.h`
Change inheritance from `juce::AudioProcessorEditor` to `KlangCoreEditor`. Remove duplicated header UI code and LookAndFeel initialization. Focus these specific editors *only* on rendering their specific modules (e.g., the 8-card grid for Farmer, the 4-card grid for Planter).

---

## Verification Plan

### Automated Tests
1. **Compilation Check**: Run `/build-validate` to ensure C++ inheritance is correctly wired up and there are no ambiguous function calls.
2. **State Verification**: Run `pluginval` to ensure the base class correctly handles APVTS state saving and loading for both plugins.

### Manual Verification
1. Open both `TheKlangFarmer` and `TheKlangPlanter` standalone wrappers.
2. Verify that the shared Header Bar (Presets, Save buttons) appears identically in both.
3. Save a state in Farmer, load it back, and verify the base class `setStateInformation` successfully restored the APVTS.
