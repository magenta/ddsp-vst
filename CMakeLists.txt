# Copyright 2022 The DDSP-VST Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.15)

include(cmake/Config.cmake)
include(cmake/FileList.cmake)
include(cmake/Util.cmake)

project(${DDSP_PROJECT_NAME}
    VERSION ${DDSP_PROJECT_VERSION}
    DESCRIPTION ${DDSP_PROJECT_DESC}
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# -------------------------------- JUCE ------------------------------- #

option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option(JUCE_DISABLE_ASSERTIONS "Disable JUCE assertions / DBG even in debug mode" OFF)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

if(JUCE_DISABLE_ASSERTIONS)
    list(APPEND DDSP_JUCE_COMPILE_DEFS JUCE_DISABLE_ASSERTIONS=1)
endif()

add_subdirectory(externals/JUCE "${CMAKE_CURRENT_BINARY_DIR}/juce-bin" EXCLUDE_FROM_ALL)

# ------------------------------- TFLite ------------------------------ #

add_subdirectory(externals/tensorflow/tensorflow/lite "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite-bin" EXCLUDE_FROM_ALL)
# TFLite CXX std needs to be explicitly set to C++20 in Visual Studio.
target_compile_features(tensorflow-lite PUBLIC ${DDSP_CXX_STD})
target_compile_options(tensorflow-lite PUBLIC -stdlib=libc++)

# ------------------------- DDSP Binary Assets ------------------------ #

juce_add_binary_data(Assets SOURCES ${DDSP_ASSETS})

# ---------------------------- DDSP Effect ---------------------------- #

set(DDSP_EFFECT_TARGET DDSPEffect)

juce_add_plugin(${DDSP_EFFECT_TARGET}
    PRODUCT_NAME "DDSP Effect"
    COMPANY_NAME ${DDSP_COMPANY_NAME}
    MICROPHONE_PERMISSION_ENABLED TRUE
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${DDSP_COPY_PLUGIN}
    PLUGIN_MANUFACTURER_CODE ${DDSP_PLUGIN_MANU_CODE}
    PLUGIN_CODE Ranj
    FORMATS ${DDSP_PLUGIN_FORMATS}
)

target_include_directories(${DDSP_EFFECT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_sources(${DDSP_EFFECT_TARGET} PRIVATE ${DDSP_SOURCES})
target_compile_features(${DDSP_EFFECT_TARGET} PUBLIC ${DDSP_CXX_STD})
target_compile_definitions(${DDSP_EFFECT_TARGET} PUBLIC ${DDSP_JUCE_COMPILE_DEFS})
target_link_libraries(${DDSP_EFFECT_TARGET}
    PRIVATE
    ${DDSP_PRIVATE_LIBS}
    PUBLIC
    ${DDSP_PUBLIC_LIBS}
)
juce_generate_juce_header(${DDSP_EFFECT_TARGET})
regroup_juce_target_sources(${DDSP_EFFECT_TARGET})

if(APPLE)
    set_target_properties(${DDSP_EFFECT_TARGET}_AU PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${DDSP_EFFECT_TARGET}_VST3 PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${DDSP_EFFECT_TARGET}_Standalone PROPERTIES XCODE_GENERATE_SCHEME ON)
endif()

# ---------------------------- DDSP Synth ----------------------------- #

set(DDSP_SYNTH_TARGET DDSPSynth)

juce_add_plugin(${DDSP_SYNTH_TARGET}
    PRODUCT_NAME "DDSP Synth"
    COMPANY_NAME ${DDSP_COMPANY_NAME}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${DDSP_COPY_PLUGIN}
    PLUGIN_MANUFACTURER_CODE ${DDSP_PLUGIN_MANU_CODE}
    PLUGIN_CODE Ranb
    FORMATS ${DDSP_PLUGIN_FORMATS}
)

target_include_directories(${DDSP_SYNTH_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_sources(${DDSP_SYNTH_TARGET} PRIVATE ${DDSP_SOURCES})
target_compile_features(${DDSP_SYNTH_TARGET} PUBLIC ${DDSP_CXX_STD})
target_compile_definitions(${DDSP_SYNTH_TARGET} PUBLIC ${DDSP_JUCE_COMPILE_DEFS})
target_link_libraries(${DDSP_SYNTH_TARGET}
    PRIVATE
    ${DDSP_PRIVATE_LIBS}
    PUBLIC
    ${DDSP_PUBLIC_LIBS}
)
juce_generate_juce_header(${DDSP_SYNTH_TARGET})
regroup_juce_target_sources(${DDSP_SYNTH_TARGET})

if(APPLE)
    set_target_properties(${DDSP_SYNTH_TARGET}_AU PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${DDSP_SYNTH_TARGET}_VST3 PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${DDSP_SYNTH_TARGET}_Standalone PROPERTIES XCODE_GENERATE_SCHEME ON)
endif()

# ----------------------- DDSP Unit Test Runner ----------------------- #

enable_testing()

set(DDSP_UNIT_TEST_TARGET DDSPUnitTestRunner)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

juce_add_console_app(${DDSP_UNIT_TEST_TARGET} PRODUCT_NAME "DDSP Unit Test Runner")
target_sources(${DDSP_UNIT_TEST_TARGET} PRIVATE ${DDSP_TEST_SOURCES})

target_include_directories(${DDSP_UNIT_TEST_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(${DDSP_UNIT_TEST_TARGET} PUBLIC ${DDSP_CXX_STD})
target_link_libraries(${DDSP_UNIT_TEST_TARGET}
    PRIVATE
    gtest_main
    ${DDSP_EFFECT_TARGET}
    ${DDSP_PRIVATE_LIBS}
    PUBLIC
    ${DDSP_PUBLIC_LIBS}
)
juce_generate_juce_header(${DDSP_UNIT_TEST_TARGET})
regroup_juce_target_sources(${DDSP_UNIT_TEST_TARGET})

set_target_properties(${DDSP_UNIT_TEST_TARGET} PROPERTIES XCODE_GENERATE_SCHEME ON)

include(GoogleTest)
gtest_discover_tests(${DDSP_UNIT_TEST_TARGET})

# --------------------------------------------------------------------- #
