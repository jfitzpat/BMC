/*
    IldaProperties.cpp
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

#include <JuceHeader.h>
#include "IldaProperties.h"

//==============================================================================
IldaProperties::IldaProperties (FrameEditor* editor)
{
    frameEditor = editor;
    frameEditor->addActionListener (this);
    
    layerVisible.reset (new juce::ToggleButton ("layerVisible"));
    addAndMakeVisible (layerVisible.get());
    layerVisible->setTooltip ("Make this layer visible or invisible.");
    layerVisible->setButtonText ("Visible");
    layerVisible->addListener (this);

    drawLines.reset (new juce::ToggleButton ("drawLines"));
    addAndMakeVisible (drawLines.get());
    drawLines->setTooltip ("Draw lines between coordinates");
    drawLines->setButtonText ("Draw Lines");
    drawLines->addListener (this);

    showBlanking.reset (new juce::ToggleButton ("showBlanking"));
    addAndMakeVisible (showBlanking.get());
    showBlanking->setTooltip ("Show blanked (invisible) coordinates.");
    showBlanking->setButtonText ("Show Blanked Points");
    showBlanking->addListener (this);

    pointLabel.reset (new Label ("pointLabel"));
    addAndMakeVisible (pointLabel.get());
    pointLabel->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    pointLabel->setJustificationType (juce::Justification::centredLeft);
    pointLabel->setEditable (false, false, false);
    pointLabel->setColour (Label::textColourId, juce::Colours::white);
    pointLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    selectionLabel.reset (new Label ("selectionLabel", "Selected"));
    addAndMakeVisible (selectionLabel.get());
    selectionLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    selectionLabel->setJustificationType (juce::Justification::centred);
    selectionLabel->setEditable (false, false, false);
    selectionLabel->setColour (Label::textColourId, juce::Colours::grey);
    selectionLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));
    
    currentSelection.reset (new TextEditor ("currentSelection"));
    addAndMakeVisible (currentSelection.get());
    currentSelection->setMultiLine (false);
    currentSelection->setReturnKeyStartsNewLine (false);
    currentSelection->setReadOnly (false);
    currentSelection->setScrollbarsShown (true);
    currentSelection->setCaretVisible (true);
    currentSelection->setPopupMenuEnabled (true);
    currentSelection->setColour (TextEditor::textColourId, Colours::white);
    currentSelection->setTooltip ("Selected point(s).");
    currentSelection->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789 -:,;"), true);
    currentSelection->addListener (this);

    refresh();
}

IldaProperties::~IldaProperties()
{
    layerVisible = nullptr;
    drawLines = nullptr;
    showBlanking = nullptr;
    pointLabel = nullptr;
    selectionLabel = nullptr;
}

//==============================================================================
void IldaProperties::paint (juce::Graphics& g)
{
    // Clear background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void IldaProperties::resized()
{
    layerVisible->setBounds (16, 16, getWidth() - 32, 24);
    drawLines->setBounds (16, 48, getWidth() - 32, 24);
    showBlanking->setBounds (16, 80, getWidth() - 32, 24);
    pointLabel->setBounds (16, 112, getWidth() - 32, 24);
    selectionLabel->setBounds (16, 144, getWidth() - 32, 12);
    currentSelection->setBounds (16, 160, getWidth() - 32, 24);
}

//==============================================================================

void IldaProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::ildaVisibilityChanged)
        layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    else if (message == EditorActions::ildaShowBlankChanged)
        showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    else if (message == EditorActions::ildaDrawLinesChanged)
        drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
    else if (message == EditorActions::frameIndexChanged)
        refresh();
    else if (message == EditorActions::ildaSelectionChanged)
        updateSelection();
}

//==============================================================================


void IldaProperties::textEditorReturnKeyPressed (TextEditor& editor)
{
    if (&editor == currentSelection.get())
    {
        SparseSet<uint> selection;
        
        String s = editor.getText().removeCharacters(" ")
                    .replaceCharacter(':', '-')
                    .replaceCharacter(';', ',');
        
        if (s.length())
        {
            do
            {
                String parse = s.upToFirstOccurrenceOf (",", false, false);
                s = s.fromFirstOccurrenceOf (",", false, false);
                
                String startString = parse.upToFirstOccurrenceOf ("-", false, false);
                String endString = parse.fromFirstOccurrenceOf ("-", false, false);
                
                int start = startString.getIntValue() - 1;
                int end;
                if (endString.length())
                    end = endString.getIntValue();
                else
                    end = start + 1;
                
                if (start >= 0 && end > start && end <= frameEditor->getPointCount())
                    selection.addRange (Range<uint>(start, end));
            } while (s.length());
        }
        
        frameEditor->setIldaSelection (selection);
        layerVisible->grabKeyboardFocus();
    }
}

void IldaProperties::textEditorEscapeKeyPressed (TextEditor& editor)
{
    if (&editor == currentSelection.get())
        layerVisible->grabKeyboardFocus();
}

void IldaProperties::textEditorFocusLost (TextEditor& editor)
{
    if (&editor == currentSelection.get())
        updateSelection();
}

//==============================================================================
void IldaProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == layerVisible.get())
    {
        frameEditor->setIldaVisible (layerVisible->getToggleState());
    }
    else if (buttonThatWasClicked == showBlanking.get())
    {
        frameEditor->setIldaShowBlanked (showBlanking->getToggleState());
    }
    else if (buttonThatWasClicked == drawLines.get())
    {
        frameEditor->setIldaDrawLines (drawLines->getToggleState());
    }
}

//==============================================================================
void IldaProperties::updatePointDisplay()
{
    auto p = frameEditor->getPointCount();
    if (p)
    {
        float frameRate = frameEditor->getScanRate() / p;
        
        if (frameRate < 15.0)
            pointLabel->setColour (Label::textColourId, juce::Colours::yellow);
        else if (frameRate < 10.0)
            pointLabel->setColour (Label::textColourId, juce::Colours::red);
        else
            pointLabel->setColour (Label::textColourId, juce::Colours::white);
        
        pointLabel->setText (String(p) + " points (" + String(frameRate, 1) + " fps)", dontSendNotification);
    }
    else
    {
        pointLabel->setText (String(p) + " points", dontSendNotification);
        pointLabel->setColour (Label::textColourId, juce::Colours::grey);
    }
}

void IldaProperties::updateSelection()
{
    if (! frameEditor->getPointCount())
    {
        selectionLabel->setColour (Label::textColourId, juce::Colours::grey);
        currentSelection->setText (String());
        currentSelection->setEnabled (false);
    }
    else
    {
        currentSelection->setEnabled (true);
        selectionLabel->setColour (Label::textColourId, juce::Colours::white);
        auto s = frameEditor->getIldaSelection();
        if (s.isEmpty())
            currentSelection->setText (String());
        else
        {
            String sString;
            
            for (auto n = 0; n < s.getNumRanges(); ++n)
            {
                Range<uint> r = s.getRange (n);

                if (sString.length())
                    sString += ", ";
                
                sString += String(r.getStart() + 1);
                
                if (r.getLength() > 1)
                    sString += "-" + String(r.getEnd());
            }
            
            currentSelection->setText (sString);
        }
    }
}

void IldaProperties::refresh()
{
    layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
    updatePointDisplay();
    updateSelection();
}
