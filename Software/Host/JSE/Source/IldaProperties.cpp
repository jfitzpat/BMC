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
}

IldaProperties::~IldaProperties()
{
}

void IldaProperties::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("IldaProperties", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void IldaProperties::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void IldaProperties::actionListenerCallback (const String& message)
{
    
}
