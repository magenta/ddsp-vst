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

#include "audio/tflite/ModelLibrary.h"
#include "util/Constants.h"

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

namespace ddsp
{

ModelLibrary::ModelLibrary()
{
    loadEmbeddedModels();
    setPathToUserModels();
    searchPathForModels();
}

ModelLibrary::~ModelLibrary() {}

void ModelLibrary::loadEmbeddedModels()
{
    models.emplace_back (ModelInfo ("Flute",
                                    loadModelTimestamp (BinaryData::Flute_tflite, BinaryData::Flute_tfliteSize),
                                    BinaryData::Flute_tflite,
                                    BinaryData::Flute_tfliteSize));

    models.emplace_back (ModelInfo ("Violin",
                                    loadModelTimestamp (BinaryData::Violin_tflite, BinaryData::Violin_tfliteSize),
                                    BinaryData::Violin_tflite,
                                    BinaryData::Violin_tfliteSize));

    models.emplace_back (ModelInfo ("Trumpet",
                                    loadModelTimestamp (BinaryData::Trumpet_tflite, BinaryData::Trumpet_tfliteSize),
                                    BinaryData::Trumpet_tflite,
                                    BinaryData::Trumpet_tfliteSize));

    models.emplace_back (ModelInfo ("Saxophone",
                                    loadModelTimestamp (BinaryData::Saxophone_tflite, BinaryData::Saxophone_tfliteSize),
                                    BinaryData::Saxophone_tflite,
                                    BinaryData::Saxophone_tfliteSize));
    models.emplace_back (ModelInfo ("Bassoon",
                                    loadModelTimestamp (BinaryData::Bassoon_tflite, BinaryData::Bassoon_tfliteSize),
                                    BinaryData::Bassoon_tflite,
                                    BinaryData::Bassoon_tfliteSize));
    models.emplace_back (ModelInfo ("Clarinet",
                                    loadModelTimestamp (BinaryData::Clarinet_tflite, BinaryData::Clarinet_tfliteSize),
                                    BinaryData::Clarinet_tflite,
                                    BinaryData::Clarinet_tfliteSize));
    models.emplace_back (ModelInfo ("Melodica",
                                    loadModelTimestamp (BinaryData::Melodica_tflite, BinaryData::Melodica_tfliteSize),
                                    BinaryData::Melodica_tflite,
                                    BinaryData::Melodica_tfliteSize));
    models.emplace_back (ModelInfo ("Sitar",
                                    loadModelTimestamp (BinaryData::Sitar_tflite, BinaryData::Sitar_tfliteSize),
                                    BinaryData::Sitar_tflite,
                                    BinaryData::Sitar_tfliteSize));
    models.emplace_back (ModelInfo ("Trombone",
                                    loadModelTimestamp (BinaryData::Trombone_tflite, BinaryData::Trombone_tfliteSize),
                                    BinaryData::Trombone_tflite,
                                    BinaryData::Trombone_tfliteSize));
    models.emplace_back (ModelInfo ("Tuba",
                                    loadModelTimestamp (BinaryData::Tuba_tflite, BinaryData::Tuba_tfliteSize),
                                    BinaryData::Tuba_tflite,
                                    BinaryData::Tuba_tfliteSize));
    models.emplace_back (ModelInfo ("Vowels",
                                    loadModelTimestamp (BinaryData::Vowels_tflite, BinaryData::Vowels_tfliteSize),
                                    BinaryData::Vowels_tflite,
                                    BinaryData::Vowels_tfliteSize));

    jassert (models.size() == kNumEmbeddedPredictControlsModels);
}

void ModelLibrary::setPathToUserModels()
{
    auto documentsDir = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDocumentsDirectory);
    pathToUserModels = documentsDir.getChildFile ("Magenta").getChildFile ("DDSP").getChildFile ("Models");
}

int ModelLibrary::getModelIdx (juce::String modelTimestamp)
{
    // If the model exists, return its index, otherwise default to the first one.
    for (int i = 0; i < models.size(); i++)
    {
        if (models[i].timestamp == modelTimestamp)
        {
            return i;
        }
    }
    return 0;
}

juce::String ModelLibrary::getModelTimestamp (int modelIdx) { return models[modelIdx].timestamp; }

juce::File ModelLibrary::getPathToUserModels() { return pathToUserModels; }

// We don't want a call to disk I/O from the plugin on every model load,
// so we will load all models from disk into memory here.
void ModelLibrary::searchPathForModels()
{
    clearUserModels();
    if (pathToUserModels.createDirectory() == juce::Result::ok())
    {
        auto modelArray = pathToUserModels.findChildFiles (juce::File::findFiles, true, "*.tflite");
        models.reserve (modelArray.size());

        for (auto& m : modelArray)
        {
            ModelInfo modelInfo (m.getFileNameWithoutExtension(),
                                 loadModelTimestamp (m.loadFileAsString().toRawUTF8(), m.getSize()),
                                 m.loadFileAsString().toRawUTF8(),
                                 m.getSize());

            if (validateModel (modelInfo))
                models.emplace_back (modelInfo);
        }
    }
    else
    {
        juce::AlertWindow ("Error",
                           "Could not create directory " + pathToUserModels.getFullPathName(),
                           juce::AlertWindow::AlertIconType::WarningIcon);
    }
}

