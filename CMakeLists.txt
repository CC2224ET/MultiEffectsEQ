cmake_minimum_required(VERSION 3.22)
project(MultiEffectsEQ VERSION 1.0.0)
add_subdirectory(JUCE)

juce_add_plugin(MultieffectsEQ
    COMPANY_NAME "Botang"
    PLUGIN_MANUFACTURER_CODE "Btng"  
    
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE           
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    
    FORMATS VST3 AU Standalone
    
    PRODUCT_NAME "MultieffectsEQ"
)

target_sources(MultieffectsEQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_link_libraries(MultieffectsEQ PRIVATE
    juce::juce_audio_utils     
    juce::juce_dsp             
    juce::juce_audio_formats 
)

target_compile_definitions(MultieffectsEQ PUBLIC
    
    JUCE_WEB_BROWSER=0          
    JUCE_USE_CURL=0             
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(MultieffectsEQ PUBLIC cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    target_compile_options(MultieffectsEQ PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(MultieffectsEQ PRIVATE /W4)
endif()