/*
    EditProperties.cpp
    Layer Selector and Properties Display Component
 
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
#include "IldaProperties.h"
#include "RefProperties.h"
#include "EditProperties.h"

#include "MainComponent.h"  // So we can find the ApplicationCommandTarget

//==============================================================================
EditProperties::EditProperties (FrameEditor* frame)
{
    frameEditor = frame;
    frameEditor->addActionListener (this);
    
    viewButton.reset (new TextButton ("viewButton"));
    viewButton->setButtonText ("Front");
    viewButton->setTooltip ("Toggle Viewing Angle");
    addAndMakeVisible (viewButton.get());
    viewButton->addListener (this);

    zoomInIcon = Drawable::createFromImageData(BinaryData::zoomin_png, BinaryData::zoomin_pngSize);

    zoomInButton.reset (new DrawableButton ("zoomInButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (zoomInButton.get());
    zoomInButton->setImages (zoomInIcon.get());
    zoomInButton->setEdgeIndent (0);
    zoomInButton->setTooltip ("Zoom In");
    zoomInButton->addListener (this);

    zoomOutIcon = Drawable::createFromImageData(BinaryData::zoomout_png, BinaryData::zoomout_pngSize);

    zoomOutButton.reset (new DrawableButton ("zoomOutButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (zoomOutButton.get());
    zoomOutButton->setImages (zoomOutIcon.get());
    zoomOutButton->setEdgeIndent (0);
    zoomOutButton->setTooltip ("Zoom Out");
    zoomOutButton->addListener (this);

    showAllIcon = Drawable::createFromImageData(BinaryData::showall_png, BinaryData::showall_pngSize);

    showAllButton.reset (new DrawableButton ("showAllButton", DrawableButton::ImageOnButtonBackground));
    addAndMakeVisible (showAllButton.get());
    showAllButton->setImages (showAllIcon.get());
    showAllButton->setEdgeIndent (0);
    showAllButton->setTooltip ("Fit All");
    showAllButton->addListener (this);
    
    layerTabs.reset (new PropTabbedComponent (frame));
    layerTabs->setOutline (0);
    addAndMakeVisible (layerTabs.get());
    
    auto tabColor = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    
    layerTabs->addTab("Sketch",
                      tabColor,
                      new SketchProperties (frame), true);
    layerTabs->addTab("ILDA",
                      tabColor,
                      new IldaProperties (frame), true);
    layerTabs->addTab("Background",
                      tabColor,
                      new RefProperties (frame), true);
        
    updateZoomButtons();
    updateViewButton();
}

EditProperties::~EditProperties()
{
    viewButton = nullptr;
    showAllButton = nullptr;
    showAllIcon = nullptr;
    zoomInButton = nullptr;
    zoomInIcon = nullptr;
    zoomOutButton = nullptr;
    zoomOutIcon = nullptr;
    layerTabs = nullptr;
}

void EditProperties::paint (juce::Graphics& g)
{
    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds().removeFromRight(1), 1);
}

void EditProperties::resized()
{
    viewButton->setBounds (10, 8, 70, 32);
    zoomInButton->setBounds (getWidth() - 116, 8, 32, 32);
    zoomOutButton->setBounds (getWidth() - 80, 8, 32, 32);
    showAllButton->setBounds (getWidth() - 44, 8, 32, 32);
    layerTabs->setBounds (0, 48, getWidth()-1, getHeight()-48);
}

void EditProperties::updateZoomButtons()
{
    float zoom = frameEditor->getZoomFactor();
    if (zoom == MIN_ZOOM)
    {
        showAllButton->setEnabled (false);
        zoomOutButton->setEnabled (false);
        zoomInButton->setEnabled (true);
    }
    else if (zoom == MAX_ZOOM)
    {
        showAllButton->setEnabled (true);
        zoomOutButton->setEnabled (true);
        zoomInButton->setEnabled (false);
    }
    else
    {
        showAllButton->setEnabled (true);
        zoomOutButton->setEnabled (true);
        zoomInButton->setEnabled (true);
    }

}

void EditProperties::updateViewButton()
{
    switch (frameEditor->getActiveView())
    {
        case Frame::bottom:
            viewButton->setButtonText ("Bottom");
            viewButton->setColour (TextButton::textColourOffId, Colours::lightblue);
            break;
        case Frame::left:
            viewButton->setButtonText ("Left");
            viewButton->setColour (TextButton::textColourOffId, Colours::yellow);
            break;
        default:
            viewButton->setButtonText ("Front");
            viewButton->setColour (TextButton::textColourOffId, Colours::white);
            break;
    }
}

void EditProperties::actionListenerCallback (const String& message)
{
    if (message == EditorActions::layerChanged)
    {
        if (frameEditor->getActiveLayer() != layerTabs->getCurrentTabIndex())
            layerTabs->setCurrentTabIndex (frameEditor->getActiveLayer());
    }
    else if (message == EditorActions::zoomFactorChanged)
        updateZoomButtons();
    else if (message == EditorActions::viewChanged)
        updateViewButton();
}

//==============================================================================
void EditProperties::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == showAllButton.get())
    {
        MainComponent* main;
        
        main = dynamic_cast<MainComponent*> (getParentComponent());
        if (main != nullptr)
            main->commandManager.invokeDirectly (MainComponent::CommandIDs::zoomAll, true);
    }
    else if (buttonThatWasClicked == zoomInButton.get())
    {
        MainComponent* main;
        
        main = dynamic_cast<MainComponent*> (getParentComponent());
        if (main != nullptr)
            main->commandManager.invokeDirectly (MainComponent::CommandIDs::zoomIn, true);
    }
    else if (buttonThatWasClicked == zoomOutButton.get())
    {
        MainComponent* main;
        
        main = dynamic_cast<MainComponent*> (getParentComponent());
        if (main != nullptr)
            main->commandManager.invokeDirectly (MainComponent::CommandIDs::zoomOut, true);
    }
    else if (buttonThatWasClicked == viewButton.get())
    {
        switch (frameEditor->getActiveView())
        {
            case Frame::front:
                frameEditor->setActiveView (Frame::bottom);
                break;
            case Frame::bottom:
                frameEditor->setActiveView (Frame::left);
                break;
            default:
                frameEditor->setActiveView (Frame::front);
                break;
        }
    }
}
