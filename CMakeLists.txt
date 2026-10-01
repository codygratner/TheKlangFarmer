cmake_minimum_required(VERSION 3.22)

# 1. Project definition
project(TheKlangFarmer VERSION 0.1.3 LANGUAGES C CXX)

# Enforce modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Add the cloned JUCE repository
add_subdirectory(JUCE)

# 3. Configure the plugin target
juce_add_plugin(TheKlangFarmer
    COMPANY_NAME "R'lyeh Sound"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    # 4-character codes (required by VST/AU specs; must have at least 1 uppercase)
    PLUGIN_MANUFACTURER_CODE Rlyh
    PLUGIN_CODE Tkf1
    BUNDLE_ID com.rlyehsound.theklangfarmer
    
    # Builds both the DAW plugin and a standalone runnable executable for rapid DSP iteration
    FORMATS VST3 Standalone
    PRODUCT_NAME "The Klang Farmer"
    VST3_CAN_REPLACE_VST2 FALSE
)

# 4. Add source files
target_sources(TheKlangFarmer PRIVATE
    source/DSPBlock.h
    source/ModularBlocks.h
    source/PluginProcessor.h
    source/PluginProcessor.cpp
    source/PluginEditor.h
    source/PluginEditor.cpp
)

target_include_directories(TheKlangFarmer PRIVATE
    source
)

target_compile_definitions(TheKlangFarmer PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# 5. Link required JUCE modules
target_link_libraries(TheKlangFarmer PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# 6. Test suite executable
add_executable(dsp_tests
    test/dsp_tests.cpp
    source/DSPBlock.h
    source/ModularBlocks.h
)
target_include_directories(dsp_tests PRIVATE source)

# 7. Screenshot snapshot generator
add_executable(capture_screenshot
    test/capture_screenshot.cpp
)
target_link_libraries(capture_screenshot PRIVATE
    TheKlangFarmer
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_screenshot PRIVATE source)