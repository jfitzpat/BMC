/*
    ColourButton.h
    Colour Picker Button
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

//==============================================================================
class ColourButton  : public TextButton,
                      public ChangeListener,
                      public ChangeBroadcaster,
                      public ComponentListener
{
public:
    ColourButton()
        : TextButton ("")
    {
        setColour (TextButton::buttonColourId, Colours::transparentBlack);
    }
    
    void clicked() override
    {
        auto* colourSelector = new ColourSelector (ColourSelector::showColourAtTop
                                                   | ColourSelector::editableColour
                                                   | ColourSelector::showSliders
                                                   | ColourSelector::showColourspace);

        colourSelector->setName ("background");
        colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
        colourSelector->addChangeListener (this);
        colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        colourSelector->addComponentListener (this);
        colourSelector->setSize (300, 400);

        CallOutBox::launchAsynchronously (colourSelector, getScreenBounds(), nullptr);
    }


    void componentBeingDeleted (Component&) override { sendChangeMessage(); }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* cs = dynamic_cast<ColourSelector*> (source))
        {
            if (cs->getCurrentColour() != findColour (TextButton::buttonColourId))
                setColour (TextButton::buttonColourId, cs->getCurrentColour());
        }
    }
};
