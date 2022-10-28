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

/*
The noise is synthesised by filtering uniform white noise with an FIR
filter created using the frequency-sampling technique, where coeffiencents
are generated from the DDSP model. A zero-phase Hann Window is then applied
to the impulse response in the time domain in order to smooth and enhance it.
 
See https://ccrma.stanford.edu/~jos/sasp/Windowing_Desired_Impulse_Response.html
for more details.
 
The filter and noise are generated dynamically for every call to render().
*/

#include "audio/NoiseSynthesizer.h"
#include "util/Constants.h"

namespace ddsp
{

using namespace juce;

NoiseSynthesizer::NoiseSynthesizer (int nna, int nos)
    : impulseResponseSize ((nna - 1) * 2), numOutputSamples (nos), windowFFT (7), convolveFFT (9)
{
    createZeroPhaseHannWindow();
    noiseAudio.resize (numOutputSamples);
    magnitudes.resize (windowFFT.getSize());
    windowedImpulseResponse.resize (convolveFFT.getSize() * 2);
    whiteNoise.resize (convolveFFT.getSize() * 2);
}

const std::vector<float>& NoiseSynthesizer::render (const std::vector<float>& mags)
{
    applyWindowToImpulseResponse (mags);
    convolve();
    return noiseAudio;
}

void NoiseSynthesizer::applyWindowToImpulseResponse (const std::vector<float>& mags)
{
    // Clear and fill complex vector for ifft
    std::fill (magnitudes.begin(), magnitudes.end(), 0.f);
    for (int i = 0; i < mags.size(); i++)
        magnitudes[i].real (mags[i]);

    // Cast complex* to float* for use with JUCE fft
    auto impulseResponse = reinterpret_cast<float*> (magnitudes.data());

    // Obtain impulse response
    windowFFT.performRealOnlyInverseTransform (impulseResponse);

    // Apply the window to the IR
    juce::FloatVectorOperations::multiply (impulseResponse, zpHannWindow.data(), impulseResponseSize);

    // Put into causal form
    std::rotate (impulseResponse, impulseResponse + windowFFT.getSize() / 2, impulseResponse + windowFFT.getSize());

    std::fill (windowedImpulseResponse.begin(), windowedImpulseResponse.end(), 0.f);
    std::copy (impulseResponse, impulseResponse + impulseResponseSize, windowedImpulseResponse.begin());
}

void NoiseSynthesizer::convolve()
{
    for (int i = 0; i < whiteNoise.size(); i++)
        whiteNoise[i] = jmap (random.nextFloat(), -1.f, 1.f);

    convolveFFT.performRealOnlyForwardTransform (whiteNoise.data());
    convolveFFT.performRealOnlyForwardTransform (windowedImpulseResponse.data());

    auto whiteNoiseFreqs = reinterpret_cast<std::complex<float>*> (whiteNoise.data());
    auto impulseResponseFreqs = reinterpret_cast<std::complex<float>*> (windowedImpulseResponse.data());

    // Filter the white noise
    for (int i = 0; i < convolveFFT.getSize() / 2 + 1; i++)
        whiteNoiseFreqs[i] *= impulseResponseFreqs[i];

    convolveFFT.performRealOnlyInverseTransform (whiteNoise.data());

    cropAndCompensateDelay (whiteNoise, impulseResponseSize);
}

void NoiseSynthesizer::cropAndCompensateDelay (const std::vector<float>& inputAudio, int irSize)
{
    // Compensate for the group delay of the filter by trimming the front.
    // The group delay is constant because the filter is linear phase.
    auto start = inputAudio.begin() + ((irSize - 1) / 2 - 1);
    std::copy (start, start + numOutputSamples, noiseAudio.begin());
}

void NoiseSynthesizer::createZeroPhaseHannWindow()
{
    // Create Hann Window
    zpHannWindow.resize (impulseResponseSize);
    for (int i = 0; i < zpHannWindow.size(); i++)
        zpHannWindow[i] = 0.5f * (1.f - cos (MathConstants<float>::twoPi * i / (float) zpHannWindow.size()));

    // Put in zero-phase form
    std::rotate (zpHannWindow.begin(), zpHannWindow.begin() + zpHannWindow.size() / 2, zpHannWindow.end());
}

void NoiseSynthesizer::reset()
{
    std::fill (noiseAudio.begin(), noiseAudio.end(), 0.f);
    std::fill (whiteNoise.begin(), whiteNoise.end(), 0.f);
    std::fill (windowedImpulseResponse.begin(), windowedImpulseResponse.end(), 0.f);
    std::fill (magnitudes.begin(), magnitudes.end(), 0.f);
    random.setSeed (42);
}

} // namespace ddsp
