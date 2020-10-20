/*
    SketchRotatePopup.h
    Rotate sketch Popup Controls
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

class SketchRotatePopup : public Component,
                          public juce::Slider::Listener,
                          public juce::Button::Listener
{
public:
    SketchRotatePopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        scaleZSlider.reset (new juce::Slider ("scaleZSlider"));
        addAndMakeVisible (scaleZSlider.get());
        scaleZSlider->setRange (-359.9, 359.9, 0.1);
        scaleZSlider->setSliderStyle (juce::Slider::LinearVertical);
        scaleZSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        scaleZSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        scaleZSlider->addListener (this);

        scaleZSlider->setBounds (72, 32, 56, 216);


        zlabel.reset (new juce::Label ("zlabel",
                                       TRANS("Z")));
        addAndMakeVisible (zlabel.get());
        zlabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        zlabel->setJustificationType (juce::Justification::centred);
        zlabel->setEditable (false, false, false);
        zlabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        zlabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        zlabel->setBounds (80, 8, 40, 24);

        centerButton.reset (new juce::ToggleButton ("centerButton"));
        addAndMakeVisible (centerButton.get());
        centerButton->setButtonText (TRANS("Center on selection"));
        centerButton->addListener (this);

        centerButton->setBounds (24, 260, 150, 24);

        setSize (200, 300);
        scaleZSlider->setValue (0, dontSendNotification);
        centerButton->setToggleState (true, dontSendNotification);
        
        frameEditor->startTransform ("Rotate Shape(s)");
    }
    
    ~SketchRotatePopup()
    {
        frameEditor->endTransform();
        scaleZSlider = nullptr;
        centerButton = nullptr;
        zlabel = nullptr;
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
        float z = (float)scaleZSlider->getValue();
        bool b = centerButton->getToggleState();
        frameEditor->rotateSketchSelected (z, b, false);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<juce::ToggleButton> centerButton;
    std::unique_ptr<juce::Slider> scaleZSlider;
    std::unique_ptr<juce::Label> zlabel;
};

//==============================================================================
class SketchRotateButton : public PopupTool
{
public:
    SketchRotateButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<SketchRotatePopup>(frameEditor);
    }
};


