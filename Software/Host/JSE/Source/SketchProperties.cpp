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
    else if (message == EditorActions::viewChanged)
        updateTools();
    else if (message == EditorActions::deleteRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            frameEditor->deletePaths();
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
    updateTools();
}

void SketchProperties::updateTools()
{
    if (frameEditor->getActiveView() != Frame::front)
    {
        selectToolButton->setEnabled (false);
        moveToolButton->setEnabled (false);
        ellipseToolButton->setEnabled (false);
        penToolButton->setEnabled (false);
        toolColorButton->setEnabled (false);
    }
    else
    {
        selectToolButton->setEnabled (true);
        moveToolButton->setEnabled (true);
        ellipseToolButton->setEnabled (true);
        penToolButton->setEnabled (true);
        toolColorButton->setEnabled (true);
    }

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
