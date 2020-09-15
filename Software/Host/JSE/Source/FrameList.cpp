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
    
    addButton.reset (new TextButton ("addButton"));
    addAndMakeVisible (addButton.get());
    addButton->setButtonText ("+");
    addButton->setTooltip ("Add a new frame");
    addButton->addListener (this);

    delButton.reset (new TextButton ("delButton"));
    addAndMakeVisible (delButton.get());
    delButton->setButtonText ("-");
    delButton->setTooltip ("Delete the current frame");
    delButton->addListener (this);
    
    dupIcon = Drawable::createFromImageData (BinaryData::duplicatewhite_png,
                                             BinaryData::duplicatewhite_pngSize);
    
    dupButton.reset (new DrawableButton ("dupButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (dupButton.get());
    dupButton->setImages (dupIcon.get());
    dupButton->setEdgeIndent (0);
    dupButton->setTooltip ("Duplicate the current frame");
    dupButton->addListener (this);
    
    upIcon = Drawable::createFromImageData (BinaryData::upwhite_png,
                                            BinaryData::upwhite_pngSize);
    
    upButton.reset (new DrawableButton ("upButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (upButton.get());
    upButton->setImages (upIcon.get());
    upButton->setTooltip ("Move the current frame up");
    upButton->addListener (this);

    downIcon = Drawable::createFromImageData (BinaryData::downwhite_png,
                                              BinaryData::downwhite_pngSize);

    downButton.reset (new DrawableButton ("downButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (downButton.get());
    downButton->setImages (downIcon.get());
    downButton->setTooltip ("Move the current frame down");
    downButton->addListener (this);
    
    frameList.reset (new ListBox ("frameList", this));
    addAndMakeVisible (frameList.get());
    frameList->setRowHeight (150);
    frameList->setColour (ListBox::backgroundColourId, getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

//    frameList->setMultipleSelectionEnabled (true);
    refresh();
}

FrameList::~FrameList()
{
    frameList = nullptr;
    dupButton = nullptr;
    dupIcon = nullptr;
    downButton = nullptr;
    downIcon = nullptr;
    upButton = nullptr;
    upIcon = nullptr;
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
    addButton->setBounds (12, 8, 32, 32);
    delButton->setBounds (48, 8, 32, 32);
    dupButton->setBounds (84, 8, 32, 32);
    upButton->setBounds (120, 8, 32, 32);
    downButton->setBounds (156, 8, 32, 32);
    frameList->setBounds (10, 48, 180, getHeight()-58);
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
                 Rectangle<float>::leftTopRightBottom (0, 0, (float)width, (float)height),
                 0);
    
    g.setColour (Colour (0x30000000));
    g.fillRect (0, height-22, width, 22);
    
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
        frameEditor->setFrameIndex ((uint16)lastRowSelected);
}

//==============================================================================
void FrameList::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == addButton.get())
        frameEditor->newFrame();
    else if (buttonThatWasClicked == delButton.get())
        frameEditor->deleteFrame();
    else if (buttonThatWasClicked == dupButton.get())
        frameEditor->dupFrame();
    else if (buttonThatWasClicked == upButton.get())
        frameEditor->moveFrameUp();
    else if (buttonThatWasClicked == downButton.get())
        frameEditor->moveFrameDown();
}

//==============================================================================
void FrameList::actionListenerCallback (const String& message)
{
    if (message == EditorActions::framesChanged)
    {
        frameList->updateContent();
        frameList->repaint();
        refresh();
    }
    if (message == EditorActions::frameIndexChanged)
        refresh();
}

//==============================================================================
void FrameList::refresh()
{
    if (frameEditor->getFrameCount() > 1)
    {
        delButton->setEnabled (true);
        upButton->setEnabled (true);
        downButton->setEnabled (true);
    }
    else
    {
        delButton->setEnabled (false);
        upButton->setEnabled (false);
        downButton->setEnabled (false);
    }
    
    if (!frameEditor->getFrameIndex())
        upButton->setEnabled (false);
    
    if (frameEditor->getFrameIndex() >= frameEditor->getFrameCount() - 1)
        downButton->setEnabled (false);
    
    frameList->selectRow (frameEditor->getFrameIndex());
}
