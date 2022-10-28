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

#include "ui/ContentComponent.h"

static void setVerticalRotatedWithBounds (juce::Component& component,
                                          bool clockWiseRotation,
                                          juce::Rectangle<int> verticalBounds)
{
    auto angle = juce::MathConstants<float>::pi / 2.0f;

    if (! clockWiseRotation)
        angle *= -1.0f;

    component.setTransform ({});
    component.setSize (verticalBounds.getHeight(), verticalBounds.getWidth());
    component.setCentrePosition (0, 0);
    component.setTransform (
        juce::AffineTransform::rotation (angle).translated (verticalBounds.getCentreX(), verticalBounds.getCentreY()));
}

ContentComponent::ContentComponent (DDSPAudioProcessor& audioProcessor)
    : audioProcessor (audioProcessor),
      topPanel (audioProcessor),
      modelVisualizer (audioProcessor),
      bottomPanel (audioProcessor)
{
    pitchLabel.reset (new juce::Label ("new label", TRANS ("Pitch")));
    addAndMakeVisible (pitchLabel.get());
    pitchLabel->setFont (juce::Font ("Google Sans", kTextSize, juce::Font::plain)
                             .withTypefaceStyle ("Regular")
                             .withExtraKerningFactor (0.025f));
    pitchLabel->setJustificationType (juce::Justification::centred);
    pitchLabel->setEditable (false, false, false);
    pitchLabel->setColour (juce::Label::textColourId, juce::Colour (juce::Colour (kLabelTextColour)));

    pitchLabel->setBounds (248, 424, 96, 24);

    loudnessLabel.reset (new juce::Label ("new label", TRANS ("Volume")));
    addAndMakeVisible (loudnessLabel.get());
    loudnessLabel->setFont (juce::Font ("Google Sans", kTextSize, juce::Font::plain)
                                .withTypefaceStyle ("Regular")
                                .withExtraKerningFactor (0.025f));
    loudnessLabel->setJustificationType (juce::Justification::centred);
    loudnessLabel->setEditable (false, false, false);
    loudnessLabel->setColour (juce::Label::textColourId, juce::Colour (juce::Colour (kLabelTextColour)));

    loudnessLabel->setBounds (8, 248, 96, 24);

    loudnessLabel->setFont (juce::Font (kFont, kTextSize, juce::Font::plain)
                                .withTypefaceStyle ("Regular")
                                .withExtraKerningFactor (kKerningFactor));
    pitchLabel->setFont (juce::Font (kFont, kTextSize, juce::Font::plain)
                             .withTypefaceStyle ("Regular")
                             .withExtraKerningFactor (kKerningFactor));
    addAndMakeVisible (&topPanel);
    addAndMakeVisible (&bottomPanel);
    addAndMakeVisible (&modelVisualizer);

    topPanel.addChangeListener (&modelVisualizer);

    setSize (656, 634);
}

ContentComponent::~ContentComponent()
{
    topPanel.removeAllChangeListeners();

    pitchLabel = nullptr;
    loudnessLabel = nullptr;
}

//==============================================================================
void ContentComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);

    // Model visualization background.
    auto localArea = middlePanelArea.reduced (kPadding, kPadding / 2);

    juce::Path middle;
    middle.addRoundedRectangle (localArea.getX(),
                                localArea.getY(),
                                localArea.getWidth(),
                                localArea.getHeight(),
                                kRoundedRectangleCornerSize,
                                kRoundedRectangleCornerSize,
                                true,
                                true,
                                true,
                                true);
    juce::DropShadow dsMiddle (juce::Colours::black.withMultipliedAlpha (0.4f), 4, juce::Point<int> (0, 0));
    dsMiddle.drawForPath (g, middle);
    g.setColour (juce::Colour (DDSPColourPalette::kGrey));
    g.fillPath (middle);
}

void ContentComponent::resized()
{
    constexpr int topPanelHeight = 165;
    constexpr int bottomPanelHeight = 200;

    juce::Rectangle<int> localArea (getLocalBounds());

    localArea.removeFromTop (kPadding / 2.0f);
    localArea.removeFromBottom (kPadding / 2.0f);
    juce::Rectangle<int> topPanelArea (localArea.removeFromTop (topPanelHeight));
    juce::Rectangle<int> bottomPanelArea (localArea.removeFromBottom (bottomPanelHeight));
    middlePanelArea = localArea;
    juce::Rectangle<int> modelVisualizerArea = localArea.reduced (kPadding / 2.0f, 0.0f);

    topPanel.setBounds (topPanelArea);
    bottomPanel.setBounds (bottomPanelArea);

    pitchLabel->setBounds (modelVisualizerArea.removeFromBottom (40));
    setVerticalRotatedWithBounds (*loudnessLabel, false, modelVisualizerArea.removeFromLeft (60));
    modelVisualizerArea.removeFromTop (40);
    modelVisualizerArea.removeFromRight (30);
    modelVisualizer.setBounds (modelVisualizerArea);
}
