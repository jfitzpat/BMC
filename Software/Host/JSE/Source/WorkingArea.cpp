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

void WorkingArea::insertAnchor (uint16 index, Colour c)
{
    Frame::XYPoint point;
    frameEditor->getPoint (index, point);
    point.red = c.getRed();
    point.green = c.getGreen();
    point.blue = c.getBlue();
    
    if (c == Colours::black)
        point.status = Frame::BlankedPoint;
    else
        point.status = 0;
    
    frameEditor->insertPoint (point);
}

void WorkingArea::mouseDownIldaMove (const MouseEvent& event)
{
    // Left mouse only for now
    if (! event.mods.isLeftButtonDown())
        return;

    if (drawMark)
    {
        // Select the point in question
        SparseSet<uint16> selection;
        selection.addRange (Range<uint16>(markIndex, markIndex + 1));
        drawMark = false;
        frameEditor->setIldaSelection (selection);
    }

    if (! frameEditor->getIldaSelection().isEmpty())
        frameEditor->startTransform ("Move Selection");
}

void WorkingArea::mouseDownIldaPoint (const MouseEvent& event)
{
    // Left mouse only for now
    if (! event.mods.isLeftButtonDown())
        return;

    if (drawMark)
    {
        // Select the point in question
        SparseSet<uint16> selection;
        selection.addRange (Range<uint16>(markIndex, markIndex + 1));
        drawMark = false;
        frameEditor->setIldaSelection (selection);

        // Right click insert an anchor
        if (event.mods.isAltDown())
            insertAnchor (markIndex, frameEditor->getPointToolColor());
    }
    else if (drawDot)
    {
        Frame::XYPoint fromPoint;
        zerostruct (fromPoint);
        if (dotFrom >= 0)
            frameEditor->getPoint (dotFrom, fromPoint);
        
        Frame::XYPoint point;
        zerostruct (point);
    
        FrameEditor::View view = frameEditor->getActiveView();
        
        point.x.w = view == Frame::left ? fromPoint.x.w : Frame::getIldaX (dotAt);
        point.y.w = view == Frame::bottom ? fromPoint.y.w : Frame::getIldaY (dotAt);
        if (view == Frame::front)
            point.z.w = fromPoint.z.w;
        else if (view == Frame::bottom)
            point.z.w = Frame::getIldaY (dotAt);
        else
            point.z.w = Frame::getIldaX (dotAt);
        Colour c = frameEditor->getPointToolColor();
        point.red = c.getRed();
        point.green = c.getGreen();
        point.blue = c.getBlue();
        
        if (c == Colours::black)
            point.status = Frame::BlankedPoint;
        
        drawDot = false;
        frameEditor->insertPoint (point);
    }
}

void WorkingArea::mouseDownIldaSelect (const MouseEvent& event)
{
    // Left mouse only for now
    if (! event.mods.isLeftButtonDown())
        return;
    

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
    // Only dealing with ILDA layer
    if (frameEditor->getActiveLayer() != FrameEditor::ilda)
        return;
    
    if (frameEditor->getActiveIldaTool() == FrameEditor::selectTool)
        mouseDownIldaSelect (event);
    else if (frameEditor->getActiveIldaTool() == FrameEditor::pointTool)
        mouseDownIldaPoint (event);
    else if (frameEditor->getActiveIldaTool() == FrameEditor::moveTool)
        mouseDownIldaMove (event);
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
        Rectangle<int> r = Frame::getIldaRect (lastDrawRect);
        
        // Check if we should be using the existing selection
        SparseSet<uint16> selection;
        if (event.mods.isAltDown() || event.mods.isCommandDown())
            selection = frameEditor->getIldaSelection();
        
        FrameEditor::View view = frameEditor->getActiveView();
        
        // Loop through all the points
        for (uint16 n = 0; n < frameEditor->getPointCount(); ++n)
        {
            Frame::XYPoint point;
            frameEditor->getPoint (n, point);

            int tx = view == Frame::left ? point.z.w : point.x.w;
            int ty = view == Frame::bottom ? point.z.w : point.y.w;

            if (r.contains (tx, ty))
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
    
    if (frameEditor->getActiveIldaTool() == FrameEditor::moveTool)
    {
        if (frameEditor->isTransforming())
            frameEditor->endTransform();
    }
}

void WorkingArea::mouseMoveIldaMove (const MouseEvent& event)
{
    mouseMoveIldaSelect(event);
}

void WorkingArea::mouseMoveIldaPoint (const MouseEvent& event)
{
    int x = event.x;
    int y = event.y;
    FrameEditor::View view = frameEditor->getActiveView();
    
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
        // Ctrl snaps to nearest point (if one)
        // Shift forces to nearest 45 degree angle
        if (event.mods.isCtrlDown())
        {
            int nearest = findCloseMouseMatch (event);
            if (nearest >= 0)
            {
                Frame::XYPoint point;
                frameEditor->getPoint ((uint16)nearest, point);
                x = Frame::getCompXInt (point, view);
                y = Frame::getCompYInt (point, view);
            }
        }
        else if (event.mods.isShiftDown())
        {
            // Since 45 dgrees is tan 1, could probably
            // do this more efficiently with two distances and ratio
            // but this is easy...
            Frame::XYPoint point;
            frameEditor->getPoint (dotFrom, point);
            Point<int> p = Frame::getCompPoint (point, view);
            float angle = p.getAngleToPoint (Point<int>(x, y));
            
            float absA = abs(angle);
            
            if ((absA >= 0.0f && absA < 0.3926991f) ||
                (abs(angle) >= 2.7488936f && abs(angle) < 3.15f))
                x = p.getX();   // Force vertical
            else if (absA >= 0.3926991f && absA < 1.178097f)
                y = p.y - p.getDistanceFrom (Point<int>(x, p.getY()));  // 45 up
            else if (absA >= 1.178097f && absA < 1.9634954f )
                y = p.getY();   // Force horizontal
            else if (absA >= 1.9634954f && absA < 2.7488936f)
                y = p.y + p.getDistanceFrom (Point<int>(x, p.getY()));  // 45 down
        }

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
        
        // Figure out the rectangle that encompasses all points
        // and repaint it
        Path p;
        Frame::XYPoint point;
        frameEditor->getPoint (dotFrom, point);
        p.startNewSubPath (Frame::getCompX (point, view),
                           Frame::getCompY (point, view) );
        p.lineTo (dotAt.getX(), dotAt.getY());
        frameEditor->getPoint (dotTo, point);
        p.lineTo (Frame::getCompX (point, view),
                  Frame::getCompY (point, view));
        
        Rectangle<float> rect = p.getBounds();
        rect = rect.expanded (30 * activeInvScale, 30 * activeInvScale);
        lastDotRect = Rectangle<int> ((int)rect.getX(), (int)rect.getY(),
                                      (int)rect.getWidth(), (int)rect.getHeight());
        repaint (lastDotRect);
    }
    else
        mouseMoveIldaSelect (event);
}

