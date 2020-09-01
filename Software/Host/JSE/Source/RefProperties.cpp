/*
    RefProperties.cpp
    Properties Pane for Reference layer
 
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
#include "RefProperties.h"

//==============================================================================
RefProperties::RefProperties (FrameEditor* editor)
{
    frameEditor = editor;
    frameEditor->addActionListener (this);

    layerVisible.reset (new juce::ToggleButton ("layerVisible"));
    addAndMakeVisible (layerVisible.get());
    layerVisible->setTooltip ("Make this layer visible or invisible.");
    layerVisible->setButtonText ("Visible");
    layerVisible->addListener (this);

    imageFileLabel.reset (new juce::Label ("imageFileLabel",
                                           "<None>"));
    addAndMakeVisible (imageFileLabel.get());
    imageFileLabel->setTooltip (TRANS("Filename of background image."));
    imageFileLabel->setFont (juce::Font (10.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    imageFileLabel->setJustificationType (juce::Justification::centredLeft);
    imageFileLabel->setEditable (false, false, false);
    imageFileLabel->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    imageFileLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    selectImageButton.reset (new juce::TextButton ("selectImageButton"));
    addAndMakeVisible (selectImageButton.get());
    selectImageButton->setTooltip ("Select the background image file to display.");
    selectImageButton->setButtonText ("Select Image");
    selectImageButton->addListener (this);

    backgroundAlpha.reset (new juce::Slider ("backgroundAlpha"));
    addAndMakeVisible (backgroundAlpha.get());
    backgroundAlpha->setTooltip (TRANS("Adjust the opacity of the background image."));
    backgroundAlpha->setRange (0, 100, 1);
    backgroundAlpha->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundAlpha->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    backgroundAlpha->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundAlpha->setTextValueSuffix ("% Opacity");
    backgroundAlpha->addListener (this);

    backgroundScale.reset (new juce::Slider ("backgroundScale"));
    addAndMakeVisible (backgroundScale.get());
    backgroundScale->setTooltip (TRANS("Adjust the size of the background image."));
    backgroundScale->setRange (1, 200, 1);
    backgroundScale->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundScale->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
    backgroundScale->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundScale->setTextValueSuffix ("% Scale");
    backgroundScale->addListener (this);

    backgroundRotation.reset (new juce::Slider ("backgroundRotation"));
    addAndMakeVisible (backgroundRotation.get());
    backgroundRotation->setTooltip (TRANS("Rotate the background image."));
    backgroundRotation->setRange (0, 359, 1);
    backgroundRotation->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundRotation->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 20);
    backgroundRotation->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundRotation->setTextValueSuffix (String(CharPointer_UTF8("° Rotation")));
    backgroundRotation->addListener (this);

    backgroundXoffset.reset (new juce::Slider ("backgroundXoffset"));
    addAndMakeVisible (backgroundXoffset.get());
    backgroundXoffset->setTooltip (TRANS("Shift the background image left/right."));
    backgroundXoffset->setRange (-100, 100, 1);
    backgroundXoffset->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundXoffset->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 20);
    backgroundXoffset->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundXoffset->setTextValueSuffix ("% XOffset");
    backgroundXoffset->addListener (this);

    backgroundYoffset.reset (new juce::Slider ("backgroundYoffset"));
    addAndMakeVisible (backgroundYoffset.get());
    backgroundYoffset->setTooltip (TRANS("Shift the background image up/down."));
    backgroundYoffset->setRange (-100, 100, 1);
    backgroundYoffset->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundYoffset->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 20);
    backgroundYoffset->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundYoffset->setTextValueSuffix ("% YOffset");
    backgroundYoffset->addListener (this);

    layerVisible->setToggleState (frameEditor->getRefVisible(), dontSendNotification);
    
    String s = frameEditor->getImageFile().getFileName();
    if (s.length())
        imageFileLabel->setText(s, dontSendNotification);
    else
        imageFileLabel->setText("<none>", dontSendNotification);
    
    backgroundAlpha->setValue (frameEditor->getRefOpacity() * 100.0, dontSendNotification);
    backgroundScale->setValue (frameEditor->getImageScale() * 100.0, dontSendNotification);
    backgroundRotation->setValue (frameEditor->getImageRotation(), dontSendNotification);
    backgroundXoffset->setValue (frameEditor->getImageXoffset(), dontSendNotification);
    backgroundYoffset->setValue (frameEditor->getImageYoffset(), dontSendNotification);
}

RefProperties::~RefProperties()
{
    layerVisible = nullptr;
    imageFileLabel = nullptr;
    selectImageButton = nullptr;
    backgroundAlpha = nullptr;
    backgroundScale = nullptr;
    backgroundRotation = nullptr;
    backgroundXoffset = nullptr;
    backgroundYoffset = nullptr;
}

//==============================================================================
void RefProperties::paint (juce::Graphics& g)
{
    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void RefProperties::resized()
{
    layerVisible->setBounds (16, 16, 150, 24);
    imageFileLabel->setBounds (10, 48, 182, 24);
    selectImageButton->setBounds (16, 72, 166, 24);
    backgroundAlpha->setBounds (16, 112, 166, 40);
    backgroundScale->setBounds (16, 160, 166, 40);
    backgroundRotation->setBounds (16, 208, 166, 40);
    backgroundXoffset->setBounds (16, 256, 166, 40);
    backgroundYoffset->setBounds (16, 304, 166, 40);
}

//==============================================================================
void RefProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == layerVisible.get())
    {
        frameEditor->setRefVisible (layerVisible->getToggleState());
    }
    else if (buttonThatWasClicked == selectImageButton.get())
    {
        frameEditor->selectImage();
    }
}

//==============================================================================
void RefProperties::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == backgroundAlpha.get())
        frameEditor->setRefOpacity (backgroundAlpha->getValue() / 100.0);
    else if (sliderThatWasMoved == backgroundScale.get())
        frameEditor->setImageScale (backgroundScale->getValue() / 100.0);
    else if (sliderThatWasMoved == backgroundRotation.get())
        frameEditor->setImageRotation (backgroundRotation->getValue());
    else if (sliderThatWasMoved == backgroundXoffset.get())
        frameEditor->setImageXoffset (backgroundXoffset->getValue());
    else if (sliderThatWasMoved == backgroundYoffset.get())
        frameEditor->setImageYoffset (backgroundYoffset->getValue());
            
}

//==============================================================================
void RefProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::refVisibilityChanged)
        layerVisible->setToggleState (frameEditor->getRefVisible(), dontSendNotification);
    else if (message == EditorActions::backgroundImageChanged)
        imageFileLabel->setText (frameEditor->getImageFile().getFileName(), dontSendNotification);
    else if (message == EditorActions::refOpacityChanged)
        backgroundAlpha->setValue (frameEditor->getRefOpacity() * 100, dontSendNotification);
    else if (message == EditorActions::backgroundImageAdjusted)
    {
        backgroundScale->setValue (frameEditor->getImageScale() * 100, dontSendNotification);
        backgroundRotation->setValue (frameEditor->getImageRotation(), dontSendNotification);
        backgroundXoffset->setValue (frameEditor->getImageXoffset(), dontSendNotification);
        backgroundYoffset->setValue (frameEditor->getImageYoffset(), dontSendNotification);
    }
}
