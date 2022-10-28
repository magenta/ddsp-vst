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

class NoiseSynthesizer
{
public:
    NoiseSynthesizer (int numNoiseAmplitudes, int numOutputSamples);

    // Clears all internal scratch buffers and state variables.
    void reset();

    const std::vector<float>& render (const std::vector<float>& mags);

private:
    void createZeroPhaseHannWindow();

    void applyWindowToImpulseResponse (const std::vector<float>& mags);
    void convolve();
    void cropAndCompensateDelay (const std::vector<float>& audio, int irSize);

    std::vector<float> zpHannWindow, noiseAudio, windowedImpulseResponse, whiteNoise;
    std::vector<std::complex<float>> magnitudes;

    const int impulseResponseSize, numOutputSamples;

    juce::dsp::FFT windowFFT, convolveFFT;
    juce::Random random;
};

} // namespace ddsp
