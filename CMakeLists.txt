cmake_minimum_required(VERSION 3.22)

# macOS deployment target and architecture definitions MUST precede project()
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary architectures" FORCE)
    endif()
endif()

# 1. Project definition
project(TheKlangFarmer VERSION 0.1.7 LANGUAGES C CXX)

# Enforce modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Add the cloned JUCE repository
add_subdirectory(JUCE)

# Configure plugin formats per platform
set(TKF_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND TKF_FORMATS AU)
endif()

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
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "The Klang Farmer"
    VST3_CAN_REPLACE_VST2 FALSE
)

# 4. Add source files
target_sources(TheKlangFarmer PRIVATE
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/UIComponents.h
    source/UIComponents.cpp
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
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(TheKlangFarmer PROPERTIES VST3_AUTO_MANIFEST FALSE)

set(TKF_JUCE_MODULES
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
)

# 5. Link required JUCE modules for The Klang Farmer
target_link_libraries(TheKlangFarmer PRIVATE
    ${TKF_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# --- 6. The Klang Planter Plugin Target ---
juce_add_plugin(TheKlangPlanter
    COMPANY_NAME "R'lyeh Sound"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    PLUGIN_MANUFACTURER_CODE Rlyh
    PLUGIN_CODE Tkp1
    BUNDLE_ID com.rlyehsound.theklangplanter
    
    FORMATS ${TKF_FORMATS}
    PRODUCT_NAME "The Klang Planter"
    VST3_CAN_REPLACE_VST2 FALSE
)

target_sources(TheKlangPlanter PRIVATE
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/PlanterEngine.h
    source/UIComponents.h
    source/UIComponents.cpp
    source/PlanterProcessor.h
    source/PlanterProcessor.cpp
    source/PlanterEditor.h
    source/PlanterEditor.cpp
)

target_include_directories(TheKlangPlanter PRIVATE source)

target_compile_definitions(TheKlangPlanter PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

set_target_properties(TheKlangPlanter PROPERTIES VST3_AUTO_MANIFEST FALSE)

target_link_libraries(TheKlangPlanter PRIVATE
    ${TKF_JUCE_MODULES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Cross-platform Linux static stdlib linking
if (UNIX AND NOT APPLE)
    target_link_options(TheKlangFarmer PRIVATE -static-libstdc++ -static-libgcc)
    target_link_options(TheKlangPlanter PRIVATE -static-libstdc++ -static-libgcc)
endif()

# 7. Test suite executable
add_executable(dsp_tests
    test/dsp_tests.cpp
    source/FastMath.h
    source/DSPBlock.h
    source/ModularBlocks.h
    source/PlanterEngine.h
)
target_include_directories(dsp_tests PRIVATE source)

# 8. FastMath & DSP performance benchmark
add_executable(benchmark_dsp
    test/benchmark_dsp.cpp
    source/FastMath.h
)
target_include_directories(benchmark_dsp PRIVATE source)

# 9. Screenshot snapshot generator
add_executable(capture_screenshot
    test/capture_screenshot.cpp
)
target_link_libraries(capture_screenshot PRIVATE
    TheKlangFarmer
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_screenshot PRIVATE source)

add_executable(capture_planter_screenshot
    test/capture_planter_screenshot.cpp
)
target_link_libraries(capture_planter_screenshot PRIVATE
    TheKlangPlanter
    juce::juce_gui_basics
    juce::juce_graphics
)
target_include_directories(capture_planter_screenshot PRIVATE source)