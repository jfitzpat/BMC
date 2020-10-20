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
    : selectionColour (Colours::transparentBlack)
{
    frameEditor = editor;
    frameEditor->addActionListener (this);
    
    layerVisible.reset (new juce::ToggleButton ("layerVisible"));
    addAndMakeVisible (layerVisible.get());
    layerVisible->setTooltip ("Make this layer visible or invisible");
    layerVisible->setButtonText ("Visible");
    layerVisible->addListener (this);

    drawLines.reset (new juce::ToggleButton ("drawLines"));
    addAndMakeVisible (drawLines.get());
    drawLines->setTooltip ("Draw lines between coordinates");
    drawLines->setButtonText ("Draw Lines");
    drawLines->addListener (this);

    showBlanking.reset (new juce::ToggleButton ("showBlanking"));
    addAndMakeVisible (showBlanking.get());
    showBlanking->setTooltip ("Show blanked (invisible) coordinates");
    showBlanking->setButtonText ("Show Blanked Points");
    showBlanking->addListener (this);

    pointLabel.reset (new Label ("pointLabel"));
    addAndMakeVisible (pointLabel.get());
    pointLabel->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    pointLabel->setJustificationType (juce::Justification::centredLeft);
    pointLabel->setEditable (false, false, false);
    pointLabel->setColour (Label::textColourId, juce::Colours::white);
    pointLabel->setColour (Label::backgroundColourId, juce::Colour (0x00000000));

    selectIcon = Drawable::createFromImageData (BinaryData::pointinghand_png,
                                                BinaryData::pointinghand_pngSize);
    
    selectToolButton.reset (new DrawableButton ("selToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (selectToolButton.get());
    selectToolButton->setImages (selectIcon.get());
    selectToolButton->setEdgeIndent (0);
    selectToolButton->setTooltip ("Point Selection Tool");
    selectToolButton->addListener (this);

    moveIcon = Drawable::createFromImageData (BinaryData::move_png,
                                              BinaryData::move_pngSize);
    
    moveToolButton.reset (new DrawableButton ("moveToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (moveToolButton.get());
    moveToolButton->setImages (moveIcon.get());
    moveToolButton->setEdgeIndent (0);
    moveToolButton->setTooltip ("Move Selection Tool");
    moveToolButton->addListener (this);

    pointIcon = Drawable::createFromImageData (BinaryData::croshair_png,
                                               BinaryData::croshair_pngSize);
    
    pointToolButton.reset (new DrawableButton ("pointToolButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (pointToolButton.get());
    pointToolButton->setImages (pointIcon.get());
    pointToolButton->setEdgeIndent (0);
    pointToolButton->setTooltip ("Point Insertion Tool");
    pointToolButton->addListener (this);

    pointToolColorButton.reset (new ColourButton ());
    addAndMakeVisible (pointToolColorButton.get());
    pointToolColorButton->setTooltip ("Select color for Point Insertion Tool");
    pointToolColorButton->addChangeListener (this);
    pointToolColorButton->setColour (TextButton::buttonColourId, frameEditor->getPointToolColor());

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
    currentSelection->setTooltip ("Selected point(s)");
    currentSelection->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789 -:,;"), true);
    currentSelection->addListener (this);

    decSelection.reset (new juce::TextButton ("decSelection"));
    addAndMakeVisible (decSelection.get());
    decSelection->setTooltip ("Shift the selection down");
    decSelection->setButtonText ("-");
    decSelection->addListener (this);

    incSelection.reset (new juce::TextButton ("incSelection"));
    addAndMakeVisible (incSelection.get());
    incSelection->setTooltip ("Shift the selection up");
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
    selectionX->setTooltip ("X coordinate of selected point(s)");
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
    selectionY->setTooltip ("Y coordinate of selected point(s)");
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
    selectionZ->setTooltip ("Z coordinate of selected point(s)");
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
    selectionR->setTooltip ("Red level of selected point(s)");
    selectionR->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789ABCDEFabcdef*"), true);
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
    selectionG->setTooltip ("Green level of selected point(s)");
    selectionG->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789ABCDEFabcdef*"), true);
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
    selectionB->setTooltip ("Blue level of selected point(s)");
    selectionB->setInputFilter (new TextEditor::LengthAndCharacterRestriction(-1, "0123456789ABCDEFabcdef*"), true);
    selectionB->addListener (this);
    
    colorButton.reset (new ColourButton ());
    addAndMakeVisible (colorButton.get());
    colorButton->setTooltip ("Select color for selected point(s)");
    colorButton->addChangeListener (this);

    trashIcon = Drawable::createFromImageData (BinaryData::trash_png,
                                               BinaryData::trash_pngSize);
    
    trashButton.reset (new DrawableButton ("trashButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (trashButton.get());
    trashButton->setImages (trashIcon.get());
    trashButton->setEdgeIndent (0);
    trashButton->setTooltip ("Delete the selected point(s)");
    trashButton->addListener (this);

    centerIcon = Drawable::createFromImageData (BinaryData::center_png,
                                                BinaryData::center_pngSize);
    
    centerButton.reset (new DrawableButton ("centerButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerButton.get());
    centerButton->setImages (centerIcon.get());
    centerButton->setEdgeIndent (0);
    centerButton->setTooltip ("Center the selected point(s)");
    centerButton->addListener (this);

    centerXIcon = Drawable::createFromImageData (BinaryData::centerx_png,
                                                 BinaryData::centerx_pngSize);
    
    centerXButton.reset (new DrawableButton ("centerXButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerXButton.get());
    centerXButton->setImages (centerXIcon.get());
    centerXButton->setEdgeIndent (0);
    centerXButton->setTooltip ("Center the selected point(s) up/down");
    centerXButton->addListener (this);

    centerYIcon = Drawable::createFromImageData (BinaryData::centery_png,
                                                 BinaryData::centery_pngSize);
    
    centerYButton.reset (new DrawableButton ("centerYButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerYButton.get());
    centerYButton->setImages (centerYIcon.get());
    centerYButton->setEdgeIndent (0);
    centerYButton->setTooltip ("Center the selected point(s) left/right");
    centerYButton->addListener (this);

    centerZIcon = Drawable::createFromImageData (BinaryData::centerz_png,
                                                 BinaryData::centerz_pngSize);
    
    centerZButton.reset (new DrawableButton ("centerZButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (centerZButton.get());
    centerZButton->setImages (centerZIcon.get());
    centerZButton->setEdgeIndent (0);
    centerZButton->setTooltip ("Center the selected point(s) front/back");
    centerZButton->addListener (this);

    scaleIcon = Drawable::createFromImageData (BinaryData::scale_png,
                                            BinaryData::scale_pngSize);

    scaleButton.reset (new ScaleButton (frameEditor));
    addAndMakeVisible (scaleButton.get());
    scaleButton->setImages (scaleIcon.get());
    scaleButton->setEdgeIndent (0);
    scaleButton->setTooltip ("Scale the selected point(s)");

    rotateIcon = Drawable::createFromImageData (BinaryData::rotate_png,
                                                BinaryData::rotate_pngSize);

    rotateButton.reset (new RotateButton (frameEditor));
    addAndMakeVisible (rotateButton.get());
    rotateButton->setImages (rotateIcon.get());
    rotateButton->setEdgeIndent (0);
    rotateButton->setTooltip ("Rotate the selected point(s)");

    shearIcon = Drawable::createFromImageData  (BinaryData::shear_png,
                                                BinaryData::shear_pngSize);

    barberIcon = Drawable::createFromImageData (BinaryData::barber_png,
                                                BinaryData::barber_pngSize);
    
    barberButton.reset (new BarberButton (frameEditor));
    addAndMakeVisible (barberButton.get());
    barberButton->setImages (barberIcon.get());
    barberButton->setEdgeIndent (0);
    barberButton->setTooltip ("Barber Pole selected point(s)");
    
    shearButton.reset (new ShearButton (frameEditor));
    addAndMakeVisible (shearButton.get());
    shearButton->setImages (shearIcon.get());
    shearButton->setEdgeIndent (0);
    shearButton->setTooltip ("Shear/Skew the selected point(s)");

    bulgeIcon = Drawable::createFromImageData (BinaryData::bulge_png,
                                               BinaryData::bulge_pngSize);

    bulgeButton.reset (new BulgeButton (frameEditor));
    addAndMakeVisible (bulgeButton.get());
    bulgeButton->setImages (bulgeIcon.get());
    bulgeButton->setEdgeIndent (0);
    bulgeButton->setTooltip ("Bulge (2D) selected point(s)");

    pinchIcon = Drawable::createFromImageData (BinaryData::pinch_png,
                                               BinaryData::pinch_pngSize);

    pinchButton.reset (new PinchButton (frameEditor));
    addAndMakeVisible (pinchButton.get());
    pinchButton->setImages (pinchIcon.get());
    pinchButton->setEdgeIndent (0);
    pinchButton->setTooltip ("Pinch (2D) selected point(s)");

    spiralIcon = Drawable::createFromImageData (BinaryData::spiral_png,
                                                BinaryData::spiral_pngSize);

    spiralButton.reset (new SpiralButton (frameEditor));
    addAndMakeVisible (spiralButton.get());
    spiralButton->setImages (spiralIcon.get());
    spiralButton->setEdgeIndent (0);
    spiralButton->setTooltip ("Spiral selected point(s)");

    sphereIcon = Drawable::createFromImageData (BinaryData::sphere_png,
                                                BinaryData::sphere_pngSize);

    sphereButton.reset (new SphereButton (frameEditor));
    addAndMakeVisible (sphereButton.get());
    sphereButton->setImages (sphereIcon.get());
    sphereButton->setEdgeIndent (0);
    sphereButton->setTooltip ("Map selected point(s) to a sphere");

    gradientIcon = Drawable::createFromImageData (BinaryData::gradient_png,
                                                  BinaryData::gradient_pngSize);

    gradientButton.reset (new GradientButton (frameEditor));
    addAndMakeVisible (gradientButton.get());
    gradientButton->setImages (gradientIcon.get());
    gradientButton->setEdgeIndent (0);
    gradientButton->setTooltip ("Apply a 2 color gradient to the selected point(s)");

    duplicateIcon = Drawable::createFromImageData (BinaryData::duplicatewhite_png,
                                                    BinaryData::duplicatewhite_pngSize);

    gradient3Icon = Drawable::createFromImageData (BinaryData::gradient3_png,
                                                   BinaryData::gradient3_pngSize);

    gradient3Button.reset (new Gradient3Button (frameEditor));
    addAndMakeVisible (gradient3Button.get());
    gradient3Button->setImages (gradient3Icon.get());
    gradient3Button->setEdgeIndent (0);
    gradient3Button->setTooltip ("Apply a 3 color gradient to the selected point(s)");

    hueIcon = Drawable::createFromImageData (BinaryData::coloradjust_png,
                                             BinaryData::coloradjust_pngSize);

    hueButton.reset (new HueButton (frameEditor));
    addAndMakeVisible (hueButton.get());
    hueButton->setImages (hueIcon.get());
    hueButton->setEdgeIndent (0);
    hueButton->setTooltip ("Adjust hue, saturation, and brightness");

    duplicateIcon = Drawable::createFromImageData (BinaryData::duplicatewhite_png,
                                                    BinaryData::duplicatewhite_pngSize);

    duplicateButton.reset (new DrawableButton ("duplicateButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (duplicateButton.get());
    duplicateButton->setImages (duplicateIcon.get());
    duplicateButton->setEdgeIndent (0);
    duplicateButton->setTooltip ("Duplicate the selected point(s)");
    duplicateButton->addListener (this);

    anchorIcon = Drawable::createFromImageData (BinaryData::anchor_png,
                                                BinaryData::anchor_pngSize);

    anchorButton.reset (new DrawableButton ("anchorButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (anchorButton.get());
    anchorButton->setImages (anchorIcon.get());
    anchorButton->setEdgeIndent (0);
    anchorButton->setTooltip ("Insert a blanked anchor point before each selected point");
    anchorButton->addListener (this);

    refresh();
}

IldaProperties::~IldaProperties()
{
    layerVisible = nullptr;
    drawLines = nullptr;
    showBlanking = nullptr;
    pointLabel = nullptr;
    selectToolButton = nullptr;
    selectIcon = nullptr;
    moveToolButton = nullptr;
    moveIcon = nullptr;
    pointToolButton = nullptr;
    pointToolColorButton = nullptr;
    pointIcon = nullptr;
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
    colorButton = nullptr;
    trashButton = nullptr;
    trashIcon = nullptr;
    centerButton = nullptr;
    centerIcon = nullptr;
    centerXButton = nullptr;
    centerXIcon = nullptr;
    centerYButton = nullptr;
    centerYIcon = nullptr;
    centerZButton = nullptr;
    centerZIcon = nullptr;
    scaleButton = nullptr;
    scaleIcon = nullptr;
    rotateButton = nullptr;
    rotateIcon = nullptr;
    barberButton = nullptr;
    barberIcon = nullptr;
    bulgeButton = nullptr;
    bulgeIcon = nullptr;
    pinchButton = nullptr;
    pinchIcon = nullptr;
    spiralButton = nullptr;
    spiralIcon = nullptr;
    sphereButton = nullptr;
    sphereIcon = nullptr;
    gradientButton = nullptr;
    gradientIcon = nullptr;
    gradient3Button = nullptr;
    gradient3Icon = nullptr;
    hueButton = nullptr;
    hueIcon = nullptr;
    duplicateButton = nullptr;
    duplicateIcon = nullptr;
    anchorButton = nullptr;
    anchorIcon = nullptr;
    shearButton = nullptr;
    shearIcon = nullptr;
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
    selectToolButton->setBounds (46 + 12, 144, 32, 32);
    moveToolButton->setBounds (82 + 12, 144, 32, 32);
    pointToolButton->setBounds (118 + 12, 144, 32, 32);
    pointToolColorButton->setBounds (154 + 12, 150, 20, 20);
    selectionLabel->setBounds (16, 184, getWidth() - 32, 12);
    currentSelection->setBounds (10, 200, getWidth() - 20, 24);
    decSelection->setBounds (59, 228, 40, 20);
    incSelection->setBounds (101, 228, 40, 20);
    xLabel->setBounds(16, 256, 54, 12);
    yLabel->setBounds(16 + 56, 256, 54, 12);
    zLabel->setBounds(16 + 112, 256, 54, 12);
    selectionX->setBounds (16, 272, 54, 24);
    selectionY->setBounds (16 + 56, 272, 54, 24);
    selectionZ->setBounds (16 + 112, 272, 54, 24);
    rLabel->setBounds(16, 304, 54, 12);
    gLabel->setBounds(16 + 56, 304, 54, 12);
    bLabel->setBounds(16 + 112, 304, 54, 12);
    selectionR->setBounds (16, 320, 54, 24);
    selectionG->setBounds (16 + 56, 320, 54, 24);
    selectionB->setBounds (16 + 112, 320, 54, 24);
    colorButton->setBounds (89, 348, 20, 20);
    centerButton->setBounds (46, 376, 32, 32);
    centerXButton->setBounds (82, 376, 32, 32);
    centerYButton->setBounds (118, 376, 32, 32);
    centerZButton->setBounds (154, 376, 32, 32);
    scaleButton->setBounds (46, 412, 32, 32);
    rotateButton->setBounds (82, 412, 32, 32);
    shearButton->setBounds (118, 412, 32, 32);
    barberButton->setBounds (154, 412, 32, 32);
    bulgeButton->setBounds (46, 448, 32, 32);
    pinchButton->setBounds (82, 448, 32, 32);
    spiralButton->setBounds (118, 448, 32, 32);
    sphereButton->setBounds(154, 448, 32, 32);
    gradientButton->setBounds (82, 484, 32, 32);
    gradient3Button->setBounds (118, 484, 32, 32);
    hueButton->setBounds(154, 484, 32, 32);
    duplicateButton->setBounds (118, 520, 32, 32);
    anchorButton->setBounds (154, 520, 32, 32);
    trashButton->setBounds (154, 556, 32, 32);
}

//==============================================================================
void IldaProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == layerVisible.get())
        frameEditor->setIldaVisible (layerVisible->getToggleState());
    else if (buttonThatWasClicked == showBlanking.get())
        frameEditor->setIldaShowBlanked (showBlanking->getToggleState());
    else if (buttonThatWasClicked == drawLines.get())
        frameEditor->setIldaDrawLines (drawLines->getToggleState());
    else if (buttonThatWasClicked == selectToolButton.get())
        frameEditor->setActiveIldaTool (FrameEditor::selectTool);
    else if (buttonThatWasClicked == moveToolButton.get())
        frameEditor->setActiveIldaTool (FrameEditor::moveTool);
    else if (buttonThatWasClicked == pointToolButton.get())
        frameEditor->setActiveIldaTool (FrameEditor::pointTool);
    else if (buttonThatWasClicked == decSelection.get())
        frameEditor->adjustIldaSelection (-1);
    else if (buttonThatWasClicked == incSelection.get())
        frameEditor->adjustIldaSelection (1);
    else if (buttonThatWasClicked == trashButton.get())
        frameEditor->deletePoints();
    else if (buttonThatWasClicked == centerButton.get())
        frameEditor->centerIldaSelected();
    else if (buttonThatWasClicked == centerXButton.get())
        frameEditor->centerIldaSelected (false, true, false);
    else if (buttonThatWasClicked == centerYButton.get())
        frameEditor->centerIldaSelected (true, false, false);
    else if (buttonThatWasClicked == centerZButton.get())
        frameEditor->centerIldaSelected (false, false, true);
    else if (buttonThatWasClicked == duplicateButton.get())
        frameEditor->duplicateIldaSelected();
    else if (buttonThatWasClicked == anchorButton.get())
        frameEditor->anchorIldaSelected();
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
    else if (message == EditorActions::ildaPointsChanged)
    {
        updatePointDisplay();
        updateSelection();
    }
    else if (message == EditorActions::ildaToolChanged)
        updateTools();
    else if (message == EditorActions::ildaPointToolColorChanged)
        updateTools();
    else if (message == EditorActions::cancelRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (! frameEditor->isTransforming())
                frameEditor->setIldaSelection (SparseSet<uint16>());
        }
    }
    else if (message == EditorActions::deleteRequest)
    {
        // Note, WorkingArea manages PointTool Response
        if (frameEditor->getActiveLayer() == FrameEditor::ilda &&
            frameEditor->getActiveIldaTool() == FrameEditor::selectTool)
            frameEditor->deletePoints();
    }
    else if (message == EditorActions::upRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::bottom)
                frameEditor->moveIldaSelected (0, 0, 256);
            else
                frameEditor->moveIldaSelected (0, 256, 0);
        }
    }
    else if (message == EditorActions::downRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::bottom)
                frameEditor->moveIldaSelected (0, 0, -256);
            else
                frameEditor->moveIldaSelected (0, -256, 0);
        }
    }
    else if (message == EditorActions::leftRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::left)
                frameEditor->moveIldaSelected (0, 0, -256);
            else
                frameEditor->moveIldaSelected (-256, 0, 0);
        }
    }
    else if (message == EditorActions::rightRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::left)
                frameEditor->moveIldaSelected (0, 0, 256);
            else
                frameEditor->moveIldaSelected (256, 0, 0);
        }
    }
    else if (message == EditorActions::smallUpRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::bottom)
                frameEditor->moveIldaSelected (0, 0, 16);
            else
                frameEditor->moveIldaSelected (0, 16, 0);
        }
    }
    else if (message == EditorActions::smallDownRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::bottom)
                frameEditor->moveIldaSelected (0, 0, -16);
            else
                frameEditor->moveIldaSelected (0, -16, 0);
        }
    }
    else if (message == EditorActions::smallLeftRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::left)
                frameEditor->moveIldaSelected (0, 0, -16);
            else
                frameEditor->moveIldaSelected (-16, 0, 0);
        }
    }
    else if (message == EditorActions::smallRightRequest)
    {
        if (frameEditor->getActiveLayer() == FrameEditor::ilda)
        {
            if (frameEditor->getActiveView() == Frame::left)
                frameEditor->moveIldaSelected (0, 0, 16);
            else
                frameEditor->moveIldaSelected (16, 0, 0);
        }
    }
}

void IldaProperties::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == colorButton.get())
    {
        Colour c = colorButton->findColour (TextButton::buttonColourId);
        if (c != selectionColour)
            frameEditor->setIldaSelectedRGB (c);
    }
    else if (source == pointToolColorButton.get())
    {
        Colour c = pointToolColorButton->findColour (TextButton::buttonColourId);
        frameEditor->setPointToolColor (c);
    }
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
    else if (&editor == selectionX.get())
    {
        if (! selectionX->getText().containsChar ('*'))
            frameEditor->setIldaSelectedX ((int16) selectionX->getText().getIntValue());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == selectionY.get())
    {
        if (! selectionY->getText().containsChar ('*'))
            frameEditor->setIldaSelectedY ((int16) selectionY->getText().getIntValue());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == selectionZ.get())
    {
        if (! selectionZ->getText().containsChar ('*'))
            frameEditor->setIldaSelectedZ ((int16) selectionZ->getText().getIntValue());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == selectionR.get())
    {
        if (! selectionR->getText().containsChar ('*'))
            frameEditor->setIldaSelectedR ((uint8) selectionR->getText().getHexValue32());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == selectionG.get())
    {
        if (! selectionG->getText().containsChar ('*'))
            frameEditor->setIldaSelectedG ((uint8) selectionG->getText().getHexValue32());

        layerVisible->grabKeyboardFocus();
    }
    else if (&editor == selectionB.get())
    {
        if (! selectionB->getText().containsChar ('*'))
            frameEditor->setIldaSelectedB ((uint8) selectionB->getText().getHexValue32());

        layerVisible->grabKeyboardFocus();
    }
}

void IldaProperties::textEditorEscapeKeyPressed (TextEditor&)
{
        layerVisible->grabKeyboardFocus();
}

void IldaProperties::textEditorFocusLost (TextEditor&)
{
        updateSelection();
}

//==============================================================================
void IldaProperties::updatePointDisplay()
{
    auto p = frameEditor->getPointCount();
    if (p)
    {
        float frameRate = (float)frameEditor->getScanRate() / (float)p;
        
        if (frameRate < 10.0)
            pointLabel->setColour (Label::textColourId, juce::Colours::red);
        else if (frameRate < 15.0)
            pointLabel->setColour (Label::textColourId, juce::Colours::yellow);
        else
            pointLabel->setColour (Label::textColourId, juce::Colours::white);
        
        String s(p);
        if (p > 1)
            s += " points";
        else
            s += " point";
        s += " (" + String(frameRate, 1) + " fps)";
        pointLabel->setText (s, dontSendNotification);
    }
    else
    {
        pointLabel->setText (String(p) + " points", dontSendNotification);
        pointLabel->setColour (Label::textColourId, juce::Colours::grey);
    }
}

void IldaProperties::disableSelectionTools()
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
    colorButton->setEnabled (false);
    colorButton->setColour (TextButton::buttonColourId, Colours::transparentBlack);
    trashButton->setEnabled (false);
    centerButton->setEnabled (false);
    centerXButton->setEnabled (false);
    centerYButton->setEnabled (false);
    centerZButton->setEnabled (false);
    scaleButton->setEnabled (false);
    rotateButton->setEnabled (false);
    duplicateButton->setEnabled (false);
    anchorButton->setEnabled (false);
    shearButton->setEnabled (false);
    barberButton->setEnabled (false);
    bulgeButton->setEnabled (false);
    pinchButton->setEnabled (false);
    spiralButton->setEnabled (false);
    sphereButton->setEnabled (false);
    gradientButton->setEnabled (false);
    gradient3Button->setEnabled (false);
    hueButton->setEnabled (false);
}

