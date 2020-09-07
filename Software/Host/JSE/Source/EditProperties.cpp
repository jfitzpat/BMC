/*
    EditProperties.cpp
    Layer Selector and Properties Display Component
 
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
#include "SketchProperties.h"
#include "IldaProperties.h"
#include "RefProperties.h"
#include "EditProperties.h"

//==============================================================================
EditProperties::EditProperties (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
    
    layerTabs.reset (new PropTabbedComponent (frame));
    layerTabs->setOutline (0);
    addAndMakeVisible (layerTabs.get());
    
    auto tabColor = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    
    layerTabs->addTab("Sketch",
                      tabColor,
                      new SketchProperties (frame), true);
    layerTabs->addTab("ILDA",
                      tabColor,
                      new IldaProperties (frame), true);
    layerTabs->addTab("Background",
                      tabColor,
                      new RefProperties (frame), true);
    
    frameEditor->setActiveLayer (FrameEditor::sketch);
    setWantsKeyboardFocus (true);
}

EditProperties::~EditProperties()
{
}

void EditProperties::paint (juce::Graphics& g)
{
    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds().removeFromLeft(1), 1);
}

void EditProperties::resized()
{
    layerTabs->setBounds (1, 0, getWidth()-1, getHeight());
}

void EditProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::layerChanged)
    {
        if (frameEditor->getActiveLayer() != layerTabs->getCurrentTabIndex())
            layerTabs->setCurrentTabIndex (frameEditor->getActiveLayer());
    }
}