int WorkingArea::findCloseMouseMatch (const MouseEvent& event)
{
    FrameEditor::View view = frameEditor->getActiveView();
    
    // Create a test rectangle in ILDA space with a 6 pixel margin
    int x = Frame::toIldaX (event.x);
    int y = Frame::toIldaY (event.y);
    
    Rectangle<int16> r(x - (int16)(3 * activeInvScale), y - (int16)(3 * activeInvScale), (int16)(6 * activeInvScale), (int16)(6 * activeInvScale));
    
    // Loop through the points and look for a match
    uint16 n;
    for (n = 0; n < frameEditor->getPointCount(); ++n)
    {
        Frame::XYPoint point;
        frameEditor->getPoint (n, point);
        
        uint16 tx = view == Frame::left ? point.z.w : point.x.w;
        uint16 ty = view == Frame::bottom ? point.z.w : point.y.w;
        
        if (r.contains (tx, ty))
            if (frameEditor->getIldaShowBlanked() ||
                (! (point.status & Frame::BlankedPoint)))
                return n;
    }
    
    return -1;
}

void WorkingArea::mouseMoveIldaSelect (const MouseEvent& event)
{
    int n = findCloseMouseMatch (event);
    
    if (n >= 0)
    {
        markIndex = (uint16)n;

        if (drawMark == true)
            repaint (lastMarkRect);
        drawMark = true;

        lastMarkRect = Rectangle<int>(event.x - (int)(15 * activeInvScale),
                                      event.y - (int)(15 * activeInvScale),
                                      (int)(30 * activeInvScale),
                                      (int)(30 * activeInvScale));
        repaint (lastMarkRect);
    }
    else
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
    else if (frameEditor->getActiveIldaTool() == FrameEditor::moveTool)
        mouseMoveIldaMove (event);
}

void WorkingArea::mouseDrag (const MouseEvent& event)
{
    // Left mouse only for now
    if (! event.mods.isLeftButtonDown())
        return;

    if (drawRect)
    {
        int expansion = (int)(activeInvScale * 3);
        repaint (lastDrawRect.expanded (expansion, expansion));
        lastDrawRect = Rectangle<int>(event.getMouseDownPosition(), event.getPosition());
        repaint (lastDrawRect.expanded (expansion, expansion));
    }
    
    if (frameEditor->getActiveLayer() == FrameEditor::ilda &&
        frameEditor->getActiveIldaTool() == FrameEditor::moveTool &&
        frameEditor->isTransforming())
    {
        FrameEditor::View view = frameEditor->getActiveView();
        
        int dx = event.getDistanceFromDragStartX();
        int dy = 0 - event.getDistanceFromDragStartY();
        
        int xOffset = view == Frame::left ? 0 : dx;
        int yOffset = view == Frame::bottom ? 0 : dy;
        int zOffset;
        if (view == Frame::left)
            zOffset = dx;
        else if (view == Frame::bottom)
            zOffset = dy;
        else
            zOffset = 0;
        
        frameEditor->translateIldaSelected (xOffset, yOffset, zOffset, false);
    }
}

