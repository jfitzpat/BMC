/*
    FrameList.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"

//==============================================================================
/*
*/
class FrameList  : public Component,
                   public ListBoxModel,
                   public Button::Listener,
                   public ActionListener
{
public:
    FrameList (FrameEditor* frame);
    ~FrameList() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    int getNumRows() override;

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected) override;

    void selectedRowsChanged (int lastRowSelected) override;

    //==============================================================================
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

private:
    void refresh();
    
    FrameEditor* frameEditor;
    
    std::unique_ptr<TextButton> addButton;
    std::unique_ptr<TextButton> delButton;
    std::unique_ptr<Drawable> dupIcon;
    std::unique_ptr<DrawableButton> dupButton;
    std::unique_ptr<Drawable> upIcon;
    std::unique_ptr<DrawableButton> upButton;
    std::unique_ptr<Drawable> downIcon;
    std::unique_ptr<DrawableButton> downButton;
    std::unique_ptr<ListBox> frameList;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameList)
};
