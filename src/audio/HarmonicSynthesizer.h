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

#include <optional>

#include "JuceHeader.h"

namespace ddsp
{

class HarmonicSynthesizer
{
public:
    HarmonicSynthesizer (int numHarmonics, int numOutputSamples, float sampleRate);

    // Clears all internal scratch buffers and state variables.
    void reset();

    const std::vector<float>& render (std::vector<float>& harmonicDistribution, float amplitude, float f0);

private:
    void normalizeHarmonicDistribution (std::vector<float>& harmonicDistribution, float amplitude, float f0);
    const std::vector<float>& synthesizeHarmonics();
    void midwayLerp (float first, float last, std::vector<float>& result);

    // Harmonic synthesizer state-related variables.
    std::vector<float> previousHarmonicDistribution;
    float previousPhase;
    std::optional<float> previousF0;
    float previousAmplitude;

    int numHarmonics, numOutputSamples;
    float sampleRate;
    std::vector<float> harmonicSeries, frequencyEnvelope, frameFrequencies, phases, renderBuffer;
    std::vector<std::vector<float>> harmonicAmplitudes;
    juce::dsp::Matrix<float> sinusoids;
};

} // namespace ddsp
