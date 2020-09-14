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

// Keys for our properties files
#define KEY_RECENT_FILES "RecentFiles"

// Base ID for recent file menu
#define RECENT_BASE_ID (200)

//==============================================================================
MainComponent::MainComponent()
{
    // Properties File
    PropertiesFile::Options options;
    options.applicationName = "JSE";
    options.folderName = "me.scrootch.jse";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
        
    propertiesFile.reset (new PropertiesFile (options));
    
    // Recent Files
    recentFileList.reset (new RecentlyOpenedFilesList());
    recentFileList->restoreFromString (propertiesFile->getValue(KEY_RECENT_FILES));
    recentFileList->removeNonExistentFiles();
    
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

    Rectangle<int> r = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    if (r.getWidth() >= 1200 && r.getHeight() >= 800)
    {
        #ifdef JUCE_WINDOWS
            setSize(1200 - getLookAndFeel().getDefaultMenuBarHeight(), 800);
        #else
            setSize (1200, 800);
        #endif
    }
    else
        setSize(r.getWidth(), r.getHeight());
    
    Timer::callAfterDelay (300, [this] { actionListenerCallback(EditorActions::framesChanged); });
}

MainComponent::~MainComponent()
{
    // Save our recent files changes
    propertiesFile->setValue (KEY_RECENT_FILES, recentFileList->toString());
    propertiesFile->saveIfNeeded();
    
    recentFileList = nullptr;
    propertiesFile = nullptr;
    
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
    Logger::outputDebugString("w: " + String(getWidth()) + " h: " + String(getHeight()));
    toolBar->setBounds (0, 0, 200, 97);
    frameList->setBounds (0, 97, 200, getHeight() - 97);
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

        PopupMenu recentFilesMenu;
        if (recentFileList->getNumFiles())
        {
            recentFileList->createPopupMenuItems(recentFilesMenu,
                                                 RECENT_BASE_ID, true, false);
            recentFilesMenu.addSeparator();
            recentFilesMenu.addCommandItem(&commandManager, CommandIDs::clearRecentFiles);
            menu.addSubMenu ("Open Recent", recentFilesMenu);
        }
        else
            menu.addSubMenu ("Open Recent", recentFilesMenu, false);
        
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::fileSave);
        menu.addCommandItem (&commandManager, CommandIDs::fileSaveAs);
        menu.addCommandItem (&commandManager, CommandIDs::fileExport);

        #if JUCE_WINDOWS
            menu.addSeparator();
            menu.addCommandItem(&commandManager, CommandIDs::appPreferences);
            menu.addSeparator();
            menu.addCommandItem(&commandManager, CommandIDs::appExit);
        #endif
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem (&commandManager, CommandIDs::editUndo);
        menu.addCommandItem (&commandManager, CommandIDs::editRedo);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::editSelectAll);
        menu.addCommandItem (&commandManager, CommandIDs::editClearSelection);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::newFrame);
        menu.addCommandItem (&commandManager, CommandIDs::duplicateFrame);
        menu.addCommandItem (&commandManager, CommandIDs::deleteFrame);
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem (&commandManager, CommandIDs::helpWebSite);
        #if JUCE_WINDOWS
            menu.addCommandItem (&commandManager, CommandIDs::appAbout);
        #endif
    }
    return menu;
}

void MainComponent::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
    if (menuItemID >= RECENT_BASE_ID)
    {
        File f = recentFileList->getFile (menuItemID - RECENT_BASE_ID);
        if (f.getFullPathName() != frameEditor->getLoadedFile().getFullPathName())
        {
            frameEditor->loadFile (f);
            if (frameEditor->getLoadedFile().getFileName().length())
                recentFileList->addFile (frameEditor->getLoadedFile());
        }
    }
}

//==============================================================================
PopupMenu MainComponent::getExtraAppleMenu()
{
    PopupMenu menu;
    
    menu.addCommandItem (&commandManager, CommandIDs::appAbout);
    menu.addSeparator();
    menu.addCommandItem (&commandManager, CommandIDs::appPreferences);
    menu.addSeparator();
    
    return menu;
}

bool MainComponent::isFileDirty()
{
    if (frameEditor->getDirtyCounter())
        return true;
    
    return false;
}

