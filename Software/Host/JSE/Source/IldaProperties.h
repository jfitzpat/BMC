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
#include "FrameEditor.h"

//==============================================================================
/*
*/
class IldaProperties  : public Component,
                        public Button::Listener,
                        public TextEditor::Listener,
                        public ActionListener
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
    void textEditorReturnKeyPressed (TextEditor& editor) override;
    void textEditorEscapeKeyPressed (TextEditor& editor) override;
    void textEditorFocusLost (TextEditor& editor) override;
    
private:
    void updatePointDisplay();
    void updateSelection();
    void adjustSelection (int offset);
    
    void refresh();

    FrameEditor* frameEditor;
    std::unique_ptr<ToggleButton> layerVisible;
    std::unique_ptr<ToggleButton> showBlanking;
    std::unique_ptr<ToggleButton> drawLines;
    std::unique_ptr<Label> pointLabel;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IldaProperties)
};
