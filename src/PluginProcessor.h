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

#include "audio/tflite/InferencePipeline.h"

//==============================================================================
/**
*/
class DDSPAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    DDSPAudioProcessor (bool singleThreaded);
    ~DDSPAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadModel (int modelIdx);

    // Getters.
    int getCurrentModel() const;
    float getRMS() const;
    float getPitch() const;
    float getPitchOffset() const;
    float getLoudnessOffset() const;
    const ddsp::PredictControlsModel::Metadata getPredictControlsModelMetadata() const;
    juce::AudioProcessorValueTreeState& getValueTree();
    ddsp::ModelLibrary& getModelLibrary();

private:
    bool singleThreaded = false;
    bool modelLoaded = false;
    int currentModel = 0;

    // Param state.
    juce::AudioProcessorValueTreeState tree;

    ddsp::ModelLibrary modelLibrary;
    ddsp::InferencePipeline ddspPipeline;
    juce::Reverb reverb;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DDSPAudioProcessor)
};
