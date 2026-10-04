# Implementation Plan: Extract Tooltips & Text into JSON

This plan outlines the refactoring strategy to extract all hardcoded UI text, tooltips, and Quickstart Guide copywriting into a centralized JSON dictionary. The JSON file will be embedded directly into the VST3 executable using CMake's `juce_add_binary_data` and served at runtime by a highly optimized `StringManager` singleton.

## Phase 1: Asset Creation & JSON Structure
Create a new directory at the project root `assets/` and generate the single source of truth for copywriting.

### [NEW] `assets/en_strings.json`
```json
{
  "quickstart": {
    "title": "THE KLANG FARMER — QUICKSTART GUIDE",
    "subtitle": "Paged Modular Dual FM Drum Voice with 13 Multi-Instance Effects",
    "panels": [
      {
        "title": "1. ARCHITECTURE & SIGNAL FLOW",
        "bullets": [
          "• DUAL FM VOICES: Two parallel voices each with Carrier (MIDI/Freq/Note)...",
          "• TRANSIENT NOISE: Analog-modeled White/Pink/Metallic noise source..."
        ]
      }
    ]
  },
  "tooltips": {
    "navigation_pages": [
      "PAGE 1: VOICE 1 — Carrier 1, Modulator 1, Pitch Envelope 1, Filter 1...",
      "PAGE 2: VOICE 2 — Carrier 2, Modulator 2, Pitch Envelope 2, Filter 2..."
    ],
    "fx_algorithms": {
      "0": "BYPASS: FX processing bypassed, audio passes through clean.",
      "1": "BELL EQ: Parametric peaking/notching equalizer with variable frequency..."
    },
    "fx_knobs": {
      "1": [
        {"title": "Center Frequency", "desc": "Peak/notch filter frequency (20 Hz - 24 kHz)", "default": "1.00 kHz", "isBipolar": false},
        {"title": "Width", "desc": "Bandwidth / Q factor in octaves (0.1 to 10.0 oct)", "default": "1.00 oct", "isBipolar": false}
      ]
    },
    "led_selectors": {
      "target": [
        "CAR: Routes pitch envelope modulation strictly to Carrier frequency.",
        "MOD: Routes pitch envelope modulation strictly to Modulator frequency..."
      ]
    },
    "params": {
      "carrier1_pitch": {"desc": "Carrier 1 pitch transpose / frequency offset", "isBipolar": false},
      "carrier1_depth": {"desc": "Frequency modulation depth from Modulator 1", "isBipolar": true}
    }
  }
}
```

## Phase 2: CMake Binary Data Generation
Configure CMake to bake the JSON file into the binary block `TkfAssets`.

### [MODIFY] `CMakeLists.txt`
- **Define BinaryData Target**: Insert below `add_subdirectory(JUCE)`:
  ```cmake
  # Add a BinaryData target for our assets
  juce_add_binary_data(TkfAssets
      HEADER_NAME "BinaryData.h"
      NAMESPACE BinaryData
      SOURCES
          assets/en_strings.json
  )
  ```
- **Add Source Files**: In the `target_sources` block for both `TheKlangFarmer` and `TheKlangPlanter`, add:
  ```cmake
      source/StringManager.h
      source/StringManager.cpp
  ```
- **Link Libraries**: In the `target_link_libraries` block for `TheKlangFarmer` and `TheKlangPlanter`, add `TkfAssets` right after `${TKF_JUCE_MODULES}`.
- **Test Binaries**: Add `StringManager` source files to `dsp_tests`'s `add_executable` block and link `TkfAssets` to it.

## Phase 3: Runtime String Manager
Create the C++ wrapper that automatically parses the JSON on initialization and queries strings cleanly using a dot-delimited path structure.

### [NEW] `source/StringManager.h`
```cpp
#pragma once
#include <juce_core/juce_core.h>

class StringManager {
public:
    static juce::String getString(const juce::String& keyPath);
    static juce::StringArray getStringArray(const juce::String& keyPath);
    
    // Extracted property structure for UI knobs
    struct FxKnob {
        juce::String title;
        juce::String desc;
        juce::String defaultVal;
        bool isBipolar = false;
    };
    static FxKnob getFxKnob(int fxIndex, int knobIndex);

    // Dynamic property fetcher for Parameters
    static juce::var getVar(const juce::String& keyPath);

private:
    static juce::var dict;
    static void initializeIfNeeded();
};
```

