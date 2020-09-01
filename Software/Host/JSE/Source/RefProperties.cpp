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
    backgroundAlpha->setTooltip (TRANS("Adjusts the opacity of the background image."));
    backgroundAlpha->setRange (0, 100, 1);
    backgroundAlpha->setSliderStyle (juce::Slider::LinearHorizontal);
    backgroundAlpha->setTextBoxStyle (juce::Slider::TextBoxBelow, true, 90, 20);
    backgroundAlpha->setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x008e989b));
    backgroundAlpha->setTextValueSuffix ("% Opacity");
    backgroundAlpha->addListener (this);

    layerVisible->setToggleState (frameEditor->getRefVisible(), dontSendNotification);
    
    String s = frameEditor->getImageFile().getFileName();
    if (s.length())
        imageFileLabel->setText(s, dontSendNotification);
    else
        imageFileLabel->setText("<none>", dontSendNotification);
    
    backgroundAlpha->setValue (frameEditor->getRefOpacity() * 100.0, dontSendNotification);
}

RefProperties::~RefProperties()
{
    layerVisible = nullptr;
    imageFileLabel = nullptr;
    selectImageButton = nullptr;
    backgroundAlpha = nullptr;
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
}
