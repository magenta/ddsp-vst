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

#include "audio/AudioRingBuffer.h"
#include "audio/HarmonicSynthesizer.h"
#include "audio/MidiInputProcessor.h"
#include "audio/NoiseSynthesizer.h"
#include "audio/tflite/FeatureExtractionModel.h"
#include "audio/tflite/ModelBase.h"
#include "audio/tflite/ModelLibrary.h"
#include "audio/tflite/PredictControlsModel.h"

namespace ddsp
{

class InferencePipeline : public juce::HighResolutionTimer
{
public:
    InferencePipeline (juce::AudioProcessorValueTreeState& t);
    ~InferencePipeline() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void reset();

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void getNextBlock (juce::AudioBuffer<float>& bufferToFill);
    void render();
    void hiResTimerCallback() override;

    void loadModel (const ModelInfo& mi);

    float getRMS() const;
    float getPitch() const;

private:
    int userFrameSize = 0;
    int userHopSize = 0;
    double sampleRate = 0.0;

    std::atomic<float> currentPitch = { 0.0f };
    std::atomic<float> currentRMS = { 0.0f };
    std::atomic<bool> swappingModel = false;

    void changeModel();

    // Param state.
    juce::AudioProcessorValueTreeState& tree;

    // DSP components.
    juce::WindowedSincInterpolator inputInterpolator;
    juce::WindowedSincInterpolator outputInterpolator;

    // Scratch buffers.
    juce::AudioBuffer<float> modelInputBuffer;
    juce::AudioBuffer<float> synthesisBuffer;
    juce::AudioBuffer<float> resampledModelInputBuffer;
    juce::AudioBuffer<float> resampledModelOutputBuffer;

    // FIFOs.
    AudioRingBuffer inputRingBuffer;
    AudioRingBuffer outputRingBuffer;

    // TF models.
    std::unique_ptr<FeatureExtractionModel> featureExtractionModel;
    std::unique_ptr<PredictControlsModel> currentPredictControlsModel;
    std::unique_ptr<PredictControlsModel> nextPredictControlsModel;

    // Synthesis.
    NoiseSynthesizer noiseSynthesizer;
    HarmonicSynthesizer harmonicSynthesizer;
    AudioFeatures predictControlsInput;
    SynthesisControls synthesisInput;

    // MIDI input.
    MidiInputProcessor midiInputProcessor;
};

} // namespace ddsp
