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
#include "WorkingArea.h"


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

    //==============================================================================
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    void mouseMagnify (const MouseEvent&, float) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

    //==============================================================================
    void setZoom (float zoom);
    float getZoom () { return frameEditor->getZoomFactor(); }
    
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    
private:
    void keepOnscreen (int x, int y);
    void translateWorkingToMain (int& x, int& y);
    void findZoomPoint (const MouseEvent& event, int& x, int& y);
    
    FrameEditor* frameEditor;
    std::unique_ptr<WorkingArea> workingArea;
    
    Rectangle<int> activeArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainEditor)
};
