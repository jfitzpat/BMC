/*
    HuePopup.h
    Hue ilda Popup Controls
    Copyright 2020 Scrootch.me!

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

#include "FrameEditor.h"
#include "PopupTool.h"
#include <JuceHeader.h>

class HuePopup : public Component,
                 public juce::Slider::Listener
{
public:
    HuePopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        hueSlider.reset (new juce::Slider ("hueSlider"));
        addAndMakeVisible (hueSlider.get());
        hueSlider->setRange (-1, 1, 0.01);
        hueSlider->setSliderStyle (juce::Slider::LinearVertical);
        hueSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        hueSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        hueSlider->addListener (this);

        hueSlider->setBounds (8, 32, 56, 216);

        saturationSlider.reset (new juce::Slider ("satSlider"));
        addAndMakeVisible (saturationSlider.get());
        saturationSlider->setRange (-1, 1, 0.01);
        saturationSlider->setSliderStyle (juce::Slider::LinearVertical);
        saturationSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        saturationSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        saturationSlider->addListener (this);

        saturationSlider->setBounds (72, 32, 56, 216);

        brightnessSlider.reset (new juce::Slider ("brtSlider"));
        addAndMakeVisible (brightnessSlider.get());
        brightnessSlider->setRange (-1, 1, 0.01);
        brightnessSlider->setSliderStyle (juce::Slider::LinearVertical);
        brightnessSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        brightnessSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        brightnessSlider->addListener (this);

        brightnessSlider->setBounds (136, 32, 56, 216);

        hueSLabel.reset (new juce::Label ("hueSLabel",
                                       TRANS("Hue")));
        addAndMakeVisible (hueSLabel.get());
        hueSLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        hueSLabel->setJustificationType (juce::Justification::centred);
        hueSLabel->setEditable (false, false, false);
        hueSLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        hueSLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        hueSLabel->setBounds (16, 8, 40, 24);

        saturationLabel.reset (new juce::Label ("saturationLabel",
                                       TRANS("Saturation")));
        addAndMakeVisible (saturationLabel.get());
        saturationLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        saturationLabel->setJustificationType (juce::Justification::centred);
        saturationLabel->setEditable (false, false, false);
        saturationLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        saturationLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        saturationLabel->setBounds (70, 8, 60, 24);

        brightnessLabel.reset (new juce::Label ("brightnessLabel",
                                       TRANS("Brightness")));
        addAndMakeVisible (brightnessLabel.get());
        brightnessLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        brightnessLabel->setJustificationType (juce::Justification::centred);
        brightnessLabel->setEditable (false, false, false);
        brightnessLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        brightnessLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        brightnessLabel->setBounds (134, 8, 60, 24);

        setSize (200, 260);
        
        hueSlider->setValue (0, dontSendNotification);
        saturationSlider->setValue (0, dontSendNotification);
        frameEditor->startTransform ("Adjust Color");
    }
    
    ~HuePopup()
    {
        frameEditor->endTransform();
        hueSlider = nullptr;
        hueSLabel = nullptr;
        saturationSlider = nullptr;
        saturationLabel = nullptr;
        brightnessSlider = nullptr;
        brightnessLabel = nullptr;
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);
    }
    
    void resized() override
    {
    }
    
    void sliderValueChanged (juce::Slider* /*sliderThatWasMoved*/) override
    {
        frameEditor->adjustHueIldaSelected ((float)hueSlider->getValue(),
                                            (float)saturationSlider->getValue(),
                                            (float)brightnessSlider->getValue());
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<juce::ToggleButton> centerButton;
    std::unique_ptr<juce::Slider> hueSlider;
    std::unique_ptr<juce::Label> hueSLabel;
    std::unique_ptr<juce::Slider> saturationSlider;
    std::unique_ptr<juce::Label> saturationLabel;
    std::unique_ptr<juce::Slider> brightnessSlider;
    std::unique_ptr<juce::Label> brightnessLabel;

};

//==============================================================================
class HueButton : public PopupTool
{
public:
    HueButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<HuePopup>(frameEditor);
    }
};



