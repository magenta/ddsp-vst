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

#!/bin/bash

# Suppress path echo.
pushd () {
    command pushd "$@" > /dev/null
}
popd () {
    command popd "$@" > /dev/null
}

SCRIPTS_DIR="$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )"
BUILD_DIR="$SCRIPTS_DIR/../build"

pushd "$BUILD_DIR"

# CMake adds m.lib to all the plugin targets as a dependency
# This isn't required for Visual Studio and causes a build error
# We search and replace all occurences of m.lib in the .vcxproj files
for PLUGIN_TARGET in DDSPEffect_VST3 \
                     DDSPEffect_Standalone \
                     DDSPSynth_VST3 \
                     DDSPSynth_Standalone \
                     DDSPUnitTestRunner
do
    grep -rl ';m.lib;' ${PLUGIN_TARGET}.vcxproj | xargs sed -i 's/;m.lib;/;/g'
done
