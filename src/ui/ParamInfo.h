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

struct ParamInfo
{
    juce::String paramID;
    juce::String paramName;
    // Slider-related variables.
    float rangeMin;
    float rangeMax;
    float defaultValue;
    float rangeInterval;
    bool isIntParam;
    int numDecimalPlaces;
    juce::String unit;
};

std::map<juce::String, std::vector<ParamInfo>> getSliderParamsInfo();

// Creates and adds AudioProcessor params to the value tree state.
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
