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

const float DotPitch = 20.0;
const float CenterPitch = 10.0;

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
    g.setColour (Colours::grey);
    g.drawRect (activeArea, 1);
    
    if (frameEditor->getRefVisible() && frameEditor->getRefDrawGrid())
    {
        // Draw the cross hair, we calculate in float so that it will
        // scale well
        Line<float> vline ((float)activeArea.getCentreX(),
                           (float)activeArea.getY(),
                           (float)activeArea.getCentreX(),
                           (float)(activeArea.getY() + activeArea.getHeight()));

        Line<float> hline ((float)activeArea.getX(),
                           (float)activeArea.getCentreY(),
                           (float)activeArea.getX() + activeArea.getWidth(),
                           (float)(activeArea.getCentreY()));

        float segments = (float)activeArea.getWidth() / 73.0f;
        float dashes[] = { segments, segments };
        
        g.drawDashedLine (hline, dashes, 2);
        g.drawDashedLine (vline, dashes, 2);
        
        g.setColour (Colours::darkgrey);
        
        // Draw the dots
        float dotpitch = (float)activeArea.getWidth() / DotPitch;
        for (float y = 1; y < DotPitch; ++y)
        {
            if (y != CenterPitch)
            {
                for (float x = 1; x < DotPitch; ++x)
                {
                    if (x != CenterPitch)
                    {
                        g.fillRect ((float)activeArea.getX() + (x * dotpitch) -1,
                                    (float)activeArea.getY() + (y * dotpitch) -1,
                                    2.0, 2.0);
                    }
                }
            }
        }
    }
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
    float invScale = 1.0f / scale;
    
    workingArea->setTransform (AffineTransform::scale (scale));
    
    workingArea->setBounds ((int)(activeArea.getX() * invScale),
                            (int)(activeArea.getY() * invScale),
                            65536, 65536);
    workingArea->setActiveScale (scale);
    workingArea->setActiveInvScale (invScale);
}

void MainEditor::actionListenerCallback (const String& /*message*/)
{
    
}

//==============================================================================
MainEditor::WorkingArea::WorkingArea (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
    
    drawMark = false;
}

MainEditor::WorkingArea::~WorkingArea()
{
}

void MainEditor::WorkingArea::mouseDown (const MouseEvent& event)
{
}

void MainEditor::WorkingArea::mouseUp (const MouseEvent& event)
{
}

void MainEditor::WorkingArea::mouseMove (const MouseEvent& event)
{
    if (frameEditor->getActiveLayer() != FrameEditor::ilda)
        return;
    
    int x = event.x - 32768;
    int y = 32767 - event.y;
    
    Rectangle<int16> r(x - (int16)(5 * activeInvScale), y - (int16)(5 * activeInvScale), (int16)(10 * activeInvScale), (int16)(10 * activeInvScale));
    
    uint16 n;
    for (n = 0; n < frameEditor->getPointCount(); ++n)
    {
        Frame::XYPoint point;
        frameEditor->getPoint (n, point);
        if (r.contains (point.x.w, point.y.w))
        {
            markIndex = n;

            if (drawMark == true)
                repaint (lastMarkRect);
            drawMark = true;

            lastMarkRect = Rectangle<int>(event.x - (int)(15 * activeInvScale),
                                          event.y - (int)(15 * activeInvScale),
                                          (int)(30 * activeInvScale),
                                          (int)(30 * activeInvScale));
            repaint (lastMarkRect);
            break;
        }
    }
    
    if (n == frameEditor->getPointCount())
    {
        if (drawMark == true)
        {
            drawMark = false;
            repaint (lastMarkRect);
        }
    }
}

void MainEditor::WorkingArea::mouseDrag (const MouseEvent& event)
{
}

