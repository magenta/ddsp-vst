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

#include "JuceHeader.h"
#include "util/Constants.h"

namespace ddsp
{

struct SynthesisControls
{
    float amplitude;
    float f0_hz;
    std::vector<float> noiseAmps = std::vector<float> (kNoiseAmpsSize);
    std::vector<float> harmonics = std::vector<float> (kHarmonicsSize);
};

struct AudioFeatures
{
    float f0_hz = 0.0f;
    float loudness_db = 0.0f;
    float f0_norm = 0.0f;
    float loudness_norm = 0.0f;
};

} // namespace ddsp