### [NEW] `source/StringManager.cpp`
```cpp
#include "StringManager.h"

namespace BinaryData {
    extern const char*   en_strings_json;
    extern const int     en_strings_jsonSize;
}

juce::var StringManager::dict;

void StringManager::initializeIfNeeded() {
    if (dict.isVoid()) {
        auto jsonStr = juce::String::createStringFromData(BinaryData::en_strings_json, BinaryData::en_strings_jsonSize);
        dict = juce::JSON::parse(jsonStr);
    }
}

juce::var StringManager::getVar(const juce::String& keyPath) {
    initializeIfNeeded();
    juce::var current = dict;
    auto keys = juce::StringArray::fromTokens(keyPath, ".", "");
    for (const auto& key : keys) {
        if (current.isObject() && current.getDynamicObject() != nullptr) {
            current = current.getProperty(juce::Identifier(key), juce::var());
        } else if (current.isArray() && current.getArray() != nullptr) {
            current = current[key.getIntValue()];
        } else {
            return juce::var();
        }
    }
    return current;
}

juce::String StringManager::getString(const juce::String& keyPath) {
    return getVar(keyPath).toString();
}

juce::StringArray StringManager::getStringArray(const juce::String& keyPath) {
    juce::StringArray result;
    if (auto* arr = getVar(keyPath).getArray()) {
        for (auto& item : *arr) {
            result.add(item.toString());
        }
    }
    return result;
}

StringManager::FxKnob StringManager::getFxKnob(int fxIndex, int knobIndex) {
    FxKnob knob;
    auto fxObj = getVar("tooltips.fx_knobs." + juce::String(fxIndex));
    if (auto* arr = fxObj.getArray()) {
        if (knobIndex >= 0 && knobIndex < arr->size()) {
            auto& k = arr->getReference(knobIndex);
            knob.title = k["title"].toString();
            knob.desc = k["desc"].toString();
            if (!k["default"].isVoid()) knob.defaultVal = k["default"].toString();
            if (!k["isBipolar"].isVoid()) knob.isBipolar = static_cast<bool>(k["isBipolar"]);
        }
    }
    return knob;
}
```

## Phase 4: Refactoring UI Components
Replace massive C++ switch/conditional code blocks with simple dynamic dictionary fetches.

### [MODIFY] `source/UIComponents.cpp`
- Include `#include "StringManager.h"`.
- Locate the `TooltipHelper` namespace definitions near the bottom of the file. Update the large hardcoded blocks:
```cpp
juce::StringArray getLedSelectorItemTooltips(const juce::String& selectorCategory) {
    auto cat = selectorCategory.trim().toLowerCase();
    juce::String key;
    
    // Map internal names to JSON structure
    if (cat.contains("target") || cat.contains("routing")) key = "target";
    else if (cat.contains("carrier_tracking") || cat.contains("tracking")) key = "carrier_tracking";
    else if (cat.contains("mod_track")) key = "mod_track";
    else if (cat.contains("mod_type")) key = "mod_type";
    else if (cat == "filter_type_5" || (cat.contains("filter_type") && cat.contains("5"))) key = "filter_type_5";
    else if (cat.contains("filter_type")) key = "filter_type";
    else if (cat == "filter_slope_5" || (cat.contains("filter_slope") && cat.contains("5"))) key = "filter_slope_5";
    else if (cat.contains("filter_slope") || cat.contains("slope")) key = "filter_slope";
    else if (cat.contains("limiter") || cat.contains("toggle")) key = "toggle";
    else if (cat.contains("smear") || cat.contains("order")) key = "phase_smear";

    if (key.isNotEmpty()) {
        return StringManager::getStringArray("tooltips.led_selectors." + key);
    }
    return {};
}

juce::String getFxAlgorithmTooltip(int fxIndex) {
    juce::String tooltip = StringManager::getString("tooltips.fx_algorithms." + juce::String(fxIndex));
    if (tooltip.isNotEmpty()) return tooltip;
    return "BYPASS: FX processing bypassed, audio passes through clean.";
}

juce::String getFxKnobTooltip(int fxIndex, int knobIndex) {
    auto knob = StringManager::getFxKnob(fxIndex, knobIndex);
    if (knob.title.isEmpty()) {
        return makeKnobTooltip("Parameter " + juce::String(knobIndex + 1), "Slot parameter for the active effect");
    }
    return makeKnobTooltip(knob.title, knob.desc, knob.defaultVal, knob.isBipolar);
}
```

## Phase 5: Refactoring Plugin Editor
Strip hardcoded text from the quickstart guide and parameter lookups.

### [MODIFY] `source/PluginEditor.cpp`
- Include `#include "StringManager.h"`.
- Locate `NavigationCardComponent::NavigationCardComponent()` and update the tooltip load:
  ```cpp
  // Inside the loop:
  if (i < 7) btn->setTooltip(StringManager::getStringArray("tooltips.navigation_pages")[i]);
  ```
- Locate `QuickstartGuideModalComponent::paint` and replace the raw string arrays with `StringManager` dynamic rendering logic. Refactor the `drawPanel` section to dynamically pull from `"quickstart.panels"` arrays.
- Locate the `getFarmerParamDescription()` block and eliminate the massive `if` chain:
  ```cpp
  static juce::String getFarmerParamDescription(const juce::String& paramId, bool& isBipolar) {
      isBipolar = false;
      juce::String id = paramId;
      
      // Normalize regex-style parameterized envelopes down to generic JSON keys
      if (paramId.startsWith("modenv") && paramId.endsWith("_slope")) id = "modenv_slope";
      else if (paramId.startsWith("modenv") && paramId.endsWith("_depth")) id = "modenv_depth";
      else if (paramId.startsWith("modenv") && paramId.endsWith("_decay")) id = "modenv_decay";

      auto paramVar = StringManager::getVar("tooltips.params." + id);
      if (paramVar.isObject()) {
          if (!paramVar["isBipolar"].isVoid()) isBipolar = static_cast<bool>(paramVar["isBipolar"]);
          return paramVar["desc"].toString();
      }
      return "";
  }
  ```
