/*
    WorkingArea.cpp
    Working Area component, kept at ILDA resolution

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
#include "WorkingArea.h"

//==============================================================================
WorkingArea::WorkingArea (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
    
    updateCursor();
    
    drawMark = false;
    drawRect = false;
    drawDot = false;
}

WorkingArea::~WorkingArea()
{
}

void WorkingArea::killMarkers()
{
    if (drawMark)
    {
        drawMark = false;
        repaint (lastMarkRect);
    }
    
    if (drawDot)
    {
        drawDot = false;
        repaint (lastDotRect);
    }
}

void WorkingArea::mouseDownIldaPoint (const MouseEvent& event)
{
    if (drawMark)
    {
        SparseSet<uint16> selection;
        
        selection.addRange (Range<uint16>(markIndex, markIndex + 1));
        
        drawMark = false;
        frameEditor->setIldaSelection (selection);
    }
}

void WorkingArea::mouseDownIldaSelect (const MouseEvent& event)
{
    // Has user highlighted a point?
    if (drawMark)
    {
        SparseSet<uint16> selection;
        
        if (event.mods.isCommandDown() || event.mods.isAltDown())
            selection = frameEditor->getIldaSelection();
        
        if (selection.contains (markIndex) || event.mods.isAltDown())
            selection.removeRange (Range<uint16>(markIndex, markIndex + 1));
        else
            selection.addRange (Range<uint16>(markIndex, markIndex + 1));
        
        drawMark = false;
        frameEditor->setIldaSelection (selection);
    }
    else
    {
        drawRect = true;
        lastDrawRect = Rectangle<int>(event.getMouseDownPosition(), event.getPosition());
    }
}

void WorkingArea::mouseDown (const MouseEvent& event)
{
    // Left mouse only for now
    if (! event.mods.isLeftButtonDown())
        return;
    
    // Only dealing with ILDA layer
    if (frameEditor->getActiveLayer() != FrameEditor::ilda)
        return;
    
    if (frameEditor->getActiveIldaTool() == FrameEditor::selectTool)
        mouseDownIldaSelect (event);
    else if (frameEditor->getActiveIldaTool() == FrameEditor::pointTool)
        mouseDownIldaPoint (event);
}

void WorkingArea::mouseUp (const MouseEvent& event)
{
    if (drawRect)
    {
        drawRect = false;
        int expansion = (int)(activeInvScale * 3);
        repaint (lastDrawRect.expanded (expansion, expansion));
        
        // Search for points to alter to/from selection
        // First, rectangle to ILDA space
        
        int x = lastDrawRect.getX() - 32768;
        int y = 32767 - (lastDrawRect.getY() + lastDrawRect.getHeight());

        Rectangle<int16> r (x, y, (uint16)lastDrawRect.getWidth(), (uint16)lastDrawRect.getHeight());
        
        // Check if we should be using the existing selection
        SparseSet<uint16> selection;
        if (event.mods.isAltDown() || event.mods.isCommandDown())
            selection = frameEditor->getIldaSelection();
        
        // Loop through all the points
        for (uint16 n = 0; n < frameEditor->getPointCount(); ++n)
        {
            Frame::XYPoint point;
            frameEditor->getPoint (n, point);
            if (r.contains (point.x.w, point.y.w))
            {
                if (frameEditor->getIldaShowBlanked() ||
                    (! (point.status & Frame::BlankedPoint)))
                {
                    if (event.mods.isAltDown())
                        selection.removeRange (Range<uint16>(n, n + 1));
                    else
                        selection.addRange (Range<uint16>(n, n + 1));
                }
            }
        }
        
        // Make the selection
        frameEditor->setIldaSelection (selection);
        
        // Discard the rect
        lastDrawRect = Rectangle<int>();
    }
}

void WorkingArea::mouseMoveIldaPoint (const MouseEvent& event)
{
    int x = event.x;
    int y = event.y;
    
    if (!frameEditor->getPointCount())
    {
        dotAt = Point<int> (x, y);
        dotFrom = -1;
        dotTo = -1;
        
        if (drawDot == true)
            repaint (lastDotRect);
        drawDot = true;
        
        lastDotRect = Rectangle<int>(x - (int)(15 * activeInvScale),
                                     y - (int)(15 * activeInvScale),
                                     (int)(30 * activeInvScale),
                                     (int)(30 * activeInvScale));
        repaint (lastDotRect);
    }
    else if (! frameEditor->getIldaSelection().isEmpty())
    {
        dotAt = Point<int> (x, y);

        SparseSet<uint16> selection = frameEditor->getIldaSelection();
        Range<uint16> r = selection.getRange (selection.getNumRanges() - 1);
        dotFrom = r.getEnd() - 1;
        dotTo = dotFrom + 1;
        if (dotTo >= frameEditor->getPointCount())
            dotTo = 0;

        if (drawDot == true)
            repaint (lastDotRect);
        drawDot = true;
        
        // !!!!
        lastDotRect = getBounds();
        repaint (lastDotRect);
    }
    else
        mouseMoveIldaSelect (event);
}

void WorkingArea::mouseMoveIldaSelect (const MouseEvent& event)
{
    // Create a test rectangle in ILDA space with a 6 pixel margin
    int x = event.x - 32768;
    int y = 32767 - event.y;
    
    Rectangle<int16> r(x - (int16)(3 * activeInvScale), y - (int16)(3 * activeInvScale), (int16)(6 * activeInvScale), (int16)(6 * activeInvScale));
    
    // Loop through the points and look for a match
    uint16 n;
    for (n = 0; n < frameEditor->getPointCount(); ++n)
    {
        Frame::XYPoint point;
        frameEditor->getPoint (n, point);
        if (r.contains (point.x.w, point.y.w))
        {
            // Dont' match blanked points unless they are visible
            if (frameEditor->getIldaShowBlanked() ||
                (! (point.status & Frame::BlankedPoint)))
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
    }
    
    // No matches, clear any pending mark
    if (n == frameEditor->getPointCount())
    {
        if (drawMark == true)
        {
            drawMark = false;
            repaint (lastMarkRect);
        }
    }
}

void WorkingArea::mouseMove (const MouseEvent& event)
{
    // If we aren't the ILDA layer case, clear all hover marks
    // and bail
    if (frameEditor->getActiveLayer() != FrameEditor::ilda)
    {
        killMarkers();
        return;
    }
    
    if (frameEditor->getActiveIldaTool() == FrameEditor::selectTool)
        mouseMoveIldaSelect (event);
    else if (frameEditor->getActiveIldaTool() == FrameEditor::pointTool)
        mouseMoveIldaPoint (event);
}

void WorkingArea::mouseDrag (const MouseEvent& event)
{
    if (drawRect)
    {
        int expansion = (int)(activeInvScale * 3);
        repaint (lastDrawRect.expanded (expansion, expansion));
        lastDrawRect = Rectangle<int>(event.getMouseDownPosition(), event.getPosition());
        repaint (lastDrawRect.expanded (expansion, expansion));
    }
}

void WorkingArea::paint (juce::Graphics& g)
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
                // Draw lines
                if (frameEditor->getIldaDrawLines())
                {
                    Frame::XYPoint nextPoint;
                    bool b;
                    
                    if (n < (frameEditor->getPointCount() - 1))
                        b = frameEditor->getPoint (n+1, nextPoint);
                    else
                        b = frameEditor->getPoint (0, nextPoint);

                    if (b && ((n != dotFrom) || (! drawDot)))
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
        
        if (frameEditor->getActiveLayer() == FrameEditor::ilda && drawRect)
        {
            g.setColour (Colours::grey);
            g.drawRect (lastDrawRect, (int)activeInvScale >> 1);
        }

        if (frameEditor->getActiveLayer() == FrameEditor::ilda && drawDot)
        {
            Colour c = frameEditor->getPointToolColor();

            // draw lines
            if (dotFrom != -1)
            {
                Frame::XYPoint point;
                frameEditor->getPoint (dotFrom, point);
                
                if (point.status & Frame::BlankedPoint)
                {
                    g.setColour (Colours::darkgrey);
                    g.drawLine ((float)(point.x.w + 32768),
                                (float)(32767 - point.y.w),
                                (float)dotAt.getX(),
                                (float)dotAt.getY(),
                                activeInvScale);
                }
                else
                {
                    g.setColour (Colour (point.red, point.green, point.blue));
                    g.drawLine ((float)(point.x.w + 32768),
                                (float)(32767 - point.y.w),
                                (float)dotAt.getX(),
                                (float)dotAt.getY(),
                                activeInvScale);
                }
            }

            if (dotTo != -1)
            {
                Frame::XYPoint point;
                frameEditor->getPoint (dotTo, point);
                
                if (c == Colours::black)
                {
                    g.setColour (Colours::darkgrey);
                    g.drawLine ((float)dotAt.getX(),
                                (float)dotAt.getY(),
                                (float)(point.x.w + 32768),
                                (float)(32767 - point.y.w),
                                activeInvScale);
                }
                else
                {
                    g.setColour (c);
                    g.drawLine ((float)dotAt.getX(),
                                (float)dotAt.getY(),
                                (float)(point.x.w + 32768),
                                (float)(32767 - point.y.w),
                                activeInvScale);
                }
            }

            // Draw Dot
            if (c == Colours::black)
            {
                g.setColour (Colours::darkgrey);
                g.drawEllipse((float)dotAt.getX() - halfSelectSize,
                              (float)dotAt.getY() - halfSelectSize,
                              selectSize, selectSize, 2 * activeInvScale);
            }
            else
            {
                g.setColour (frameEditor->getPointToolColor());
                g.fillEllipse((float)dotAt.getX() - halfSelectSize,
                              (float)dotAt.getY() - halfSelectSize,
                              selectSize, selectSize);
            }
        }
    }
}

void WorkingArea::resized()
{
}

void WorkingArea::updateCursor()
{
    if (frameEditor->getActiveLayer() == FrameEditor::ilda)
    {
        switch (frameEditor->getActiveIldaTool())
        {
            case FrameEditor::selectTool:
                setMouseCursor (MouseCursor (MouseCursor::PointingHandCursor));
                break;
                
            case FrameEditor::pointTool:
                setMouseCursor (MouseCursor (MouseCursor::CrosshairCursor));
                break;

            default:
                setMouseCursor (MouseCursor (MouseCursor::NormalCursor));
                break;
        }
    }
    else
        setMouseCursor (MouseCursor (MouseCursor::NormalCursor));
}

void WorkingArea::actionListenerCallback (const String& message)
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
    {
        killMarkers();
        updateCursor();
        repaint();
    }
    else if (message == EditorActions::ildaSelectionChanged)
    {
        killMarkers();
        repaint();
    }
    else if (message == EditorActions::ildaPointsChanged)
        repaint();
    else if (message == EditorActions::ildaToolChanged)
    {
        killMarkers();
        updateCursor();
    }
    else if (message == EditorActions::framesChanged)
        killMarkers();
}
