/*
    GradientPopup.h
    Gradient ilda Popup Controls
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
#include "ColourButton.h"
#include <JuceHeader.h>

class GradientPopup : public Component,
                      public Slider::Listener,
                      public Button::Listener,
                      public ChangeListener
{
public:
    GradientPopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        angleSlider.reset (new juce::Slider ("angleSlider"));
        addAndMakeVisible (angleSlider.get());
        angleSlider->setRange (-359, 359, 1);
        angleSlider->setSliderStyle (juce::Slider::LinearVertical);
        angleSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        angleSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        angleSlider->addListener (this);

        angleSlider->setBounds (36, 32, 56, 216);

        lengthSlider.reset (new juce::Slider ("lengthSlider"));
        addAndMakeVisible (lengthSlider.get());
        lengthSlider->setRange (0.1, 100, 0.1);
        lengthSlider->setSliderStyle (juce::Slider::LinearVertical);
        lengthSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        lengthSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        lengthSlider->addListener (this);
        lengthSlider->setTextValueSuffix ("%");

        lengthSlider->setBounds (100, 32, 56, 216);

        radiusLabel.reset (new juce::Label ("radiusLabel",
                                       TRANS("Angle")));
        addAndMakeVisible (radiusLabel.get());
        radiusLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        radiusLabel->setJustificationType (juce::Justification::centred);
        radiusLabel->setEditable (false, false, false);
        radiusLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        radiusLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        radiusLabel->setBounds (34, 8, 60, 24);

        gainLabel.reset (new juce::Label ("ylabel",
                                       TRANS("Length")));
        addAndMakeVisible (gainLabel.get());
        gainLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        gainLabel->setJustificationType (juce::Justification::centred);
        gainLabel->setEditable (false, false, false);
        gainLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        gainLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        gainLabel->setBounds (100, 8, 60, 24);

        radialButton.reset (new ToggleButton ("radialButton"));
        addAndMakeVisible (radialButton.get());
        radialButton->setButtonText ("Radial");
        radialButton->addListener (this);
        
        radialButton->setBounds (14, 260, 150, 24);
        
        centerButton.reset (new juce::ToggleButton ("centerButton"));
        addAndMakeVisible (centerButton.get());
        centerButton->setButtonText (TRANS("Center on selection"));
        centerButton->addListener (this);

        centerButton->setBounds (14, 288, 150, 24);

        color1Button.reset (new ColourButton);
        addAndMakeVisible (color1Button.get());
        color1Button->addChangeListener (this);
        
        color1Button->setBounds (18,40,20,20);

        color2Button.reset (new ColourButton);
        addAndMakeVisible (color2Button.get());
        color2Button->addChangeListener (this);
        
        color2Button->setBounds (18,200,20,20);

        setSize (180, 320);
        
        color1Button->setColour (TextButton::buttonColourId, Colours::white);
        color2Button->setColour (TextButton::buttonColourId, Colours::red);
        angleSlider->setValue (0, dontSendNotification);
        lengthSlider->setValue (50, dontSendNotification);
        centerButton->setToggleState (true, dontSendNotification);
        radialButton->setToggleState (false, dontSendNotification);
        
        frameEditor->startTransform ("Gradient Effect");
        
        buttonClicked (nullptr);
    }
    
    ~GradientPopup()
    {
        frameEditor->endTransform();
        angleSlider = nullptr;
        lengthSlider = nullptr;
        radialButton = nullptr;
        centerButton = nullptr;
        radiusLabel = nullptr;
        gainLabel = nullptr;
        color1Button = nullptr;
        color2Button = nullptr;
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

    void changeListenerCallback (ChangeBroadcaster* /*source*/) override
    {
        buttonClicked (nullptr);
    }

    void buttonClicked (juce::Button* /*buttonThatWasClicked*/) override
    {
        Colour c1 = color1Button->findColour (TextButton::buttonColourId);
        Colour c2 = color2Button->findColour (TextButton::buttonColourId);

        float angle = (float)(angleSlider->getValue());
        float length = (float)lengthSlider->getValue();
        bool b = centerButton->getToggleState();
        bool r = radialButton->getToggleState();
        frameEditor->gradientIldaSelected (c1, c2, angle, length, r, b);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<ToggleButton> centerButton;
    std::unique_ptr<ToggleButton> radialButton;
    std::unique_ptr<Slider> angleSlider;
    std::unique_ptr<Slider> lengthSlider;
    std::unique_ptr<Label> radiusLabel;
    std::unique_ptr<Label> gainLabel;
    std::unique_ptr<ColourButton> color1Button;
    std::unique_ptr<ColourButton> color2Button;
};

//==============================================================================
class GradientButton : public PopupTool
{
public:
    GradientButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<GradientPopup>(frameEditor);
    }
};




