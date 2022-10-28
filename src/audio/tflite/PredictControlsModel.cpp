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

#include "audio/tflite/PredictControlsModel.h"
#include "util/Constants.h"

namespace ddsp
{

PredictControlsModel::PredictControlsModel (const ModelInfo& mi)
    : ModelBase (mi.data.begin(), mi.data.getSize(), kNumPredictControlsThreads)
{
    reset();
}

void PredictControlsModel::call (const AudioFeatures& input, SynthesisControls& output)
{
    for (int i = 0; i < interpreter->inputs().size(); ++i)
    {
        const std::string_view inputName (interpreter->GetInputName (i));

        if (inputName == kInputTensorName_F0)
        {
            *interpreter->typed_input_tensor<float> (i) = input.f0_norm;
        }
        else if (inputName == kInputTensorName_Loudness)
        {
            *interpreter->typed_input_tensor<float> (i) = input.loudness_norm;
        }
        else if (inputName == kInputTensorName_State)
        {
            for (int j = 0; j < gruState.size(); j++)
            {
                interpreter->typed_input_tensor<float> (i)[j] = gruState[j];
            }
        }
        else
        {
            std::cerr << "Invalid tensor name: " + juce::StringRef (inputName.data()) << std::endl;
        }
    }

    // Run tflite graph computation on input.
    if (auto status = interpreter->Invoke(); status != kTfLiteOk)
    {
        std::cerr << "Failed to compute, status code: " << status << std::endl;
    }

    for (int i = 0; i < interpreter->outputs().size(); ++i)
    {
        const std::string_view outputName (interpreter->GetOutputName (i));

        if (outputName == kOutputTensorName_Amplitude)
        {
            output.amplitude = *interpreter->typed_output_tensor<float> (i);
        }
        else if (outputName == kOutputTensorName_Harmonics)
        {
            for (int j = 0; j < kHarmonicsSize; j++)
            {
                output.harmonics[j] = interpreter->typed_output_tensor<float> (i)[j];
            }
        }
        else if (outputName == kOutputTensorName_NoiseAmps)
        {
            for (int j = 0; j < kNoiseAmpsSize; j++)
            {
                output.noiseAmps[j] = interpreter->typed_output_tensor<float> (i)[j];
            }
        }
        else if (outputName == kOutputTensorName_State)
        {
            for (int j = 0; j < gruState.size(); j++)
            {
                gruState[j] = interpreter->typed_output_tensor<float> (i)[j];
            }
        }
        else
        {
            std::cerr << "Invalid tensor name: " + juce::StringRef (outputName.data()) << std::endl;
        }
    }

    for (int i = 0; i < kHarmonicsSize; ++i)
    {
        if (isnan (output.harmonics[i]))
        {
            DBG ("is_nan");
            output.harmonics[i] = 0.f;
            output.amplitude = 0.f;
        }
    }

    output.f0_hz = input.f0_hz;
}

void PredictControlsModel::reset()
{
    juce::FloatVectorOperations::clear (gruState.data(), static_cast<int> (gruState.size()));
}

const PredictControlsModel::Metadata PredictControlsModel::getMetadata (const ModelInfo& mi)
{
    PredictControlsModel::Metadata metadata;
    // read model metadata
    juce::MemoryInputStream modelBufferStream (static_cast<const void*> (mi.data.begin()), mi.data.getSize(), false);
    juce::ZipFile zf (&modelBufferStream, false);

    if (const juce::ZipFile::ZipEntry* e = zf.getEntry ("metadata.json", true))
    {
        if (juce::InputStream* is = zf.createStreamForEntry (*e))
        {
            juce::var json = juce::JSON::parse (is->readEntireStreamAsString());

            DBG (juce::JSON::toString (json));

            metadata.minPitch_Hz = json["mean_min_pitch_note_hz"];
            metadata.maxPitch_Hz = json["mean_max_pitch_note_hz"];
            metadata.minPower_dB = json["mean_min_power_note"];
            metadata.maxPower_dB = json["mean_max_power_note"];
            metadata.version = json["version"].toString().toUTF8();
            metadata.exportTime = json["export_time"].toString().toUTF8();

            delete is;
        }
    }
    else
    {
        DBG ("Cannot access model metadata.");
    }

    return metadata;
}

} // namespace ddsp
