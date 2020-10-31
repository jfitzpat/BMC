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
    mainEditor.reset (new MainEditor (frameEditor.get()));
    addAndMakeVisible (mainEditor.get());
//    laserControls.reset (new LaserControls (frameEditor.get()));
//    addAndMakeVisible (laserControls.get());
    editProperties.reset (new EditProperties (frameEditor.get()));
    addAndMakeVisible (editProperties.get());
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
//    laserControls->setBounds (getWidth() - 200, 0, 200, 64);
    frameList->setBounds (getWidth() - 200, 0 /*64*/, 200, getHeight() /*- 64*/);

    editProperties->setBounds (0, 0, 200, getHeight());
    mainEditor->setBounds (200, 0, getWidth() - 400, getHeight());
}

//==============================================================================
StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Edit", "View", "Help" };
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
        if (recentFileList->getNumFiles() && (! frameEditor->isTransforming()))
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
        PopupMenu toolMenu;

        if (frameEditor->getActiveLayer() == FrameEditor::ilda ||
            frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            toolMenu.addCommandItem (&commandManager, CommandIDs::selectToolRequest);
            toolMenu.addCommandItem (&commandManager, CommandIDs::moveToolRequest);
            toolMenu.addCommandItem (&commandManager, CommandIDs::pointPenToolRequest);
            if (frameEditor->getActiveLayer() == FrameEditor::sketch)
            {
                toolMenu.addCommandItem (&commandManager, CommandIDs::lineToolRequest);
                toolMenu.addCommandItem (&commandManager, CommandIDs::rectToolRequest);
                toolMenu.addCommandItem (&commandManager, CommandIDs::ellipseToolRequest);
                toolMenu.addCommandItem (&commandManager, CommandIDs::centerRectToolRequest);
                toolMenu.addCommandItem (&commandManager, CommandIDs::centerEllipseToolRequest);
            }
        }
        
        menu.addCommandItem (&commandManager, CommandIDs::editUndo);
        menu.addCommandItem (&commandManager, CommandIDs::editRedo);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::editSelectAll);
        menu.addCommandItem (&commandManager, CommandIDs::editClearSelection);
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            menu.addCommandItem (&commandManager, CommandIDs::editCut);
            menu.addCommandItem (&commandManager, CommandIDs::editCopy);
            menu.addCommandItem (&commandManager, CommandIDs::editPaste);
        }
        menu.addCommandItem (&commandManager, CommandIDs::deleteSelection);
        if (frameEditor->getActiveLayer() == FrameEditor::ilda ||
            frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            menu.addCommandItem (&commandManager, CommandIDs::selectionDownRequest);
            menu.addCommandItem (&commandManager, CommandIDs::selectionUpRequest);
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::blankingToggleRequest);
            menu.addCommandItem (&commandManager, CommandIDs::cycleColorsRequest);
            menu.addSeparator();
            menu.addSubMenu ("Tools", toolMenu);
        }
        if (frameEditor->getActiveLayer() == FrameEditor::sketch)
        {
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::forceStraight);
            menu.addCommandItem (&commandManager, CommandIDs::zeroExit);
            menu.addCommandItem (&commandManager, CommandIDs::forceCurve);
            menu.addCommandItem (&commandManager, CommandIDs::selectEntry);
            menu.addCommandItem (&commandManager, CommandIDs::selectExit);
        }
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::newFrame);
        menu.addCommandItem (&commandManager, CommandIDs::duplicateFrame);
        menu.addCommandItem (&commandManager, CommandIDs::deleteFrame);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::moveFrameUp);
        menu.addCommandItem (&commandManager, CommandIDs::moveFrameDown);
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem (&commandManager, CommandIDs::zoomIn);
        menu.addCommandItem (&commandManager, CommandIDs::zoomOut);
        menu.addCommandItem (&commandManager, CommandIDs::zoomAll);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::frontView);
        menu.addCommandItem (&commandManager, CommandIDs::topView);
        menu.addCommandItem (&commandManager, CommandIDs::sideView);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::sketchLayer);
        menu.addCommandItem (&commandManager, CommandIDs::ildaLayer);
        menu.addCommandItem (&commandManager, CommandIDs::refLayer);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::toggleSketchVisible);
        menu.addCommandItem (&commandManager, CommandIDs::toggleIldaVisible);
        menu.addCommandItem (&commandManager, CommandIDs::toggleRefVisible);
    }
    else if (menuIndex == 3)
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
                                CommandIDs::editCut,
                                CommandIDs::editCopy,
                                CommandIDs::editPaste,
                                CommandIDs::editSelectAll,
                                CommandIDs::editClearSelection,
                                CommandIDs::deleteFrame,
                                CommandIDs::newFrame,
                                CommandIDs::moveFrameUp,
                                CommandIDs::moveFrameDown,
                                CommandIDs::duplicateFrame,
                                CommandIDs::helpWebSite,
                                CommandIDs::appAbout,
                                CommandIDs::appPreferences,
                                CommandIDs::clearRecentFiles,
                                CommandIDs::frontView,
                                CommandIDs::topView,
                                CommandIDs::sideView,
                                CommandIDs::sketchLayer,
                                CommandIDs::ildaLayer,
                                CommandIDs::refLayer,
                                CommandIDs::toggleSketchVisible,
                                CommandIDs::toggleIldaVisible,
                                CommandIDs::toggleRefVisible,
                                CommandIDs::zoomAll,
                                CommandIDs::zoomOut,
                                CommandIDs::zoomIn,
                                CommandIDs::panLeft,
                                CommandIDs::panRight,
                                CommandIDs::panUp,
                                CommandIDs::panDown,
                                CommandIDs::deleteSelection,
                                CommandIDs::cancelRequest,
                                CommandIDs::deleteRequest,
                                CommandIDs::upRequest,
                                CommandIDs::downRequest,
                                CommandIDs::leftRequest,
                                CommandIDs::rightRequest,
                                CommandIDs::smallUpRequest,
                                CommandIDs::smallDownRequest,
                                CommandIDs::smallLeftRequest,
                                CommandIDs::smallRightRequest,
                                CommandIDs::blankingToggleRequest,
                                CommandIDs::cycleColorsRequest,
                                CommandIDs::selectionDownRequest,
                                CommandIDs::selectionUpRequest,
                                CommandIDs::pointPenToolRequest,
                                CommandIDs::moveToolRequest,
                                CommandIDs::selectToolRequest,
                                CommandIDs::lineToolRequest,
                                CommandIDs::rectToolRequest,
                                CommandIDs::ellipseToolRequest,
                                CommandIDs::centerRectToolRequest,
                                CommandIDs::centerEllipseToolRequest,
                                CommandIDs::forceCurve,
                                CommandIDs::forceStraight,
                                CommandIDs::zeroExit,
                                CommandIDs::selectEntry,
                                CommandIDs::selectExit };
    
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
            #if JUCE_WINDOWS
                result.addDefaultKeypress(KeyPress::F4Key, ModifierKeys::altModifier);
            #endif
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
        case CommandIDs::editCut:
            result.setInfo ("Cut", "Cut the current selection", "Menu", 0);
            result.addDefaultKeypress('x', ModifierKeys::commandModifier);
            result.setActive (frameEditor->canCopy());
            break;
        case CommandIDs::editCopy:
            result.setInfo ("Copy", "Copy the current selection", "Menu", 0);
            result.addDefaultKeypress('c', ModifierKeys::commandModifier);
            result.setActive (frameEditor->canCopy());
            break;
        case CommandIDs::editPaste:
            result.setInfo ("Paste", "Paste the current selection", "Menu", 0);
            result.addDefaultKeypress('v', ModifierKeys::commandModifier);
            result.setActive (frameEditor->canPaste());
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
        case CommandIDs::moveFrameDown:
            result.setInfo ("Move Frame Down", "Move the current frame down", "Menu", 0);
            result.setActive (frameEditor->getFrameIndex() < (frameEditor->getFrameCount() - 1));
            break;
        case CommandIDs::moveFrameUp:
            result.setInfo ("Move Frame Up", "Move the current frame up", "Menu", 0);
            result.setActive (frameEditor->getFrameIndex());
            break;

        case CommandIDs::frontView:
            result.setInfo ("Front View", "View along Z axis", "Menu", 0);
            result.addDefaultKeypress('1', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveView() == Frame::front);
            break;
        case CommandIDs::topView:
            result.setInfo ("Bottom View", "View along Y axis", "Menu", 0);
            result.addDefaultKeypress('2', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveView() == Frame::bottom);
            break;
        case CommandIDs::sideView:
            result.setInfo ("Left View", "View along Xaxis", "Menu", 0);
            result.addDefaultKeypress('3', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveView() == Frame::left);
            break;

        case CommandIDs::sketchLayer:
            result.setInfo ("Sketch Layer", "Select the Sketch Layer", "Menu", 0);
            result.addDefaultKeypress ('4', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveLayer() == FrameEditor::sketch);
            break;
        case CommandIDs::ildaLayer:
            result.setInfo ("ILDA Layer", "Select the ILDA Layer", "Menu", 0);
            result.addDefaultKeypress ('5', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveLayer() == FrameEditor::ilda);
            break;
        case CommandIDs::refLayer:
            result.setInfo ("Background Layer", "Select the Background Layer", "Menu", 0);
            result.addDefaultKeypress ('6', ModifierKeys::commandModifier);
            result.setTicked (frameEditor->getActiveLayer() == FrameEditor::reference);
            break;

        case CommandIDs::toggleSketchVisible:
            result.setInfo ("Sketch Visible", "Toggle Sketch Layer Visibility", "Menu", 0);
            result.addDefaultKeypress ('4', ModifierKeys::altModifier);
            result.setTicked (frameEditor->getSketchVisible());
            break;
        case CommandIDs::toggleIldaVisible:
            result.setInfo ("ILDA Visible", "Toggle Ilda Layer Visibility", "Menu", 0);
            result.addDefaultKeypress ('5', ModifierKeys::altModifier);
            result.setTicked (frameEditor->getIldaVisible());
            break;
        case CommandIDs::toggleRefVisible:
            result.setInfo ("Background Visible", "Toggle Background Layer Visibility", "Menu", 0);
            result.addDefaultKeypress ('6', ModifierKeys::altModifier);
            result.setTicked (frameEditor->getRefVisible());
            break;

        case CommandIDs::zoomAll:
            result.setInfo ("Fit All", "Fit entire edit field onscreen", "Menu", 0);
            result.addDefaultKeypress('0', ModifierKeys::commandModifier);
            result.setActive (mainEditor->getZoom() != MIN_ZOOM);
            break;
        case CommandIDs::zoomIn:
            result.setInfo ("Zoom In", "Zoom in on the selection", "Menu", 0);
            result.addDefaultKeypress('+', ModifierKeys::commandModifier);
            result.setActive (mainEditor->getZoom() != MAX_ZOOM);
            break;
        case CommandIDs::zoomOut:
            result.setInfo ("Zoom Out", "Zoom out on the selection", "Menu", 0);
            result.addDefaultKeypress('-', ModifierKeys::commandModifier);
            result.setActive (mainEditor->getZoom() != MIN_ZOOM);
            break;
        case CommandIDs::panUp:
            result.setInfo ("Pan Up", "Pan the edit view up", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::upKey, ModifierKeys::commandModifier);
            break;
        case CommandIDs::panDown:
            result.setInfo ("Pan Down", "Pan the edit view down", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::downKey, ModifierKeys::commandModifier);
            break;
        case CommandIDs::panLeft:
            result.setInfo ("Pan Left", "Pan the edit view left", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::leftKey, ModifierKeys::commandModifier);
            break;
        case CommandIDs::panRight:
            result.setInfo ("Pan Right", "Pan the edit view right", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::rightKey, ModifierKeys::commandModifier);
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
        case CommandIDs::cancelRequest:
            result.setInfo ("Cancel", "Cancel current operation", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::escapeKey, 0);
            break;
        case CommandIDs::deleteRequest:
            result.setInfo ("Delete", "Delete", "ShortCut", 0);
            result.addDefaultKeypress (KeyPress::deleteKey, 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, 0);
            break;
        case CommandIDs::deleteSelection:
            result.setInfo ("Delete", "Delete the current selection", "Menu", 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::commandModifier);
            result.setActive (frameEditor->hasSelection());
            break;
        case CommandIDs::upRequest:
            result.setInfo ("Move Up", "Move the current selectiop up", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::upKey, 0);
            break;
        case CommandIDs::downRequest:
            result.setInfo ("Move Down", "Move the current selectiop down", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::downKey, 0);
            break;
        case CommandIDs::leftRequest:
            result.setInfo ("Move Left", "Move the current selectiop left", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::leftKey, 0);
            break;
        case CommandIDs::rightRequest:
            result.setInfo ("Move Right", "Move the current selectiop right", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::rightKey, 0);
            break;
        case CommandIDs::smallUpRequest:
            result.setInfo ("Nudge Up", "Nudge the current selectiop up", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::upKey, ModifierKeys::shiftModifier);
            break;
        case CommandIDs::smallDownRequest:
            result.setInfo ("Nudge Down", "Nudge the current selectiop down", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::downKey, ModifierKeys::shiftModifier);
            break;
        case CommandIDs::smallLeftRequest:
            result.setInfo ("Nudge Left", "Nudge the current selectiop left", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::leftKey, ModifierKeys::shiftModifier);
            break;
        case CommandIDs::smallRightRequest:
            result.setInfo ("Nudge Right", "Nudge the current selectiop right", "ShortCut", 0);
            result.addDefaultKeypress(KeyPress::rightKey, ModifierKeys::shiftModifier);
            break;

        case CommandIDs::blankingToggleRequest:
            result.setInfo ("Toggle Blanking", "Toggle Blanking on/off", "Menu", 0);
            result.addDefaultKeypress ('b', 0);
            break;
        case CommandIDs::cycleColorsRequest:
            result.setInfo ("Cycle Colors", "Cycle through standard colors", "Menu", 0);
            result.addDefaultKeypress ('c', 0);
            break;
        case CommandIDs::selectionUpRequest:
            result.setInfo ("Shift Selection Up", "Shift selection to higher points", "Menu", 0);
            result.addDefaultKeypress (']', 0);
            result.setActive (frameEditor->hasMovableSelection());
            break;
        case CommandIDs::selectionDownRequest:
            result.setInfo ("Shift Selection Down", "Shift selection to lower points", "Menu", 0);
            result.addDefaultKeypress ('[', 0);
            result.setActive (frameEditor->hasMovableSelection());
            break;
        case CommandIDs::pointPenToolRequest:
            result.setInfo (frameEditor->getActiveLayer() == FrameEditor::ilda ? "Point Tool" : "Pen Tool", "Select Point/Pen Tool", "Menu", 0);
            result.addDefaultKeypress ('p', 0);
            result.setActive (frameEditor->getActiveLayer() != FrameEditor::reference);
            result.setTicked ((frameEditor->getActiveLayer() == FrameEditor::sketch &&
                               frameEditor->getActiveSketchTool() == FrameEditor::sketchPenTool) ||
                              (frameEditor->getActiveLayer() == FrameEditor::ilda &&
                                                 frameEditor->getActiveIldaTool() == FrameEditor::pointTool));
            break;
        case CommandIDs::moveToolRequest:
            result.setInfo ("Move Tool", "Select Move Tool", "Menu", 0);
            result.addDefaultKeypress ('o', 0);
            result.setActive (frameEditor->getActiveLayer() != FrameEditor::reference);
            result.setTicked ((frameEditor->getActiveLayer() == FrameEditor::sketch &&
                               frameEditor->getActiveSketchTool() == FrameEditor::sketchMoveTool) ||
                              (frameEditor->getActiveLayer() == FrameEditor::ilda &&
                                                 frameEditor->getActiveIldaTool() == FrameEditor::moveTool));
            break;
        case CommandIDs::selectToolRequest:
            result.setInfo ("Select Tool", "Select Selection Tool", "Menu", 0);
            result.addDefaultKeypress ('i', 0);
            result.setActive (frameEditor->getActiveLayer() != FrameEditor::reference);
            result.setTicked ((frameEditor->getActiveLayer() == FrameEditor::sketch &&
                               frameEditor->getActiveSketchTool() == FrameEditor::sketchSelectTool) ||
                              (frameEditor->getActiveLayer() == FrameEditor::ilda &&
                                                 frameEditor->getActiveIldaTool() == FrameEditor::selectTool));
            break;
        case CommandIDs::lineToolRequest:
            result.setInfo ("Line Tool", "Select Line Tool", "Menu", 0);
            result.addDefaultKeypress ('l', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch);
            result.setTicked (frameEditor->getActiveSketchTool() == FrameEditor::sketchLineTool);
            break;
        case CommandIDs::rectToolRequest:
            result.setInfo ("Rectangle Tool", "Select Rectangle Tool", "Menu", 0);
            result.addDefaultKeypress ('k', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch);
            result.setTicked (frameEditor->getActiveSketchTool() == FrameEditor::sketchRectTool);
            break;
        case CommandIDs::ellipseToolRequest:
            result.setInfo ("Ellipse Tool", "Select Ellipse Tool", "Menu", 0);
            result.addDefaultKeypress ('j', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch);
            result.setTicked (frameEditor->getActiveSketchTool() == FrameEditor::sketchEllipseTool);
            break;
        case CommandIDs::centerEllipseToolRequest:
            result.setInfo ("Centered Ellipse Tool", "Select Centered Ellipse Tool", "Menu", 0);
            result.addDefaultKeypress ('n', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch);
            result.setTicked (frameEditor->getActiveSketchTool() == FrameEditor::sketchCenterEllipseTool);
            break;
        case CommandIDs::centerRectToolRequest:
            result.setInfo ("Centered Rectangle Tool", "Select Centered Rectangle Tool", "Menu", 0);
            result.addDefaultKeypress ('m', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch);
            result.setTicked (frameEditor->getActiveSketchTool() == FrameEditor::sketchCenterRectTool);
            break;
         case CommandIDs::forceCurve:
            result.setInfo ("Curve Anchor", "", "Menu", 0);
            result.addDefaultKeypress (',', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch && (frameEditor->getIPathSelection().getAnchor() != -1));
            break;
        case CommandIDs::forceStraight:
            result.setInfo ("Straighten Anchor", "", "Menu", 0);
            result.addDefaultKeypress ('.', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch && (frameEditor->getIPathSelection().getAnchor() != -1));
            break;
        case CommandIDs::selectEntry:
            result.setInfo ("Select Anchor Entry", "", "Menu", 0);
            result.addDefaultKeypress (';', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch && (frameEditor->getIPathSelection().getAnchor() != -1));
            break;
        case CommandIDs::selectExit:
            result.setInfo ("Select Anchor Exit", "", "Menu", 0);
            result.addDefaultKeypress ('\'', 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch && (frameEditor->getIPathSelection().getAnchor() != -1));
            break;
        case CommandIDs::zeroExit:
            result.setInfo ("Straighten Anchor Exit", "", "Menu", 0);
            result.setActive (frameEditor->getActiveLayer() == FrameEditor::sketch && (frameEditor->getIPathSelection().getAnchor() != -1));
            break;

        default:
            break;
    }
    
    // If we are transforming (quasi modal), disable all the action keys/menus
    if (frameEditor->isTransforming())
        result.setActive (false);
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
        case CommandIDs::editCut:
            frameEditor->cut();
            break;
        case CommandIDs::editCopy:
            frameEditor->copy();
            break;
        case CommandIDs::editPaste:
            frameEditor->paste();
            break;
        case CommandIDs::editSelectAll:
            frameEditor->selectAll();
            break;
        case CommandIDs::editClearSelection:
            frameEditor->clearSelection();
            break;
        case CommandIDs::deleteFrame:
            frameEditor->deleteFrame();
            break;
        case CommandIDs::newFrame:
            frameEditor->newFrame();
            break;
        case CommandIDs::duplicateFrame:
            frameEditor->dupFrame();
            break;
        case CommandIDs::moveFrameUp:
            frameEditor->moveFrameUp();
            break;
        case CommandIDs::moveFrameDown:
            frameEditor->moveFrameDown();
            break;
            
        case CommandIDs::zoomAll:
            mainEditor->setZoom (1.0);
            break;
            
        case CommandIDs::zoomIn:
            {
                float f;
                f = mainEditor->getZoom();
                if (f < MAX_ZOOM)
                {
                    f = round (f) * 2.0f;
                    if (f > MAX_ZOOM)
                        f = MAX_ZOOM;
                    
                    mainEditor->setZoom (f);
                }
            }
            break;
        case CommandIDs::zoomOut:
            {
                float f;
                f = mainEditor->getZoom();
                if (f > MIN_ZOOM)
                {
                    f = round (f) / 2.0f;
                    if (f < MIN_ZOOM)
                        f = MIN_ZOOM;
                    
                    mainEditor->setZoom (f);
                }
            }
            break;
        case CommandIDs::panUp:
            mainEditor->panUp();
            break;
        case CommandIDs::panDown:
            mainEditor->panDown();
            break;
        case CommandIDs::panLeft:
            mainEditor->panLeft();
            break;
        case CommandIDs::panRight:
            mainEditor->panRight();
            break;

        case CommandIDs::frontView:
            frameEditor->setActiveView (Frame::front);
            break;
        case CommandIDs::topView:
            frameEditor->setActiveView (Frame::bottom);
            break;
        case CommandIDs::sideView:
            frameEditor->setActiveView (Frame::left);
            break;

        case CommandIDs::sketchLayer:
            frameEditor->setActiveLayer (FrameEditor::sketch);
            break;
        case CommandIDs::ildaLayer:
            frameEditor->setActiveLayer (FrameEditor::ilda);
            break;
        case CommandIDs::refLayer:
            frameEditor->setActiveLayer (FrameEditor::reference);
            break;

        case CommandIDs::toggleSketchVisible:
            frameEditor->setSketchVisible (! frameEditor->getSketchVisible());
            break;
        case CommandIDs::toggleIldaVisible:
            frameEditor->setIldaVisible (! frameEditor->getIldaVisible());
            break;
        case CommandIDs::toggleRefVisible:
            frameEditor->setRefVisible (! frameEditor->getRefVisible());
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
        
        case CommandIDs::upRequest:
            frameEditor->upRequest();
            break;
        case CommandIDs::downRequest:
            frameEditor->downRequest();
            break;
        case CommandIDs::leftRequest:
            frameEditor->leftRequest();
            break;
        case CommandIDs::rightRequest:
            frameEditor->rightRequest();
            break;
        case CommandIDs::smallUpRequest:
            frameEditor->smallUpRequest();
            break;
        case CommandIDs::smallDownRequest:
            frameEditor->smallDownRequest();
            break;
        case CommandIDs::smallLeftRequest:
            frameEditor->smallLeftRequest();
            break;
        case CommandIDs::smallRightRequest:
            frameEditor->smallRightRequest();
            break;
        case CommandIDs::cancelRequest:
            frameEditor->cancelRequest();
            break;
        case CommandIDs::deleteSelection:
        case CommandIDs::deleteRequest:
            frameEditor->deleteRequest();
            break;
            
        case CommandIDs::blankingToggleRequest:
            frameEditor->toggleBlanking();
            break;
        case CommandIDs::cycleColorsRequest:
            frameEditor->cycleColors();
            break;
        
        case CommandIDs::selectionUpRequest:
            frameEditor->adjustSelection (1);
            break;
        case CommandIDs::selectionDownRequest:
            frameEditor->adjustSelection (-1);
            break;
            
        case CommandIDs::pointPenToolRequest:
            if (frameEditor->getActiveLayer() == FrameEditor::sketch)
                frameEditor->setActiveSketchTool (FrameEditor::sketchPenTool);
            else
                frameEditor->setActiveIldaTool (FrameEditor::pointTool);
            break;
        case CommandIDs::moveToolRequest:
            if (frameEditor->getActiveLayer() == FrameEditor::sketch)
                frameEditor->setActiveSketchTool (FrameEditor::sketchMoveTool);
            else
                frameEditor->setActiveIldaTool (FrameEditor::moveTool);
            break;
        case CommandIDs::selectToolRequest:
            if (frameEditor->getActiveLayer() == FrameEditor::sketch)
                frameEditor->setActiveSketchTool (FrameEditor::sketchSelectTool);
            else
                frameEditor->setActiveIldaTool (FrameEditor::selectTool);
            break;
        case CommandIDs::lineToolRequest:
            frameEditor->setActiveSketchTool (FrameEditor::sketchLineTool);
            break;
        case CommandIDs::rectToolRequest:
            frameEditor->setActiveSketchTool (FrameEditor::sketchRectTool);
            break;
        case CommandIDs::ellipseToolRequest:
            frameEditor->setActiveSketchTool (FrameEditor::sketchEllipseTool);
            break;
        case CommandIDs::centerRectToolRequest:
            frameEditor->setActiveSketchTool (FrameEditor::sketchCenterRectTool);
            break;
        case CommandIDs::centerEllipseToolRequest:
            frameEditor->setActiveSketchTool (FrameEditor::sketchCenterEllipseTool);
            break;
        case CommandIDs::forceCurve:
            frameEditor->forceAnchorCurved();
            break;
        case CommandIDs::forceStraight:
            frameEditor->forceAnchorStraight();
            break;
        case CommandIDs::selectEntry:
            frameEditor->selectEntry();
            break;
        case CommandIDs::selectExit:
            frameEditor->selectExit();
            break;
        case CommandIDs::zeroExit:
            frameEditor->zeroExitControl();
            break;
    }
    return true;
}
