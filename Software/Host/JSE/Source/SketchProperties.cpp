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
}

//==============================================================================
void SketchProperties::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == toolColorButton.get())
    {
        Colour c = toolColorButton->findColour (TextButton::buttonColourId);
        frameEditor->setSketchToolColor (c);
    }
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
        pointsLabel->setText ("", dontSendNotification);
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
                p += 4;
            }
        }
        
        if (p)
        {
            float frameRate = (float)frameEditor->getScanRate() / (float)p;
            
            if (frameRate < 15.0)
                pointsLabel->setColour (Label::textColourId, juce::Colours::yellow);
            else if (frameRate < 10.0)
                pointsLabel->setColour (Label::textColourId, juce::Colours::red);
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
