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

set(DDSP_SOURCES
    # plugin
    src/PluginProcessor.h
    src/PluginProcessor.cpp
    src/PluginEditor.h
    src/PluginEditor.cpp

    # audio
    src/audio/AudioRingBuffer.h
    src/audio/MidiInputProcessor.h
    src/audio/MidiInputProcessor.cpp
    src/audio/HarmonicSynthesizer.h
    src/audio/HarmonicSynthesizer.cpp
    src/audio/NoiseSynthesizer.h
    src/audio/NoiseSynthesizer.cpp

    # tflite
    src/audio/tflite/ModelBase.h
    src/audio/tflite/ModelTypes.h
    src/audio/tflite/ModelLibrary.h
    src/audio/tflite/ModelLibrary.cpp
    src/audio/tflite/FeatureExtractionModel.h
    src/audio/tflite/FeatureExtractionModel.cpp
    src/audio/tflite/PredictControlsModel.h
    src/audio/tflite/PredictControlsModel.cpp
    src/audio/tflite/InferencePipeline.h
    src/audio/tflite/InferencePipeline.cpp

    # ui
    src/ui/DDSPLookAndFeel.h
    src/ui/ParamInfo.h
    src/ui/ParamInfo.cpp
    src/ui/RadioButtonGroupGomponent.h
    src/ui/RadioButtonGroupGomponent.cpp
    src/ui/SliderWithDynamicLabel.h
    src/ui/SliderWithDynamicLabel.cpp
    src/ui/TopPanelComponent.h
    src/ui/TopPanelComponent.cpp
    src/ui/ModelRangeVisualizerComponent.h
    src/ui/ModelRangeVisualizerComponent.cpp
    src/ui/BottomPanelComponent.h
    src/ui/BottomPanelComponent.cpp
    src/ui/ContentComponent.h
    src/ui/ContentComponent.cpp

    # util
    src/util/Constants.h
    src/util/InputUtils.h
)

set(DDSP_ASSETS

    assets/icons/folder_icon.svg
    assets/icons/info_icon.svg
    assets/icons/refresh_icon.svg
    assets/icons/logo_thin.svg

    # pitch detection model
    models/pitch-detection/extract_features_micro.tflite

    # DDSP models
    models/ddsp/Bassoon.tflite
    models/ddsp/Clarinet.tflite
    models/ddsp/Flute.tflite
    models/ddsp/Melodica.tflite
    models/ddsp/Saxophone.tflite
    models/ddsp/Sitar.tflite
    models/ddsp/Trombone.tflite
    models/ddsp/Trumpet.tflite
    models/ddsp/Tuba.tflite
    models/ddsp/Violin.tflite
    models/ddsp/Vowels.tflite

)

set(DDSP_TEST_SOURCES

    tests/InferencePipeline_Test.cpp
)