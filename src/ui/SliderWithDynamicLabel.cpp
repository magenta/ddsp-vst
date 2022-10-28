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

#include "ui/SliderWithDynamicLabel.h"

//==============================================================================
SliderWithDynamicLabel::SliderWithDynamicLabel (const ParamInfo& i)
{
    info = i;

    slider.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (slider.get());
    slider->setRange (info.rangeMin, info.rangeMax, info.rangeInterval);
    slider->setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    slider->setColour (juce::Slider::backgroundColourId, juce::Colour (0x136236ff));
    slider->setColour (juce::Slider::thumbColourId, juce::Colour (0xff6236ff));
    slider->setColour (juce::Slider::trackColourId, juce::Colour (0x706236ff));
    slider->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0x706236ff));
    slider->setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0x136236ff));

    label.reset (new juce::Label ("new label", TRANS (info.paramName)));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (kFont, kTextSize, juce::Font::plain)
                        .withTypefaceStyle ("Regular")
                        .withExtraKerningFactor (kKerningFactor));
    label->setJustificationType (juce::Justification::centredTop);
    label->setEditable (false, false, false);
    label->setColour (juce::Label::textColourId, juce::Colour (DDSPColourPalette::kLabelTextColour));
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    slider->setLookAndFeel (&lookAndFeel);
    slider->addListener (this);

    updateLabel();
}

SliderWithDynamicLabel::~SliderWithDynamicLabel()
{
    slider->removeListener (this);
    slider = nullptr;
    label = nullptr;
}

void SliderWithDynamicLabel::paint (juce::Graphics& g) {}

void SliderWithDynamicLabel::resized()
{
    const auto margin = 48.0f;
    auto localArea = getLocalBounds();
    localArea.removeFromTop (8.0f);
    slider->setBounds (localArea.removeFromTop (getWidth() - margin));
    label->setBounds (localArea);
}

void SliderWithDynamicLabel::sliderValueChanged (juce::Slider* slider) { updateLabel(); }

void SliderWithDynamicLabel::updateLabel()
{
    juce::String labelText;
    // Hack to fix weird bug that shows -0.0f as the initial value.
    float value = slider->getValue();
    if (info.paramID.equalsIgnoreCase ("OutputGain"))
    {
        value += 0.001f;
    }

    if (info.isIntParam)
        labelText = juce::String (static_cast<int> (value)) + " " + info.unit + "\n" + info.paramName;
    else
        labelText = juce::String (value, info.numDecimalPlaces) + " " + info.unit + "\n" + info.paramName;

    label->setText (labelText, juce::dontSendNotification);
}