//==============================================================================
void MainComponent::actionListenerCallback (const String& message)
{
    commandManager.commandStatusChanged();
    
    if (message == EditorActions::framesChanged ||
        message == EditorActions::dirtyStatusChanged)
    {
        DocumentWindow* w = dynamic_cast<DocumentWindow*>(getTopLevelComponent());
        if (w)
        {
            String name (ProjectInfo::projectName);
            name += " - ";
            if (frameEditor->getDirtyCounter())
                name += "*";
            if (frameEditor->getLoadedFile().getFileName().length())
                name += frameEditor->getLoadedFile().getFileName();
            else
                name += "<New File>";
            
            w->setName (name);
        }
    }
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
                                CommandIDs::fileSave,
                                CommandIDs::fileSaveAs,
                                CommandIDs::fileExport,
                                CommandIDs::appExit,
                                CommandIDs::editUndo,
                                CommandIDs::editRedo,
                                CommandIDs::editSelectAll,
                                CommandIDs::editClearSelection,
                                CommandIDs::deleteFrame,
                                CommandIDs::newFrame,
                                CommandIDs::duplicateFrame,
                                CommandIDs::helpWebSite,
                                CommandIDs::appAbout,
                                CommandIDs::appPreferences,
                                CommandIDs::clearRecentFiles };
    
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
            result.setInfo ("Open...", "Open an existing file...", "Menu", 0);
            result.addDefaultKeypress('o', ModifierKeys::commandModifier);
            break;
        case CommandIDs::fileSave:
            result.setInfo ("Save", "Save changes to the current file", "Menu", 0);
            result.addDefaultKeypress('s', ModifierKeys::commandModifier);
            result.setActive (isFileDirty());
            break;
        case CommandIDs::fileSaveAs:
            result.setInfo ("Save As...", "Save to a new file", "Menu", 0);
            break;
        case CommandIDs::fileExport:
            result.setInfo ("Export ILDA File...", "Save to an ILDA file", "Menu", 0);
            break;
        case CommandIDs::clearRecentFiles:
            result.setInfo ("Clear Menu","Clear recent file list", "Menu", 0);
            break;
        case CommandIDs::appExit:
            result.setInfo("Exit", "Exit the application", "Menu", 0);
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
        case CommandIDs::deleteFrame:
            result.setInfo ("Delete Frame", "Delete the current frame", "Menu", 0);
            result.setActive (frameEditor->getFrameCount() > 1);
            break;
        case CommandIDs::newFrame:
            result.setInfo ("New Frame", "Insert a new frame", "Menu", 0);
            break;
        case CommandIDs::duplicateFrame:
            result.setInfo ("Duplicate Frame", "Duplicate selected frame", "Menu", 0);
            break;

        case CommandIDs::editSelectAll:
            result.setInfo ("Select All", "Select entire layer", "Menu", 0);
            result.addDefaultKeypress('a', ModifierKeys::commandModifier);
            break;
        case CommandIDs::editClearSelection:
            result.setInfo ("Deselect", "Clear current selection", "Menu", 0);
            result.addDefaultKeypress('d', ModifierKeys::commandModifier);
            break;
        case CommandIDs::helpWebSite:
            result.setInfo ("BMC Website...", "Open the BMC Website", "Menu", 0);
            break;
        case CommandIDs::appAbout:
            result.setInfo ("About...", "Info about the application", "Menu", 0);
            break;
        case CommandIDs::appPreferences:
            result.setInfo ("Preferences...", "Set app preferences", "Menu", 0);
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
        case CommandIDs::editSelectAll:
            frameEditor->selectAll();
            break;
        case CommandIDs::editClearSelection:
            frameEditor->clearSelection();
            break;
        case CommandIDs::deleteFrame:
            frameEditor->deleteFrame (frameEditor->getFrameIndex());
            break;
        case CommandIDs::newFrame:
            frameEditor->newFrame();
            break;
        case CommandIDs::duplicateFrame:
            frameEditor->dupFrame();
            break;

        case CommandIDs::fileOpen:
            frameEditor->loadFile();
            if (frameEditor->getLoadedFile().getFileName().length())
                recentFileList->addFile (frameEditor->getLoadedFile());
            break;
        case CommandIDs::fileNew:
            frameEditor->newFile();
            break;
        case CommandIDs::fileSave:
            frameEditor->fileSave();
            if (frameEditor->getLoadedFile().getFileName().length())
                recentFileList->addFile (frameEditor->getLoadedFile());
            break;
        case CommandIDs::fileSaveAs:
            frameEditor->fileSaveAs();
            if (frameEditor->getLoadedFile().getFileName().length())
                recentFileList->addFile (frameEditor->getLoadedFile());
            break;
        case CommandIDs::fileExport:
            frameEditor->fileIldaExport();
            break;
            
        case CommandIDs::clearRecentFiles:
            recentFileList->clear();
            break;
            
        case CommandIDs::helpWebSite:
            {
                URL url ("http://scrootch.me/bmc");
                if (url.isWellFormed())
                    url.launchInDefaultBrowser();
            }
            break;
        
        case CommandIDs::appAbout:
            {
                String s = "Version: " + String(ProjectInfo::versionString);
                // Alpha or Beta?
                uint32 version = ProjectInfo::versionNumber;
                if (! (version & 0xFF0000))
                {
                    if ((version & 0xFF00) == 0x100)
                        s += " (Pre-Alpha " + String (version & 0xFF) + ")";
                    else if ((version & 0xFF00) == 0x200)
                        s += " (Alpha " + String (version & 0xFF) + ")";
                    else if ((version & 0xFF00) == 0x300)
                        s += " (Beta " + String (version & 0xFF) + ")";
                }
                s += "\rCopyright 2020 Scrootch.me!";
                AlertWindow::showMessageBox(AlertWindow::InfoIcon, "About JSE",
                                            s, "ok");
            }
            break;
            
        case CommandIDs::appPreferences:
            {
                AlertWindow::showMessageBox(AlertWindow::InfoIcon, "Preferences",
                                            "Placeholder for Preferences Dialog", "ok");
            }
            break;

        case CommandIDs::appExit:
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            break;
    }
    return true;
}
