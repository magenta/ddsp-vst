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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/ParamInfo.h"
#include "util/InputUtils.h"

using namespace ddsp;

//==============================================================================
DDSPAudioProcessor::DDSPAudioProcessor (bool st)
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                          ),
#endif
      singleThreaded (st),
      tree (*this, nullptr, "PARAMETERS", createParameterLayout()),
      ddspPipeline (tree)
{
    ddspPipeline.reset();
}

DDSPAudioProcessor::~DDSPAudioProcessor() {}

//==============================================================================
const juce::String DDSPAudioProcessor::getName() const { return JucePlugin_Name; }

bool DDSPAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DDSPAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DDSPAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DDSPAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DDSPAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int DDSPAudioProcessor::getCurrentProgram() { return 0; }

void DDSPAudioProcessor::setCurrentProgram (int index) {}

const juce::String DDSPAudioProcessor::getProgramName (int index) { return {}; }

void DDSPAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

//==============================================================================
void DDSPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // The pitch detection model needs a full 64ms frame to get an accurate reading.
    // Therefore the pulgin latency is set to 64ms.
    setLatencySamples ((kTotalInferenceLatency_ms / 1000.0f) * sampleRate);

    reverb.setSampleRate (sampleRate);

    ddspPipeline.prepareToPlay (sampleRate, samplesPerBlock);

    if (isNonRealtime())
    {
        DBG ("PrepareToPlay non real time");
        ddspPipeline.reset();
    }
    else
    {
        DBG ("PrepareToPlay realtime");
        loadModel (currentModel);
    }

    if (! singleThreaded)
    {
        ddspPipeline.startTimer (kModelInferenceTimerCallbackInterval_ms);
    }
}

void DDSPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

    // Only engage timer thread if not in single-threaded mode.
    if (! singleThreaded)
    {
        ddspPipeline.stopTimer();
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DDSPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void DDSPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (! modelLoaded)
    {
        buffer.clear();
        return;
    }

    juce::ScopedNoDenormals noDenormals;

    // Mono mixdown.
    if (buffer.getNumChannels() > 1)
    {
        buffer.applyGain (0.5f);
        buffer.addFrom (0, 0, buffer, 1, 0, buffer.getNumSamples());
    }

    // Enqueue input.
    ddspPipeline.processBlock (buffer, midiMessages);
    buffer.clear();

    // Synchronous model inference block.
    if (singleThreaded || isNonRealtime())
    {
        // We have to stop the timer here and not in PrepareToPlay so it will block the
        // Audio thread until it is done with the last timer callback.
        ddspPipeline.stopTimer();
        ddspPipeline.render();
    }

    ddspPipeline.getNextBlock (buffer);

    // Convert to stereo.
    if (buffer.getNumChannels() > 1)
    {
        buffer.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
    }

    // Post-processing: modify output gain + add reverb.
    {
        const float outputGain = *tree.getRawParameterValue ("OutputGain");
        // Apply overall gain.
        buffer.applyGain (juce::Decibels::decibelsToGain (outputGain));

        if (kEnableReverb)
        {
            juce::Reverb::Parameters reverbParams;
            reverbParams.roomSize = *tree.getRawParameterValue ("ReverbSize");
            reverbParams.damping = *tree.getRawParameterValue ("ReverbDamping");
            reverbParams.wetLevel = *tree.getRawParameterValue ("ReverbWet");
            reverbParams.dryLevel = 1.0f;
            reverb.setParameters (reverbParams);

            if (buffer.getNumChannels() == 1)
                reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
            else if (buffer.getNumChannels() == 2)
                reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool DDSPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DDSPAudioProcessor::createEditor() { return new DDSPAudioProcessorEditor (*this); }

//==============================================================================
void DDSPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement parentXML ("Parent");
    // Add plugin params to XML.
    auto state = tree.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    parentXML.addChildElement (xml.release());

    // Add model timestamp to XML.
    juce::XmlElement* modelTimestamp = parentXML.createNewChildElement ("modelTimestamp");
    modelTimestamp->setAttribute ("timestamp", modelLibrary.getModelTimestamp (currentModel));
    DBG ("Parameter count: " + juce::String (parentXML.getNumChildElements()));
    DBG (parentXML.toString());

    copyXmlToBinary (parentXML, destData);
}

void DDSPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    juce::XmlElement* paramsXML = xmlState->getChildByName (tree.state.getType());
    juce::XmlElement* modelTimestampXML = xmlState->getChildByName ("modelTimestamp");

    if (xmlState.get() != nullptr)
        DBG ("Parameter count: " + juce::String (xmlState->getNumChildElements()));
    DBG (xmlState->toString());
    {
        if (paramsXML != nullptr)
        {
            tree.replaceState (juce::ValueTree::fromXml (*paramsXML));
        }
        else
        {
            // This is a temp fix so that the plugin doesn't crash on loading a state from an earlier version.
            if (xmlState->hasTagName (tree.state.getType()))
                tree.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
        if (modelTimestampXML != nullptr)
        {
            auto currentModelTimestamp = modelTimestampXML->getStringAttribute ("timestamp");
            currentModel = modelLibrary.getModelIdx (currentModelTimestamp);
            loadModel (currentModel);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DDSPAudioProcessor (kInferenceOnAudioThread); }

// ----------------------------------------- PUBLIC METHODS ----------------------------------------

void DDSPAudioProcessor::loadModel (int modelIdx)
{
    modelLoaded = false;
    ddspPipeline.loadModel (modelLibrary.getModelList()[modelIdx]);
    currentModel = modelIdx;
    modelLoaded = true;
}

// ----------------------------------------- GETTER METHODS ----------------------------------------

int DDSPAudioProcessor::getCurrentModel() const { return currentModel; }

float DDSPAudioProcessor::getRMS() const { return ddspPipeline.getRMS(); }

float DDSPAudioProcessor::getPitch() const { return ddspPipeline.getPitch(); }

float DDSPAudioProcessor::getPitchOffset() const { return *tree.getRawParameterValue ("InputPitch"); }

float DDSPAudioProcessor::getLoudnessOffset() const { return *tree.getRawParameterValue ("InputGain"); }

const PredictControlsModel::Metadata DDSPAudioProcessor::getPredictControlsModelMetadata() const
{
    return PredictControlsModel::getMetadata (modelLibrary.getModelList()[currentModel]);
}

juce::AudioProcessorValueTreeState& DDSPAudioProcessor::getValueTree() { return tree; }

ModelLibrary& DDSPAudioProcessor::getModelLibrary() { return modelLibrary; }
