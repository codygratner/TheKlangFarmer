# Data-Driven UI Layout & Colors (`theme.json`)

**Objective:** Extract the remaining hardcoded UI configuration out of C++ (`PluginEditor.cpp`) into JSON assets (`assets/controls/theme.json` or `layout.json`).

## Phase 1: Migrate UI Configurations to JSON [x]
- [x] Move all **Knob Colors** (e.g. `juce::Colour(0xff00d2ff)`) into JSON.
- [x] Move **UI Coordinates & Sizes** (`setBounds(x,y,w,h)`) into JSON.
- [x] Move **Card Groupings** (which knobs belong to which physical "Cards" on the screen) into JSON.

## Phase 2: Connect to ParameterManager [x]
- [x] Hook this up to `ParameterManager` so the UI can be fully skinned and reconfigured dynamically without recompiling C++.
