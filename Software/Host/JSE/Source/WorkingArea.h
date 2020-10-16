/*
    WorkingArea.h
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

#pragma once

#include "FrameEditor.h"
#include <JuceHeader.h>

//==============================================================================
/*
*/
//==============================================================================
class WorkingArea : public Component,
                    public ActionListener
{
public:
    WorkingArea (FrameEditor* frame);
    ~WorkingArea() override;

    //==============================================================================
    void mouseDown (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;
    void mouseDrag (const MouseEvent& event) override;
    void mouseMove (const MouseEvent& event) override;
    
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

    //==============================================================================
    float getActiveScale() { return activeScale; }
    void setActiveScale (float scale) { activeScale = scale; }
    float getActiveInvScale() { return activeInvScale; }
    void setActiveInvScale (float scale) { activeInvScale = scale; }

    //==============================================================================
    void updateCursor();
    
private:
    void killMarkers();
    void insertAnchor (uint16 index, Colour c);
    int findCloseMouseMatch (const MouseEvent& event);
    void findAllCloseSiblings (uint16 index, SparseSet<uint16>& set);
    void findAllSameColor (Colour color, SparseSet<uint16>& set);
    void findAllSameColor (uint16 index, SparseSet<uint16>& set);
    void findAllSameVisibility (bool blanked, SparseSet<uint16>& set);
    void findAllSameVisibility (uint16 index, SparseSet<uint16>& set);
    void rightClickIldaSelect (const MouseEvent& event);
    void mouseDownIldaSelect (const MouseEvent& event);
    void mouseDownIldaMove (const MouseEvent& event);
    void mouseDownIldaPoint (const MouseEvent& event);
    void mouseDownSketchSelect (const MouseEvent& event);
    void mouseUpIlda (const MouseEvent& event);
    void mouseUpSketch (const MouseEvent& event);
    void mouseMoveIldaSelect (const MouseEvent& event);
    void mouseMoveIldaMove (const MouseEvent& event);
    void mouseMoveIldaPoint (const MouseEvent& event);
    void mouseMoveSketchSelect (const MouseEvent& event);

    FrameEditor* frameEditor;
    float activeScale;
    float activeInvScale;
    
    bool drawMark;
    uint16 markIndex;
    Rectangle<int> lastMarkRect;
    
    bool drawRect;
    Rectangle<int> lastDrawRect;
    
    bool drawDot;
    Point<int> dotAt;
    int dotFrom;
    int dotTo;
    Rectangle<int> lastDotRect;

    bool drawSMark;
    int sMarkIndex;
    int sMarkAnchorIndex;
    Rectangle<int> lastSMarkRect;
    
    bool drawSDot;
    Point<int> sDotAt;
    int sDotFrom;
    int sDotTo;
    Rectangle<int> lastSDotRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WorkingArea)
};

