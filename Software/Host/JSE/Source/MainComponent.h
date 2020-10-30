/*
    MainComponent.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"
#include "LaserControls.h"
#include "EditProperties.h"
#include "FrameList.h"
#include "MainEditor.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public Component,
                       public MenuBarModel,
                       public ApplicationCommandTarget,
                       public ActionListener
{
public:
    // Commands we respond to
    enum CommandIDs
    {
        fileNew = 1,
        fileOpen,
        fileSave,
        fileSaveAs,
        fileExport,
        appExit,
        editUndo,
        editRedo,
        editCut,
        editCopy,
        editPaste,
        editSelectAll,
        editClearSelection,
        deleteFrame,
        newFrame,
        duplicateFrame,
        moveFrameUp,
        moveFrameDown,
        helpWebSite,
        appAbout,
        appPreferences,
        clearRecentFiles,
        frontView,
        topView,
        sideView,
        zoomAll,
        zoomIn,
        zoomOut,
        panLeft,
        panRight,
        panUp,
        panDown,
        deleteSelection,
        cancelRequest,
        deleteRequest,
        upRequest,
        downRequest,
        leftRequest,
        rightRequest,
        smallUpRequest,
        smallDownRequest,
        smallRightRequest,
        smallLeftRequest,
        blankingToggleRequest,
        cycleColorsRequest,
        selectionDownRequest,
        selectionUpRequest,
        pointPenToolRequest,
        moveToolRequest,
        selectToolRequest,
        lineToolRequest,
        rectToolRequest,
        ellipseToolRequest,
        centerRectToolRequest,
        centerEllipseToolRequest,
        selectEntry,
        selectExit,
        forceCurve,
        forceStraight,
        zeroExit
    };

    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int menuIndex, const String& /*menuName*/) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    
    PopupMenu getExtraAppleMenu();
    bool isFileDirty();
    
    //==============================================================================
    void actionListenerCallback (const String& message) override;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>& c) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

    ApplicationCommandManager commandManager;
    
private:
    //==============================================================================
    TooltipWindow toolTipWindow;
    
    std::unique_ptr<PropertiesFile> propertiesFile;
    std::unique_ptr<RecentlyOpenedFilesList> recentFileList;
    
    std::unique_ptr<FrameEditor> frameEditor;
    std::unique_ptr<LaserControls> laserControls;
    std::unique_ptr<EditProperties> editProperties;
    std::unique_ptr<MainEditor>mainEditor;
    std::unique_ptr<FrameList> frameList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
