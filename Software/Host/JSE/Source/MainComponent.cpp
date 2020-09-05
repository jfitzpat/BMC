/*
    MainComponent.cpp
    Contents component for our main window

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

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Shared frame editor worker class
    frameEditor.reset (new FrameEditor());
    frameEditor->addActionListener (this);
    
    // GUI components
    toolBar.reset (new EditToolBar (frameEditor.get()));
    addAndMakeVisible (toolBar.get());
    laserControls.reset (new LaserControls (frameEditor.get()));
    addAndMakeVisible (laserControls.get());
    editProperties.reset (new EditProperties (frameEditor.get()));
    addAndMakeVisible (editProperties.get());
    mainEditor.reset (new MainEditor (frameEditor.get()));
    addAndMakeVisible (mainEditor.get());
    frameList.reset (new FrameList (frameEditor.get()));
    addAndMakeVisible (frameList.get());
    
    setApplicationCommandManagerToWatch (&commandManager);
    commandManager.registerAllCommandsForTarget (this);

    // this lets the command manager use keypresses that arrive in our window to send out commands
    addKeyListener (commandManager.getKeyMappings());

    setWantsKeyboardFocus(true);

    Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    if (r.getWidth() >= 1200 && r.getHeight() >= 800)
        setSize (1200, 800);
    else
        setSize(r.getWidth(), r.getHeight());
}

MainComponent::~MainComponent()
{
    frameList = nullptr;
    toolBar = nullptr;
    laserControls = nullptr;
    editProperties = nullptr;
    mainEditor = nullptr;
    frameEditor = nullptr;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    toolBar->setBounds (0, 0, 200, 160);
    frameList->setBounds (0, 160, 200, getHeight() - 160);
    laserControls->setBounds (getWidth() - 200, 0, 200, 97);
    editProperties->setBounds (getWidth() - 200, 97, 200, getHeight() - 97);
    mainEditor->setBounds (200, 0, getWidth() - 400, getHeight());
}

//==============================================================================
StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Edit", "Help" };
}

PopupMenu MainComponent::getMenuForIndex (int menuIndex, const String& /*menuName*/)
{
    PopupMenu menu;
    
    if (menuIndex == 0)
    {
        menu.addCommandItem (&commandManager, CommandIDs::fileNew);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::fileOpen);
        menu.addCommandItem (&commandManager, CommandIDs::fileClose);
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem (&commandManager, CommandIDs::editUndo);
        menu.addCommandItem (&commandManager, CommandIDs::editRedo);
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem (&commandManager, CommandIDs::helpWebSite);
    }
    return menu;
}

void MainComponent::actionListenerCallback (const String& message)
{
    commandManager.commandStatusChanged();
}

//==============================================================================
ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void MainComponent::getAllCommands (Array<CommandID>& c)
{
    Array<CommandID> commands { CommandIDs::fileNew,
                                CommandIDs::fileOpen,
                                CommandIDs::editUndo,
                                CommandIDs::editRedo,
                                CommandIDs::helpWebSite };
    c.addArray (commands);
}

void MainComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::fileNew:
            result.setInfo ("New", "Create a new file", "Menu", 0);
            result.addDefaultKeypress('n', ModifierKeys::commandModifier);
            break;
        case CommandIDs::fileOpen:
            result.setInfo ("Open", "Open an existing file...", "Menu", 0);
            result.addDefaultKeypress('o', ModifierKeys::commandModifier);
            break;
        case CommandIDs::editUndo:
            result.addDefaultKeypress('z', ModifierKeys::commandModifier);
            if (frameEditor->canUndo())
            {
                result.setInfo ("Undo " + frameEditor->getUndoDescription(),
                                "Undo the last operation", "Menu", 0);
                result.setActive(true);
            }
            else
            {
                result.setInfo ("Undo", "Undo the last operation", "Menu", 0);
                result.setActive(false);
            }
            break;
        case CommandIDs::editRedo:
            result.addDefaultKeypress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            if (frameEditor->canRedo())
            {
                result.setInfo ("Redo " + frameEditor->getRedoDescription(),
                                "Redo the last undo", "Menu", 0);
                result.setActive(true);
            }
            else
            {
                result.setInfo ("Redo", "Redo the last undo", "Menu", 0);
                result.setActive(false);
            }
            break;
        case CommandIDs::helpWebSite:
            result.setInfo ("BMC Website", "Open the BMC Website", "Menu", 0);
            break;
        default:
            break;
    }
}

bool MainComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::editUndo:
            frameEditor->undo();
            break;
        case CommandIDs::editRedo:
            frameEditor->redo();
            break;
            
        case CommandIDs::fileOpen:
            frameEditor->loadFile();
            break;
    }
    return true;
}
