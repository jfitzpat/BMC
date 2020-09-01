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
    
    workingArea.reset (new WorkingArea(frame));
    addAndMakeVisible (workingArea.get());
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
    
    // Scale our working area to fit on screen, but still
    // draw in full ILDA space
    activeScale = (float)activeArea.getHeight() / 65535.0f;
    activeInvScale = 1.0 / activeScale;
    workingArea->setTransform (AffineTransform::scale (activeScale));
    
    workingArea->setBounds (activeArea.getX() * activeInvScale,
                            activeArea.getY() * activeInvScale,
                            65535, 65535);
}

void MainEditor::actionListenerCallback (const String& message)
{
    
}

//==============================================================================
MainEditor::WorkingArea::WorkingArea (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
}

MainEditor::WorkingArea::~WorkingArea()
{
}

void MainEditor::WorkingArea::mouseDown(const MouseEvent& event)
{
    Logger::outputDebugString ("Mouse: " + String(event.getMouseDownX() - 32768) +
                               ", " + String(32768 - event.getMouseDownY()));
}

void MainEditor::WorkingArea::paint (juce::Graphics& g)
{
    // Black background
    g.fillAll (Colours::transparentBlack);
    
    if (frameEditor->getRefVisible())
    {
        g.setOpacity (frameEditor->getRefOpacity());
        const Image* i = frameEditor->getImage();
        if (i != nullptr)
            g.drawImage (*i, {0, 0, (float)getWidth(), (float)getHeight()},
                     0);
    }
}

void MainEditor::WorkingArea::resized()
{
}

void MainEditor::WorkingArea::actionListenerCallback (const String& message)
{
    if (message == EditorActions::backgroundImageChanged)
        repaint();
    else if (message == EditorActions::sketchVisibilityChanged)
        repaint();
    else if (message == EditorActions::ildaVisibilityChanged)
        repaint();
    else if (message == EditorActions::refVisibilityChanged)
        repaint();
    else if (message == EditorActions::refOpacityChanged)
        repaint();
}
