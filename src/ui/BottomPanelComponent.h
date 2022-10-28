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
#include "ui/DDSPLookAndFeel.h"
#include "ui/RadioButtonGroupGomponent.h"
#include "ui/SliderWithDynamicLabel.h"

using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;

//==============================================================================
/*
*/
class BottomPanelComponent : public juce::Component, public RadioButtonGroupGomponent::Listener
{
public:
    BottomPanelComponent (DDSPAudioProcessor& p);
    ~BottomPanelComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void selectionChanged (const juce::String& buttonName) override;

private:
    void showSliderGroup (const juce::String& groupName);

    DDSPAudioProcessor& audioProcessor;

    std::map<juce::String, std::vector<std::unique_ptr<SliderWithDynamicLabel>>> sliderGroups;
    std::vector<std::unique_ptr<SliderAttach>> sliderAttachments;
    std::unique_ptr<RadioButtonGroupGomponent> buttonGroup;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomPanelComponent)
};
