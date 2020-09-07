/*
    FrameList.cpp
    Manage and Display File Frames

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
#include "FrameList.h"

//==============================================================================
FrameList::FrameList (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
    
    frameList.reset (new ListBox ("frameList", this));
    addAndMakeVisible (frameList.get());
    frameList->setRowHeight (150);
    frameList->setColour (ListBox::backgroundColourId, getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

//    frameList->setMultipleSelectionEnabled (true);

    frameList->selectRow (frameEditor->getFrameIndex());
}

FrameList::~FrameList()
{
}

void FrameList::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    
    g.drawRect (getLocalBounds().removeFromRight(1), 1);
}

void FrameList::resized()
{
    frameList->setBounds (10, 30, 180, getHeight()-40);
}

//==============================================================================
int FrameList::getNumRows()
{
    return frameEditor->getFrameCount();
}

void FrameList::paintListBoxItem (int rowNumber,
                                  Graphics& g,
                                  int width, int height,
                                  bool rowIsSelected)
{
    if ( (rowNumber < 0) || (rowNumber >= frameEditor->getFrameCount()) )
        return;

    g.fillAll (getLookAndFeel().findColour (ListBox::backgroundColourId));
    
    g.drawImage (frameEditor->getThumbNail ((uint16)rowNumber),
                 Rectangle<float>::leftTopRightBottom (0, 0, width, height),
                 0);
    
    g.setColour (Colours::white);
    g.setFont (12);
    g.drawText ("Frame " + String(rowNumber + 1), 0, height-22, width, 22, Justification::centred);
    
    if (rowIsSelected)
    {
        g.setColour (Colours::whitesmoke);
        g.drawRect (0, 0, width, height, 2);
    }
}

void FrameList::selectedRowsChanged (int lastRowSelected)
{
    if (lastRowSelected >= 0)
        frameEditor->setFrameIndex (lastRowSelected);
}

//==============================================================================
void FrameList::actionListenerCallback (const String& message)
{
    if (message == EditorActions::framesChanged)
    {
        frameList->updateContent();
        frameList->repaint();
    }
    if (message == EditorActions::frameIndexChanged)
        frameList->selectRow (frameEditor->getFrameIndex());
}
