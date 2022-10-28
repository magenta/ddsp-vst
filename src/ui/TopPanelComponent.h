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
#include "ui/ModelRangeVisualizerComponent.h"

class TopPanelComponent : public juce::Component, public juce::ChangeBroadcaster
{
public:
    TopPanelComponent (DDSPAudioProcessor& p);
    ~TopPanelComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    DDSPAudioProcessor& audioProcessor;
    DDSPLookAndFeel lookAndFeel;
    RefreshButtonLookAndFeel refreshButtonLookAndFeel;
    OpenFolderButtonLookAndFeel openFolderButtonLookAndFeel;
    InfoButtonLookAndFeel infoButtonLookAndFeel;

    void openFileBrowser();
    void refreshModels();
    void fillComboBox();
    void changeDDSPModel();

    std::unique_ptr<juce::ComboBox> modelList;
    std::unique_ptr<juce::Drawable> ddspLogo;
    std::unique_ptr<juce::Label> modelsLabel;
    std::unique_ptr<juce::Label> versionLabel;
    std::unique_ptr<juce::TextButton> customModelsButton;
    std::unique_ptr<juce::TextButton> refreshButton;
    std::unique_ptr<juce::TextButton> urlButton;
    std::unique_ptr<juce::TextButton> infoButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopPanelComponent)
};
