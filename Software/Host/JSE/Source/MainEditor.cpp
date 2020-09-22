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
    : frameEditor (frame)
{
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
    FrameEditor::View view = frameEditor->getActiveView();
    
    if (view == Frame::top)
        g.setColour (Colour (0, Colours::grey.getGreen(), Colours::grey.getBlue()));
    else if (view == Frame::side)
        g.setColour (Colour (Colours::grey.getRed(), Colours::grey.getGreen(), 0));
    else
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
        
        if (view == Frame::top)
            g.setColour (Colour (0, Colours::darkgrey.getGreen(), Colours::darkgrey.getBlue()));
        else if (view == Frame::side)
            g.setColour (Colour (Colours::darkgrey.getRed(), Colours::darkgrey.getGreen(), 0));
        else
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
    // Manage zoom clippoing
    keepOnscreen (getX(), getY());
    
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

void MainEditor::translateWorkingToMain (int& x, int& y)
{
    workingArea->getTransform().transformPoint (x, y);
    x += getX() + activeArea.getX();
    y += getY() + activeArea.getY();
}

void MainEditor::keepOnscreen (int x, int y)
{
    int tx = 200;
    int ty = 0;
    int bx = getParentWidth() - 200;
    int by = getParentHeight();
    
    getTransform().inverted().transformPoints(tx, ty, bx, by);
    
    if (x > tx)
        x = tx;

    if ((x + getWidth()) < bx)
        x += (bx - (x + getWidth()));
    
    if (y > ty)
        y = ty;
    
    if ((y + getHeight()) < by)
        y += (by - (y + getHeight()));
    
    setTopLeftPosition (x, y);
}

void MainEditor::setZoom (float zoom)
{
    frameEditor->_setZoomFactor (zoom);
    float zoomFactor = frameEditor->getZoomFactor();
    
    int x;
    int y;

    frameEditor->getComponentCenterOfIldaSelection (x, y);
    translateWorkingToMain (x, y);
    
    if (zoomFactor == 1.0)
        setTransform (AffineTransform());
    else
        setTransform (AffineTransform::scale (zoomFactor, zoomFactor, (float)x, (float)y));
        
    keepOnscreen (getX(), getY());
}

#ifdef JUCE_WINDOWS
#define ZOOM_STEP (0.4f)
#else
#define ZOOM_STEP (0.2f)
#endif

void MainEditor::findZoomPoint (const MouseEvent& event, int& x, int& y)
{
    if (! frameEditor->getIldaSelection().isEmpty())
    {
        frameEditor->getComponentCenterOfIldaSelection (x, y);
        translateWorkingToMain (x, y);
    }
    else
    {
        x = event.getEventRelativeTo (workingArea.get()).x;
        y = event.getEventRelativeTo (workingArea.get()).y;
        if (workingArea->getBounds().contains(x, y))
            translateWorkingToMain (x, y);
        else
        {
            x = getBounds().getCentreX();
            y = getBounds().getCentreY();
        }
    }
}

void MainEditor::mouseMagnify (const MouseEvent& event, float scale)
{
    float zoomFactor = frameEditor->getZoomFactor();
    
    if (scale > 1.0)
    {
        zoomFactor += ZOOM_STEP;
        if (zoomFactor > MAX_ZOOM)
            zoomFactor = MAX_ZOOM;
    }
    else
    {
        zoomFactor -= ZOOM_STEP;
        if (zoomFactor < MIN_ZOOM)
            zoomFactor = MIN_ZOOM;
    }

    frameEditor->_setZoomFactor (zoomFactor);
    
    int x;
    int y;
    findZoomPoint (event, x, y);

    if (zoomFactor == 1.0)
        setTransform (AffineTransform());
    else
        setTransform (AffineTransform::scale (zoomFactor, zoomFactor, (float)x, (float)y));

    keepOnscreen (getX(), getY());
}

void MainEditor::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel)
{
    if (event.mods.isCtrlDown())
    {
        float zoomFactor = frameEditor->getZoomFactor();
        
        if (wheel.deltaY > 0)
        {
            zoomFactor += ZOOM_STEP;
            if (zoomFactor > MAX_ZOOM)
                zoomFactor = MAX_ZOOM;
        }
        else
        {
            zoomFactor -= ZOOM_STEP;
            if (zoomFactor < MIN_ZOOM)
                zoomFactor = MIN_ZOOM;
        }

        frameEditor->_setZoomFactor (zoomFactor);
        
        int x;
        int y;
        findZoomPoint (event, x, y);
        
        if (zoomFactor == 1.0)
            setTransform (AffineTransform());
        else
            setTransform (AffineTransform::scale (zoomFactor, zoomFactor, (float)x, (float)y));

        keepOnscreen (getX(), getY());
    }
    else if (wheel.deltaY != 0 && event.mods.isShiftDown())
    {
        auto bounds = getBounds();
        int x = bounds.getX();
        if (wheel.deltaY< 0)
            x -= wheel.isInertial ? 1 : 10;
        else
            x += wheel.isInertial ? 1 : 10;
        
        keepOnscreen (x, bounds.getY());
    }
    else if (wheel.deltaY != 0)
    {
        auto bounds = getBounds();
        int y = bounds.getY();
        if (wheel.deltaY< 0)
            y -= wheel.isInertial ? 1 : 10;
        else
            y += wheel.isInertial ? 1 : 10;
        
        keepOnscreen (bounds.getX(), y);
    }
    else if (wheel.deltaX != 0)
    {
        auto bounds = getBounds();
        int x = bounds.getX();
        if (wheel.deltaX < 0)
            x -= wheel.isInertial ? 1 : 10;
        else
            x += wheel.isInertial ? 1 : 10;

        keepOnscreen (x, bounds.getY());
    }
}

void MainEditor::panLeft()
{
    int x = getX();
    x -= 10;
    keepOnscreen (x, getY());
}

void MainEditor::panRight()
{
    int x = getX();
    x += 10;
    keepOnscreen (x, getY());
}

void MainEditor::panUp()
{
    int y = getY();
    y -= 10;
    keepOnscreen (getX(), y);
}

void MainEditor::panDown()
{
    int y = getY();
    y += 10;
    keepOnscreen (getX(), y);
}

