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

#include "PluginEditor.h"
#include "PluginProcessor.h"

DDSPAudioProcessorEditor::DDSPAudioProcessorEditor (DDSPAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), contentComponent (p)
{
    addAndMakeVisible (contentComponent);

           setSize (656, 672);
    setResizable (true, true);
    setResizeLimits (656, 624, 1200, 800);
}

DDSPAudioProcessorEditor::~DDSPAudioProcessorEditor() {}

void DDSPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    contentComponent.setBounds (area);
}
