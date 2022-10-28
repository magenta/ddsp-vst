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

#include "audio/tflite/FeatureExtractionModel.h"

namespace ddsp
{

FeatureExtractionModel::FeatureExtractionModel()
    : ModelBase (BinaryData::extract_features_micro_tflite,
                 BinaryData::extract_features_micro_tfliteSize,
                 kNumFeatureExtractionThreads)
{
}

void FeatureExtractionModel::call (const juce::AudioBuffer<float>& audioInput, AudioFeatures& output)
{
    // Fill tensor with audio buffer.
    for (int i = 0; i < audioInput.getNumSamples(); ++i)
    {
        interpreter->typed_input_tensor<float> (0)[i] = audioInput.getSample (0, i);
    }

    // Call model.
    if (auto status = interpreter->Invoke(); status != kTfLiteOk)
    {
        std::cerr << "Failed to compute, status code: " << status << std::endl;
    }

    std::vector<int> tensorIndices (interpreter->outputs().size());

    // pw_db, f0_hz, pw_scaled, f0_scaled
    for (size_t i = 0; i < interpreter->outputs().size(); ++i)
    {
        tensorIndices[i] = interpreter->outputs()[i];
    }

    output.loudness_db = *interpreter->typed_tensor<float> (tensorIndices[3]);
    output.f0_hz = *interpreter->typed_tensor<float> (tensorIndices[2]);
    // TODO: change loudness to power.
    output.loudness_norm = *interpreter->typed_tensor<float> (tensorIndices[1]);
    output.f0_norm = *interpreter->typed_tensor<float> (tensorIndices[0]);
}

} // namespace ddsp
