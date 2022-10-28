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
The harmonic components of the signal are synthesised from DDSP model
outputs - ratio of harmonic series amplitudes, overall amplitude, and
fundamental frequency - via additive synthesis. Initially, the values
are prepared through normalization and then interpolated with previously
generated values. The instantaneous phase is calculated from the frequency
shift and subsequently totaled and wrapped. Values are calculated for
each harmonic at each sample and then summed to yield the final buffer.
*/

#include "audio/HarmonicSynthesizer.h"
#include "util/Constants.h"

namespace ddsp
{

namespace
{
    // Fills the half-interval `[begin, end)` with an arithmetic progression
    // starting at `from` and proceeding so that `end` would attain `to`
    // if it were inclusive (i.e. the last value filled in is less than `to`
    // by the progression's common difference).
    // Example: Filling a 4-element sequence with values from 12 to 20
    //          produces [12, 14, 16, 18]. Note how 20 is next up but missing.
    template <typename Iter>
    void interpolateLinearly (Iter begin, Iter end, float from, float to)
    {
        if (begin == end)
        {
            return;
        }
        const float delta = (to - from) / static_cast<float> (end - begin);
        float value = from;
        for (auto it = begin; it != end; ++it, value += delta)
        {
            *it = value;
        }
    }
} // namespace

using namespace juce;

HarmonicSynthesizer::HarmonicSynthesizer (int nh, int nos, float sr)
    : previousPhase (0.f),
      previousF0 (0.f),
      previousAmplitude (0.f),
      numHarmonics (nh),
      numOutputSamples (nos),
      sampleRate (sr),
      harmonicSeries (numHarmonics),
      harmonicAmplitudes (numHarmonics, std::vector<float> (numOutputSamples, 0)),
      phases (numOutputSamples),
      sinusoids (numOutputSamples, numHarmonics)
{
    previousHarmonicDistribution.resize (numHarmonics);
    frameFrequencies.resize (numHarmonics);
    renderBuffer.resize (numOutputSamples);

    harmonicSeries.resize (numHarmonics);
    frequencyEnvelope.resize (numOutputSamples);
    std::iota (std::begin (harmonicSeries), std::end (harmonicSeries), 1.f);
}

const std::vector<float>&
    HarmonicSynthesizer::render (std::vector<float>& harmonicDistribution, float amplitude, float f0)
{
    normalizeHarmonicDistribution (harmonicDistribution, amplitude, f0);
    previousAmplitude = amplitude;

    // Interpolate frequency envelope, harmonic distribution and store state.
    midwayLerp (previousF0.value_or (f0), f0, frequencyEnvelope);
    previousF0 = f0;

    for (int i = 0; i < numHarmonics; i++)
    {
        midwayLerp (previousHarmonicDistribution[i], harmonicDistribution[i], harmonicAmplitudes[i]);
    }
    previousHarmonicDistribution = harmonicDistribution;

    return synthesizeHarmonics();
}

void HarmonicSynthesizer::normalizeHarmonicDistribution (std::vector<float>& harmonicDistribution,
                                                         float amplitude,
                                                         float f0)
{
    // The DDSP models sometimes predict harmonic values above their nyquist frequency.
    // Here we remove those and normalize the sum to 1.

    // Calculate the frequencies for this frame: f0 x harmonic series.
    FloatVectorOperations::multiply (frameFrequencies.data(), harmonicSeries.data(), f0, numHarmonics);

    // Remove harmonics above Nyquist: this is at the model sample rate, not the DAW sample rate.
    // This step is prior to normalization during training, so we replicate that order here.
    for (int i = 0; i < frameFrequencies.size(); i++)
    {
        if (frameFrequencies[i] >= sampleRate / 2.f)
        {
            harmonicDistribution[i] = 0.f;
        }
    }

    // Normalize so the frequency coeffecients sum up to 1 again.
    auto total = std::accumulate (harmonicDistribution.begin(), harmonicDistribution.end(), 0.f);
    if (total != 0.f)
    {
        FloatVectorOperations::multiply (harmonicDistribution.data(), 1.f / total, numHarmonics);
    }

    FloatVectorOperations::multiply (harmonicDistribution.data(), amplitude, numHarmonics);
}

/*
This method creates sinusoids according to the properties described by the DDSP
model outputs, after which they are summed to create the final waveform.

Description of periodic movement:
y(t) = A sin (ω(t) + φ) = A sin (θ(t))

where:
A is amplitude
ω is frequency in radians
φ is phase offset
θ is instantaneous phase

We are given a series of frequencies in Hz. To get θ(t), we first convert
frequency to radians (ω). Since the instantaneous frequency of a sinusoid
is defined as the derivative of the instantaneous phase (see
https://ccrma.stanford.edu/~jos/fp/Sinusoids.html for more), we can take the
integral of ω(t) at each sample to get θ(t) (you can think of std::partial_sum
as doing a sort of Riemann sum style integration). For each harmonic sinusoid
at each sample t, y(t) is then calculated by taking the sin() and applying
the appropriate amplitude value as calculated by the model. Finally, these
harmonics are added together to create the final wave.
*/
const std::vector<float>& HarmonicSynthesizer::synthesizeHarmonics()
{
    // Generates audio from sample-wise frequencies for a bank of oscillators.

    // Calculate the angular frequency. Hz -> radians per sample.
    FloatVectorOperations::multiply (
        frequencyEnvelope.data(), MathConstants<float>::twoPi / sampleRate, numOutputSamples);

    // Calculate the integration of angular frequency as instantaneous phase.
    std::partial_sum (frequencyEnvelope.begin(), frequencyEnvelope.end(), phases.begin());

    // Add previous total phase.
    FloatVectorOperations::add (phases.data(), previousPhase, numOutputSamples);

    // Wrap and store the total phase.
    previousPhase = fmod (phases.back(), MathConstants<float>::twoPi);

    // Apply phases for each sample to its harmonic series.
    // Apply the appropriate DDSP model amplitudes to each harmonic.
    // `sinusoids` has the shape `numOutputSamples` x `numHarmonics`.
    for (int i = 0; i < sinusoids.getNumRows(); i++)
    {
        for (int j = 0, harmonicOrder = 1; j < sinusoids.getNumColumns(); j++, harmonicOrder++)
        {
            sinusoids (i, j) = (sin (phases[i] * harmonicOrder)) * harmonicAmplitudes[j][i];
        }
    }

    // Sum up the harmonics for each timestep.
    for (int i = 0; i < sinusoids.getNumRows(); i++)
    {
        auto rowStart = sinusoids.begin() + (i * sinusoids.getNumColumns());
        auto rowEnd = rowStart + sinusoids.getNumColumns();
        renderBuffer[i] = std::accumulate (rowStart, rowEnd, 0.f);
    }

    return renderBuffer;
}

void HarmonicSynthesizer::midwayLerp (float first, float last, std::vector<float>& result)
{
    // This interpolation is a mix between linear and nearest neighbor, with the first half
    // being linear between the two given values and the last half repeating the last value.
    // This type of interpolation was chosen over a simple linear approach due to "swooping"
    // artifacts generated over the 20ms hop size when the two endpoint values are sufficiently
    // far apart.
    const auto middle = result.begin() + result.size() / 2;
    interpolateLinearly (result.begin(), middle, first, last);
    std::fill (middle, result.end(), last);
}

void HarmonicSynthesizer::reset()
{
    previousPhase = 0;
    previousF0.reset();
    previousAmplitude = 0;
    sinusoids.clear();
    harmonicAmplitudes = std::vector<std::vector<float>> (numHarmonics, std::vector<float> (numOutputSamples, 0.0f));

    std::fill (previousHarmonicDistribution.begin(), previousHarmonicDistribution.end(), 0.f);
    std::fill (frameFrequencies.begin(), frameFrequencies.end(), 0.f);
    std::fill (frequencyEnvelope.begin(), frequencyEnvelope.end(), 0.f);
    std::fill (phases.begin(), phases.end(), 0.f);
    std::fill (renderBuffer.begin(), renderBuffer.end(), 0.f);
}

} // namespace ddsp