void MainEditor::WorkingArea::paint (juce::Graphics& g)
{
    // Black background
    g.fillAll (Colours::transparentBlack);
    
    // Background Image
    if (frameEditor->getRefVisible())
    {
        g.setOpacity (frameEditor->getImageOpacity());
        const Image* i = frameEditor->getImage();
        if (i != nullptr)
        {
            auto w = i->getWidth();
            auto h = i->getHeight();
            float scale;
            float iscale = frameEditor->getImageScale();
            
            if (w > h)
                scale = iscale * 65536.0f / w;
            else
                scale = iscale * 65536.0f / h;

            float x = 32768.0f - (w * scale / 2) +
                (frameEditor->getImageXoffset() / 100.0f * 65536.0f);
            float y = 32768.0f - (h * scale / 2) +
                (frameEditor->getImageYoffset() / 100.0f * 65536.0f);

            AffineTransform t = \
                AffineTransform::rotation (frameEditor->getImageRotation() * MathConstants<float>::pi / 180.0f,
                    w / 2.0f,
                    h / 2.0f) \
                .followedBy (AffineTransform::scale (scale)) \
                .followedBy (AffineTransform::translation(x, y));
            
            g.drawImageTransformed (*i, t);
        }
    }
    
    // ILDA points
    if (frameEditor->getIldaVisible())
    {
        float dotSize = 3.0f * activeInvScale;
        float halfDotSize = dotSize / 2.0f;
        float selectSize = 6.0f * activeInvScale;
        float halfSelectSize = selectSize / 2.0f;
        
        for (uint16 n = 0; n < frameEditor->getPointCount(); ++n)
        {
            Frame::XYPoint point;
            
            if (frameEditor->getPoint (n, point))
            {
                // Draw the dots
                if (point.status & Frame::BlankedPoint)
                {
                    if (frameEditor->getIldaShowBlanked())
                    {
                        g.setColour (Colours::darkgrey);
                        g.drawEllipse((float)(point.x.w + (32768 - halfDotSize)),
                                   (float)((32767 - halfDotSize) - point.y.w), dotSize, dotSize, activeInvScale);
                    }

                    // Mark selected even if ShowBlanked is off, since can be edited
                    if (frameEditor->getIldaSelection().contains (n) &&
                        frameEditor->getActiveLayer() == FrameEditor::ilda)
                    {
                        g.setColour (Colours::lightblue);
                        g.drawEllipse((float)(point.x.w + (32768 - halfSelectSize)),
                                   (float)((32767 - halfSelectSize) - point.y.w), selectSize, selectSize, activeInvScale);
                    }
                }
                else
                {
                    g.setColour (Colour (point.red, point.green, point.blue));
                    g.fillEllipse(point.x.w + (32768 - halfDotSize), (32768 - halfDotSize) - point.y.w, dotSize, dotSize);
                    
                    if (frameEditor->getIldaSelection().contains (n) &&
                        frameEditor->getActiveLayer() == FrameEditor::ilda)
                    {
                        g.setColour (Colours::whitesmoke);
                        g.drawEllipse((float)(point.x.w + (32768 - halfSelectSize)),
                                   (float)((32767 - halfSelectSize) - point.y.w), selectSize, selectSize, activeInvScale);
                    }
                }
                
                // Draw lines
                if (frameEditor->getIldaDrawLines())
                {
                    Frame::XYPoint nextPoint;
                    bool b;
                    
                    if (n < (frameEditor->getPointCount() - 1))
                        b = frameEditor->getPoint (n+1, nextPoint);
                    else
                        b = frameEditor->getPoint (0, nextPoint);

                    if (b)
                    {
                        if (point.status & Frame::BlankedPoint)
                        {
                            if (frameEditor->getIldaShowBlanked())
                            {
                                g.setColour (Colours::darkgrey);
                                g.drawLine ((float)(point.x.w + 32768),
                                            (float)(32767 - point.y.w),
                                            (float)(nextPoint.x.w + 32768),
                                            (float)(32767 - nextPoint.y.w),
                                            activeInvScale);
                            }
                        }
                        else
                        {
                            g.setColour (Colour (point.red, point.green, point.blue));
                            g.drawLine ((float)(point.x.w + 32768),
                                        (float)(32767 - point.y.w),
                                        (float)(nextPoint.x.w + 32768),
                                        (float)(32767 - nextPoint.y.w),
                                        halfDotSize);
                        }
                    }
                }
            }
            if (frameEditor->getActiveLayer() == FrameEditor::ilda && drawMark)
            {
                if (n == markIndex)
                {
                    g.setColour (Colours::white);
                    g.drawEllipse((float)(point.x.w + (32768 - selectSize)),
                               (float)((32767 - selectSize) - point.y.w), 2 * selectSize, 2 * selectSize, 2 * activeInvScale);
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
    else if (message == EditorActions::ildaShowBlankChanged)
        repaint();
    else if (message == EditorActions::ildaDrawLinesChanged)
        repaint();
    else if (message == EditorActions::refDrawGridChanged)
        repaint();
    else if (message == EditorActions::layerChanged)
        repaint();
    else if (message == EditorActions::ildaSelectionChanged)
        repaint();
    else if (message == EditorActions::ildaPointsChanged)
        repaint();
}
