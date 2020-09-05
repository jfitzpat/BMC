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
    float scale = (float)activeArea.getHeight() / 65536.0f;
    float invScale = 1.0 / scale;
    
    workingArea->setTransform (AffineTransform::scale (scale));
    
    workingArea->setBounds (activeArea.getX() * invScale,
                            activeArea.getY() * invScale,
                            65536, 65536);
    workingArea->setActiveScale (scale);
    workingArea->setActiveInvScale (invScale);
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
    
    // Background Image
    if (frameEditor->getRefVisible())
    {
        g.setOpacity (frameEditor->getRefOpacity());
        const Image* i = frameEditor->getImage();
        if (i != nullptr)
        {
            auto w = i->getWidth();
            auto h = i->getHeight();
            float scale;
            float iscale = frameEditor->getImageScale();
            
            if (w > h)
                scale = iscale * 65536.0 / w;
            else
                scale = iscale * 65536.0 / h;

            float x = 32768 - (w * scale / 2) +
                (frameEditor->getImageXoffset() / 100.0 * 65536.0);
            float y = 32768 - (h * scale / 2) +
                (frameEditor->getImageYoffset() / 100.0 * 65536.0);

            AffineTransform t = \
                AffineTransform::rotation (frameEditor->getImageRotation() * MathConstants<float>::pi / 180.0,
                    w / 2.0,
                    h / 2.0) \
                .followedBy (AffineTransform::scale (scale)) \
                .followedBy (AffineTransform::translation(x, y));
            
            g.drawImageTransformed (*i, t);
        }
    }
    
    // ILDA points
    if (frameEditor->getIldaVisible())
    {
        float size = 3.0 * activeInvScale;
        float halfSize = size / 2.0;
        
        for (auto n = 0; n < frameEditor->getPointCount(); ++n)
        {
            Frame::XYPoint point;
            
            if (frameEditor->getPoint (n, point))
            {
                if (point.status & Frame::BlankedPoint)
                {
                    g.setColour (Colours::darkgrey);
                    g.drawEllipse((float)(point.x.w + (32768 - halfSize)),
                               (float)((32768 - halfSize) - point.y.w), size, size, activeInvScale);
                }
                else
                {
                    g.setColour (Colour (point.red, point.blue, point.green));
                    g.fillEllipse(point.x.w + (32768 - halfSize), (32768 - halfSize) - point.y.w, size, size);
                }
            }
        }
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
    else if (message == EditorActions::backgroundImageAdjusted)
        repaint();
    else if (message == EditorActions::frameIndexChanged)
        repaint();
}
