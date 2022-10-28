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

#include "JuceHeader.h"

#include "ui/RadioButtonGroupGomponent.h"

constexpr int radioGroupId = 1001;

//==============================================================================
RadioButtonGroupGomponent::RadioButtonGroupGomponent (const std::vector<juce::String>& buttonNames)
{
    int buttonIdx = 0;
    for (const auto& name : buttonNames)
    {
        auto button = std::make_unique<juce::TextButton> (name);

        if (buttonIdx == 0)
        {
            button->setConnectedEdges (juce::Button::ConnectedOnRight);
        }
        else if (buttonIdx == buttonNames.size() - 1)
        {
            button->setConnectedEdges (juce::Button::ConnectedOnLeft);
        }
        else
        {
            button->setConnectedEdges (juce::Button::ConnectedOnRight | juce::Button::ConnectedOnLeft);
        }

        button->setLookAndFeel (&lookAndFeel);
        button->setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colour (DDSPColourPalette::kGrey));
        button->setColour (juce::TextButton::ColourIds::buttonOnColourId,
                           juce::Colour (DDSPColourPalette::kMagenta).withMultipliedAlpha (0.13));
        button->setColour (juce::TextButton::ColourIds::textColourOnId,
                           juce::Colour (DDSPColourPalette::kLabelTextColour));
        button->setColour (juce::TextButton::ColourIds::textColourOffId,
                           juce::Colour (DDSPColourPalette::kLabelTextColour));
        button->setColour (juce::ComboBox::ColourIds::outlineColourId, juce::Colour (DDSPColourPalette::kDarkerGrey));

        button->setRadioGroupId (radioGroupId);

        auto buttonPtr = button.get();
        button->onClick = [this, buttonPtr, name] { updateToggleState (buttonPtr, name); };

        addAndMakeVisible (button.get());
        buttons.push_back (std::move (button));

        ++buttonIdx;
    }

    // Set first tab to true, make sure the parent component handles the initial case when constructed.
    jassert (! buttons.empty());

    if (! buttons.empty())
        buttons[0]->setToggleState (true, juce::dontSendNotification);
}

RadioButtonGroupGomponent::~RadioButtonGroupGomponent() { listeners.clear(); }

void RadioButtonGroupGomponent::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0x00000000)); }

void RadioButtonGroupGomponent::resized()
{
    juce::FlexBox buttonFlex;
    buttonFlex.flexWrap = juce::FlexBox::Wrap::noWrap;
    buttonFlex.flexDirection = juce::FlexBox::Direction::row;
    buttonFlex.justifyContent = juce::FlexBox::JustifyContent::center;

    const float buttonWidth = 120.0f;
    const float buttonHeight = 30.0f;

    for (const auto& button : buttons)
    {
        buttonFlex.items.add (juce::FlexItem (*button)
                                  .withWidth (buttonWidth)
                                  .withHeight (buttonHeight)
                                  .withAlignSelf (juce::FlexItem::AlignSelf::flexStart));
    }

    buttonFlex.performLayout (getLocalBounds().toFloat());
}

void RadioButtonGroupGomponent::updateToggleState (juce::Button* button, const juce::String& name)
{
    if (! button->getToggleState())
    {
        button->setToggleState (true, juce::dontSendNotification);
        listeners.call ([name] (Listener& l) { l.selectionChanged (name); });
    }
}
