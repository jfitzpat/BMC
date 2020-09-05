/*
    IldaProperties.cpp
    Properties Pane for ILDA layer
 
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

#include <JuceHeader.h>
#include "IldaProperties.h"

//==============================================================================
IldaProperties::IldaProperties (FrameEditor* editor)
{
    frameEditor = editor;
    frameEditor->addActionListener (this);
    
    layerVisible.reset (new juce::ToggleButton ("layerVisible"));
    addAndMakeVisible (layerVisible.get());
    layerVisible->setTooltip ("Make this layer visible or invisible.");
    layerVisible->setButtonText ("Visible");
    layerVisible->addListener (this);

    drawLines.reset (new juce::ToggleButton ("drawLines"));
    addAndMakeVisible (drawLines.get());
    drawLines->setTooltip ("Draw lines between coordinates");
    drawLines->setButtonText ("Draw Lines");
    drawLines->addListener (this);

    showBlanking.reset (new juce::ToggleButton ("showBlanking"));
    addAndMakeVisible (showBlanking.get());
    showBlanking->setTooltip ("Show blanked (invisible) coordinates.");
    showBlanking->setButtonText ("Show Blanked Points");
    showBlanking->addListener (this);

    refresh();
}

IldaProperties::~IldaProperties()
{
}

//==============================================================================
void IldaProperties::paint (juce::Graphics& g)
{
    // Clear background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void IldaProperties::resized()
{
    layerVisible->setBounds (16, 16, 150, 24);
    drawLines->setBounds (16, 48, 150, 24);
    showBlanking->setBounds (16, 80, 150, 24);
}

//==============================================================================

void IldaProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::ildaVisibilityChanged)
        layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    else if (message == EditorActions::ildaShowBlankChanged)
        showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    else if (message == EditorActions::ildaDrawLinesChanged)
        drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
    else if (message == EditorActions::frameIndexChanged)
        refresh();
}

//==============================================================================
void IldaProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == layerVisible.get())
    {
        frameEditor->setIldaVisible (layerVisible->getToggleState());
    }
    else if (buttonThatWasClicked == showBlanking.get())
    {
        frameEditor->setIldaShowBlanked (showBlanking->getToggleState());
    }
    else if (buttonThatWasClicked == drawLines.get())
    {
        frameEditor->setIldaDrawLines (drawLines->getToggleState());
    }
}

//==============================================================================
void IldaProperties::refresh()
{
    layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
}