juce::String ModelLibrary::loadModelTimestamp (const char* modelDataPtr, size_t dataSize)
{
    // Reading model metadata.
    juce::MemoryInputStream modelBufferStream (static_cast<const void*> (modelDataPtr), dataSize, false);
    juce::ZipFile zf (&modelBufferStream, false);

    if (const juce::ZipFile::ZipEntry* e = zf.getEntry ("metadata.json", true))
    {
        if (juce::InputStream* is = zf.createStreamForEntry (*e))
        {
            juce::var json = juce::JSON::parse (is->readEntireStreamAsString());

            DBG (juce::JSON::toString (json));

            auto exportTime = json["export_time"].toString().toUTF8();

            delete is;
            return exportTime;
        }
    }
    else
    {
        DBG ("Cannot access model metadata.");
    }
    // Maybe something else should go here...
    return "";
}

bool ModelLibrary::validateModel (ModelInfo modelInfo)
{
    juce::StringArray errorMsg;

    std::unique_ptr<tflite::FlatBufferModel> modelBuffer;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::ops::builtin::BuiltinOpResolver resolver;

    // Check if the model is able to load.
    modelBuffer = tflite::FlatBufferModel::VerifyAndBuildFromBuffer (modelInfo.data.begin(), modelInfo.data.getSize());

    if (modelBuffer == nullptr)
    {
        errorMsg.add ("Invalid .tflite file.\n");
        showAlertWindow (modelInfo.name, errorMsg);
        return false;
    }

    // Continue setting up model.
    tflite::InterpreterBuilder initBuilder (*modelBuffer, resolver);
    initBuilder (&interpreter);
    jassert (interpreter != nullptr);

    interpreter->SetNumThreads (1);
    jassert (interpreter->AllocateTensors() == kTfLiteOk);

    // Check if model has correct number of I/O tensors.
    if (interpreter->inputs().size() != kNumPredictControlsInputTensors)
    {
        errorMsg.add ("Invalid number of input tensors: " + std::to_string (interpreter->inputs().size()) + "\n");
    }

    if (interpreter->outputs().size() != kNumPredictControlsOutputTensors)
    {
        errorMsg.add ("Invalid number of output tensors: " + std::to_string (interpreter->outputs().size()) + "\n");
    }

    if (! errorMsg.isEmpty())
    {
        showAlertWindow (modelInfo.name, errorMsg);
        return false;
    }

    // Check if tensors have the correct names. Sometimes the colab
    // puts them in different orders so we will stay order-agnostic here.
    // We also do not want repeats.
    juce::StringArray inputNames;
    inputNames.add (kInputTensorName_F0.data());
    inputNames.add (kInputTensorName_Loudness.data());
    inputNames.add (kInputTensorName_State.data());

    for (int i = 0; i < interpreter->inputs().size(); i++)
    {
        auto name = interpreter->GetInputName (i);
        if (int idx = inputNames.indexOf (name); idx != -1)
        {
            inputNames.remove (idx);
        }
        else
        {
            errorMsg.add ("Invalid input tensor name " + std::string (name) + "\n");
        }
    }

    juce::StringArray outputNames;
    outputNames.add (kOutputTensorName_Amplitude.data());
    outputNames.add (kOutputTensorName_Harmonics.data());
    outputNames.add (kOutputTensorName_NoiseAmps.data());
    outputNames.add (kOutputTensorName_State.data());

    for (int i = 0; i < interpreter->outputs().size(); i++)
    {
        auto name = interpreter->GetOutputName (i);
        if (int idx = outputNames.indexOf (name); idx != -1)
        {
            outputNames.remove (idx);
        }
        else
        {
            errorMsg.add ("Invalid output tensor name " + std::string (name) + "\n");
        }
    }

    if (! errorMsg.isEmpty())
    {
        showAlertWindow (modelInfo.name, errorMsg);
        return false;
    }

    // Check if tensors have correct sizes.
    for (int i = 0; i < interpreter->inputs().size(); i++)
    {
        const std::string_view name = interpreter->GetInputName (i);
        auto size = interpreter->input_tensor (i)->bytes / sizeof (float);

        if (name == kInputTensorName_F0)
        {
            if (size != kF0Size)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
        else if (name == kInputTensorName_Loudness)
        {
            if (size != kLoudnessSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
        else if (name == kInputTensorName_State)
        {
            if (size != kGruModelStateSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
    }

    for (int i = 0; i < interpreter->outputs().size(); i++)
    {
        const std::string_view name = interpreter->GetOutputName (i);
        auto size = interpreter->output_tensor (i)->bytes / sizeof (float);

        if (name == kOutputTensorName_Amplitude)
        {
            if (size != kAmplitudeSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
        else if (name == kOutputTensorName_Harmonics)
        {
            if (size != kHarmonicsSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
        else if (name == kOutputTensorName_NoiseAmps)
        {
            if (size != kNoiseAmpsSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
        else if (name == kOutputTensorName_State)
        {
            if (size != kGruModelStateSize)
            {
                errorMsg.add ("Invalid tensor size " + std::to_string (size) + " for " + name.data() + "\n");
            }
        }
    }

    if (! errorMsg.isEmpty())
    {
        showAlertWindow (modelInfo.name, errorMsg);
        return false;
    }

    return true;
}

void ModelLibrary::clearUserModels()
{
    for (size_t i = models.size(); i > kNumEmbeddedPredictControlsModels; --i)
    {
        models.pop_back();
    }
}

void ModelLibrary::showAlertWindow (juce::String modelName, juce::StringArray messages)
{
    juce::String message;
    for (int i = 0; i < messages.size(); i++)
    {
        message.append (messages[i], messages[i].length());
    }

    juce::NativeMessageBox::showMessageBoxAsync (
        juce::AlertWindow::AlertIconType::WarningIcon, "DDSP - Error loading model: " + modelName, message);
}

} // namespace ddsp
