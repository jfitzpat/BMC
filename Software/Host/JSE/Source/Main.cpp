/*
    Main.cpp
    JUCE Application Class

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
#include "MainComponent.h"

static PopupMenu appleExtraMenu;

//==============================================================================
class JSEApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    JSEApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& /*commandLine*/) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& /*commandLine*/) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            MainComponent* main = new MainComponent();
            setContentOwned (main, true);

            // this lets the command manager use keypresses that arrive in our window to send out commands
            addKeyListener (main->commandManager.getKeyMappings());

            #if JUCE_IOS || JUCE_ANDROID
             setFullScreen (true);
            #else
             setResizable (true, true);
             setResizeLimits (640, 480, 32000, 32000);
             centreWithSize (getWidth(), getHeight());
            #endif

            #if JUCE_MAC
             appleExtraMenu = main->getExtraAppleMenu();
             MenuBarModel::setMacMainMenu (main, &appleExtraMenu);
            #else
             setMenuBar(main);
            #endif
            
            setVisible (true);
            
            // Force activation of the child
            // This fixes a quirk if you use the OS X native
            // menu without ever having clicked on the app
            Timer::callAfterDelay (300, [this] { getContentComponent()->grabKeyboardFocus(); });
        }

        ~MainWindow()
        {
            setMenuBar (nullptr);
            
            #if JUCE_MAC
              MenuBarModel::setMacMainMenu (nullptr);
            #endif
        }
        
        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (JSEApplication)
