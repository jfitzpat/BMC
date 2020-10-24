/*
    RenderPopup.h
    Render Sketch Popup Controls
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

class RenderPopup : public Component,
                   public juce::Button::Listener
{
public:
    RenderPopup (FrameEditor* editor)
    : frameEditor (editor)
    {
        pathButton.reset (new juce::ToggleButton ("pathButton"));
        addAndMakeVisible (pathButton.get());
        pathButton->setButtonText (TRANS("Use shortest path"));
        pathButton->addListener (this);

        pathButton->setBounds (16, 16, 150, 24);

        updateSketchButton.reset (new juce::ToggleButton ("updateSketchButton"));
        addAndMakeVisible (updateSketchButton.get());
        updateSketchButton->setButtonText (TRANS("Update sketch"));
        updateSketchButton->addListener (this);

        updateSketchButton->setBounds (16, 44, 150, 24);

        goButton.reset (new juce::TextButton ("goButton"));
        addAndMakeVisible (goButton.get());
        goButton->setButtonText ("Render");
        goButton->addListener (this);
        
        goButton->setBounds (50, 84, 80, 40);

        setSize (180, 140);
        pathButton->setToggleState (true, dontSendNotification);
    }
    
    ~RenderPopup()
    {
        pathButton = nullptr;
        updateSketchButton = nullptr;
        goButton = nullptr;
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);
    }
    
    void resized() override
    {
    }
    
    void buttonClicked (juce::Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == goButton.get())
        {
            frameEditor->renderSketch (pathButton->getToggleState(),
                                       updateSketchButton->getToggleState());
            
            CallOutBox* box = findParentComponentOfClass<CallOutBox>();
            box->dismiss();
        }
    }

private:
    FrameEditor* frameEditor;

    std::unique_ptr<TextButton> goButton;
    std::unique_ptr<ToggleButton> pathButton;
    std::unique_ptr<ToggleButton> updateSketchButton;
};

//==============================================================================
class RenderButton : public PopupTool
{
public:
    RenderButton (FrameEditor* editor)
    : PopupTool (editor)
    {;}
    
    std::unique_ptr<Component> makeComponent() override
    {
        return std::make_unique<RenderPopup>(frameEditor);
    }
};


