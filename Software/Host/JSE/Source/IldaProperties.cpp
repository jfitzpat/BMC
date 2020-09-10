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

    decSelection.reset (new juce::TextButton ("decSelection"));
    addAndMakeVisible (decSelection.get());
    decSelection->setTooltip ("Shift the selection down.");
    decSelection->setButtonText ("-");
    decSelection->addListener (this);

    incSelection.reset (new juce::TextButton ("incSelection"));
    addAndMakeVisible (incSelection.get());
    incSelection->setTooltip ("Shift the selection up.");
    incSelection->setButtonText ("+");
    incSelection->addListener (this);

    refresh();
}

IldaProperties::~IldaProperties()
{
    layerVisible = nullptr;
    drawLines = nullptr;
    showBlanking = nullptr;
    pointLabel = nullptr;
    selectionLabel = nullptr;
    incSelection = nullptr;
    decSelection = nullptr;
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
    decSelection->setBounds (16, 188, 40, 20);
    incSelection->setBounds (58, 188, 40, 20);
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
    else if (buttonThatWasClicked == decSelection.get())
    {
        adjustSelection (-1);
    }
    else if (buttonThatWasClicked == incSelection.get())
    {
        adjustSelection (1);
    }
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
        // We want to parse any user input into coherent ranges
        // Start with an empty selection
        SparseSet<uint16> selection;
        
        // Get rid of white space and standardize - for ranges
        // and , as the range seperator
        String s = editor.getText().removeCharacters(" ")
                    .replaceCharacter(':', '-')
                    .replaceCharacter(';', ',');
        
        // Check if there is anything left
        // If there isn't an empty range will get submitted below
        if (s.length())
        {
            do
            {
                // Pull off anthing up to the first ','
                String parse = s.upToFirstOccurrenceOf (",", false, false);
                s = s.fromFirstOccurrenceOf (",", false, false);
                
                // If there are two value, split them
                String startString = parse.upToFirstOccurrenceOf ("-", false, false);
                String endString = parse.fromFirstOccurrenceOf ("-", false, false);
                
                // Get the start value, turn into base 1
                int start = startString.getIntValue() - 1;
                
                // Parse or just fake our end
                // End is exclussive in the range, so 1,2 is just the value 1
                int end;
                if (endString.length())
                    end = endString.getIntValue();
                else
                    end = start + 1;
                
                // Check if the range is valid
                // The user might input redundant or adjacent
                // ranges, but SpareSet::addRange should simplify
                if (start >= 0 && end > start && end <= frameEditor->getPointCount())
                    selection.addRange (Range<uint16>((uint16)start, (uint16)end));

            } while (s.length());
        }
        
        // Submit the range and release focus
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
void IldaProperties::updatePointDisplay()
{
    auto p = frameEditor->getPointCount();
    if (p)
    {
        float frameRate = (float)frameEditor->getScanRate() / (float)p;
        
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
        decSelection->setEnabled (false);
        incSelection->setEnabled (false);
    }
    else
    {
        currentSelection->setEnabled (true);
        selectionLabel->setColour (Label::textColourId, juce::Colours::white);
        auto s = frameEditor->getIldaSelection();
        if (s.isEmpty())
        {
            currentSelection->setText (String());
            decSelection->setEnabled (false);
            incSelection->setEnabled (false);
        }
        else
        {
            String sString;
            
            for (auto n = 0; n < s.getNumRanges(); ++n)
            {
                Range<uint16> r = s.getRange (n);

                if (sString.length())
                    sString += ", ";
                
                sString += String(r.getStart() + 1);
                
                if (r.getLength() > 1)
                    sString += "-" + String(r.getEnd());
            }
            
            currentSelection->setText (sString);
            decSelection->setEnabled (true);
            incSelection->setEnabled (true);
        }
    }
}

void IldaProperties::adjustSelection (int offset)
{
    SparseSet<uint16> newSelection;
    int points = (int)frameEditor->getPointCount();
    
    if (offset == 0 || offset >= points)
        return;

    // Valid range
    Range<int> valid (0, points);

    for (auto n = 0; n < frameEditor->getIldaSelection().getNumRanges(); ++n)
    {
        Range<uint16> r = frameEditor->getIldaSelection().getRange (n);
        Range<int> shifted ((int)r.getStart() + offset, (int)r.getEnd() + offset);
        
        // Add the valid part of the shifted range
        Range<int> newRange = valid.getIntersectionWith (shifted);
        newSelection.addRange (Range<uint16> ((uint16)newRange.getStart(),
                                              (uint16)newRange.getEnd()));
        
        // Deal with any wrap off either end with an additional range
        int extra = (int)r.getLength() - newRange.getLength();
        if (extra)
        {
            int s, e;
            
            if (offset > 0)
                s = shifted.getEnd() - extra - points;
            else
                s = shifted.getStart() + points;

            e = s + extra;
            newSelection.addRange (Range<uint16> ((uint16)s, (uint16)e));
        }
    }
    
    frameEditor->setIldaSelection (newSelection);
}

void IldaProperties::refresh()
{
    layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
    updatePointDisplay();
    updateSelection();
}
