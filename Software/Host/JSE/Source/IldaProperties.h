/*
    IldaProperties.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"

//==============================================================================
/*
*/
class IldaProperties  : public Component,
                        public Button::Listener,
                        public ActionListener
{
public:
    IldaProperties (FrameEditor* editor);
    ~IldaProperties() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    void actionListenerCallback (const String& message) override;

    void refresh();
    
private:
    FrameEditor* frameEditor;
    std::unique_ptr<ToggleButton> layerVisible;
    std::unique_ptr<ToggleButton> showBlanking;
    std::unique_ptr<ToggleButton> drawLines;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IldaProperties)
};
