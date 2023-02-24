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

#include "audio/tflite/InferencePipeline.h"
#include "util/InputUtils.h"

namespace ddsp
{

InferencePipeline::InferencePipeline (juce::AudioProcessorValueTreeState& t)
    : tree (t),
      inputRingBuffer (/*size=*/61440),
      outputRingBuffer (/*size=*/61440),
      noiseSynthesizer (kNoiseAmpsSize, kModelHopSize),
      harmonicSynthesizer (kHarmonicsSize, kModelHopSize, kModelSampleRate_Hz)
{
    featureExtractionModel = std::make_unique<FeatureExtractionModel>();
}

InferencePipeline::~InferencePipeline() {}

void InferencePipeline::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;

    // Calculate the hopsize and framesize of the model at the
    // user's sample rate in order to load the holding input buffer.

    // The frame size is the amount that is copied for downsampling and is
    // independent of the amount that is removed from the buffer after processing (hopsize).
    // Use ceil to ensure there are enough samples to correctly perform downsampling later.
    userFrameSize = static_cast<int> (ceil (sampleRate * kModelFrameSize / kModelSampleRate_Hz));

    // If the hopsize of the model does not convert evenly to the user's sample rate, logic
    // must be written to handle this when advancing the pointer in the holding input buffer.
    // E.g. 44100 * 320 / 16000 = 882
    // As is, this works with standard sample rates of 44.1k, 48k, 88.2k, 96k, 176.4k and 192k.
    userHopSize = static_cast<int> (sampleRate * kModelHopSize / kModelSampleRate_Hz);

    DBG ("User Sample Rate: " << sampleRate);
    DBG ("User Frame Size: " << userFrameSize);
    DBG ("User Hop Size: " << userHopSize);

    modelInputBuffer.setSize (1, userFrameSize);
    resampledModelInputBuffer.setSize (1, kModelFrameSize);
    synthesisBuffer.setSize (1, kModelHopSize);
    resampledModelOutputBuffer.setSize (1, userHopSize);

    midiInputProcessor.prepareToPlay (sampleRate, userHopSize);

    reset();
}

void InferencePipeline::reset()
{
    if (currentPredictControlsModel)
    {
        currentPredictControlsModel->reset();
    }

    noiseSynthesizer.reset();
    harmonicSynthesizer.reset();

    modelInputBuffer.clear();
    synthesisBuffer.clear();
    resampledModelInputBuffer.clear();
    resampledModelOutputBuffer.clear();

    inputRingBuffer.clear();
    // Zero pad.
    if (userFrameSize > 0)
    {
        juce::AudioBuffer<float> zeroBuf (1, userFrameSize);
        zeroBuf.clear();
        // Do we need to reset input/outputRingBuffer here?
        inputRingBuffer.push (zeroBuf);
    }

    outputRingBuffer.clear();

    inputInterpolator.reset();
    outputInterpolator.reset();
}

void InferencePipeline::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (JucePlugin_IsSynth)
    {
        midiInputProcessor.processMidiMessages (midiMessages);

        // TODO: move this to slider callback
        midiInputProcessor.setAttack (*tree.getRawParameterValue ("Attack"));
        midiInputProcessor.setDecay (*tree.getRawParameterValue ("Decay"));
        midiInputProcessor.setSustain (*tree.getRawParameterValue ("Sustain"));
        midiInputProcessor.setRelease (*tree.getRawParameterValue ("Release"));
    }

    inputRingBuffer.push (buffer);
}

void InferencePipeline::getNextBlock (juce::AudioBuffer<float>& bufferToFill)
{
    if (outputRingBuffer.getNumReady() >= bufferToFill.getNumSamples())
    {
        outputRingBuffer.copy (bufferToFill);
        outputRingBuffer.pop (bufferToFill.getNumSamples());
    }
    else
    {
        DBG ("Not enough samples (this rarely happens)");
    }
}

void InferencePipeline::render()
{
    if (swappingModel)
    {
        currentPredictControlsModel = std::move (nextPredictControlsModel);
        swappingModel = false;
    }

    while (inputRingBuffer.getNumReady() >= userFrameSize)
    {
        if (JucePlugin_IsSynth)
        {
            predictControlsInput = midiInputProcessor.getCurrentPredictControlsInput();
        }
        else
        {
            // 2a: Downsample user frame's worth of input buffer.
            jassert (modelInputBuffer.getNumSamples() == userFrameSize);
            inputRingBuffer.copy (modelInputBuffer);
            inputInterpolator.process (sampleRate / kModelSampleRate_Hz,
                                       modelInputBuffer.getReadPointer (0),
                                       resampledModelInputBuffer.getWritePointer (0),
                                       resampledModelInputBuffer.getNumSamples());
            jassert (resampledModelInputBuffer.getNumSamples() == kModelFrameSize);

            // 2b: Run through the model.
            featureExtractionModel->call (resampledModelInputBuffer, predictControlsInput);
        }

        // Shift the pitch before the UI and model.
        predictControlsInput.f0_hz =
            offsetPitch (predictControlsInput.f0_hz, *tree.getRawParameterValue ("PitchShift"));
        predictControlsInput.f0_norm = normalizedPitch (predictControlsInput.f0_hz);

        // Store and scale the normalized pitch and loudness.
        currentPitch.store (predictControlsInput.f0_norm);
        currentRMS.store (predictControlsInput.loudness_norm);
        predictControlsInput.f0_norm -= *tree.getRawParameterValue ("InputPitch");
        predictControlsInput.loudness_norm -= *tree.getRawParameterValue ("InputGain");

        currentPredictControlsModel->call (predictControlsInput, synthesisInput);

        synthesisInput.amplitude *= *tree.getRawParameterValue ("HarmonicGain");
        juce::FloatVectorOperations::multiply (
            synthesisInput.noiseAmps.data(), *tree.getRawParameterValue ("NoiseGain"), synthesisInput.noiseAmps.size());

        const auto& harmonicOutput =
            harmonicSynthesizer.render (synthesisInput.harmonics, synthesisInput.amplitude, synthesisInput.f0_hz);

        const auto& noiseOutput = noiseSynthesizer.render (synthesisInput.noiseAmps);

        for (int i = 0; i < synthesisBuffer.getNumSamples(); ++i)
        {
            synthesisBuffer.getWritePointer (0)[i] = harmonicOutput[i] + noiseOutput[i];
        }

        outputInterpolator.process (kModelSampleRate_Hz / sampleRate,
                                    synthesisBuffer.getReadPointer (0),
                                    resampledModelOutputBuffer.getWritePointer (0),
                                    resampledModelOutputBuffer.getNumSamples());
        // 2d: Enqueue to outputRingBuffer.
        outputRingBuffer.push (resampledModelOutputBuffer);
        // 2e: Dequeue hop size samples from input buffer.
        inputRingBuffer.pop (userHopSize);
    }
}

void InferencePipeline::hiResTimerCallback() { render(); }

void InferencePipeline::loadModel (const ModelInfo& mi)
{
    nextPredictControlsModel = std::make_unique<PredictControlsModel> (mi);
    swappingModel = true;
}

float InferencePipeline::getRMS() const { return currentRMS.load(); }

float InferencePipeline::getPitch() const { return currentPitch.load(); }

} // namespace ddsp
