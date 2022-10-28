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

namespace ddsp
{

struct ModelInfo
{
    // Name describing the model.
    const juce::String name;
    // Unique timestamp used for differentiating models of the same name.
    const juce::String timestamp;
    // Memory blob corresponding to the model contents.
    const juce::MemoryBlock data;

    ModelInfo (juce::String n, juce::String t, const char* d, size_t s) : name (n), timestamp (t), data (d, s) {}
};

class ModelLibrary
{
public:
    ModelLibrary();
    ~ModelLibrary();

    int getModelIdx (juce::String modelName);
    juce::String getModelTimestamp (int modelIdx);
    void searchPathForModels();
    juce::File getPathToUserModels();
    const std::vector<ModelInfo>& getModelList() const { return models; }

private:
    bool validateModel (ModelInfo modelInfo);
    void showAlertWindow (juce::String modelName, juce::StringArray messages);
    void setPathToUserModels();
    void loadEmbeddedModels();
    void clearUserModels();
    juce::String loadModelTimestamp (const char* modelDataPtr, size_t dataSize);

    std::vector<ModelInfo> models;
    juce::File pathToUserModels;
};

} // namespace ddsp
