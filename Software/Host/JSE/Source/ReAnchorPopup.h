/*
    ReAnchorPopup.h
    Create Anchors along Shapre Popup Controls
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

class ReAnchorPopup : public Component,
                      public juce::Slider::Listener
{
public:
    ReAnchorPopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        pointsSlider.reset (new juce::Slider ("pointsSlider"));
        addAndMakeVisible (pointsSlider.get());
        pointsSlider->setRange (2, 200, 1);
        pointsSlider->setSliderStyle (juce::Slider::LinearVertical);
        pointsSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        pointsSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        pointsSlider->addListener (this);

        pointsSlider->setBounds (26, 32, 56, 216);

        extraSlider.reset (new juce::Slider ("extraSlider"));
        addAndMakeVisible (extraSlider.get());
        extraSlider->setRange (1, 50, 1);
        extraSlider->setSliderStyle (juce::Slider::LinearVertical);
        extraSlider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        extraSlider->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff0d1112));
        extraSlider->addListener (this);

        extraSlider->setBounds (90, 32, 56, 216);


        pointslabel.reset (new juce::Label ("plabel",
                                       TRANS("Anchors")));
        addAndMakeVisible (pointslabel.get());
        pointslabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        pointslabel->setJustificationType (juce::Justification::centred);
        pointslabel->setEditable (false, false, false);
        pointslabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        pointslabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        pointslabel->setBounds (24, 8, 60, 24);

        extraLabel.reset (new juce::Label ("elabel",
                                       TRANS("Points")));
        addAndMakeVisible (extraLabel.get());
        extraLabel->setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
        extraLabel->setJustificationType (juce::Justification::centred);
        extraLabel->setEditable (false, false, false);
        extraLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
        extraLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

        extraLabel->setBounds (88, 8, 60, 24);

        setSize (164, 260);
        pointsSlider->setValue (8, dontSendNotification);
        extraSlider->setValue (1, dontSendNotification);
        
        if (frameEditor->getIPathSelection().getAnchor() != -1)
        {
            IPathSelection selection = frameEditor->getIPathSelection();
            selection.setAnchor (-1);
            selection.setControl (-1);
            frameEditor->setIPathSelection (selection);
        }
        frameEditor->startTransform ("Re-Anchor Shape(s)");
        sliderValueChanged (nullptr);
    }
    
    ~ReAnchorPopup()
    {
        frameEditor->endTransform();
        pointsSlider = nullptr;
        pointslabel = nullptr;
        extraSlider = nullptr;
        extraLabel = nullptr;
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
        int p = (int)pointsSlider->getValue();
        int e = (int)extraSlider->getValue();
        frameEditor->reAnchorSketchSelected (p, e);
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<Slider> pointsSlider;
    std::unique_ptr<Label> pointslabel;
    std::unique_ptr<Slider> extraSlider;
    std::unique_ptr<Label> extraLabel;
};

//==============================================================================
class ReAnchorButton : public PopupTool
{
public:
    ReAnchorButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<ReAnchorPopup>(frameEditor);
    }
};



