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

class CBSelector : public Component,
                   public Button::Listener
{
public:
    CBSelector (ChangeListener* listener, Colour color)
    {
        colorSelector.reset ( new ColourSelector (ColourSelector::showColourAtTop
                                                   | ColourSelector::editableColour
                                                   | ColourSelector::showSliders
                                                  | ColourSelector::showColourspace));
        addAndMakeVisible (colorSelector.get());
        colorSelector->setName ("background");
        colorSelector->setCurrentColour (color);
        colorSelector->addChangeListener (listener);
        colorSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        colorSelector->addComponentListener (dynamic_cast<ComponentListener*>(listener));
        
        whiteButton.reset (new juce::TextButton ("white"));
        addAndMakeVisible (whiteButton.get());
        whiteButton->addListener (this);
        whiteButton->setButtonText ("");
        whiteButton->setColour (TextButton::buttonColourId, Colours::white);

        redButton.reset (new juce::TextButton ("red"));
        addAndMakeVisible (redButton.get());
        redButton->addListener (this);
        redButton->setButtonText ("");
        redButton->setColour (TextButton::buttonColourId, Colours::red);

        greenButton.reset (new juce::TextButton ("green"));
        addAndMakeVisible (greenButton.get());
        greenButton->addListener (this);
        greenButton->setButtonText ("");
        greenButton->setColour (TextButton::buttonColourId, Colour(0, 255, 0));

        blueButton.reset (new juce::TextButton ("blue"));
        addAndMakeVisible (blueButton.get());
        blueButton->addListener (this);
        blueButton->setButtonText ("");
        blueButton->setColour (TextButton::buttonColourId, Colours::blue);

        yellowButton.reset (new juce::TextButton ("yellow"));
        addAndMakeVisible (yellowButton.get());
        yellowButton->addListener (this);
        yellowButton->setButtonText ("");
        yellowButton->setColour (TextButton::buttonColourId, Colours::yellow);

        cyanButton.reset (new juce::TextButton ("cyan"));
        addAndMakeVisible (cyanButton.get());
        cyanButton->addListener (this);
        cyanButton->setButtonText ("");
        cyanButton->setColour (TextButton::buttonColourId, Colours::cyan);

        magentaButton.reset (new juce::TextButton ("magenta"));
        addAndMakeVisible (magentaButton.get());
        magentaButton->addListener (this);
        magentaButton->setButtonText ("");
        magentaButton->setColour (TextButton::buttonColourId, Colours::magenta);

        blackButton.reset (new juce::TextButton ("black"));
        addAndMakeVisible (blackButton.get());
        blackButton->addListener (this);
        blackButton->setButtonText ("");
        blackButton->setColour (TextButton::buttonColourId, Colours::black);

        setSize (300, 472);
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (Colours::transparentBlack);
    }
    
    void resized() override
    {
        colorSelector->setBounds (0, 0, 300, 400);
        whiteButton->setBounds (8, 408, 65, 24);
        redButton->setBounds (81, 408, 65, 24);
        greenButton->setBounds (154, 408, 65, 24);
        blueButton->setBounds (227, 408, 65, 24);
        yellowButton->setBounds (8, 440, 65, 24);
        cyanButton->setBounds (81, 440, 65, 24);
        magentaButton->setBounds (154, 440, 65, 24);
        blackButton->setBounds (227, 440, 65, 24);
    }
    
    void buttonClicked (juce::Button* buttonThatWasClicked) override
    {
        colorSelector->setCurrentColour (buttonThatWasClicked->findColour(TextButton::buttonColourId));
    }

private:
    std::unique_ptr<ColourSelector> colorSelector;
    std::unique_ptr<TextButton> whiteButton;
    std::unique_ptr<TextButton> redButton;
    std::unique_ptr<TextButton> greenButton;
    std::unique_ptr<TextButton> blueButton;
    std::unique_ptr<TextButton> yellowButton;
    std::unique_ptr<TextButton> cyanButton;
    std::unique_ptr<TextButton> magentaButton;
    std::unique_ptr<TextButton> blackButton;
};

//==============================================================================
class ColourButton  : public TextButton,
                      public ChangeListener,
                      public ChangeBroadcaster,
                      public ComponentListener
{
public:
    ColourButton()
    : TextButton (""), lockout (false)
    {
        setColour (TextButton::buttonColourId, Colours::transparentBlack);
    }
    
    void clicked() override
    {
        if (! lockout)
        {
            lockout = true;
            auto colourSelector = std::make_unique<CBSelector>(this, findColour (TextButton::buttonColourId));
            CallOutBox::launchAsynchronously (std::move(colourSelector), getScreenBounds(), nullptr);
        }
    }

    void componentBeingDeleted (Component&) override
    {
        sendChangeMessage();
        Timer::callAfterDelay (300, [this] { clearLockOut(); });
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* cs = dynamic_cast<ColourSelector*> (source))
        {
            if (cs->getCurrentColour() != findColour (TextButton::buttonColourId))
                setColour (TextButton::buttonColourId, cs->getCurrentColour());
        }
    }

    void clearLockOut() { lockout = false; }
private:
    bool lockout;
};
