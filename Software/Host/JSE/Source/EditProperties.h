/*
    EditProperties.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"

//==============================================================================
class PropTabbedComponent : public TabbedComponent
{
public:
    PropTabbedComponent (FrameEditor* editor)
    : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        frameEditor = editor;
    }
    
    virtual ~PropTabbedComponent() {;}
    
    void currentTabChanged (int newTabIndex, const String & /* newTabName */) override
    {
        frameEditor->setActiveLayer ((FrameEditor::Layer) newTabIndex);
    }
    
private:
    FrameEditor* frameEditor;
};

//==============================================================================
class EditProperties  : public Component,
                        public Button::Listener,
                        public ActionListener
{
public:
    EditProperties (FrameEditor* frame);
    ~EditProperties() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    //==============================================================================
    void actionListenerCallback (const String& message) override;

private:
    void updateZoomButtons();
    void updateViewButton();
    
    FrameEditor* frameEditor;
    
    std::unique_ptr<TextButton> viewButton;
    std::unique_ptr<Drawable> showAllIcon;
    std::unique_ptr<DrawableButton> showAllButton;
    std::unique_ptr<Drawable> zoomInIcon;
    std::unique_ptr<DrawableButton> zoomInButton;
    std::unique_ptr<Drawable> zoomOutIcon;
    std::unique_ptr<DrawableButton> zoomOutButton;
    
    std::unique_ptr<PropTabbedComponent> layerTabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditProperties)
};