void WorkingArea::paint (juce::Graphics& g)
{
    // Black background
    g.fillAll (Colours::transparentBlack);
    
    // Background Image
    if (frameEditor->getRefVisible() && (frameEditor->getActiveView() == Frame::front))
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
        FrameEditor::View view = frameEditor->getActiveView();
        
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
                                g.drawLine (Frame::getCompX (point, view),
                                            Frame::getCompY (point, view),
                                            Frame::getCompX (nextPoint, view),
                                            Frame::getCompY (nextPoint, view),
                                            activeInvScale);
                            }
                        }
                        else
                        {
                            g.setColour (Colour (point.red, point.green, point.blue));
                            g.drawLine (Frame::getCompX (point, view),
                                        Frame::getCompY (point, view),
                                        Frame::getCompX (nextPoint, view),
                                        Frame::getCompY (nextPoint, view),
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
                        g.drawEllipse(Frame::getCompX (point, view) - halfDotSize,
                                      Frame::getCompY (point, view) - halfDotSize,
                                      dotSize, dotSize, activeInvScale);
                    }

                    // Mark selected even if ShowBlanked is off, since can be edited
                    if (frameEditor->getIldaSelection().contains (n) &&
                        frameEditor->getActiveLayer() == FrameEditor::ilda)
                    {
                        g.setColour (Colours::lightblue);
                        g.drawEllipse(Frame::getCompX (point, view) - halfSelectSize,
                                      Frame::getCompY (point, view) - halfSelectSize,
                                      selectSize, selectSize, activeInvScale);
                    }
                }
                else
                {
                    g.setColour (Colour (point.red, point.green, point.blue));
                    g.fillEllipse(Frame::getCompX (point, view) - halfDotSize,
                                  Frame::getCompY (point, view) - halfDotSize,
                                  dotSize, dotSize);
                    
                    if (frameEditor->getIldaSelection().contains (n) &&
                        frameEditor->getActiveLayer() == FrameEditor::ilda)
                    {
                        g.setColour (Colours::whitesmoke);
                        g.drawEllipse(Frame::getCompX (point, view) - halfSelectSize,
                                      Frame::getCompY (point, view) - halfSelectSize,
                                      selectSize, selectSize, activeInvScale);
                    }
                }
            }
            
            if (frameEditor->getActiveLayer() == FrameEditor::ilda && drawMark)
            {
                if (n == markIndex)
                {
                    g.setColour (Colours::white);
                    g.drawEllipse (Frame::getCompX (point, view) - selectSize,
                                   Frame::getCompY (point, view) - selectSize,
                                   2 * selectSize, 2 * selectSize, 2 * activeInvScale);
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
                    g.drawLine (Frame::getCompX (point, view),
                                Frame::getCompY (point, view),
                                (float)dotAt.getX(),
                                (float)dotAt.getY(),
                                activeInvScale);
                }
                else
                {
                    g.setColour (Colour (point.red, point.green, point.blue));
                    g.drawLine (Frame::getCompX (point, view),
                                Frame::getCompY (point, view),
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
                                Frame::getCompX (point, view),
                                Frame::getCompY (point, view),
                                activeInvScale);
                }
                else
                {
                    g.setColour (c);
                    g.drawLine ((float)dotAt.getX(),
                                (float)dotAt.getY(),
                                Frame::getCompX (point, view),
                                Frame::getCompY (point, view),
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
                setMouseCursor (MouseCursor::PointingHandCursor);
                break;
                
            case FrameEditor::pointTool:
                setMouseCursor (MouseCursor::CrosshairCursor);
                break;

            case FrameEditor::moveTool:
                setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
                break;

            default:
                setMouseCursor (MouseCursor::NormalCursor);
                break;
        }
    }
    else
        setMouseCursor (MouseCursor::NormalCursor);
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
    else if (message == EditorActions::viewChanged)
    {
        killMarkers();
        repaint();
    }
    else if (message == EditorActions::ildaSelectionChanged)
    {
        killMarkers();
        repaint();
    }
    else if (message == EditorActions::ildaPointsChanged)
        repaint();
    else if (message == EditorActions::ildaPointToolColorChanged)
        repaint();
    else if (message == EditorActions::ildaToolChanged)
    {
        killMarkers();
        updateCursor();
    }
    else if (message == EditorActions::framesChanged)
        killMarkers();
    else if (message == EditorActions::deleteRequest)
    {
        if (drawDot)
        {
            uint16 i = dotFrom;
            
            if (i > 0)
                i--;
            
            killMarkers();
            frameEditor->deletePoints();
            SparseSet<uint16> selection;
            selection.addRange (Range<uint16> (i, i+1));
            frameEditor->setIldaSelection (selection);
        }
    }
}
