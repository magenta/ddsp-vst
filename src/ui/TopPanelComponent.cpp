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

#include "ui/TopPanelComponent.h"
#include "util/Constants.h"

using namespace ddsp;

TopPanelComponent::TopPanelComponent (DDSPAudioProcessor& p) : audioProcessor (p)
{
    modelList.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (modelList.get());
    modelList->setEditableText (false);
    modelList->setJustificationType (juce::Justification::centredLeft);
    modelList->setTextWhenNothingSelected (juce::String());
    modelList->setTextWhenNoChoicesAvailable (TRANS ("(no choices)"));
    modelList->onChange = [this] { changeDDSPModel(); };

    modelList->setLookAndFeel (&lookAndFeel);
    modelList->setColour (juce::ComboBox::backgroundColourId, juce::Colours::white);
    modelList->setColour (juce::ComboBox::outlineColourId, juce::Colour (DDSPColourPalette::kDarkGrey));
    modelList->setColour (juce::ComboBox::textColourId, juce::Colour (DDSPColourPalette::kLabelTextColour));
    modelList->setColour (juce::ComboBox::arrowColourId, juce::Colour (DDSPColourPalette::kMagenta));

    fillComboBox();
    modelList->setSelectedId (audioProcessor.getCurrentModel() + 1, juce::dontSendNotification);

    lookAndFeel.setColour (juce::PopupMenu::backgroundColourId, juce::Colours::white);
    lookAndFeel.setColour (juce::PopupMenu::highlightedBackgroundColourId,
                           juce::Colour (DDSPColourPalette::kMagenta).withMultipliedAlpha (0.13f));
    lookAndFeel.setColour (juce::PopupMenu::textColourId, juce::Colour (DDSPColourPalette::kLabelTextColour));
    lookAndFeel.setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (DDSPColourPalette::kMagenta));

    ddspLogo = juce::Drawable::createFromImageData (BinaryData::logo_thin_svg, BinaryData::logo_thin_svgSize);
    addAndMakeVisible (ddspLogo.get());

    const auto versionString = "Version " + juce::String (ProjectInfo::versionString);
    versionLabel.reset (new juce::Label ("new label", TRANS (versionString)));
    addAndMakeVisible (versionLabel.get());
    versionLabel->setFont (juce::Font (kFont, 10.00f, juce::Font::plain).withExtraKerningFactor (kKerningFactor));
    versionLabel->setJustificationType (juce::Justification::right);
    versionLabel->setEditable (false, false, false);
    versionLabel->setColour (juce::Label::textColourId,
                             juce::Colour (DDSPColourPalette::kGrey).withMultipliedAlpha (0.5f));

    customModelsButton.reset (new juce::TextButton (""));
    addAndMakeVisible (customModelsButton.get());
    customModelsButton->setLookAndFeel (&openFolderButtonLookAndFeel);
    customModelsButton->onClick = [this] { openFileBrowser(); };
    customModelsButton->setColour (juce::TextButton::ColourIds::buttonColourId,
                                   juce::Colour (DDSPColourPalette::kMagenta));

    refreshButton.reset (new juce::TextButton (""));
    addAndMakeVisible (refreshButton.get());
    refreshButton->setLookAndFeel (&refreshButtonLookAndFeel);
    refreshButton->onClick = [this] { refreshModels(); };
    refreshButton->setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colour (DDSPColourPalette::kMagenta));

    urlButton.reset (new juce::TextButton ("Train New Model"));
    addAndMakeVisible (urlButton.get());
    urlButton->setLookAndFeel (&lookAndFeel);
    urlButton->setColour (juce::TextButton::ColourIds::buttonColourId, juce::Colour (DDSPColourPalette::kMagenta));
    urlButton->setColour (juce::TextButton::ColourIds::buttonOnColourId, juce::Colour (DDSPColourPalette::kMagenta));
    urlButton->setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colour (DDSPColourPalette::kGrey));
    urlButton->setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colour (DDSPColourPalette::kGrey));
    urlButton->setColour (juce::ComboBox::ColourIds::outlineColourId, juce::Colour (DDSPColourPalette::kMagenta));

    urlButton->onClick = []
    {
        juce::URL modelTrainingUrl (kModelTrainingColabUrl.data());
        modelTrainingUrl.launchInDefaultBrowser();
    };

    infoButton.reset (new juce::TextButton (""));
    addAndMakeVisible (infoButton.get());
    infoButton->setLookAndFeel (&infoButtonLookAndFeel);
    infoButton->setColour (juce::TextButton::ColourIds::buttonColourId,
                           juce::Colour (DDSPColourPalette::kMagentaHeader));
    infoButton->onClick = []
    {
        juce::URL infoUrl (kInfoUrl.data());
        infoUrl.launchInDefaultBrowser();
    };
}

TopPanelComponent::~TopPanelComponent()
{
    modelList = nullptr;
    ddspLogo = nullptr;
    customModelsButton = nullptr;
    refreshButton = nullptr;
    infoButton = nullptr;
    urlButton = nullptr;
}

