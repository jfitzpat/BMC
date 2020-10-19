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
                          public TextEditor::Listener,
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
    void textEditorReturnKeyPressed (TextEditor& editor) override;
    void textEditorEscapeKeyPressed (TextEditor& editor) override;
    void textEditorFocusLost (TextEditor& editor) override;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

private:
    void refresh();
    void updateTools();
    void updateSelection();
    
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
    std::unique_ptr<Label> selectLabel;
    std::unique_ptr<Label> pointsLabel;
    std::unique_ptr<Label> spacingLabel;
    std::unique_ptr<TextEditor> spacing;
    std::unique_ptr<Label> extraPerLabel;
    std::unique_ptr<TextEditor> extraPerAnchor;
    std::unique_ptr<Label> blankBeforeLabel;
    std::unique_ptr<TextEditor> blankBefore;
    std::unique_ptr<Label> blankAfterLabel;
    std::unique_ptr<TextEditor> blankAfter;
    std::unique_ptr<ColourButton> selectColorButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SketchProperties)
};
