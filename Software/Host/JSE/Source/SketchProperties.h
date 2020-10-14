/*
    SketchProperties.h
    Properties Pane for Sketch layer
 
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
#include "ColourButton.h"
#include "FrameEditor.h"

//==============================================================================
/*
*/
class SketchProperties  : public Component,
                          public ActionListener,
                          public Button::Listener,
                          public ChangeListener
{
public:
    SketchProperties (FrameEditor* editor);
    ~SketchProperties() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

private:
    void refresh();
    void updateTools();
    
    FrameEditor* frameEditor;

    std::unique_ptr<ToggleButton> layerVisible;
    std::unique_ptr<Drawable> selectIcon;
    std::unique_ptr<DrawableButton> selectToolButton;
    std::unique_ptr<Drawable> moveIcon;
    std::unique_ptr<DrawableButton> moveToolButton;
    std::unique_ptr <Drawable> ellipseIcon;
    std::unique_ptr<DrawableButton> ellipseToolButton;
    std::unique_ptr <Drawable> penIcon;
    std::unique_ptr<DrawableButton> penToolButton;
    std::unique_ptr<ColourButton> toolColorButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SketchProperties)
};
