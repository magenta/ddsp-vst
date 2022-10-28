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

# Sets up the source groups for a JUCE target.
# The directory structure for the plugin source is maintained as is.
# Enabling JUCE_ENABLE_MODULE_SOURCE_GROUPS is recommended.
function(regroup_juce_target_sources JUCE_TARGET)
    # Get all plugin sources.
    get_target_property(PLUGIN_SOURCES ${JUCE_TARGET} SOURCES)

    foreach(FILE ${PLUGIN_SOURCES})
        # Get the absolute path of source file.
        get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
        # Get the directory of source file.
        get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
        # Remove common directory prefix to make the group.
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        # Replace with windows slashes.
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        # If a JuceHeader.h is generated, it will placed in the JUCE Modules group.
        if ("${FILE}" MATCHES ".*\\JuceHeader.h")
            set(GROUP "JUCE Modules")
        endif()

        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction(regroup_juce_target_sources)