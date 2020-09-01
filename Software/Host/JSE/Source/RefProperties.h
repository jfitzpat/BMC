/*
    RefProperties.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"

//==============================================================================
/*
*/
class RefProperties  : public Component,
                       public Button::Listener,
                       public Slider::Listener,
                       public ActionListener
{
public:
    RefProperties (FrameEditor* editor);
    ~RefProperties() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;

    void actionListenerCallback (const String& message) override;

private:
    FrameEditor* frameEditor;
    std::unique_ptr<ToggleButton> layerVisible;
    std::unique_ptr<Label> imageFileLabel;
    std::unique_ptr<TextButton> selectImageButton;
    std::unique_ptr<Slider> backgroundAlpha;
    std::unique_ptr<Slider> backgroundScale;
    std::unique_ptr<Slider> backgroundRotation;
    std::unique_ptr<Slider> backgroundXoffset;
    std::unique_ptr<Slider> backgroundYoffset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefProperties)
};
