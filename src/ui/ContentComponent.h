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
#include "ui/BottomPanelComponent.h"
#include "ui/ModelRangeVisualizerComponent.h"
#include "ui/TopPanelComponent.h"

class ContentComponent : public juce::Component
{
public:
    ContentComponent (DDSPAudioProcessor& audioProcessor);
    ~ContentComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void updateModelMetadata();
    DDSPAudioProcessor& audioProcessor;
    TopPanelComponent topPanel;
    juce::Rectangle<int> middlePanelArea;
    ModelRangeVisualizerComponent modelVisualizer;
    BottomPanelComponent bottomPanel;

    std::unique_ptr<juce::Label> pitchLabel;
    std::unique_ptr<juce::Label> loudnessLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComponent)
};
