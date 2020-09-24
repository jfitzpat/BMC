/*
    ShearPopup.h
    Shear ilda Popup Controls
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

class ShearPopup : public Component,
                   public juce::Slider::Listener,
                   public juce::Button::Listener
{
public:
    ShearPopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        scaleXSlider.reset (new juce::Slider ("scaleXSlider"));
        addAndMakeVisible (scaleXSlider.get());
        scaleXSlider->setRange (-3, 3, 0.05);
        scaleXSlider->setSliderStyle (juce::Slider::LinearVertical);
        scaleXSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        scaleXSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        scaleXSlider->addListener (this);

        scaleXSlider->setBounds (26, 32, 56, 216);

        scaleYSlider.reset (new juce::Slider ("scaleYSlider"));
        addAndMakeVisible (scaleYSlider.get());
        scaleYSlider->setRange (-3, 3, 0.05);
        scaleYSlider->setSliderStyle (juce::Slider::LinearVertical);
        scaleYSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        scaleYSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        scaleYSlider->addListener (this);

        scaleYSlider->setBounds (90, 32, 56, 216);

        xlabel.reset (new juce::Label ("xlabel",
                                       TRANS("L/R")));
        addAndMakeVisible (xlabel.get());
        xlabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        xlabel->setJustificationType (juce::Justification::centred);
        xlabel->setEditable (false, false, false);
        xlabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        xlabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        xlabel->setBounds (34, 8, 40, 24);

        ylabel.reset (new juce::Label ("ylabel",
                                       TRANS("U/D")));
        addAndMakeVisible (ylabel.get());
        ylabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        ylabel->setJustificationType (juce::Justification::centred);
        ylabel->setEditable (false, false, false);
        ylabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        ylabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        ylabel->setBounds (98, 8, 40, 24);

        centerButton.reset (new juce::ToggleButton ("centerButton"));
        addAndMakeVisible (centerButton.get());
        centerButton->setButtonText (TRANS("Center on selection"));
        centerButton->addListener (this);

        centerButton->setBounds (14, 260, 150, 24);

        setSize (180, 300);
        
        scaleXSlider->setValue (0, dontSendNotification);
        scaleYSlider->setValue (0, dontSendNotification);
        centerButton->setToggleState (true, dontSendNotification);
        
        frameEditor->startTransform ("Shear Point(s)");
    }
    
    ~ShearPopup()
    {
        frameEditor->endTransform();
        scaleXSlider = nullptr;
        scaleYSlider = nullptr;
        centerButton = nullptr;
        xlabel = nullptr;
        ylabel = nullptr;
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);
    }
    
    void resized() override
    {
    }
    
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override
    {
        buttonClicked (nullptr);
    }

    void buttonClicked (juce::Button* buttonThatWasClicked) override
    {
        float x = scaleXSlider->getValue();
        float y = scaleYSlider->getValue();
        bool b = centerButton->getToggleState();
        frameEditor->shearIldaSelected (x, y, b, false);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<juce::ToggleButton> centerButton;
    std::unique_ptr<juce::Slider> scaleXSlider;
    std::unique_ptr<juce::Slider> scaleYSlider;
    std::unique_ptr<juce::Label> xlabel;
    std::unique_ptr<juce::Label> ylabel;
};

//==============================================================================
class ShearButton : public PopupTool
{
public:
    ShearButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<ShearPopup>(frameEditor);
    }
};