void TopPanelComponent::paint (juce::Graphics& g)
{
    juce::Rectangle<int> localArea (getLocalBounds());
    auto topArea = localArea.reduced (kPadding, kPadding / 2).removeFromTop (100 - kPadding / 2);
    auto bottomArea = localArea.reduced (kPadding, kPadding / 2).removeFromBottom (65 - kPadding / 2);

    juce::DropShadow dsTop (juce::Colours::black.withMultipliedAlpha (0.4f), 4, juce::Point<int> (0, 0));

    juce::Path header;
    header.addRoundedRectangle (topArea.getX(),
                                topArea.getY(),
                                topArea.getWidth(),
                                topArea.getHeight(),
                                kRoundedRectangleCornerSize,
                                kRoundedRectangleCornerSize,
                                true,
                                true,
                                false,
                                false);

    juce::Path footer;
    footer.addRoundedRectangle (bottomArea.getX(),
                                bottomArea.getY(),
                                bottomArea.getWidth(),
                                bottomArea.getHeight(),
                                10,
                                10,
                                false,
                                false,
                                true,
                                true);

    juce::Path shadowPath;
    auto shadowRect = localArea.reduced (kPadding, kPadding / 2);
    shadowPath.addRoundedRectangle (shadowRect.getX(),
                                    shadowRect.getY(),
                                    shadowRect.getWidth(),
                                    shadowRect.getHeight(),
                                    kRoundedRectangleCornerSize,
                                    kRoundedRectangleCornerSize,
                                    true,
                                    true,
                                    true,
                                    true);

    dsTop.drawForPath (g, shadowPath);

    g.setColour (juce::Colour (DDSPColourPalette::kMagentaHeader));
    g.fillPath (header);
    g.setColour (juce::Colour (DDSPColourPalette::kGrey));
    g.fillPath (footer);
}

void TopPanelComponent::resized()
{
    const int height = 24;
    const int modelListWidth = 230;
    const int customModelsButtonWidth = height;
    const int refreshButtonWidth = height;
    const int infoButtonWidth = height;
    const int urlButtonWidth = 120;
    const float logoPadding = 22.0f;
    const float margin = 8.0f;

    juce::Rectangle<int> localArea (getLocalBounds());
    auto topArea = localArea.reduced (kPadding, kPadding / 2).removeFromTop (100 - kPadding / 2);
    auto bottomArea = localArea.reduced (kPadding, kPadding / 2).removeFromBottom (65 - kPadding / 2);

    juce::FlexBox fbBottom;
    fbBottom.flexWrap = juce::FlexBox::Wrap::noWrap;
    fbBottom.flexDirection = juce::FlexBox::Direction::row;
    fbBottom.justifyContent = juce::FlexBox::JustifyContent::center;

    fbBottom.items.add (juce::FlexItem (*modelList)
                            .withWidth (modelListWidth)
                            .withHeight (height)
                            .withMargin (margin)
                            .withAlignSelf (juce::FlexItem::AlignSelf::center));
    fbBottom.items.add (juce::FlexItem (*customModelsButton)
                            .withWidth (customModelsButtonWidth)
                            .withMargin (margin)
                            .withHeight (height)
                            .withAlignSelf (juce::FlexItem::AlignSelf::center));
    fbBottom.items.add (juce::FlexItem (*refreshButton)
                            .withWidth (refreshButtonWidth)
                            .withMargin (margin)
                            .withHeight (height)
                            .withAlignSelf (juce::FlexItem::AlignSelf::center));
    fbBottom.items.add (juce::FlexItem (*urlButton)
                            .withWidth (urlButtonWidth)
                            .withMargin (margin)
                            .withHeight (height)
                            .withAlignSelf (juce::FlexItem::AlignSelf::center));

    fbBottom.performLayout (bottomArea);

    auto logoArea = topArea;
    logoArea.removeFromLeft (logoPadding);
    ddspLogo->setTransformToFit (logoArea.removeFromLeft (150).toFloat().reduced (kPadding),
                                 juce::RectanglePlacement::centred);

    juce::FlexBox fbTop;
    fbTop.flexWrap = juce::FlexBox::Wrap::noWrap;
    fbTop.flexDirection = juce::FlexBox::Direction::column;
    fbTop.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

    fbTop.items.add (juce::FlexItem (*infoButton)
                         .withWidth (infoButtonWidth)
                         .withMargin (margin)
                         .withHeight (height)
                         .withAlignSelf (juce::FlexItem::AlignSelf::flexEnd));
    fbTop.items.add (juce::FlexItem (*versionLabel)
                         .withWidth (120)
                         .withHeight (height)
                         .withMargin (juce::FlexItem::Margin (4, 8, 0, 0))
                         .withAlignSelf (juce::FlexItem::AlignSelf::flexEnd));

    fbTop.performLayout (topArea);
}

void TopPanelComponent::changeDDSPModel()
{
    audioProcessor.loadModel (modelList->getSelectedId() - 1);
    sendChangeMessage();
}

void TopPanelComponent::openFileBrowser() { audioProcessor.getModelLibrary().getPathToUserModels().startAsProcess(); }

void TopPanelComponent::refreshModels()
{
    audioProcessor.getModelLibrary().searchPathForModels();
    fillComboBox();

    // If the user removes models while in use, go back to using flute. Maybe we should have a
    // "No Model Loaded" state?
    if (audioProcessor.getCurrentModel() > audioProcessor.getModelLibrary().getModelList().size() - 1)
    {
        modelList->setSelectedId (1);
    }
    else
    {
        modelList->setSelectedId (audioProcessor.getCurrentModel() + 1, juce::dontSendNotification);
    }
}

void TopPanelComponent::fillComboBox()
{
    modelList->clear (juce::dontSendNotification);
    // Must start from 1.
    int comboBoxId = 1;
    for (auto& model : audioProcessor.getModelLibrary().getModelList())
    {
        modelList->addItem (model.name, comboBoxId++);
    }
}
