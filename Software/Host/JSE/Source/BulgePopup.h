/*
    BulgePopup.h
    Bulge Tranform ilda Popup Controls
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

class BulgePopup : public Component,
                   public juce::Slider::Listener,
                   public juce::Button::Listener
{
public:
    BulgePopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        radiusSlider.reset (new juce::Slider ("radiusSlider"));
        addAndMakeVisible (radiusSlider.get());
        radiusSlider->setRange (0, .6, 0.0025);
        radiusSlider->setSliderStyle (juce::Slider::LinearVertical);
        radiusSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        radiusSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        radiusSlider->addListener (this);

        radiusSlider->setBounds (26, 32, 56, 216);

        gainSlider.reset (new juce::Slider ("gainSlider"));
        addAndMakeVisible (gainSlider.get());
        gainSlider->setRange (100, 30000, 1);
        gainSlider->setSliderStyle (juce::Slider::LinearVertical);
        gainSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        gainSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        gainSlider->addListener (this);
        gainSlider->setTextValueSuffix ("%");

        gainSlider->setBounds (90, 32, 56, 216);

        radiusLabel.reset (new juce::Label ("radiusLabel",
                                       TRANS("Radius")));
        addAndMakeVisible (radiusLabel.get());
        radiusLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        radiusLabel->setJustificationType (juce::Justification::centred);
        radiusLabel->setEditable (false, false, false);
        radiusLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        radiusLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        radiusLabel->setBounds (24, 8, 60, 24);

        gainLabel.reset (new juce::Label ("ylabel",
                                       TRANS("Gain")));
        addAndMakeVisible (gainLabel.get());
        gainLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        gainLabel->setJustificationType (juce::Justification::centred);
        gainLabel->setEditable (false, false, false);
        gainLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        gainLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        gainLabel->setBounds (98, 8, 40, 24);

        centerButton.reset (new juce::ToggleButton ("centerButton"));
        addAndMakeVisible (centerButton.get());
        centerButton->setButtonText (TRANS("Center on selection"));
        centerButton->addListener (this);

        centerButton->setBounds (14, 260, 150, 24);

        setSize (180, 300);
        
        radiusSlider->setValue (0, dontSendNotification);
        gainSlider->setValue (100, dontSendNotification);
        centerButton->setToggleState (true, dontSendNotification);
        
        frameEditor->startTransform ("Bulge Transform");
    }
    
    ~BulgePopup()
    {
        frameEditor->endTransform();
        radiusSlider = nullptr;
        gainSlider = nullptr;
        centerButton = nullptr;
        radiusLabel = nullptr;
        gainLabel = nullptr;
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
        float rad = (float)(0.0 - radiusSlider->getValue());
        float gain = (float)gainSlider->getValue();
        gain /= 100.0f;
        bool b = centerButton->getToggleState();
        frameEditor->bulgeIldaSelected (rad, gain, b, false);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<ToggleButton> centerButton;
    std::unique_ptr<Slider> radiusSlider;
    std::unique_ptr<Slider> gainSlider;
    std::unique_ptr<Label> radiusLabel;
    std::unique_ptr<Label> gainLabel;
};

//==============================================================================
class BulgeButton : public PopupTool
{
public:
    BulgeButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<BulgePopup>(frameEditor);
    }
};


