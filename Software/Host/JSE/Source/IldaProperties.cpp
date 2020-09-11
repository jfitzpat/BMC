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

    xLabel.reset (new Label ("xLabel", "X"));
    addAndMakeVisible (xLabel.get());
    xLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    xLabel->setJustificationType (juce::Justification::centred);
    xLabel->setEditable (false, false, false);
    xLabel->setColour (Label::textColourId, juce::Colours::grey);
    xLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    yLabel.reset (new Label ("yLabel", "Y"));
    addAndMakeVisible (yLabel.get());
    yLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    yLabel->setJustificationType (juce::Justification::centred);
    yLabel->setEditable (false, false, false);
    yLabel->setColour (Label::textColourId, juce::Colours::grey);
    yLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));
    
    zLabel.reset (new Label ("zLabel", "Z"));
    addAndMakeVisible (zLabel.get());
    zLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    zLabel->setJustificationType (juce::Justification::centred);
    zLabel->setEditable (false, false, false);
    zLabel->setColour (Label::textColourId, juce::Colours::grey);
    zLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    rLabel.reset (new Label ("rLabel", "R"));
    addAndMakeVisible (rLabel.get());
    rLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    rLabel->setJustificationType (juce::Justification::centred);
    rLabel->setEditable (false, false, false);
    rLabel->setColour (Label::textColourId, juce::Colours::grey);
    rLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    gLabel.reset (new Label ("gLabel", "G"));
    addAndMakeVisible (gLabel.get());
    gLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    gLabel->setJustificationType (juce::Justification::centred);
    gLabel->setEditable (false, false, false);
    gLabel->setColour (Label::textColourId, juce::Colours::grey);
    gLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    bLabel.reset (new Label ("bLabel", "B"));
    addAndMakeVisible (bLabel.get());
    bLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    bLabel->setJustificationType (juce::Justification::centred);
    bLabel->setEditable (false, false, false);
    bLabel->setColour (Label::textColourId, juce::Colours::grey);
    bLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    selectionX.reset (new TextEditor ("selectionX"));
    addAndMakeVisible (selectionX.get());
    selectionX->setMultiLine (false);
    selectionX->setReturnKeyStartsNewLine (false);
    selectionX->setReadOnly (false);
    selectionX->setScrollbarsShown (true);
    selectionX->setCaretVisible (true);
    selectionX->setPopupMenuEnabled (true);
    selectionX->setColour (TextEditor::textColourId, Colours::white);
    selectionX->setTooltip ("X coordinate of selected point(s).");
    selectionX->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    selectionX->addListener (this);

    selectionY.reset (new TextEditor ("selectionY"));
    addAndMakeVisible (selectionY.get());
    selectionY->setMultiLine (false);
    selectionY->setReturnKeyStartsNewLine (false);
    selectionY->setReadOnly (false);
    selectionY->setScrollbarsShown (true);
    selectionY->setCaretVisible (true);
    selectionY->setPopupMenuEnabled (true);
    selectionY->setColour (TextEditor::textColourId, Colours::white);
    selectionY->setTooltip ("Y coordinate of selected point(s).");
    selectionY->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    selectionY->addListener (this);

    selectionZ.reset (new TextEditor ("selectionZ"));
    addAndMakeVisible (selectionZ.get());
    selectionZ->setMultiLine (false);
    selectionZ->setReturnKeyStartsNewLine (false);
    selectionZ->setReadOnly (false);
    selectionZ->setScrollbarsShown (true);
    selectionZ->setCaretVisible (true);
    selectionZ->setPopupMenuEnabled (true);
    selectionZ->setColour (TextEditor::textColourId, Colours::white);
    selectionZ->setTooltip ("Z coordinate of selected point(s).");
    selectionZ->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    selectionZ->addListener (this);

    selectionR.reset (new TextEditor ("selectionR"));
    addAndMakeVisible (selectionR.get());
    selectionR->setMultiLine (false);
    selectionR->setReturnKeyStartsNewLine (false);
    selectionR->setReadOnly (false);
    selectionR->setScrollbarsShown (true);
    selectionR->setCaretVisible (true);
    selectionR->setPopupMenuEnabled (true);
    selectionR->setColour (TextEditor::textColourId, Colours::white);
    selectionR->setTooltip ("Red level of selected point(s).");
    selectionR->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789*"), true);
    selectionR->addListener (this);

    selectionG.reset (new TextEditor ("selectionG"));
    addAndMakeVisible (selectionG.get());
    selectionG->setMultiLine (false);
    selectionG->setReturnKeyStartsNewLine (false);
    selectionG->setReadOnly (false);
    selectionG->setScrollbarsShown (true);
    selectionG->setCaretVisible (true);
    selectionG->setPopupMenuEnabled (true);
    selectionG->setColour (TextEditor::textColourId, Colours::white);
    selectionG->setTooltip ("Green level of selected point(s).");
    selectionG->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789*"), true);
    selectionG->addListener (this);

    selectionB.reset (new TextEditor ("selectionB"));
    addAndMakeVisible (selectionB.get());
    selectionB->setMultiLine (false);
    selectionB->setReturnKeyStartsNewLine (false);
    selectionB->setReadOnly (false);
    selectionB->setScrollbarsShown (true);
    selectionB->setCaretVisible (true);
    selectionB->setPopupMenuEnabled (true);
    selectionB->setColour (TextEditor::textColourId, Colours::white);
    selectionB->setTooltip ("Blue level of selected point(s).");
    selectionB->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789*"), true);
    selectionB->addListener (this);

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
    xLabel = nullptr;
    yLabel = nullptr;
    zLabel = nullptr;
    rLabel = nullptr;
    gLabel = nullptr;
    bLabel = nullptr;
    selectionX = nullptr;
    selectionY = nullptr;
    selectionZ = nullptr;
    selectionR = nullptr;
    selectionG = nullptr;
    selectionB = nullptr;
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
    xLabel->setBounds(16, 216, 54, 12);
    yLabel->setBounds(16 + 56, 216, 54, 12);
    zLabel->setBounds(16 + 112, 216, 54, 12);
    selectionX->setBounds (16, 232, 54, 24);
    selectionY->setBounds (16 + 56, 232, 54, 24);
    selectionZ->setBounds (16 + 112, 232, 54, 24);
    rLabel->setBounds(16, 264, 54, 12);
    gLabel->setBounds(16 + 56, 264, 54, 12);
    bLabel->setBounds(16 + 112, 264, 54, 12);
    selectionR->setBounds (16, 280, 54, 24);
    selectionG->setBounds (16 + 56, 280, 54, 24);
    selectionB->setBounds (16 + 112, 280, 54, 24);
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
        xLabel->setColour (Label::textColourId, juce::Colours::grey);
        yLabel->setColour (Label::textColourId, juce::Colours::grey);
        zLabel->setColour (Label::textColourId, juce::Colours::grey);
        rLabel->setColour (Label::textColourId, juce::Colours::grey);
        gLabel->setColour (Label::textColourId, juce::Colours::grey);
        bLabel->setColour (Label::textColourId, juce::Colours::grey);
        selectionX->setEnabled (false); selectionY->setEnabled (false);
        selectionZ->setEnabled (false); selectionR->setEnabled (false);
        selectionG->setEnabled (false); selectionB->setEnabled (false);
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
            xLabel->setColour (Label::textColourId, juce::Colours::grey);
            yLabel->setColour (Label::textColourId, juce::Colours::grey);
            zLabel->setColour (Label::textColourId, juce::Colours::grey);
            rLabel->setColour (Label::textColourId, juce::Colours::grey);
            gLabel->setColour (Label::textColourId, juce::Colours::grey);
            bLabel->setColour (Label::textColourId, juce::Colours::grey);
            selectionX->setText (String()); selectionY->setText (String());
            selectionZ->setText (String()); selectionR->setText (String());
            selectionG->setText (String()); selectionB->setText (String());
            selectionX->setEnabled (false); selectionY->setEnabled (false);
            selectionZ->setEnabled (false); selectionR->setEnabled (false);
            selectionG->setEnabled (false); selectionB->setEnabled (false);
        }
        else
        {
            String sString;
            bool mx = false;    bool my = false;
            bool mz = false;    bool mr = false;
            bool mg = false;    bool mb = false;
            
            Frame::XYPoint lastPoint;
            
            for (auto n = 0; n < s.getNumRanges(); ++n)
            {
                // Get the range
                Range<uint16> r = s.getRange (n);

                // Build the string
                if (sString.length())
                    sString += ", ";
                
                sString += String(r.getStart() + 1);
                
                if (r.getLength() > 1)
                    sString += "-" + String(r.getEnd());
                
                // Check values
                if (n == 0)
                    frameEditor->getPoint (r.getStart(), lastPoint);
                
                Frame::XYPoint newPoint;
                
                for (auto i = 0; i < r.getLength(); ++i)
                {
                    frameEditor->getPoint (r.getStart() + i, newPoint);
                    
                    mx |= newPoint.x.w != lastPoint.x.w;
                    my |= newPoint.y.w != lastPoint.y.w;
                    mz |= newPoint.z.w != lastPoint.z.w;
                    mr |= newPoint.red != lastPoint.red;
                    mg |= newPoint.green != lastPoint.green;
                    mb |= newPoint.blue != lastPoint.blue;
                }
            }
            
            currentSelection->setText (sString);
            decSelection->setEnabled (true);
            incSelection->setEnabled (true);
            xLabel->setColour (Label::textColourId, juce::Colours::white);
            yLabel->setColour (Label::textColourId, juce::Colours::white);
            zLabel->setColour (Label::textColourId, juce::Colours::white);
            rLabel->setColour (Label::textColourId, juce::Colours::white);
            gLabel->setColour (Label::textColourId, juce::Colours::white);
            bLabel->setColour (Label::textColourId, juce::Colours::white);
            selectionX->setEnabled (true); selectionY->setEnabled (true);
            selectionZ->setEnabled (true); selectionR->setEnabled (true);
            selectionG->setEnabled (true); selectionB->setEnabled (true);
            
            selectionX->setText ( mx ? "*" : String (lastPoint.x.w));
            selectionY->setText ( my ? "*" : String (lastPoint.y.w));
            selectionZ->setText ( mz ? "*" : String (lastPoint.z.w));
            selectionR->setText ( mr ? "*" : String (lastPoint.red));
            selectionG->setText ( mg ? "*" : String (lastPoint.green));
            selectionB->setText ( mb ? "*" : String (lastPoint.blue));

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
