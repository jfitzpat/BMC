/*
    PopupTool.h
    Tool button with popup component
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

//==============================================================================
class PopupTool  : public DrawableButton,
                   public ComponentListener
{
public:
    PopupTool (FrameEditor* editor)
    : DrawableButton ("", DrawableButton::ImageOnButtonBackground),
      frameEditor (editor),
      lockout (false)
    {
    }
    
    void clicked() override
    {
        if (! lockout)
        {
            lockout = true;
            auto controls = makeComponent();
            controls->addComponentListener (this);
            CallOutBox::launchAsynchronously (std::move(controls), getScreenBounds(), nullptr);
        }
    }

    void componentBeingDeleted (Component&) override
    {
        Timer::callAfterDelay (300, [this] { clearLockOut(); });
    }

    virtual std::unique_ptr<Component> makeComponent() = 0;

protected:
    FrameEditor* frameEditor;

private:
    void clearLockOut() { lockout = false; }
    bool lockout;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupTool)
};
