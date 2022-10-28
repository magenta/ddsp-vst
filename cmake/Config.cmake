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

set(DDSP_PROJECT_NAME DDSP)
# This is the version number displayed on the plugin UI.
set(DDSP_PROJECT_VERSION 1.1.0)
set(DDSP_PROJECT_DESC "DDSP audio effect and synthesis plugins")
set(DDSP_COMPANY_NAME "Magenta")
set(DDSP_PLUGIN_MANU_CODE Mage)
set(DDSP_PLUGIN_FORMATS AU VST3 Standalone)
set(DDSP_CXX_STD cxx_std_20)

message(STATUS "~~~~~~~~~ DDSP VST ~~~~~~~~~")
message(STATUS "VERSION: ${DDSP_PROJECT_VERSION}")
message(STATUS "~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

set(DDSP_JUCE_COMPILE_DEFS

    DONT_SET_USING_JUCE_NAMESPACE=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

set(DDSP_PRIVATE_LIBS
    # DDSP libs.
    Assets
    tensorflow-lite

    # JUCE libs.
    juce::juce_core
    juce::juce_audio_utils
    juce::juce_dsp
)

set(DDSP_PUBLIC_LIBS

    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(APPLE)
    # Xcode: Disable automatic build scheme generation globally.
    set(CMAKE_XCODE_GENERATE_SCHEME OFF)

    # TODO: add universal binary support once tensorflow is supported on arm.
    set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE INTERNAL "")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)

    # Automatically copy plugins post build on OSX.
    set(DDSP_COPY_PLUGIN TRUE)

    # XNNPACK is incompatible with Xcode, use it only during development.
    # Build with CMake or Ninja for release.
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        option(TFLITE_ENABLE_XNNPACK "XNNPACK" OFF)
    endif()
elseif(WIN32)
    # Static linking on Windows.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # Manually copy the plugins post build on Windows.
    set(DDSP_COPY_PLUGIN FALSE)
endif()