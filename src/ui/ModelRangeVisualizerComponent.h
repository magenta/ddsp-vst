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
#include "PluginProcessor.h"

class ModelRangeVisualizerComponent;

class DraggableRangeBox : public juce::Component
{
public:
    DraggableRangeBox (ModelRangeVisualizerComponent& o);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    void updateRangeBox();
    juce::Rectangle<float> getModelRangeRect (float pitchOffset, float loudnessOffset);

    std::atomic<bool> isBallInModelRange = { false };

private:
    ModelRangeVisualizerComponent& owner;
    juce::Point<int> lastMousePosition;
};

class ModelRangeVisualizerComponent : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    ModelRangeVisualizerComponent (DDSPAudioProcessor& processor);
    ~ModelRangeVisualizerComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool hitTest (int x, int y) override;
    void drawBackground (juce::Graphics&, const int width, const int height);

    void timerCallback() override;

    void setModelMetadata (const ddsp::PredictControlsModel::Metadata& metadata);
    void modelRangeChanged();

private:
    void changeListenerCallback (juce::ChangeBroadcaster* b) override;

    DDSPAudioProcessor& audioProcessor;
    DraggableRangeBox rangeBox;
    juce::ParameterAttachment pitchOffsetAttach;
    juce::ParameterAttachment loudnessOffsetAttach;

    bool ignoreCallbacks = false;
    float modelPitchRangeMinNorm;
    float modelPitchRangeMaxNorm;
    float modelPitchOffset;
    std::list<float> pitchHistoryBuffer;
    juce::LinearSmoothedValue<float> pitchSmoother;

    float modelLoudnessRangeMinNorm;
    float modelLoudnessRangeMaxNorm;
    float modelLoudnessOffset;
    std::list<float> loudnessHistoryBuffer;
    juce::LinearSmoothedValue<float> loudnessSmoother;

    friend class DraggableRangeBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModelRangeVisualizerComponent)
};
