/*
    BarberPopup.h
    Barber Pole Tranform ilda Popup Controls
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

class BarberPopup : public Component,
                    public juce::Slider::Listener,
                    public juce::Button::Listener
{
public:
    BarberPopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        radiusSlider.reset (new juce::Slider ("radiusSlider"));
        addAndMakeVisible (radiusSlider.get());
        radiusSlider->setRange (100, 3200, 1);
        radiusSlider->setSliderStyle (juce::Slider::LinearVertical);
        radiusSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        radiusSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        radiusSlider->addListener (this);

        radiusSlider->setBounds (8, 32, 56, 216);

        skewSlider.reset (new juce::Slider ("skewSlider"));
        addAndMakeVisible (skewSlider.get());
        skewSlider->setRange (-3, 3, 0.05);
        skewSlider->setSliderStyle (juce::Slider::LinearVertical);
        skewSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        skewSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        skewSlider->addListener (this);

        skewSlider->setBounds (72, 32, 56, 216);

        rotateSlider.reset (new juce::Slider ("rotateSlider"));
        addAndMakeVisible (rotateSlider.get());
        rotateSlider->setRange (-359.9, 359.9, 0.1);
        rotateSlider->setSliderStyle (juce::Slider::LinearVertical);
        rotateSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        rotateSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        rotateSlider->addListener (this);

        rotateSlider->setBounds (136, 32, 56, 216);

        radiusLabel.reset (new juce::Label ("radiusLabel",
                                       TRANS("Radius")));
        addAndMakeVisible (radiusLabel.get());
        radiusLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        radiusLabel->setJustificationType (juce::Justification::centred);
        radiusLabel->setEditable (false, false, false);
        radiusLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        radiusLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        radiusLabel->setBounds (16, 8, 40, 24);

        skewLabel.reset (new juce::Label ("skewLabel",
                                       TRANS("Skew")));
        addAndMakeVisible (skewLabel.get());
        skewLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        skewLabel->setJustificationType (juce::Justification::centred);
        skewLabel->setEditable (false, false, false);
        skewLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        skewLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        skewLabel->setBounds (80, 8, 40, 24);

        rotateLabel.reset (new juce::Label ("rotateLabel",
                                       TRANS("Rotate")));
        addAndMakeVisible (rotateLabel.get());
        rotateLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        rotateLabel->setJustificationType (juce::Justification::centred);
        rotateLabel->setEditable (false, false, false);
        rotateLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        rotateLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        rotateLabel->setBounds (144, 8, 40, 24);

        centerButton.reset (new juce::ToggleButton ("centerButton"));
        addAndMakeVisible (centerButton.get());
        centerButton->setButtonText (TRANS("Center on selection"));
        centerButton->addListener (this);

        centerButton->setBounds (24, 260, 150, 24);

        setSize (200, 300);
        
        radiusSlider->setValue (1000, dontSendNotification);
        skewSlider->setValue (0, dontSendNotification);
        rotateSlider->setValue (0, dontSendNotification);
        centerButton->setToggleState (true, dontSendNotification);
        
        frameEditor->startTransform ("Barber Pole Transform");
        buttonClicked (nullptr);
    }
    
    ~BarberPopup()
    {
        frameEditor->endTransform();
        radiusSlider = nullptr;
        skewSlider = nullptr;
        rotateSlider = nullptr;
        centerButton = nullptr;
        radiusLabel = nullptr;
        skewLabel = nullptr;
        rotateLabel = nullptr;
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
        buttonClicked (nullptr);
    }

    void buttonClicked (juce::Button* /*buttonThatWasClicked*/) override
    {
        float rad = (float)radiusSlider->getValue() * 10.0;
        float skew = (float)skewSlider->getValue();
        float rot = (float)rotateSlider->getValue();
        bool b = centerButton->getToggleState();
        frameEditor->barberPoleIldaSelected (rad, skew, rot, b, false);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<juce::ToggleButton> centerButton;
    std::unique_ptr<juce::Slider> radiusSlider;
    std::unique_ptr<juce::Slider> skewSlider;
    std::unique_ptr<juce::Slider> rotateSlider;
    std::unique_ptr<juce::Label> radiusLabel;
    std::unique_ptr<juce::Label> skewLabel;
    std::unique_ptr<juce::Label> rotateLabel;
};

//==============================================================================
class BarberButton : public PopupTool
{
public:
    BarberButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<BarberPopup>(frameEditor);
    }
};

