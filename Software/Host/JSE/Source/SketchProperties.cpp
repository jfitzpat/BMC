/*
    SketchProperties.cpp
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

#include <JuceHeader.h>
#include "SketchProperties.h"

//==============================================================================
SketchProperties::SketchProperties (FrameEditor* editor)
{
    frameEditor = editor;
    frameEditor->addActionListener (this);
    
    layerVisible.reset (new juce::ToggleButton ("layerVisible"));
    addAndMakeVisible (layerVisible.get());
    layerVisible->setTooltip ("Make this layer visible or invisible");
    layerVisible->setButtonText ("Visible");
    layerVisible->addListener (this);

    selectIcon = Drawable::createFromImageData (BinaryData::pointinghand_png,
                                                BinaryData::pointinghand_pngSize);
    
    selectToolButton.reset (new DrawableButton ("selToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (selectToolButton.get());
    selectToolButton->setImages (selectIcon.get());
    selectToolButton->setEdgeIndent (0);
    selectToolButton->setTooltip ("Path Selection Tool");
    selectToolButton->addListener (this);

    moveIcon = Drawable::createFromImageData (BinaryData::move_png,
                                              BinaryData::move_pngSize);
    
    moveToolButton.reset (new DrawableButton ("moveToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (moveToolButton.get());
    moveToolButton->setImages (moveIcon.get());
    moveToolButton->setEdgeIndent (0);
    moveToolButton->setTooltip ("Move Selection Tool");
    moveToolButton->addListener (this);

    ellipseIcon = Drawable::createFromImageData (BinaryData::ellipse_png,
                                                BinaryData::ellipse_pngSize);
    
    ellipseToolButton.reset (new DrawableButton ("ellipseToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (ellipseToolButton.get());
    ellipseToolButton->setImages (ellipseIcon.get());
    ellipseToolButton->setEdgeIndent (0);
    ellipseToolButton->setTooltip ("Ellipse Tool");
    ellipseToolButton->addListener (this);

    penIcon = Drawable::createFromImageData (BinaryData::pen_png,
                                             BinaryData::pen_pngSize);
    
    penToolButton.reset (new DrawableButton ("penToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (penToolButton.get());
    penToolButton->setImages (penIcon.get());
    penToolButton->setEdgeIndent (0);
    penToolButton->setTooltip ("Pen Tool");
    penToolButton->addListener (this);

    toolColorButton.reset (new ColourButton ());
    addAndMakeVisible (toolColorButton.get());
    toolColorButton->setTooltip ("Select color for drawing tools");
    toolColorButton->addChangeListener (this);
    toolColorButton->setColour (TextButton::buttonColourId, frameEditor->getPointToolColor());

    selectLabel.reset (new Label ("selectLabel"));
    addAndMakeVisible (selectLabel.get());
    selectLabel->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    selectLabel->setJustificationType (juce::Justification::centredLeft);
    selectLabel->setEditable (false, false, false);
    selectLabel->setColour (Label::textColourId, juce::Colours::white);
    selectLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    pointsLabel.reset (new Label ("pointsLabel"));
    addAndMakeVisible (pointsLabel.get());
    pointsLabel->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    pointsLabel->setJustificationType (juce::Justification::centredLeft);
    pointsLabel->setEditable (false, false, false);
    pointsLabel->setColour (Label::textColourId, juce::Colours::white);
    pointsLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    spacingLabel.reset (new Label ("spacing", "Point Spacing"));
    addAndMakeVisible (spacingLabel.get());
    spacingLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    spacingLabel->setJustificationType (juce::Justification::centred);
    spacingLabel->setEditable (false, false, false);
    spacingLabel->setColour (Label::textColourId, juce::Colours::grey);
    spacingLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    spacing.reset (new TextEditor ("spacing"));
    addAndMakeVisible (spacing.get());
    spacing->setMultiLine (false);
    spacing->setReturnKeyStartsNewLine (false);
    spacing->setReadOnly (false);
    spacing->setScrollbarsShown (true);
    spacing->setCaretVisible (true);
    spacing->setPopupMenuEnabled (true);
    spacing->setJustification (Justification::centred);
    spacing->setColour (TextEditor::textColourId, Colours::white);
    spacing->setTooltip ("Average distance between generated points");
    spacing->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    spacing->addListener (this);

    extraPerLabel.reset (new Label ("extraPerlabel", "Extra Anchor Points"));
    addAndMakeVisible (extraPerLabel.get());
    extraPerLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    extraPerLabel->setJustificationType (juce::Justification::centred);
    extraPerLabel->setEditable (false, false, false);
    extraPerLabel->setColour (Label::textColourId, juce::Colours::grey);
    extraPerLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    extraPerAnchor.reset (new TextEditor ("extraPer"));
    addAndMakeVisible (extraPerAnchor.get());
    extraPerAnchor->setMultiLine (false);
    extraPerAnchor->setReturnKeyStartsNewLine (false);
    extraPerAnchor->setReadOnly (false);
    extraPerAnchor->setScrollbarsShown (true);
    extraPerAnchor->setCaretVisible (true);
    extraPerAnchor->setPopupMenuEnabled (true);
    extraPerAnchor->setJustification (Justification::centred);
    extraPerAnchor->setColour (TextEditor::textColourId, Colours::white);
    extraPerAnchor->setTooltip ("Extra points at each anchor position");
    extraPerAnchor->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    extraPerAnchor->addListener (this);

    blankBeforeLabel.reset (new Label ("blankBeforelabel", "Blanked Points Before"));
    addAndMakeVisible (blankBeforeLabel.get());
    blankBeforeLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    blankBeforeLabel->setJustificationType (juce::Justification::centred);
    blankBeforeLabel->setEditable (false, false, false);
    blankBeforeLabel->setColour (Label::textColourId, juce::Colours::grey);
    blankBeforeLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    blankBefore.reset (new TextEditor ("blankBefore"));
    addAndMakeVisible (blankBefore.get());
    blankBefore->setMultiLine (false);
    blankBefore->setReturnKeyStartsNewLine (false);
    blankBefore->setReadOnly (false);
    blankBefore->setScrollbarsShown (true);
    blankBefore->setCaretVisible (true);
    blankBefore->setPopupMenuEnabled (true);
    blankBefore->setJustification (Justification::centred);
    blankBefore->setColour (TextEditor::textColourId, Colours::white);
    blankBefore->setTooltip ("Blanked points before start of shape");
    blankBefore->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    blankBefore->addListener (this);

    blankAfterLabel.reset (new Label ("blankAfterlabel", "Blanked Points After"));
    addAndMakeVisible (blankAfterLabel.get());
    blankAfterLabel->setFont (Font (10.00f, Font::plain).withTypefaceStyle ("Regular"));
    blankAfterLabel->setJustificationType (juce::Justification::centred);
    blankAfterLabel->setEditable (false, false, false);
    blankAfterLabel->setColour (Label::textColourId, juce::Colours::grey);
    blankAfterLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    blankAfter.reset (new TextEditor ("blankAfter"));
    addAndMakeVisible (blankAfter.get());
    blankAfter->setMultiLine (false);
    blankAfter->setReturnKeyStartsNewLine (false);
    blankAfter->setReadOnly (false);
    blankAfter->setScrollbarsShown (true);
    blankAfter->setCaretVisible (true);
    blankAfter->setPopupMenuEnabled (true);
    blankAfter->setJustification (Justification::centred);
    blankAfter->setColour (TextEditor::textColourId, Colours::white);
    blankAfter->setTooltip ("Blanked points after end of shape");
    blankAfter->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789-*"), true);
    blankAfter->addListener (this);

    selectColorButton.reset (new ColourButton ());
    addAndMakeVisible (selectColorButton.get());
    selectColorButton->setTooltip ("Shape color");
    selectColorButton->addChangeListener (this);

    centerIcon = Drawable::createFromImageData (BinaryData::center_png,
                                                BinaryData::center_pngSize);
    
    centerButton.reset (new DrawableButton ("centerButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerButton.get());
    centerButton->setImages (centerIcon.get());
    centerButton->setEdgeIndent (0);
    centerButton->setTooltip ("Center the selected shape(s)");
    centerButton->addListener (this);

    centerXIcon = Drawable::createFromImageData (BinaryData::centerx_png,
                                                 BinaryData::centerx_pngSize);
    
    centerXButton.reset (new DrawableButton ("centerXButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerXButton.get());
    centerXButton->setImages (centerXIcon.get());
    centerXButton->setEdgeIndent (0);
    centerXButton->setTooltip ("Center the selected shape(s) up/down");
    centerXButton->addListener (this);

    centerYIcon = Drawable::createFromImageData (BinaryData::centery_png,
                                                 BinaryData::centery_pngSize);
    
    centerYButton.reset (new DrawableButton ("centerYButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerYButton.get());
    centerYButton->setImages (centerYIcon.get());
    centerYButton->setEdgeIndent (0);
    centerYButton->setTooltip ("Center the selected shape(s) left/right");
    centerYButton->addListener (this);

    scaleIcon = Drawable::createFromImageData (BinaryData::scale_png,
                                            BinaryData::scale_pngSize);

    scaleButton.reset (new SketchScaleButton (frameEditor));
    addAndMakeVisible (scaleButton.get());
    scaleButton->setImages (scaleIcon.get());
    scaleButton->setEdgeIndent (0);
    scaleButton->setTooltip ("Scale the selected shape(s)");

    refresh();
}

SketchProperties::~SketchProperties()
{
    layerVisible = nullptr;
    selectToolButton = nullptr;
    selectIcon = nullptr;
    moveToolButton = nullptr;
    moveIcon = nullptr;
    ellipseToolButton = nullptr;
    ellipseIcon = nullptr;
    penToolButton = nullptr;
    penIcon = nullptr;
    toolColorButton = nullptr;
    selectLabel = nullptr;
    pointsLabel = nullptr;
    spacingLabel = nullptr;
    spacing = nullptr;
    extraPerLabel = nullptr;
    extraPerAnchor = nullptr;
    blankBeforeLabel = nullptr;
    blankBefore = nullptr;
    blankAfterLabel = nullptr;
    blankAfter = nullptr;
    selectColorButton = nullptr;
    centerButton = nullptr;
    centerIcon = nullptr;
    centerXButton = nullptr;
    centerXIcon = nullptr;
    centerYButton = nullptr;
    centerYIcon = nullptr;
    scaleButton = nullptr;
    scaleIcon = nullptr;
}

//==============================================================================
void SketchProperties::paint (juce::Graphics& g)
{
    // Clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SketchProperties::resized()
{
    layerVisible->setBounds (16, 16, getWidth() - 32, 24);
    selectToolButton->setBounds (10 + 12, 48, 32, 32);
    moveToolButton->setBounds (46 + 12, 48, 32, 32);
    ellipseToolButton->setBounds (82 + 12, 48, 32, 32);
    penToolButton->setBounds (118 + 12, 48, 32, 32);
    toolColorButton->setBounds (154 + 12, 54, 20, 20);
    selectLabel->setBounds (16, 88, getWidth() - 32, 24);
    pointsLabel->setBounds (16, 104, getWidth() - 32, 24);
    spacingLabel->setBounds (16, 128, getWidth() - 32, 12);
    spacing->setBounds (16, 144, getWidth() - 32, 24);
    extraPerLabel->setBounds (16, 176, getWidth() - 32, 12);
    extraPerAnchor->setBounds (16, 192, getWidth() - 32, 24);
    blankBeforeLabel->setBounds (16, 224, getWidth() - 32, 12);
    blankBefore->setBounds (16, 240, getWidth() - 32, 24);
    blankAfterLabel->setBounds (16, 272, getWidth() - 32, 12);
    blankAfter->setBounds (16, 288, getWidth() - 32, 24);
    selectColorButton->setBounds (getBounds().getCentreX() - 10, 320, 20, 20);
    centerButton->setBounds (82, 348, 32, 32);
    centerXButton->setBounds (118, 348, 32, 32);
    centerYButton->setBounds (154, 348, 32, 32);
    scaleButton->setBounds (82, 384, 32, 32);
}

//==============================================================================
void SketchProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == layerVisible.get())
        frameEditor->setSketchVisible (layerVisible->getToggleState());
    else if (buttonThatWasClicked == selectToolButton.get())
        frameEditor->setActiveSketchTool (FrameEditor::sketchSelectTool);
    else if (buttonThatWasClicked == moveToolButton.get())
        frameEditor->setActiveSketchTool (FrameEditor::sketchMoveTool);
    else if (buttonThatWasClicked == ellipseToolButton.get())
        frameEditor->setActiveSketchTool (FrameEditor::sketchEllipseTool);
    else if (buttonThatWasClicked == penToolButton.get())
        frameEditor->setActiveSketchTool (FrameEditor::sketchPenTool);
    else if (buttonThatWasClicked == centerButton.get())
        frameEditor->centerSketchSelected (true, true, false);
    else if (buttonThatWasClicked == centerXButton.get())
        frameEditor->centerSketchSelected (false, true, false);
    else if (buttonThatWasClicked == centerYButton.get())
        frameEditor->centerSketchSelected (true, false, false);
}

//==============================================================================
void SketchProperties::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == toolColorButton.get())
    {
        Colour c = toolColorButton->findColour (TextButton::buttonColourId);
        frameEditor->setSketchToolColor (c);
    }
    else if (source == selectColorButton.get())
    {
        Colour c = selectColorButton->findColour (TextButton::buttonColourId);
        frameEditor->setSketchSelectedColor (c);
    }
}

//==============================================================================
void SketchProperties::textEditorReturnKeyPressed (TextEditor& editor)
{
    if (&editor == spacing.get())
    {
        if (! spacing->getText().containsChar ('*'))
            frameEditor->setSketchSelectedSpacing ((int16) spacing->getText().getIntValue());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == extraPerAnchor.get())
    {
        if (! extraPerAnchor->getText().containsChar ('*'))
            frameEditor->setSketchSelectedExtraPerAnchor ((int16) extraPerAnchor->getText().getIntValue());
        
        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == blankBefore.get())
    {
        if (! blankBefore->getText().containsChar ('*'))
            frameEditor->setSketchSelectedBlankingBefore ((int16) blankBefore->getText().getIntValue());
        
        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == blankAfter.get())
    {
        if (! blankAfter->getText().containsChar ('*'))
            frameEditor->setSketchSelectedBlankingAfter ((int16) blankAfter->getText().getIntValue());
        
        layerVisible->grabKeyboardFocus();
    }
}

void SketchProperties::textEditorEscapeKeyPressed (TextEditor&)
{
    layerVisible->grabKeyboardFocus();
}

void SketchProperties::textEditorFocusLost (TextEditor&)
{
    updateSelection();
}

//==============================================================================
void SketchProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::sketchVisibilityChanged)
        refresh();
    else if (message == EditorActions::sketchToolChanged)
        updateTools();
    else if (message == EditorActions::sketchToolColorChanged)
        updateTools();
    else if (message == EditorActions::iPathSelectionChanged)
        updateSelection();
    else if (message == EditorActions::iPathsChanged)
        updateSelection();
    else if (message == EditorActions::deleteRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            if (!frameEditor->getIPathSelection().isEmpty())
            {
                if (frameEditor->getIPathSelection().getAnchor() == -1)
                    frameEditor->deletePaths();
                else
                    frameEditor->deleteAnchor();
            }
        }
    }
    else if (message == EditorActions::cancelRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            if (! frameEditor->isTransforming())
                frameEditor->setIPathSelection (IPathSelection());
        }
    }
    else if (message == EditorActions::upRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (0, -256);
    }
    else if (message == EditorActions::downRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (0, 256);
    }
    else if (message == EditorActions::leftRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (-256, 0);
    }
    else if (message == EditorActions::rightRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (256, 0);
    }
    else if (message == EditorActions::smallUpRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (0, -16);
    }
    else if (message == EditorActions::smallDownRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (0, 16);
    }
    else if (message == EditorActions::smallLeftRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (-16, 0);
    }
    else if (message == EditorActions::smallRightRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->moveSketchSelected (16, 0);
    }
}

//==============================================================================
void SketchProperties::refresh()
{
    layerVisible->setToggleState (frameEditor->getSketchVisible(), dontSendNotification);
    updateSelection();
    updateTools();
}

void SketchProperties::updateSelection()
{
    if (frameEditor->getIPathSelection().isEmpty())
    {
        selectLabel->setText ("No Shapes Selected", dontSendNotification);
        selectLabel->setColour(Label::textColourId, Colours::grey);
        pointsLabel->setText ("0 points", dontSendNotification);
        pointsLabel->setColour (Label::textColourId, Colours::grey);
        spacingLabel->setColour (Label::textColourId, Colours::grey);
        extraPerLabel->setColour (Label::textColourId, Colours::grey);
        blankBeforeLabel->setColour (Label::textColourId, Colours::grey);
        blankAfterLabel->setColour (Label::textColourId, Colours::grey);
        spacing->setText ("", dontSendNotification);
        spacing->setEnabled (false);
        extraPerAnchor->setText ("", dontSendNotification);
        extraPerAnchor->setEnabled (false);
        blankBefore->setText ("", dontSendNotification);
        blankBefore->setEnabled (false);
        blankAfter->setText ("", dontSendNotification);
        blankAfter->setEnabled (false);
        selectColorButton->setEnabled (false);
        selectColorButton->setColour (TextButton::buttonColourId, Colours::transparentBlack);
        centerButton->setEnabled (false);
        centerXButton->setEnabled (false);
        centerYButton->setEnabled (false);
        scaleButton->setEnabled (false);
    }
    else
    {
        int selected = 0;
        for (auto n = 0; n < frameEditor->getIPathSelection().getNumRanges(); ++n)
            selected += frameEditor->getIPathSelection().getRange(n).getLength();
        
        String s (selected);
        if (selected > 1)
            s += " Shapes";
        else
            s += " Shape";
        s += " Selected";
        
        selectLabel->setText (s, dontSendNotification);
        selectLabel->setColour(Label::textColourId, Colours::white);

        // Tabulate approximate points
        int p = 0;
        for (auto n = 0; n < frameEditor->getIPathSelection().getNumRanges(); ++n)
        {
            Range<uint16> r = frameEditor->getIPathSelection().getRange (n);
            
            for (auto i = r.getStart(); i < r.getEnd(); ++i)
            {
                float length = frameEditor->getIPath (i).getPath().getLength();
                length /= (float)frameEditor->getIPath (i).getPointDensity();
                length += 0.5f;
                p += (int)length;
                p += frameEditor->getIPath (i).getAnchorCount();
                p += frameEditor->getIPath (i).getAnchorCount() * frameEditor->getIPath (i).getExtraPointsPerAnchor();
                p += frameEditor->getIPath (i).getBlankedPointsBeforeStart();
                p += frameEditor->getIPath (i).getBlankedPointsAfterEnd();
            }
        }
        
        if (p)
        {
            float frameRate = (float)frameEditor->getScanRate() / (float)p;
            
            if (frameRate < 10.0)
                pointsLabel->setColour (Label::textColourId, juce::Colours::red);
            else if (frameRate < 15.0)
                pointsLabel->setColour (Label::textColourId, juce::Colours::yellow);
            else
                pointsLabel->setColour (Label::textColourId, juce::Colours::white);
            
            String s(p);
            if (p > 1)
                s += " points";
            else
                s += " point";
            s += " (" + String(frameRate, 1) + " fps)";
            pointsLabel->setText (s, dontSendNotification);
        }
        else
            pointsLabel->setText ("", dontSendNotification);
        
        spacingLabel->setColour (Label::textColourId, Colours::white);
        extraPerLabel->setColour (Label::textColourId, Colours::white);
        blankBeforeLabel->setColour (Label::textColourId, Colours::white);
        blankAfterLabel->setColour (Label::textColourId, Colours::white);
        spacing->setEnabled (true);
        extraPerAnchor->setEnabled (true);
        blankBefore->setEnabled (true);
        blankAfter->setEnabled (true);
        selectColorButton->setEnabled (true);
        
        bool ms = false; bool me = false; bool mbb = false;
        bool mba = false; bool mc = false;
        IPath lastPath;

        for (auto n = 0; n < frameEditor->getIPathSelection().getNumRanges(); ++n)
        {
            IPath newPath;
                        
            Range<uint16> r = frameEditor->getIPathSelection().getRange (n);
            if (n == 0)
                lastPath = frameEditor->getIPath (r.getStart());

            for (auto i = r.getStart(); i < r.getEnd(); ++i)
            {
                newPath = frameEditor->getIPath (i);
                ms = lastPath.getPointDensity() != newPath.getPointDensity();
                me = lastPath.getExtraPointsPerAnchor() != newPath.getExtraPointsPerAnchor();
                mbb = lastPath.getBlankedPointsBeforeStart() != newPath.getBlankedPointsBeforeStart();
                mba = lastPath.getBlankedPointsAfterEnd() != newPath.getBlankedPointsAfterEnd();
                mc = lastPath.getColor() != newPath.getColor();
            }
        }
        
        spacing->setText (ms ? "*" : String (lastPath.getPointDensity()), dontSendNotification);
        extraPerAnchor->setText (me ? "*" : String (lastPath.getExtraPointsPerAnchor()), dontSendNotification);
        blankBefore->setText (mbb ? "*" : String (lastPath.getBlankedPointsBeforeStart()), dontSendNotification);
        blankAfter->setText (mba ? "*" : String (lastPath.getBlankedPointsAfterEnd()), dontSendNotification);
        selectColorButton->setColour (TextButton::buttonColourId, mc ? Colours::transparentBlack : lastPath.getColor());
        
        centerButton->setEnabled (true);
        centerXButton->setEnabled (true);
        centerYButton->setEnabled (true);
        scaleButton->setEnabled (true);
    }
}

void SketchProperties::updateTools()
{
    if (frameEditor->getActiveSketchTool() == FrameEditor::sketchSelectTool)
    {
        selectToolButton->setToggleState (true, dontSendNotification);
        moveToolButton->setToggleState (false, dontSendNotification);
        ellipseToolButton->setToggleState (false, dontSendNotification);
        penToolButton->setToggleState (false, dontSendNotification);
    }
    else if (frameEditor->getActiveSketchTool() == FrameEditor::sketchMoveTool)
    {
        selectToolButton->setToggleState (false, dontSendNotification);
        moveToolButton->setToggleState (true, dontSendNotification);
        ellipseToolButton->setToggleState (false, dontSendNotification);
        penToolButton->setToggleState (false, dontSendNotification);
    }
    else if (frameEditor->getActiveSketchTool() == FrameEditor::sketchEllipseTool)
    {
        selectToolButton->setToggleState (false, dontSendNotification);
        moveToolButton->setToggleState (false, dontSendNotification);
        ellipseToolButton->setToggleState (true, dontSendNotification);
        penToolButton->setToggleState (false, dontSendNotification);
    }
    else
    {
        selectToolButton->setToggleState (false, dontSendNotification);
        moveToolButton->setToggleState (false, dontSendNotification);
        ellipseToolButton->setToggleState (false, dontSendNotification);
        penToolButton->setToggleState (true, dontSendNotification);
    }

    toolColorButton->setColour (TextButton::buttonColourId, frameEditor->getSketchToolColor());
}
