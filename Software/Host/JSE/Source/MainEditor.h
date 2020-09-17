/*
    MainEditor.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"


//==============================================================================
class MainEditor  : public Component,
                    public ActionListener
{
public:
    MainEditor (FrameEditor* frame);
    ~MainEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

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
        FrameEditor* frameEditor;
        float activeScale;
        float activeInvScale;
        
        bool drawMark;
        uint16 markIndex;
        Rectangle<int> lastMarkRect;
        bool drawRect;
        Rectangle<int> lastDrawRect;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WorkingArea)
    };

private:
    void keepOnscreen (int x, int y);
    
    float zoomFactor;
    FrameEditor* frameEditor;
    std::unique_ptr<WorkingArea> workingArea;
    
    Rectangle<int> activeArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
