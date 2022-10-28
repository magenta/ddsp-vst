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

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

namespace ddsp
{

template <typename Input, typename Output>
class ModelBase
{
public:
    ModelBase (const char* modelDataPtr, size_t dataSize, int numThreads)
    {
        modelBuffer = tflite::FlatBufferModel::VerifyAndBuildFromBuffer (modelDataPtr, dataSize);
        jassert (modelBuffer != nullptr);

        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder builder (*modelBuffer, resolver);

        builder.SetNumThreads (numThreads);
        auto status = builder (&interpreter);
        jassert (status == kTfLiteOk);
        jassert (interpreter != nullptr);

        status = interpreter->AllocateTensors();
        jassert (status == kTfLiteOk);
    }

    virtual ~ModelBase() = default;

    // Describe the model's inputs and outputs.
    void describe()
    {
        tflite::PrintInterpreterState (interpreter.get());

        // Inspect inputs.
        DBG ("Interpreter num inputs: " << interpreter->inputs().size() << "\n");

        for (int i = 0; i < interpreter->inputs().size(); ++i)
        {
            const char* inputName = interpreter->GetInputName (i);
            auto type = interpreter->input_tensor (i)->type;
            auto size = interpreter->input_tensor (i)->bytes / sizeof (type);
            const int tensorIndex = interpreter->inputs()[i];

            DBG ("Input name: " << inputName);
            DBG ("Input size: " << size);
            DBG ("Input type: " << TfLiteTypeGetName (type));
            DBG ("Input tensor idx: " << tensorIndex << "\n");
        }

        // Inspect outputs.
        DBG ("Interpreter num outputs: " << interpreter->outputs().size() << "\n");

        for (int i = 0; i < interpreter->outputs().size(); ++i)
        {
            const char* outputName = interpreter->GetOutputName (i);
            auto type = interpreter->output_tensor (i)->type;
            auto size = interpreter->output_tensor (i)->bytes / sizeof (type);
            const int tensorIndex = interpreter->outputs()[i];

            DBG ("Output name: " << outputName);
            DBG ("Output size: " << size);
            DBG ("Output type: " << TfLiteTypeGetName (type));
            DBG ("Output tensor idx: " << tensorIndex << "\n");
        }
    }

    // TODO: return error code.
    virtual void call (const Input& input, Output& output) = 0;

protected:
    std::unique_ptr<tflite::FlatBufferModel> modelBuffer;
    std::unique_ptr<tflite::Interpreter> interpreter;
};

} // namespace ddsp
