/*
    MainEditor.cpp
    Main Edit Area Component

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
#include "MainEditor.h"

//==============================================================================
MainEditor::MainEditor (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
}

MainEditor::~MainEditor()
{
}

void MainEditor::paint (juce::Graphics& g)
{
    // Black background
    g.fillAll (Colours::black);
        
    // Outline working area
    g.setColour (juce::Colours::grey);
    g.drawRect (activeArea, 1);

    // Class name
    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("MainEditor", getLocalBounds(),
                juce::Justification::centred, true);
}

void MainEditor::resized()
{
    // Figure out our working area
    auto w = getWidth();
    auto h = getHeight();
    
    if (w > h)
        activeArea.setBounds ((w - h) >> 1, 0, h, h);
    else
        activeArea.setBounds (0, (h - w) >> 1, w, w);
}

void MainEditor::actionListenerCallback (const String& message)
{
    
}
