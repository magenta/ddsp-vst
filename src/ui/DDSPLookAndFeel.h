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

enum DDSPColourPalette : uint32_t
{
    kGrey = 0xFFF2F2F2,
    kMagenta = 0xFF6236FF,
    kDarkGrey = 0x25000000,
    kDarkerGrey = 0xFFACACAC,
    kLabelTextColour = 0xB3000000,
    kMagentaHeader = 0xFF6200EF,
};

constexpr int kPadding = 8;
constexpr int kModelVisPaddingMultiplier = 6;
constexpr float kTextSize = 13.0f;
constexpr char kFont[] = "Google Sans";
constexpr float kButtonIconPadding = 6.5f;
constexpr float kCornerSize = 4.0f;
constexpr float kKerningFactor = 0.025f;
constexpr float kRoundedRectangleCornerSize = 10.0f;

class DDSPLookAndFeel : public juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           juce::Slider& slider) override

    {
        auto outline = juce::Colour (DDSPColourPalette::kDarkGrey);
        auto fill = juce::Colour (DDSPColourPalette::kMagenta);
        auto thumbColour = juce::Colour (DDSPColourPalette::kMagenta);

        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);

        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 1.0f;
        auto arcRadius = radius - lineW * 0.5f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(),
                                     bounds.getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);

        g.setColour (outline);
        g.strokePath (backgroundArc,
                      juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (
                bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);

            g.setColour (fill);
            g.strokePath (valueArc,
                          juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        auto thumbWidth = 12.0f;
        juce::Point<float> thumbPoint (
            bounds.getCentreX() + arcRadius * std::cos (toAngle - juce::MathConstants<float>::halfPi),
            bounds.getCentreY() + arcRadius * std::sin (toAngle - juce::MathConstants<float>::halfPi));

        g.setColour (thumbColour);
        g.fillEllipse (juce::Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = kCornerSize;
        juce::Rectangle<int> boxBounds (0, 0, width, height);

        g.setColour (juce::Colours::white);
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

        g.setColour (juce::Colour (DDSPColourPalette::kDarkerGrey));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 0.5f);

        juce::Rectangle<int> arrowZone (width - 20, 0, 20, height);
        juce::Path path;
        path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
        path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

        juce::Path arrorBg;
        arrorBg.addRoundedRectangle (arrowZone.getX(),
                                     arrowZone.getY(),
                                     arrowZone.getWidth(),
                                     arrowZone.getHeight(),
                                     cornerSize,
                                     cornerSize,
                                     false,
                                     true,
                                     false,
                                     true);

        g.setColour (juce::Colour (DDSPColourPalette::kMagenta));
        g.fillPath (arrorBg);

        g.setColour (juce::Colour (DDSPColourPalette::kGrey));
        g.strokePath (path, juce::PathStrokeType (1.0f));
    }

    juce::Font getComboBoxFont (juce::ComboBox& comboBox) override { return getCommonMenuFont(); }

    juce::Font getPopupMenuFont() override { return getCommonMenuFont(); }

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = kCornerSize;
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

        auto baseColour = backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.1f);

        g.setColour (baseColour);

        auto flatOnLeft = button.isConnectedOnLeft();
        auto flatOnRight = button.isConnectedOnRight();
        auto flatOnTop = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
        {
            juce::Path path;
            path.addRoundedRectangle (bounds.getX(),
                                      bounds.getY(),
                                      bounds.getWidth(),
                                      bounds.getHeight(),
                                      cornerSize,
                                      cornerSize,
                                      ! (flatOnLeft || flatOnTop),
                                      ! (flatOnRight || flatOnTop),
                                      ! (flatOnLeft || flatOnBottom),
                                      ! (flatOnRight || flatOnBottom));

            g.fillPath (path);

            g.setColour (button.findColour (juce::ComboBox::outlineColourId));
            g.strokePath (path, juce::PathStrokeType (0.5f));
        }
        else
        {
            g.fillRoundedRectangle (bounds, cornerSize);

            g.setColour (button.findColour (juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle (bounds, cornerSize, 0.5f);
        }
    }

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override { return getCommonMenuFont(); }

private:
    juce::Font getCommonMenuFont()
    {
        return juce::Font (kFont, kTextSize, juce::Font::plain)
            .withTypefaceStyle ("Regular")
            .withExtraKerningFactor (kKerningFactor);
    }
};

class IconButtonLookAndFeel : public DDSPLookAndFeel
{
public:
    IconButtonLookAndFeel (std::unique_ptr<juce::Drawable> ic) : icon (std::move (ic)) {}
    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& b,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto cornerSize = kCornerSize;
        auto buttonArea = b.getLocalBounds();

        auto baseColour = backgroundColour.withMultipliedAlpha (b.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.1f);

        g.setColour (baseColour);

        g.fillRoundedRectangle (buttonArea.toFloat(), cornerSize);
        icon->drawWithin (
            g, buttonArea.reduced (kButtonIconPadding).toFloat(), juce::RectanglePlacement::stretchToFit, 1.0f);
    }

private:
    std::unique_ptr<juce::Drawable> icon;
};

class OpenFolderButtonLookAndFeel : public IconButtonLookAndFeel
{
public:
    OpenFolderButtonLookAndFeel()
        : IconButtonLookAndFeel (
            juce::DrawableImage::createFromImageData (BinaryData::folder_icon_svg, BinaryData::folder_icon_svgSize))
    {
    }
};

class RefreshButtonLookAndFeel : public IconButtonLookAndFeel
{
public:
    RefreshButtonLookAndFeel()
        : IconButtonLookAndFeel (
            juce::DrawableImage::createFromImageData (BinaryData::refresh_icon_svg, BinaryData::refresh_icon_svgSize))
    {
    }
};

class InfoButtonLookAndFeel : public IconButtonLookAndFeel
{
public:
    InfoButtonLookAndFeel()
        : IconButtonLookAndFeel (
            juce::DrawableImage::createFromImageData (BinaryData::info_icon_svg, BinaryData::info_icon_svgSize))
    {
    }
};
