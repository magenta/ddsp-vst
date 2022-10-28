/*
Copyright 2022 The DDSP-VST Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <array>

#include "audio/tflite/ModelBase.h"
#include "audio/tflite/ModelLibrary.h"
#include "audio/tflite/ModelTypes.h"

namespace ddsp
{

class PredictControlsModel : public ModelBase<AudioFeatures, SynthesisControls>
{
public:
    PredictControlsModel (const ModelInfo& mi);

    void call (const AudioFeatures& input, SynthesisControls& output) override;
    void reset();

    // Metadata for UI rendering.
    struct Metadata
    {
        float minPitch_Hz = 0.0f;
        float maxPitch_Hz = 0.0f;
        float minPower_dB = 0.0f;
        float maxPower_dB = 0.0f;
        std::string version;
        std::string exportTime;
    };

    static const Metadata getMetadata (const ModelInfo& mi);

private:
    // GRU model state.
    std::array<float, kGruModelStateSize> gruState;
};

} // namespace ddsp
