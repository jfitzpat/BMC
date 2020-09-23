/*
    IldaProperties.h
    Properties Pane for ILDA layer
 
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
#include "ScalePopup.h"
#include "RotatePopup.h"
#include "FrameEditor.h"

//==============================================================================
/*
*/
class IldaProperties  : public Component,
                        public Button::Listener,
                        public TextEditor::Listener,
                        public ActionListener,
                        public ChangeListener
{
public:
    IldaProperties (FrameEditor* editor);
    ~IldaProperties() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;

    //==============================================================================
    void textEditorReturnKeyPressed (TextEditor& editor) override;
    void textEditorEscapeKeyPressed (TextEditor& editor) override;
    void textEditorFocusLost (TextEditor& editor) override;
    
private:
    void updatePointDisplay();
    void updateSelection();
    void disableSelectionTools();
    void updateTools();
    
    void refresh();

    FrameEditor* frameEditor;
    Colour selectionColour;

    std::unique_ptr<ToggleButton> layerVisible;
    std::unique_ptr<ToggleButton> showBlanking;
    std::unique_ptr<ToggleButton> drawLines;
    std::unique_ptr<Label> pointLabel;
    std::unique_ptr<Drawable> selectIcon;
    std::unique_ptr<DrawableButton> selectToolButton;
    std::unique_ptr<Drawable> moveIcon;
    std::unique_ptr<DrawableButton> moveToolButton;
    std::unique_ptr <Drawable> pointIcon;
    std::unique_ptr<DrawableButton> pointToolButton;
    std::unique_ptr<ColourButton> pointToolColorButton;
    std::unique_ptr<Label> selectionLabel;
    std::unique_ptr<TextEditor> currentSelection;
    std::unique_ptr<TextButton> decSelection;
    std::unique_ptr<TextButton> incSelection;
    std::unique_ptr<Label> xLabel;
    std::unique_ptr<Label> yLabel;
    std::unique_ptr<Label> zLabel;
    std::unique_ptr<Label> rLabel;
    std::unique_ptr<Label> gLabel;
    std::unique_ptr<Label> bLabel;
    std::unique_ptr<TextEditor> selectionX;
    std::unique_ptr<TextEditor> selectionY;
    std::unique_ptr<TextEditor> selectionZ;
    std::unique_ptr<TextEditor> selectionR;
    std::unique_ptr<TextEditor> selectionG;
    std::unique_ptr<TextEditor> selectionB;
    std::unique_ptr<ColourButton> colorButton;
    std::unique_ptr<Drawable> trashIcon;
    std::unique_ptr<DrawableButton> trashButton;
    std::unique_ptr<Drawable> centerIcon;
    std::unique_ptr<DrawableButton> centerButton;
    std::unique_ptr<Drawable> scaleIcon;
    std::unique_ptr<ScaleButton> scaleButton;
    std::unique_ptr<Drawable> rotateIcon;
    std::unique_ptr<RotateButton> rotateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IldaProperties)
};
