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
MODELS_DIR="$SCRIPTS_DIR/../models"

# Delete models if directory exists.
if [ -d "$MODELS_DIR/ddsp" ]; then
  rm -rf "$MODELS_DIR/ddsp"
fi

MODELS_URL=https://storage.googleapis.com/ddsp-vst/releases/DDSP-VST-Models.zip

echo
echo "Downloading DDSP TF Lite Models ..."
echo

pushd "$MODELS_DIR"
mkdir temp && cd temp
curl -L -o DDSP-VST-Models.zip ${MODELS_URL} || exit 1
unzip -q DDSP-VST-Models.zip
mv DDSP-VST-Models ../ddsp
cd ../ && rm -rf temp
popd

echo
echo "DDSP TF Lite Models downloaded to models/ddsp/"
echo