void IldaProperties::updateSelection()
{
    if (! frameEditor->getPointCount())
    {
        selectionLabel->setColour (Label::textColourId, juce::Colours::grey);
        disableSelectionTools();
    }
    else
    {
        currentSelection->setEnabled (true);
        selectionLabel->setColour (Label::textColourId, juce::Colours::white);
        auto s = frameEditor->getIldaSelection();
        if (s.isEmpty())
            disableSelectionTools();
        else
        {
            String sString;
            bool mx = false;    bool my = false;
            bool mz = false;    bool mr = false;
            bool mg = false;    bool mb = false;
            
            Frame::IPoint lastPoint;
            zerostruct(lastPoint);  // Clear stupid VS warning

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
                
                Frame::IPoint newPoint;
                
                for (uint16 i = 0; i < r.getLength(); ++i)
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
            selectionR->setText ( mr ? "*" : String::toHexString (lastPoint.red).toUpperCase());
            selectionG->setText ( mg ? "*" : String::toHexString (lastPoint.green).toUpperCase());
            selectionB->setText ( mb ? "*" : String::toHexString (lastPoint.blue).toUpperCase());
            
            colorButton->setEnabled (true);
            if (mr || mg || mb)
                selectionColour = Colours::transparentBlack;
            else
                selectionColour = Colour (lastPoint.red, lastPoint.green, lastPoint.blue);
            
            colorButton->setColour (TextButton::buttonColourId, selectionColour);
            
            trashButton->setEnabled (true);
            centerButton->setEnabled (true);
            centerXButton->setEnabled (true);
            centerYButton->setEnabled (true);
            centerZButton->setEnabled (true);
            scaleButton->setEnabled (true);
            rotateButton->setEnabled (true);
            duplicateButton->setEnabled (true);
            anchorButton->setEnabled (true);
            shearButton->setEnabled (true);
            barberButton->setEnabled (true);
            bulgeButton->setEnabled (true);
            pinchButton->setEnabled (true);
            spiralButton->setEnabled (true);
            sphereButton->setEnabled (true);
            gradientButton->setEnabled (true);
            gradient3Button->setEnabled (true);
            hueButton->setEnabled (true);
        }
    }
}

void IldaProperties::updateTools()
{
    if (frameEditor->getActiveIldaTool() == FrameEditor::selectTool)
    {
        selectToolButton->setToggleState (true, dontSendNotification);
        moveToolButton->setToggleState (false, dontSendNotification);
        pointToolButton->setToggleState (false, dontSendNotification);
    }
    else if (frameEditor->getActiveIldaTool() == FrameEditor::moveTool)
    {
        selectToolButton->setToggleState (false, dontSendNotification);
        moveToolButton->setToggleState (true, dontSendNotification);
        pointToolButton->setToggleState (false, dontSendNotification);
    }
    else
    {
        selectToolButton->setToggleState (false, dontSendNotification);
        moveToolButton->setToggleState (false, dontSendNotification);
        pointToolButton->setToggleState (true, dontSendNotification);
    }

    pointToolColorButton->setColour (TextButton::buttonColourId, frameEditor->getPointToolColor());
}

void IldaProperties::refresh()
{
    layerVisible->setToggleState (frameEditor->getIldaVisible(), dontSendNotification);
    showBlanking->setToggleState (frameEditor->getIldaShowBlanked(), dontSendNotification);
    drawLines->setToggleState (frameEditor->getIldaDrawLines(), dontSendNotification);
    updateTools();
    updatePointDisplay();
    updateSelection();
}
