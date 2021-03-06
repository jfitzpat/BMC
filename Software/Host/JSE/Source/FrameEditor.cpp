/*
    FrameEditor.cpp
    Frame Editor Object that is shared between editor GUI components

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
#include "IldaLoader.h"
#include "JSEFileSaver.h"
#include "JSEFileLoader.h"
#include "IldaExporter.h"
#include "ShortestPath.h"
#include "CurveFit.h"
#include "FrameEditor.h"

#include "FrameUndo.h"      // UndoableTask classes

// !!!! Should probably be a preference
const int blankPointSpacing = 1800;

//==============================================================================
FrameEditor::FrameEditor()
    : dirtyCounter (0),
      scanRate (22000),
      zoomFactor (1.0),
      activeLayer (sketch),
      activeView (Frame::front),
      activeIldaTool (selectTool),
      pointToolColor (Colours::white),
      lastVisiblePointToolColor (Colours::white),
      activeSketchTool (sketchSelectTool),
      sketchToolColor (Colours::white),
      lastVisibleSketchToolColor (Colours::white),
      sketchVisible (true),
      ildaVisible (true),
      ildaShowBlanked (true),
      ildaDrawLines (true),
      refVisible (true),
      refDrawGrid (true),
      refOpacity (1.0),
      frameIndex (0),
      tranformInProgress (false)
{
    Frames.add (new Frame());
    currentFrame = Frames[frameIndex];    
}

FrameEditor::~FrameEditor()
{
    currentFrame = nullptr;
}

//==============================================================================
void FrameEditor::setDirtyCounter (uint32 count)
{
    dirtyCounter = count;
    sendActionMessage (EditorActions::dirtyStatusChanged);
}

void FrameEditor::incDirtyCounter()
{
    dirtyCounter++;

    if (dirtyCounter == 1)
    {
        sendActionMessage (EditorActions::dirtyStatusChanged);
        refreshThumb();
    }
}

void FrameEditor::decDirtyCounter()
{
    if (dirtyCounter)
    {
        dirtyCounter--;
        
        if (! dirtyCounter)
        {
            sendActionMessage (EditorActions::dirtyStatusChanged);
            refreshThumb();
        }
    }
}

void FrameEditor::refreshThumb()
{
    currentFrame->buildThumbNail();
    sendActionMessage (EditorActions::frameThumbsChanged);
}

//==============================================================================
void FrameEditor::fileSave()
{
    refreshThumb();
    
    File f = getLoadedFile();
    
    if ((! f.getFileName().length()) || (f.getFileExtension() == ".ild"))
    {
        fileSaveAs();
        return;
    }
        
    if (! JSEFileSaver::save (this, f))
    {
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                   "An error occurred saving the file!", "ok");
    }
    else
        setDirtyCounter (0);
}

void FrameEditor::fileSaveAs()
{
    refreshThumb();
    
    FileChooser myChooser ("Choose File to Save As...",
                           File::getSpecialLocation (File::userDocumentsDirectory),
                           "*.jse");

    if (myChooser.browseForFileToSave (true))
    {
        File f = myChooser.getResult();
        if (! JSEFileSaver::save (this, f))
        {
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                       "An error occurred saving the file!", "ok");
        }
        else
        {
            clearUndoHistory();
            _setLoadedFile (f);
            setDirtyCounter(0);
        }
    }
}

void FrameEditor::fileIldaExport()
{
    refreshThumb();
    
    FileChooser myChooser ("Choose File to Export to...",
                           File::getSpecialLocation (File::userDocumentsDirectory),
                           "*.ild");

    if (myChooser.browseForFileToSave (true))
    {
        File f = myChooser.getResult();
        if (! IldaExporter::save (Frames, f))
        {
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                       "An error occurred saving the file!", "ok");
        }
    }
}

//==========================================================================================
bool FrameEditor::hasSelection()
{
    if (activeLayer == ilda && (! ildaSelection.isEmpty()))
        return true;
    else if (activeLayer == sketch && (! iPathSelection.isEmpty()))
        return true;
        
    return false;
}

bool FrameEditor::hasMovableSelection()
{
    if (! hasSelection())
        return false;
    
    if (activeLayer == sketch && (iPathSelection.getAnchor() == -1))
        return false;
    
    return true;
}

bool FrameEditor::canCopy()
{
    if (activeLayer == sketch)
        if (! iPathSelection.isEmpty())
            if (iPathSelection.getAnchor() == -1)
                return true;
    
    return false;
}

bool FrameEditor::canPaste()
{
    return (activeLayer == sketch) && iPathCopy.size();
}

void FrameEditor::copy()
{
    if (! canCopy())
        return;
    
    iPathCopy.clear();
    getSelectedIPaths (iPathCopy);
    sendActionMessage (EditorActions::selectionCopied);
}

void FrameEditor::adjustSelection (int offset)
{
    if (activeLayer == ilda)
        adjustIldaSelection (offset);
    else if (activeLayer == sketch)
        adjustIPathSelection (offset);
}

void FrameEditor::toggleBlanking()
{
    if (activeLayer == ilda)
        togglePointToolBlank();
    else if (activeLayer == sketch)
        toggleSketchToolBlank();
}

void FrameEditor::cycleColors()
{
    if (activeLayer == ilda)
        cyclePointToolColors();
    else if (activeLayer == sketch)
        cycleSketchToolColors();
}

//==============================================================================
void FrameEditor::getCenterOfIldaSelection (int16& x, int16& y, int16& z)
{
    // Don't bother if there is no selection visible
    if (ildaSelection.isEmpty() || activeLayer != ilda)
    {
        x = y = z = 0;
        return;
    }
    
    bool first = true;
    int minx, maxx, miny, maxy, minz, maxz;
    minx = maxx = miny = maxy = minz = maxz = 0;  // For Visual Studio warning
    
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i=0; i < r.getLength(); ++i)
        {
            Frame::IPoint point;
            currentFrame->getPoint (r.getStart() + (uint16)i, point);
            
            if (first)
            {
                minx = maxx = point.x.w;
                miny = maxy = point.y.w;
                minz = maxz = point.z.w;
                first = false;
            }
            else
            {
                if (point.x.w < minx)
                    minx = point.x.w;
                else if (point.x.w > maxx)
                    maxx = point.x.w;
                
                if (point.y.w < miny)
                    miny = point.y.w;
                else if (point.y.w > maxy)
                    maxy = point.y.w;
                
                if (point.z.w < minz)
                    minz = point.z.w;
                else if (point.z.w > maxz)
                    maxz = point.z.w;
            }
        }
    }
    
    x = (int16)((minx + maxx) / 2);
    y = (int16)((miny + maxy) / 2);
    z = (int16)((minz + maxz) / 2);
}

const Point<int> FrameEditor::getComponentCenterOfIldaSelection()
{
    int16 cx, cy, cz;
    getCenterOfIldaSelection (cx, cy, cz);
    
    int x = Frame::toCompX (activeView == Frame::left ? cz : cx);
    int y = Frame::toCompY (activeView == Frame::bottom ? cz : cy);

    return Point<int> (x, y);
}

void FrameEditor::getComponentCenterOfIldaSelection (int&x, int&y)
{
    int16 cx, cy, cz;
    getCenterOfIldaSelection (cx, cy, cz);
    
    x = Frame::toCompX (activeView == Frame::left ? cz : cx);
    y = Frame::toCompY (activeView == Frame::bottom ? cz : cy);
}

void FrameEditor::getIldaSelectedPoints (Array<Frame::IPoint>& points)
{
    points.clear();
    
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        
        for (uint16 i = 0; i < r.getLength(); ++i)
        {
            Frame::IPoint point;
            
            getPoint (r.getStart() + i, point);
            points.add (point);
        }
    }
}

void FrameEditor::getIldaPoints (const SparseSet<uint16>& selection,
                                 Array<Frame::IPoint>& points)
{
    points.clear();
    
    for (auto n = 0; n < selection.getNumRanges(); ++n)
    {
        Range<uint16> r = selection.getRange (n);
        
        for (uint16 i = 0; i < r.getLength(); ++i)
        {
            Frame::IPoint point;
            
            getPoint (r.getStart() + i, point);
            points.add (point);
        }
    }
}

void FrameEditor::getCenterOfIPathSelection (int& x, int& y)
{
    Rectangle<float> rect (0.0f,0.0f,65535.0f,65535.0f);

    if (activeLayer == sketch)
    {
        bool first = true;
        
        for (auto n = 0; n < iPathSelection.getNumRanges(); ++n)
        {
            Range<uint16> r = iPathSelection.getRange (n);
            for (auto i = r.getStart(); i < r.getEnd(); ++i)
            {
                IPath p = getIPath (i);
                if (first)
                {
                    rect = p.getPath().getBounds();
                    first = false;
                }
                else
                    rect = rect.getUnion (p.getPath().getBounds());
            }
        }
    }
    
    x = (int)rect.getCentreX();
    y = (int)rect.getCentreY();
}

//==============================================================================
void FrameEditor::setActiveLayer (Layer layer)
{
    if (layer != activeLayer)
    {
        beginNewTransaction ("Layer Change");
        perform (new UndoableSetLayer (this, layer));
    }
}

void FrameEditor::setActiveView (View view)
{
    if (view != activeView)
    {
        beginNewTransaction ("View Change");
        perform (new UndoableSetView (this, view));
    }
}

void FrameEditor::setActiveIldaTool (IldaTool tool)
{
    if (activeIldaTool != tool)
    {
        beginNewTransaction ("Tool Change");
        perform (new UndoableSetIldaTool (this, tool));
    }
}

void FrameEditor::setPointToolColor (const Colour& color)
{
    if (pointToolColor != color)
    {
        beginNewTransaction ("Point Tool Color");
        perform (new UndoableSetPointToolColor (this, color));
    }
}

void FrameEditor::togglePointToolBlank()
{
    if (pointToolColor == Colours::black)
        setPointToolColor (lastVisiblePointToolColor);
    else
        setPointToolColor (Colours::black);
}

void FrameEditor::cyclePointToolColors()
{
    if (pointToolColor == Colours::white)
        setPointToolColor (Colours::red);
    else if (pointToolColor == Colours::red)
        setPointToolColor (Colour (0, 255, 0));
    else if (pointToolColor == Colour (0, 255, 0))
        setPointToolColor (Colours::blue);
    else if (pointToolColor == Colours::blue)
        setPointToolColor (Colours::yellow);
    else if (pointToolColor == Colours::yellow)
        setPointToolColor (Colours::cyan);
    else if (pointToolColor == Colours::cyan)
        setPointToolColor (Colours::magenta);
    else if (pointToolColor == Colours::magenta)
        setPointToolColor (Colours::black);
    else
        setPointToolColor (Colours::white);
}

void FrameEditor::cut()
{
    if (! canCopy())
        return;
    
    copy();
    deletePaths();
}

void FrameEditor::paste()
{
    if (! canPaste())
        return;
    
    beginNewTransaction ("Paste");
    int rangeStart = getIPathCount();
    int rangeEnd = rangeStart + iPathCopy.size();
    perform (new UndoableAddPaths (this, iPathCopy));
    IPathSelection selection;
    selection.addRange (Range<uint16> ((uint16)rangeStart, (uint16)rangeEnd));
    perform (new UndoableSetIPathSelection (this, selection));
}

void FrameEditor::setActiveSketchTool (SketchTool tool)
{
    if (activeSketchTool != tool)
    {
        beginNewTransaction ("Tool Change");
        perform (new UndoableSetSketchTool (this, tool));
    }
}

void FrameEditor::setSketchToolColor (const Colour& color)
{
    if (sketchToolColor != color)
    {
        beginNewTransaction ("Sketch Tool Color");
        perform (new UndoableSetSketchToolColor (this, color));
    }
}

void FrameEditor::toggleSketchToolBlank()
{
    if (sketchToolColor == Colours::black)
        setSketchToolColor (lastVisibleSketchToolColor);
    else
        setSketchToolColor (Colours::black);
}

void FrameEditor::cycleSketchToolColors()
{
    if (sketchToolColor == Colours::white)
        setSketchToolColor (Colours::red);
    else if (sketchToolColor == Colours::red)
        setSketchToolColor (Colour (0, 255, 0));
    else if (sketchToolColor == Colour (0, 255, 0))
        setSketchToolColor (Colours::blue);
    else if (sketchToolColor == Colours::blue)
        setSketchToolColor (Colours::yellow);
    else if (sketchToolColor == Colours::yellow)
        setSketchToolColor (Colours::cyan);
    else if (sketchToolColor == Colours::cyan)
        setSketchToolColor (Colours::magenta);
    else if (sketchToolColor == Colours::magenta)
        setSketchToolColor (Colours::black);
    else
        setSketchToolColor (Colours::white);
}

void FrameEditor::setSketchVisible (bool visible)
{
    if (visible != sketchVisible)
    {
        beginNewTransaction ("Sketch Visible Change");
        perform (new UndoableSetIPathSelection (this, IPathSelection()));
        perform (new UndoableSetSketchVisibility (this, visible));
    }
}

void FrameEditor::setIldaVisible (bool visible)
{
    if (visible != ildaVisible)
    {
        beginNewTransaction ("Ilda Visible Change");
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform (new UndoableSetIldaVisibility (this, visible));
    }
}

void FrameEditor::setRefVisible (bool visible)
{
    if (visible != refVisible)
    {
        beginNewTransaction ("Background Visible Change");
        perform(new UndoableSetRefVisibility (this, visible));
    }
}

void FrameEditor::selectAll()
{
    if (getActiveLayer() == ilda)
    {
        if (getPointCount())
        {
            SparseSet<uint16> s;

            // Grab all?
            if (ildaShowBlanked)
                s.addRange (Range<uint16> (0, getPointCount()));
            else
            {
                // Walk all the points and find ranges of visible
                for (uint16 n = 0; n < getPointCount(); ++n)
                {
                    Frame::IPoint point;
                    currentFrame->getPoint (n, point);
                    if (! (point.status & Frame::BlankedPoint))
                    {
                        uint16 start = n;
                        
                        for (; n < getPointCount(); ++n)
                        {
                            currentFrame->getPoint (n, point);
                            if (point.status & Frame::BlankedPoint)
                                break;
                        }
                        
                        s.addRange (Range<uint16> (start, n));
                    }
                }
            }
            
            setIldaSelection (s);
        }
    }
    else if (getActiveLayer() == sketch)
    {
        if (getIPathCount())
        {
            IPathSelection s;
            s.addRange (Range<uint16> (0, (uint16)getIPathCount()));
            setIPathSelection (s);
        }
    }
}

void FrameEditor::clearSelection()
{
    if (getActiveLayer() == ilda)
        setIldaSelection (SparseSet<uint16>());
    else if (getActiveLayer() == sketch)
        setIPathSelection (IPathSelection());
}

void FrameEditor::selectImage()
{
    FileChooser myChooser ("Choose Image to Load...",
                           File::getSpecialLocation (File::userDocumentsDirectory),
                           "*.png;*.jpg;*.jpeg;*.gif");
 
    if (myChooser.browseForFileToOpen())
    {
        File f = myChooser.getResult();
        
        Image i = ImageFileFormat::loadFrom (f);
        if (i.isNull())
        {
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                        "An error occurred loading the selected image file.", "ok");
        }
        else
        {
            
            beginNewTransaction ("Background Image Change");
            MemoryBlock b;
            f.loadFileAsData (b);
            perform(new UndoableSetImage (this, b));
        }
    }
}

void FrameEditor::clearImage()
{
    if (currentFrame->getBackgroundImage() != nullptr)
    {
        beginNewTransaction ("Clear Background Change");
        perform(new UndoableSetImage (this, MemoryBlock()));
    }
}

void FrameEditor::setImageOpacity (float opacity)
{
    if (opacity < 0)
        opacity = 0;
    else if (opacity > 1.0)
        opacity = 1.0;
    
    if (opacity != refOpacity)
    {
        if (getCurrentTransactionName() == "Background Opacity")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Opacity");
        perform(new UndoableSetRefAlpha (this, opacity));
    }
}

void FrameEditor::setImageScale (float scale)
{
    if (scale < 0.01f)
        scale = 0.01f;
    else if (scale > 2.00f)
        scale = 2.00f;
    
    if (scale != currentFrame->getImageScale())
    {
        if (getCurrentTransactionName() == "Background Scale")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Scale");
        perform(new UndoableSetImageScale (this, scale));
    }
}

void FrameEditor::setImageRotation (float rot)
{
    if (rot < 0.0f)
        rot = 0.0f;
    else if (rot > 359.9f)
        rot = 359.9f;
    
    if (rot != currentFrame->getImageRotation())
    {
        if (getCurrentTransactionName() == "Background Rotation")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Rotation");
        perform(new UndoableSetImageRotation (this, rot));
    }
}

void FrameEditor::setImageXoffset (float off)
{
    if (off < -100)
        off = -100;
    else if (off > 100)
        off = 100;
    
    if (off != currentFrame->getImageXoffset())
    {
        if (getCurrentTransactionName() == "Background X-Offset")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background X-Offset");
        perform(new UndoableSetImageXoffset (this, off));
    }
}

void FrameEditor::setImageYoffset (float off)
{
    if (off < -100)
        off = -100;
    else if (off > 100)
        off = 100;
    
    if (off != currentFrame->getImageYoffset())
    {
        if (getCurrentTransactionName() == "Background Y-Offset")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Y-Offset");
        perform(new UndoableSetImageYoffset (this, off));
    }
}

void FrameEditor::loadFile (File& file)
{
    // Check Dirty!
    if (dirtyCounter)
        if (! AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Unsaved Changes!",
                                    "Are you sure you want to proceed without saving?", "proceed", "cancel"))
            return;

    ReferenceCountedArray<Frame> frames;
    bool b = false;
    
    if (file.getFileExtension().toLowerCase() == ".ild")
        b = IldaLoader::load (frames, file);
    else
        b = JSEFileLoader::load (frames, file);
    
    if (! b)
    {
        AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                    "An error occurred loading the selected file.", "ok");
    }
    else
    {
        beginNewTransaction ("Load File");
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform (new UndoableSetIPathSelection (this, IPathSelection()));
        perform(new UndoableLoadFile (this, frames, file));
    }
}

void FrameEditor::loadFile()
{
    // Check Dirty!
    if (dirtyCounter)
        if (! AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Unsaved Changes!",
                                    "Are you sure you want to proceed without saving?", "proceed", "cancel"))
            return;

    FileChooser myChooser ("File to Load...",
                          File::getSpecialLocation (File::userDocumentsDirectory),
                          "*.jse;*.ild");

    if (myChooser.browseForFileToOpen())
    {
        File f = myChooser.getResult();

        ReferenceCountedArray<Frame> frames;
        bool b = false;
        
        if (f.getFileExtension().toLowerCase() == ".ild")
            b = IldaLoader::load (frames, f);
        else
            b = JSEFileLoader::load (frames, f);
        
        if (! b)
        {
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                        "An error occurred loading the selected file.", "ok");
        }
        else
        {
            beginNewTransaction ("Load File");
            perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
            perform (new UndoableSetIPathSelection (this, IPathSelection()));
            perform(new UndoableLoadFile (this, frames, f));
        }
    }
}

void FrameEditor::newFile()
{
    // Check Dirty!
    if (dirtyCounter)
        if (! AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Unsaved Changes!",
                                    "Are you sure you want to proceed without saving?", "proceed", "cancel"))
            return;

    // Don't bother if we are already a new file
    if (getFrameCount() == 1 && (! loadedFile.getFileName().length() && (! dirtyCounter)))
        return;
    
    ReferenceCountedArray<Frame> frames;
    frames.add (new Frame());
    
    beginNewTransaction ("New File");
    perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
    perform (new UndoableSetIPathSelection (this, IPathSelection()));
    perform (new UndoableLoadFile (this, frames, File()));
}

void FrameEditor::setIldaShowBlanked (bool visible)
{
    if (visible != ildaShowBlanked)
    {
        beginNewTransaction ("Show Blanked Change");
        perform(new UndoableSetIldaShowBlanked (this, visible));
    }
}

void FrameEditor::setIldaDrawLines (bool visible)
{
    if (visible != ildaDrawLines)
    {
        beginNewTransaction ("Draw Lines Change");
        perform(new UndoableSetIldaDrawLines (this, visible));
    }
}

void FrameEditor::setDrawGrid (bool draw)
{
    if (getRefDrawGrid() != draw)
    {
        beginNewTransaction ("Grid Visible Change");
        perform(new UndoableSetRefDrawGrid (this, draw));
    }
}

void FrameEditor::setFrameIndex (uint16 index)
{
    if (getFrameIndex() != index)
    {
        currentFrame->buildThumbNail();
        
        beginNewTransaction ("Select Frame");
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform(new UndoableSetFrameIndex (this, index));
    }
}

void FrameEditor::deleteFrame ()
{
    uint16 index = getFrameIndex();
    
    if ((Frames.size() > 1) && (index < Frames.size()))
    {
        beginNewTransaction ("Delete Frame");
        if (index == (Frames.size() - 1))
            perform (new UndoableSetFrameIndex (this, index - 1));
        
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform (new UndoableDeleteFrame (this, index));
    }
}

void FrameEditor::newFrame()
{
    beginNewTransaction ("New Frame");
    uint16 sel = getFrameIndex() + 1;
    perform (new UndoableNewFrame (this));
    perform (new UndoableSetFrameIndex (this, sel));
}

void FrameEditor::dupFrame()
{
    // Update the thumbnail before duplicating
    currentFrame->buildThumbNail();

    beginNewTransaction ("Duplicate Frame");
    uint16 sel = getFrameIndex() + 1;
    perform (new UndoableDupFrame (this));
    perform (new UndoableSetFrameIndex (this, sel));
}

void FrameEditor::moveFrameUp()
{
    if (getFrameIndex())
    {
        // Update the thumb just in case there are edits
        currentFrame->buildThumbNail();
        
        beginNewTransaction ("Move Frame Up");
        uint16 sel = getFrameIndex();
        perform (new UndoableSwapFrames (this, sel, sel - 1));
        perform (new UndoableSetFrameIndex (this, sel - 1));
    }
}

void FrameEditor::moveFrameDown()
{
    if (getFrameIndex() < (Frames.size() - 1))
    {
        // Update the thumb just in case there are edits
        currentFrame->buildThumbNail();

        beginNewTransaction ("Move Frame Down");
        uint16 sel = getFrameIndex();
        perform (new UndoableSwapFrames (this, sel, sel + 1));
        perform (new UndoableSetFrameIndex (this, sel + 1));
    }
}

void FrameEditor::insertPoint (const Frame::IPoint& point)
{
    uint16 index;
    
    if (! getPointCount())
        index = 0;
    else if (ildaSelection.isEmpty())
        return;
    else
    {
        Range<uint16> r = ildaSelection.getRange (ildaSelection.getNumRanges() - 1);
        index = r.getEnd();
    }
    
    beginNewTransaction ("Insert Point");
    perform (new UndoableInsertPoint (this, index, point));
    SparseSet<uint16> selection;
    selection.addRange (Range<uint16> (index, index+1));
    perform (new UndoableSetIldaSelection (this, selection));
}

void FrameEditor::deletePoints()
{
    if (ildaSelection.isEmpty())
        return;
    
    beginNewTransaction ("Delete Point(s)");
    SparseSet<uint16> selection = ildaSelection;
    perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
    perform (new UndoableDeletePoints (this, selection));
}

void FrameEditor::setIldaSelection (const SparseSet<uint16>& selection)
{
    if (selection.getTotalRange().getEnd() > getPointCount())
        return;
    
    if ((! getIldaVisible()) && (! selection.isEmpty()))
        return;
    
    if (selection != ildaSelection)
    {
        beginNewTransaction ("Selection Change");
        perform (new UndoableSetIldaSelection (this, selection));
    }
}

void FrameEditor::adjustIldaSelection (int offset)
{
    if (activeLayer != ilda || ildaSelection.isEmpty())
        return;
    
    SparseSet<uint16> newSelection;
    int points = getPointCount();
    
    if (offset == 0 || offset >= points)
        return;

    // Valid range
    Range<int> valid (0, points);

    for (auto n = 0; n < getIldaSelection().getNumRanges(); ++n)
    {
        Range<uint16> r = getIldaSelection().getRange (n);
        Range<int> shifted ((int)r.getStart() + offset, (int)r.getEnd() + offset);
        
        // Add the valid part of the shifted range
        Range<int> newRange = valid.getIntersectionWith (shifted);
        newSelection.addRange (Range<uint16> ((uint16)newRange.getStart(),
                                              (uint16)newRange.getEnd()));
        
        // Deal with any wrap off either end with an additional range
        int extra = (int)r.getLength() - newRange.getLength();
        if (extra)
        {
            int s, e;
            
            if (offset > 0)
                s = shifted.getEnd() - extra - points;
            else
                s = shifted.getStart() + points;

            e = s + extra;
            newSelection.addRange (Range<uint16> ((uint16)s, (uint16)e));
        }
    }
    
    setIldaSelection (newSelection);
}


bool FrameEditor::moveIldaSelected (int xOffset, int yOffset, int zOffset, bool constrain)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return false;
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        int x = point.x.w;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;
        
        int y = point.y.w;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
        
        int z = point.z.w;
        z += zOffset;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    if (getCurrentTransactionName() == "Point Move")
        undoCurrentTransactionOnly();

    beginNewTransaction ("Point Move");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
    return true;
}

bool FrameEditor::centerIldaSelected (bool doX, bool doY, bool doZ, bool constrain)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return false;
    
    int16 cx, cy, cz;
    getCenterOfIldaSelection (cx, cy, cz);
    int xOffset = 0 - cx;
    int yOffset = 0 - cy;
    int zOffset = 0 - cz;
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);
        
        int x = point.x.w;
        if (doX)
        {
            x += xOffset;
            if (Frame::clipIlda (x))
            {
                Frame::blankPoint (point);
                clipped = true;
            }
        }
        point.x.w = (int16)x;
        
        int y = point.y.w;
        if (doY)
        {
            y += yOffset;
            if (Frame::clipIlda (y))
            {
                Frame::blankPoint (point);
                clipped = true;
            }
        }
        point.y.w = (int16)y;

        int z = point.z.w;
        if (doZ)
        {
            z += zOffset;
            if (Frame::clipIlda (z))
            {
                Frame::blankPoint (point);
                clipped = true;
            }
        }
        point.z.w = (int16)z;

    }
    
    if (constrain && clipped)
        return false;

    beginNewTransaction ("Center Point(s)");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
    
    refreshThumb();
    
    return true;
}

void FrameEditor::duplicateIldaSelected()
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    int pIndex = points.size() - 1;
    
    beginNewTransaction ("Duplicate Point(s)");
    
    // Loop backwards through selection to insert points
    for (auto n = ildaSelection.getNumRanges() - 1; n >= 0; --n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getEnd() - 1; i >= r.getStart(); --i)
            perform (new UndoableInsertPoint (this, (uint16)i + 1, points[pIndex--]));
    }
    
    // Loop forwards to build new selection
    int pOffset = 1;
    SparseSet<uint16> newSelection;
    
    // Loop forwards to insert
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getStart(); i < r.getEnd(); ++i)
        {
            int index = i + pOffset;
            newSelection.addRange (Range<uint16>((uint16)index, (uint16)index + 1));
            pOffset++;
        }
    }

    perform (new UndoableSetIldaSelection (this, newSelection));
}

void FrameEditor::anchorIldaSelected()
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    int pIndex = points.size() - 1;
    
    Frame::IPoint point;
    zerostruct (point);
    point.status = Frame::BlankedPoint;
    
    beginNewTransaction ("Anchor Point(s)");
    
    // Loop backwards through selection to insert points
    for (auto n = ildaSelection.getNumRanges() - 1; n >= 0; --n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getEnd() - 1; i >= r.getStart(); --i)
        {
            point.x.w = points[pIndex].x.w;
            point.y.w = points[pIndex].y.w;
            point.z.w = points[pIndex].z.w;
            perform (new UndoableInsertPoint (this, (uint16)i, point));
            pIndex--;
        }
    }
    
    // Loop forwards to build new selection
    int pOffset = 0;
    SparseSet<uint16> newSelection;
    
    // Loop forwards to insert
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getStart(); i < r.getEnd(); ++i)
        {
            int index = i + pOffset;
            newSelection.addRange (Range<uint16>((uint16)index, (uint16)index + 1));
            pOffset++;
        }
    }

    perform (new UndoableSetIldaSelection (this, newSelection));
}

void FrameEditor::anchorAfterIldaSelected()
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    int pIndex = points.size() - 1;
    
    Frame::IPoint point;
    zerostruct (point);
    point.status = Frame::BlankedPoint;
    
    beginNewTransaction ("Anchor Point(s)");
    
    // Loop backwards through selection to insert points
    for (auto n = ildaSelection.getNumRanges() - 1; n >= 0; --n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getEnd() - 1; i >= r.getStart(); --i)
        {
            point.x.w = points[pIndex].x.w;
            point.y.w = points[pIndex].y.w;
            point.z.w = points[pIndex].z.w;
            perform (new UndoableInsertPoint (this, (uint16)i + 1, point));
            pIndex--;
        }
    }
    
    // Loop forwards to build new selection
    int pOffset = 1;
    SparseSet<uint16> newSelection;
    
    // Loop forwards to insert
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i = r.getStart(); i < r.getEnd(); ++i)
        {
            int index = i + pOffset;
            newSelection.addRange (Range<uint16>((uint16)index, (uint16)index + 1));
            pOffset++;
        }
    }

    perform (new UndoableSetIldaSelection (this, newSelection));
}

void FrameEditor::deletePaths()
{
    if (iPathSelection.isEmpty())
        return;
    
    beginNewTransaction ("Delete Path(s)");
    IPathSelection selection = iPathSelection;
    perform (new UndoableSetIPathSelection (this, IPathSelection()));
    perform (new UndoableDeletePaths (this, selection));
}

void FrameEditor::deleteAnchor()
{
    if (iPathSelection.isEmpty() || (iPathSelection.getAnchor() == -1))
        return;
    
    // If it is the last anchor, delete the path instead
    uint16 i = iPathSelection.getRange (0).getStart();
    if (currentFrame->getIPath (i).getAnchorCount() <= 1)
    {
        deletePaths();
        return;
    }
        
    beginNewTransaction ("Delete Anchor");
    IPathSelection selection = iPathSelection;
    perform (new UndoableSetIPathSelection (this, IPathSelection()));
    perform (new UndoableDeleteAnchor (this, selection));
}

int FrameEditor::insertRectPath (const Rectangle<int>& rect)
{
    IPath path;
    path.setColor (sketchToolColor);
    Anchor a;
    
    a.setPosition (rect.getX(), rect.getY());
    path.addAnchor (a);
    a.setPosition (rect.getRight(), rect.getY());
    path.addAnchor (a);
    a.setPosition (rect.getRight(), rect.getBottom());
    path.addAnchor (a);
    a.setPosition (rect.getX(), rect.getBottom());
    path.addAnchor (a);
    a.setPosition (rect.getX(), rect.getY());
    path.addAnchor (a);

    beginNewTransaction ("New Rectangle");
    Array<IPath> array;
    array.add (path);
    IPathSelection selection;
    int index = getIPathCount();
    selection.addRange (Range<uint16> ((uint16)index, (uint16)index + 1));
    perform (new UndoableAddPaths (this, array));
    perform (new UndoableSetIPathSelection (this, selection));
    
    return index;
}

int FrameEditor::insertEllipsePath (const Rectangle<int>& rect)
{
    const double kappa = .5522848;
    double ox = (double)rect.getWidth() / 2.0 * kappa;
    double oy = (double)rect.getHeight() / 2.0 * kappa;
    
    IPath path;
    path.setColor (sketchToolColor);
    Anchor a;
    
    a.setPosition (rect.getX(), rect.getCentreY());
    a.setEntryPosition (rect.getX(), rect.getCentreY() + (int)oy);
    a.setExitPosition (rect.getX(), rect.getCentreY() - (int)oy);
    path.addAnchor (a);
    
    a.setPosition (rect.getCentreX(), rect.getY());
    a.setEntryPosition (rect.getCentreX() - (int)ox, rect.getY());
    a.setExitPosition (rect.getCentreX() + (int)ox, rect.getY());
    path.addAnchor (a);
    
    a.setPosition(rect.getX() + rect.getWidth(), rect.getCentreY());
    a.setEntryPosition (rect.getX() + rect.getWidth(), rect.getCentreY() - (int)oy);
    a.setExitPosition (rect.getX() + rect.getWidth(), rect.getCentreY() + (int)oy);
    path.addAnchor (a);
    
    a.setPosition (rect.getCentreX(), rect.getY() + rect.getHeight());
    a.setEntryPosition (rect.getCentreX() + (int)ox, rect.getY() + rect.getHeight());
    a.setExitPosition (rect.getCentreX() - (int)ox, rect.getY() + rect.getHeight());
    path.addAnchor (a);

    a.setPosition (rect.getX(), rect.getCentreY());
    a.setEntryPosition (rect.getX(), rect.getCentreY() + (int)oy);
    a.setExitPosition (rect.getX(), rect.getCentreY() - (int)oy);
    path.addAnchor (a);

    beginNewTransaction ("New Ellipse");
    Array<IPath> array;
    array.add (path);
    IPathSelection selection;
    int index = getIPathCount();
    selection.addRange (Range<uint16> ((uint16)index, (uint16)index + 1));
    perform (new UndoableAddPaths (this, array));
    perform (new UndoableSetIPathSelection (this, selection));
    
    return index;
}

int FrameEditor::insertPath (Point<int> firstAnchor)
{
    IPath path;
    path.setColor (sketchToolColor);
    path.addAnchor (Anchor (firstAnchor.getX(), firstAnchor.getY()));

    beginNewTransaction ("New Shape");
    Array<IPath> array;
    array.add (path);
    IPathSelection selection;
    int index = getIPathCount();
    selection.addRange (Range<uint16> ((uint16)index, (uint16)index + 1));
    selection.setAnchor (0);
    perform (new UndoableAddPaths (this, array));
    perform (new UndoableSetIPathSelection (this, selection));
    
    return index;
}

int FrameEditor::insertAnchor (Point<int> location)
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return -1;
    
    int pIndex = iPathSelection.getRange(0).getStart();
    IPath path = currentFrame->getIPath (pIndex);
    path.insertAnchor (aIndex + 1, Anchor (location.getX(), location.getY()));
    
    IPathSelection selection;
    selection.addRange (Range<uint16> ((uint16)pIndex, (uint16)pIndex + 1));
    
    beginNewTransaction ("New Anchor");
    Array<IPath> paths;
    paths.add (path);
    perform (new UndoableSetPaths (this, selection, paths));

    selection.setAnchor (aIndex + 1);
    perform (new UndoableSetIPathSelection (this, selection));
    
    return aIndex + 1;
}

void FrameEditor::zeroExitControl()
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;

    IPath* path = &paths.getReference (0);
    Anchor a = path->getAnchor (aIndex);

    a.setExitXDelta(0);
    a.setExitYDelta(0);
    path->setAnchor (aIndex, a);

    beginNewTransaction ("Zero Exit Control");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::insertControls (Point<int> location)
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;

    IPath* path = &paths.getReference (0);
    Anchor a = path->getAnchor (aIndex);
    
    int x, y;
    a.getPosition (x, y);
    Point<int> position (x, y);
    
    float angle = location.getAngleToPoint (position);
    int distance = location.getDistanceFrom (position);
    
    if (distance == 0)
        return;
    
    Line<float> l = Line<float>::fromStartAndAngle (position.toFloat(), abs((float)distance), angle);
    Point<float> entry = l.getEnd();
    
    a.setEntryPosition (entry.toInt().getX(), entry.toInt().getY());
    a.setExitPosition (location.getX(), location.getY());
    path->setAnchor (aIndex, a);
    
    if (getCurrentTransactionName() == "Insert Controls")
        undoCurrentTransactionOnly();

    beginNewTransaction ("Insert Controls");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::selectEntry()
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    IPathSelection selection = iPathSelection;
    selection.setControl (1);
    
    beginNewTransaction ("Select Anchor Entry");
    perform (new UndoableSetIPathSelection (this, selection));
}

void FrameEditor::selectExit()
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    IPathSelection selection = iPathSelection;
    selection.setControl (2);
    
    beginNewTransaction ("Select Anchor Exit");
    perform (new UndoableSetIPathSelection (this, selection));
}

void FrameEditor::forceAnchorCurved()
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;

    IPath* path = &paths.getReference (0);
    Anchor a = path->getAnchor (aIndex);
    
    int x, y;
    a.getPosition (x, y);
    x -= 1000;
    y += 500;
    a.setEntryPosition (x, y);
    x += 2000;
    y -= 1000;
    a.setExitPosition (x, y);
    path->setAnchor (aIndex, a);
    
    beginNewTransaction ("Curve Anchor");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::forceAnchorStraight()
{
    int aIndex = iPathSelection.getAnchor();
    if (aIndex == -1)
        return;

    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;

    IPath* path = &paths.getReference (0);
    Anchor a = path->getAnchor (aIndex);

    a.setEntryXDelta (0);
    a.setEntryYDelta (0);
    a.setExitXDelta (0);
    a.setExitYDelta (0);
    path->setAnchor (aIndex, a);
    
    beginNewTransaction ("Straighten Anchor");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

bool FrameEditor::isClosedIPath (const IPath &path)
{
    if (path.getAnchorCount() < 2)
        return false;
    
    if (path.getStartZ() != path.getEndZ())
        return false;
    
    int end = path.getAnchorCount() - 1;
    Anchor last = path.getAnchor (end);
    for (auto n = 0; n < end; ++n)
    {
        Anchor a = path.getAnchor (n);
        if (a.getX() == last.getX() && a.getY() == last.getY())
            return true;
    }
    
    return false;
}

void FrameEditor::pointToIPointXYZ (Point<int> a, Frame::IPoint& point, int zStart, int zEnd, float zPercent)
{
    int zDelta = zEnd - zStart;
    int z = zStart + (int)((float)zDelta * zPercent);
    
    if (a.getX() < 0)
        a.setX (0);
    else if (a.getX() > 65535)
        a.setX (65535);
    if (a.getY() < 0)
        a.setY(0);
    else if (a.getY() > 65535)
        a.setY (65535);

    point.x.w = activeView == Frame::left ? (int16)z : (int16)Frame::toIldaX (a.getX());
    point.y.w = activeView == Frame::bottom ? (int16)z : (int16)Frame::toIldaY (a.getY());
    if (activeView == Frame::front)
        point.z.w = (int16)z;
    else if (activeView == Frame::bottom)
        point.z.w = (int16)Frame::toIldaY (a.getY());
    else if (activeView == Frame::left)
        point.z.w = (int16)Frame::toIldaX (a.getX());
}

void FrameEditor::anchorToPointXYZ (const Anchor& a, Frame::IPoint& point, int zStart, int zEnd, float zPercent)
{
    pointToIPointXYZ (Point<int>(a.getX(), a.getY()), point, zStart, zEnd, zPercent);
}

void FrameEditor::generatePointsFromPaths (const Array<IPath>& paths, Array<Frame::IPoint>& points, bool appendPoints)
{
    if (! appendPoints)
        points.clear();
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        Anchor lastAnchor;
        Frame::IPoint point;
        zerostruct (point);
        
        bool closed = isClosedIPath (paths[n]);
        int endA = paths[n].getAnchorCount() - 1;
        
        // Total length of path, zero rendered so far
        float totalLength = paths[n].getPath().getLength();
        float rendered = 0.0f;
        
        // Z range
        int startZ = paths[n].getStartZ();
        int endZ = paths[n].getEndZ();
        
        for (auto i = 0; i <= endA; ++i)
        {
            Colour c = paths[n].getColor();
            
            Anchor newAnchor = paths[n].getAnchor (i);

            if (i == 0) // First anchor
            {
                anchorToPointXYZ (newAnchor, point, startZ, endZ, 0.0f);
                point.red = point.green = point.blue = 0;
                point.status = Frame::BlankedPoint;
                
                for (auto bb = 0; bb < paths[n].getBlankedPointsBeforeStart(); ++bb)
                    points.add (point);
                
                point.red = c.getRed();
                point.green = c.getGreen();
                point.blue = c.getBlue();
                point.status = c == Colours::black ? Frame::BlankedPoint : 0;
                
                for (auto es = 0; es < paths[n].getExtraPointsAtStart(); ++es)
                    points.add (point);
                
                points.add (point);
                for (auto ea = 0; ea < paths[n].getExtraPointsPerAnchor(); ++ea)
                    points.add (point);
            }
            else
            {
                // Put points (if needed) from the last anchor to this one
                Path path;
                path.startNewSubPath ((float)lastAnchor.getX(), (float)lastAnchor.getY());

                int exitX, exitY;
                int entryX, entryY;

                lastAnchor.getExitPosition (exitX, exitY);
                newAnchor.getEntryPosition (entryX, entryY);
                        
                path.cubicTo((float)exitX, (float)exitY, (float)entryX, (float)entryY, (float)newAnchor.getX(), (float)newAnchor.getY());
                
                float plength = path.getLength();
                int density = paths[n].getPointDensity();

                point.red = c.getRed();
                point.green = c.getGreen();
                point.blue = c.getBlue();
                point.status = c == Colours::black ? Frame::BlankedPoint : 0;

                // At least one point required?
                if (plength > (float)density)
                {
                    int len = (int)plength;
                    int pcount = len / density;
                    pcount--;   // Don't duplicate anchor
                    int offset = (len % density) / 2;
                    if (offset)
                    {
                        pcount++;
                        offset = (density / 2) - offset;
                    }
                    
                    for (auto a = 1; a <= pcount; a++)
                    {
                        int distance = a * density - offset;
                        Point<float>p = path.getPointAlongPath ((float)distance);
                        pointToIPointXYZ (p.toInt(), point, startZ, endZ,
                                          (distance + rendered) / totalLength);
                        points.add (point);
                    }
                }
                
                rendered += plength;
                
                // Convert coordinates
                anchorToPointXYZ (newAnchor, point, startZ, endZ, rendered / totalLength);
                points.add (point);
                if (i != endA || (! closed))
                {
                    for (auto ea = 0; ea < paths[n].getExtraPointsPerAnchor(); ++ea)
                        points.add (point);
                }
            }
            
            lastAnchor = newAnchor;
        }
        
        anchorToPointXYZ (lastAnchor, point, startZ, endZ, 1.0f);
        for (auto ee = 0; ee < paths[n].getExtraPointsAtEnd(); ++ee)
            points.add (point);
        
        point.red = point.green = point.blue = 0;
        point.status = Frame::BlankedPoint;
        
        for (auto ab = 0; ab < paths[n].getBlankedPointsAfterEnd(); ++ab)
            points.add (point);
    }
}

void FrameEditor::connectIPaths (const Array<IPath>& src, Array<IPath>& dst)
{
    dst.clear();
    if (! src.size())
        return;
    
    IPath lastPath;
    IPath newPath;
    
    // Build connecting blanked paths
    for (auto n = 0; n < src.size(); ++n)
    {
        newPath = src[n];
        if (n != 0)
        {
            Anchor lastA = lastPath.getAnchor (lastPath.getAnchorCount() -1);
            Anchor newA = newPath.getAnchor (0);
            
            if (lastA.getX() != newA.getX() || lastA.getY() != newA.getY())
            {
                IPath blankPath;
                blankPath.setColor (Colours::black);
                blankPath.setPointDensity (blankPointSpacing);
                blankPath.setBlankedPointsBeforeStart (0);
                blankPath.setBlankedPointsAfterEnd (0);
                blankPath.addAnchor (Anchor (lastA.getX(), lastA.getY()));
                blankPath.addAnchor (Anchor (newA.getX(), newA.getY()));
                blankPath.setStartZ (lastPath.getEndZ());
                blankPath.setEndZ (newPath.getStartZ());
                dst.add (blankPath);
            }
            else if (lastPath.getEndZ() != newPath.getStartZ())
            {
                // Handle overlapped points with diferent Z's differently
                // Just insert anchors based on the Z gap
                // But we have to zig zag them so that we have a distance to spread
                // out Z on
                IPath blankPath;
                blankPath.setColor (Colours::black);
                blankPath.setPointDensity (blankPointSpacing);
                blankPath.setBlankedPointsBeforeStart (0);
                blankPath.setBlankedPointsAfterEnd (0);
                blankPath.addAnchor (Anchor (lastA.getX(), lastA.getY()));
                int zcnt = abs (lastPath.getEndZ() - newPath.getStartZ()) / blankPointSpacing;
                for (auto i = 0; i < zcnt; ++i)
                {
                    if (n & i)
                        blankPath.addAnchor (Anchor (lastA.getX() -2, lastA.getY() -2));
                    else
                        blankPath.addAnchor (Anchor (lastA.getX() +2, lastA.getY() +2));
                }
                blankPath.addAnchor (Anchor (newA.getX(), newA.getY()));
                blankPath.setStartZ (lastPath.getEndZ());
                blankPath.setEndZ (newPath.getStartZ());
                dst.add (blankPath);
            }
        }
        dst.add (newPath);
        lastPath = newPath;
    }
    
    Anchor startA = dst[0].getAnchor (0);
    Anchor endA = dst[dst.size() - 1].getAnchor (dst[dst.size() - 1].getAnchorCount() - 1);
    if (startA.getX() != endA.getX() || startA.getY() != endA.getY())
    {
        IPath returnPath;
        returnPath.setColor (Colours::black);
        returnPath.setPointDensity (blankPointSpacing);
        returnPath.setBlankedPointsBeforeStart (0);
        returnPath.setBlankedPointsAfterEnd (0);
        returnPath.addAnchor (Anchor (endA.getX(), endA.getY()));
        returnPath.addAnchor (Anchor (startA.getX(), startA.getY()));
        returnPath.setStartZ (dst[dst.size() - 1].getEndZ());
        returnPath.setEndZ (dst[0].getStartZ());
        dst.add (returnPath);
    }
    else if (dst[0].getStartZ() != dst[dst.size() -1].getEndZ())
    {
        // Handle overlapped points with diferent Z's differently
        // Just insert anchors based on the Z gap
        // But we have to zig zag them so that we have a distance to spread
        // out Z on
        IPath returnPath;
        returnPath.setColor (Colours::black);
        returnPath.setPointDensity (blankPointSpacing);
        returnPath.setBlankedPointsBeforeStart (0);
        returnPath.setBlankedPointsAfterEnd (0);
        returnPath.addAnchor (Anchor (endA.getX(), endA.getY()));
        int zcnt = abs (dst[dst.size() - 1].getEndZ() - dst[0].getStartZ()) / blankPointSpacing;
        for (auto n = 0; n < zcnt; ++n)
        {
            if (n & 1)
                returnPath.addAnchor (Anchor (endA.getX() -2, endA.getY() -2));
            else
                returnPath.addAnchor (Anchor (endA.getX() +2, endA.getY() +2));
        }
        returnPath.addAnchor (Anchor (startA.getX(), startA.getY()));
        returnPath.setStartZ (dst[dst.size() - 1].getEndZ());
        returnPath.setEndZ (dst[0].getStartZ());
        dst.add (returnPath);
    }
}

void FrameEditor::renderSketch (bool shortestPath, bool updateSketch)
{
    if (! currentFrame->getIPathCount())
        return;
    
    Array<Frame::IPoint> points;
    Array<IPath> sorted;
    Array<IPath> paths;

    if (shortestPath)
        ShortestPath::find (currentFrame->getIPaths(), sorted);
    else
        sorted = currentFrame->getIPaths();
    
    connectIPaths (sorted, paths);
    generatePointsFromPaths (paths, points);
    
    beginNewTransaction ("Render Sketch Layer");
    if (updateSketch)
    {
        perform (new UndoableSetIPathSelection (this, IPathSelection()));
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform (new UndoableChangesPointsAndPaths (this, points, paths));
    }
    else
    {
        perform (new UndoableSetIldaSelection (this, SparseSet<uint16>()));
        perform (new UndoableChangePoints (this, points));
    }

    refreshThumb();
}

void FrameEditor::adjustIPathSelection (int offset)
{
    if (activeLayer != sketch)
        return;
    if (iPathSelection.isEmpty())
        return;
    if (iPathSelection.getAnchor() == -1)
        return;
    
    int i = iPathSelection.getAnchor() + offset;
    int cnt = currentFrame->getIPath (iPathSelection.getRange (0).getStart()).getAnchorCount();

    if (i < 0)
        i += cnt;
    else if (i >= cnt)
        i -= cnt;
    
    IPathSelection selection (iPathSelection);
    selection.setAnchor (i);
    perform (new UndoableSetIPathSelection (this, selection));
}

void FrameEditor::setIPathSelection (const IPathSelection& selection)
{
    if (selection.getTotalRange().getEnd() > getIPathCount())
        return;
    
    if ((! getSketchVisible()) && (! selection.isEmpty()))
        return;
    
    if (selection != iPathSelection)
    {
        beginNewTransaction ("Selection Change");
        perform (new UndoableSetIPathSelection (this, selection));
    }
}

void FrameEditor::getIPaths (const IPathSelection& selection,
                             Array<IPath>& paths)
{
    paths.clear();
    
    for (auto n = 0; n < selection.getNumRanges(); ++n)
    {
        Range<uint16> r = selection.getRange (n);
        for (auto i = r.getStart(); i < r.getEnd(); ++i)
            paths.add (getIPath (i));
    }
}

void FrameEditor::setSketchSelectedSpacing (uint16 newSpacing)
{
    bool changed = false;
    
    if (! newSpacing)
        return;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getPointDensity() != newSpacing)
        {
            p->setPointDensity (newSpacing);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Point Spacing Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedExtraPerAnchor (uint16 extra)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getExtraPointsPerAnchor() != extra)
        {
            p->setExtraPointsPerAnchor(extra);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Extra Per Anchor Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedExtraBefore (uint16 extra)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getExtraPointsAtStart() != extra)
        {
            p->setExtraPointsAtStart (extra);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Extra At Start Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedExtraAfter (uint16 extra)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getExtraPointsAtEnd() != extra)
        {
            p->setExtraPointsAtEnd(extra);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Extra At End Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedBlankingBefore (uint16 points)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getBlankedPointsBeforeStart() != points)
        {
            p->setBlankedPointsBeforeStart (points);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Blanked Before Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedBlankingAfter (uint16 points)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getBlankedPointsAfterEnd() != points)
        {
            p->setBlankedPointsAfterEnd (points);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Blanked After Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedStartZ (int z)
{
    bool changed = false;

    if (z > 32767)
        z = 32767;
    else if (z < -32768)
        z = -32768;

    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getStartZ() != z)
        {
            p->setStartZ (z);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Start Depth Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedEndZ (int z)
{
    bool changed = false;
    
    if (z > 32767)
        z = 32767;
    else if (z < -32768)
        z = -32768;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getEndZ() != z)
        {
            p->setEndZ (z);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("End Depth Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::setSketchSelectedColor (const Colour& color)
{
    bool changed = false;
    
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        if (p->getColor() != color)
        {
            p->setColor (color);
            changed = true;
        }
    }
    
    if (! changed)
        return;

    beginNewTransaction ("Color Change");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
}

void FrameEditor::pathToPointsSketchSelected ()
{
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return;
    
    Array<IPath> newPaths;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath *p = &paths.getReference(n);
        int end = p->getAnchorCount();
        if (isClosedIPath (*p))
            end--;
        
        for (auto i = 0; i < end; ++i)
        {
            IPath newPath (*p);
            newPath.clearAllAnchors();
            newPath.addAnchor (p->getAnchor (i));
            newPaths.add (newPath);
        }
    }

    if (! newPaths.size())
        return;
    
    beginNewTransaction ("Convert to Anchor Points");
    IPathSelection selection = iPathSelection;
    perform (new UndoableSetIPathSelection (this, IPathSelection()));
    perform (new UndoableDeletePaths (this, selection));
    perform (new UndoableAddPaths (this, newPaths));
    IPathSelection newSelection;
    newSelection.addRange (Range<uint16> ((uint16)(currentFrame->getIPathCount() - newPaths.size()),
                                          (uint16)currentFrame->getIPathCount()));
    perform (new UndoableSetIPathSelection (this, newSelection));
}

bool FrameEditor::moveSketchSelected (int xOffset, int yOffset, bool constrain)
{
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return false;
    
    bool clipped = false;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            if (iPathSelection.getControl() == -1)
            {
                int x = a.getX();
                x += xOffset;
                if (Frame::clipSketch (x))
                    clipped = true;
                a.setX (x);
                
                int y = a.getY();
                y += yOffset;
                if (Frame::clipSketch (y))
                    clipped = true;
                a.setY (y);
            }
            else if (iPathSelection.getControl() == 1)
            {
                int x, y;
                a.getEntryPosition (x, y);
                x += xOffset;
                y += yOffset;
                a.setEntryPosition (x, y);
            }
            else if (iPathSelection.getControl() == 2)
            {
                int x, y;
                a.getExitPosition (x, y);
                x += xOffset;
                y += yOffset;
                a.setExitPosition (x, y);
            }

            p->setAnchor (i, a);
        }
    }
    
    if (constrain && clipped)
        return false;

    if (getCurrentTransactionName() == "Shape Move")
        undoCurrentTransactionOnly();

    beginNewTransaction ("Shape Move");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
    return true;
}

bool FrameEditor::centerSketchSelected (bool doX, bool doY, bool constrain)
{
    Array<IPath> paths;
    getSelectedIPaths (paths);
    if (! paths.size())
        return false;
    
    int ox, oy;
    getCenterOfIPathSelection (ox, oy);
    int xOffset = 32768 - ox;
    int yOffset = 32768 - oy;
    
    bool clipped = false;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            if (doX)
            {
                int x = a.getX();
                x += xOffset;
                if (Frame::clipSketch (x))
                    clipped = true;
                a.setX (x);
            }
            
            if (doY)
            {
                int y = a.getY();
                y += yOffset;
                if (Frame::clipSketch (y))
                    clipped = true;
                a.setY (y);
            }
            p->setAnchor (i, a);
        }
    }
    
    if (constrain && clipped)
        return false;

    beginNewTransaction ("Center Shape");
    perform (new UndoableSetPaths (this, iPathSelection, paths));
    return true;
}

//==========================================================================================
void FrameEditor::startTransform (const String& name)
{
    if (activeLayer == ilda)
    {
        getIldaSelectedPoints (transformPoints);
        getCenterOfIldaSelection (transformCenterX, transformCenterY, transformCenterZ);
    }
    else
    {
        getSelectedIPaths (transformPaths);
        getCenterOfIPathSelection (transformSketchCenterX, transformSketchCenterY);
    }
    
    transformName = name;
    transformUsed = false;
    tranformInProgress = true;
    sendActionMessage (EditorActions::transformStarted);
}

bool FrameEditor::scaleIldaSelected (float xScale,
                                     float yScale,
                                     float zScale,
                                     bool centerOnSelection,
                                     bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;
  
    int xOffset = 0;
    int yOffset = 0;
    int zOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
        zOffset = transformCenterZ;
    }
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        int x = point.x.w;
        x -= xOffset;
        x = (int)((float)x * xScale + 0.5f);
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;
        
        int y = point.y.w;
        y -= yOffset;
        y = (int)((float)y * yScale + 0.5f);
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
        
        int z = point.z.w;
        z -= zOffset;
        z = (int)((float)z * zScale + 0.5f);
        z += zOffset;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

static void Multiply3by3(double in1[3][3], double in2[3][3], double out[3][3])
{
    for (int col = 0 ; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            double d = 0;
            d += in1[row][0] * in2[0][col];
            d += in1[row][1] * in2[1][col];
            d += in1[row][2] * in2[2][col];
            out[row][col] = d;
        }
    }
}

bool FrameEditor::rotateIldaSelected (float xAngle,
                                      float yAngle,
                                      float zAngle,
                                      bool centerOnSelection,
                                      bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;
  
    int xOffset = 0;
    int yOffset = 0;
    int zOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
        zOffset = transformCenterZ;
    }
    
    bool clipped = false;
    
    // Build our rotation matrices
    double rx[3][3] = {{1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1}};
    double ry[3][3] = {{1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1}};
    double rz[3][3] = {{1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1}};

    double rotX = xAngle < 0 ? 360.0 + xAngle : xAngle;
    double rotY = yAngle < 0 ? 360.0 + yAngle : yAngle;
    double rotZ = zAngle < 0 ? 360.0 + zAngle : zAngle;
    
    // Clip X rotation
    if (rotX > 359.9)
        rotX = 0.0;

    // Get sin and cos
    const double pi = MathConstants<double>::pi;
    double sin = ::sin (rotX * pi / 180.0);
    double cos = ::cos (rotX * pi / 180.0);

    rx[1][1] = cos;
    rx[2][2] = cos;
    rx[1][2] = sin;
    rx[2][1] = 0 - sin;

    // Repeat for Y
    if (rotY > 359.9)
        rotY = 0.0;

    sin = ::sin (rotY * pi / 180.0);
    cos = ::cos (rotY * pi / 180.0);

    ry[0][0] = cos;
    ry[2][2] = cos;
    ry[2][0] = sin;
    ry[0][2] = 0 - sin;

    // And Z
    if (rotZ > 359.9)
        rotZ = 0.0;

    sin = ::sin (rotZ * pi / 180.0);
    cos = ::cos (rotZ * pi / 180.0);

    rz[0][0] = cos;
    rz[1][1] = cos;
    rz[0][1] = 0 - sin;
    rz[1][0] = sin;

    double out1[3][3];
    Multiply3by3(rx, ry, out1);
    double out2[3][3];
    Multiply3by3(out1, rz, out2);

    for (auto n = 0; n < points.size(); ++n)
    {
        double d;
        double dx, dy, dz;
        
        Frame::IPoint& point = points.getReference (n);

        dx = point.x.w - xOffset;
        dy = point.y.w - yOffset;
        dz = point.z.w - zOffset;
        
        d = dx * out2[0][0] + dy * out2[1][0] + dz * out2[2][0];
        int x = (int)d;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;
       
        d = dx * out2[0][1] + dy * out2[1][1] + dz * out2[2][1];
        int y = (int)d;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;

        d = dx * out2[0][2] + dy * out2[1][2] + dz * out2[2][2];
        int z = (int)d;
        z += zOffset;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::shearIldaSelected (float xShear,
                                     float yShear,
                                     bool centerOnSelection,
                                     bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;
  
    int xOffset = 0;
    int yOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
    }
    
    AffineTransform matrix = AffineTransform::shear (xShear, yShear);
    bool clipped = false;

    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        int x = activeView == Frame::left ? point.z.w : point.x.w;
        x -= xOffset;
        int y = activeView == Frame::bottom ? point.z.w : point.y.w;
        y -= yOffset;

        matrix.transformPoint(x,y);
        
        x += xOffset;
        y += yOffset;
        
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        if (activeView == Frame::left)
            point.z.w = (int16)x;
        else
            point.x.w = (int16)x;
        
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        if (activeView == Frame::bottom)
            point.z.w = (int16)y;
        else
            point.y.w = (int16)y;        
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::translateIldaSelected (int xOffset,
                                         int yOffset,
                                         int zOffset,
                                         bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        int x = point.x.w;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;
        
        int y = point.y.w;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
        
        int z = point.z.w;
        z += zOffset;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::barberPoleIldaSelected (float radius,
                                          float skew,
                                          float zAngle,
                                          bool centerOnSelection,
                                          bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    int xOffset = 0;
    int yOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
    }

    double rz[3][3] = {{1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1}};

    double rotZ = zAngle < 0 ? 360.0 + zAngle : zAngle;
    
    // Clip X rotation
    if (rotZ > 359.9)
        rotZ = 0.0;

    // Get sin and cos
    const double pi = MathConstants<double>::pi;
    double sin = ::sin (rotZ * pi / 180.0);
    double cos = ::cos (rotZ * pi / 180.0);

    rz[0][0] = cos;
    rz[2][2] = cos;
    rz[2][0] = sin;
    rz[0][2] = 0 - sin;

    AffineTransform matrix = AffineTransform::shear (0, skew);

    double rad = (double)radius;
    double theta = 1.0 / rad; // (2.0 * pi) / (2.0 * pi * rad);
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        // fetch next point
        int x = point.x.w;
        x -= xOffset;
        int y = point.y.w;
        y -= yOffset;

        // skew
        matrix.transformPoint(x,y);
                
        // wrap
        double dx = 0 - ::cos ((double)x * theta) * rad;
        double dy = (double)y;
        double dz = ::sin ((double)x * theta) * rad;

        // rotate
        double d;
        d = dx * rz[0][0] + dy * rz[1][0] + dz * rz[2][0];
        x = (int)d;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;

        d = dx * rz[0][1] + dy * rz[1][1] + dz * rz[2][1];
        y = (int)d;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;

        d = dx * rz[0][2] + dy * rz[1][2] + dz * rz[2][2];
        int z = (int)d;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::bulgeIldaSelected (float radius,
                                     float gain,
                                     bool centerOnSelection,
                                     bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    int xOffset = 0;
    int yOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
    }

    double dgain = gain;
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        // fetch next point
        int x = point.x.w;
        x -= xOffset;
        int y = point.y.w;
        y -= yOffset;

        double dx = x;
        double dy = y;
        
        // Distance
        double r = ::sqrt ((dx * dx) + (dy * dy));
        // Angle
        double a = atan2 (dy, dx);
        
        double pow = 1.0 + (double)radius; // 0.01 to 1.99
        double rn = ::pow (r, pow);
        double d;
        
        d = cos(a) * rn;    // adjusted X
        d *= dgain;
        x = (int)d;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;

        d = sin(a) * rn;    // Adjusted Y
        d *= dgain;
        y = (int)d;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::spiralIldaSelected (float angle,
                                      int eSize,
                                      bool centerOnSelection,
                                      bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    int xOffset = 0;
    int yOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
    }
    
    double a = (double)angle;
    double b = (double)eSize;
    b *= b;
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);
        double dx = point.x.w - xOffset;
        double dy = point.y.w - yOffset;
        double dist = (dx * dx + dy * dy) / b;
        double edist = ::exp (-dist);
        double rad = a * edist;
        double d;
        
        d = cos(rad) * dx + sin(rad) * dy;
        int x = (int)d;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;

        d = -sin(rad) * dx + cos(rad) * dy;
        int y = (int)d;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
        
        double es = (double)eSize;
        es = (edist * es);
        double ez = (double)point.z.w;
        ez -= (es / 2.0);
        int z = (int)ez;
        Frame::clipIlda (z);
        
        point.z.w = (int16)z;
    }

    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::sphereIldaSelected (double xScale,
                                      double yScale,
                                      double rScale,
                                      bool centerOnSelection,
                                      bool constrain)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    int xOffset = 0;
    int yOffset = 0;
    
    if (centerOnSelection)
    {
        xOffset = transformCenterX;
        yOffset = transformCenterY;
    }
    
    const double pi = MathConstants<double>::pi;
    double xrad = (2.0 * pi) / 65536.0;
    double yrad = pi / 65536.0;
    double r = 65536.0 / (2.0 * pi);
    r *= rScale;
    
    bool clipped = false;

    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);
        double dx = point.x.w - xOffset;
        double dy = point.y.w - yOffset;
        double d;
        
        d = r * ::sin (dx * xScale * xrad) * ::sin ((32767.0 - (dy * yScale)) * yrad);
        int x = (int)d;
        x += xOffset;
        if (Frame::clipIlda (x))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.x.w = (int16)x;

        d = r * ::cos ((32767.0 - (dy * yScale)) * yrad);
        int y = (int)d;
        y += yOffset;
        if (Frame::clipIlda (y))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.y.w = (int16)y;
        
        d = r * ::sin ((32767.0 - (dy * yScale)) * yrad) * ::cos (dx * xScale * xrad);
        int z = (int)d;
        if (Frame::clipIlda (z))
        {
            Frame::blankPoint (point);
            clipped = true;
        }
        point.z.w = (int16)z;
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::gradientIldaSelected (const Colour& color1,
                                        const Colour& color2,
                                        float angle,
                                        float length,
                                        bool radial,
                                        bool centerOnSelection,
                                        const Colour& color3)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    // We work this one in component space, since nothing is moving
    int xOffset = 32768;
    int yOffset = 32768;
    
    if (centerOnSelection)
        getComponentCenterOfIldaSelection (xOffset, yOffset);

    Point<float> center = Point<float> ((float)xOffset, (float)yOffset);
    
    float flength = length * 32768.0f / 100.0f;
    double rot = angle < 0 ? 360.0 + angle : angle;
    
    // Clip rotation
    if (rot > 359.9)
        rot = 0.0;

    // Convert to radians
    const double pi = MathConstants<double>::pi;
    float ang = (float)(rot * pi / 180.0);

    // Build our line
    Line<float> l1 = Line<float>::fromStartAndAngle (center, flength, ang);
    Line<float> l2 = Line<float>::fromStartAndAngle (center, -flength, ang);
    Line<float> line = Line<float>(l1.getEnd(), l2.getEnd());

    // Make a color gradient
    ColourGradient cg;
    if (radial)
        cg = ColourGradient(color1, 32768.0f, 32768.0f, color2, 32768.0f, 32768.0f - flength, true);
    else
        cg = ColourGradient (color1, line.getStartX(), line.getStartY(), color2, line.getEndX(), line.getEndY(), false);
    
    if (color3.isOpaque())
        cg.addColour (0.5, color3);
    
    // Walk the selected points
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        int x = Frame::getCompXInt (point, activeView);
        int y = Frame::getCompYInt (point, activeView);
                
        float proportion = 0.0;
        
        if (radial)
        {
            Line<float> l(center.getX(), center.getY(), (float)x, (float)y);
            proportion = flength / l.getLength();
        }
        else
            proportion = line.findNearestProportionalPositionTo (Point<float> ((float)x, (float)y));

        if (proportion < 0.0f)
            proportion = 0.0f;
        else if (proportion > 1.0f)
            proportion = 1.0f;
        
        Colour c = cg.getColourAtPosition ((double)proportion);
        point.red = c.getRed();
        point.green = c.getGreen();
        point.blue = c.getBlue();
        
        if (c == Colours::black)
            point.status = Frame::BlankedPoint;
        else
            point.status = 0;
    }
    
    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::adjustHueIldaSelected (float hshift,
                                         float saturation,
                                         float brightness)
{
    Array<Frame::IPoint> points = transformPoints;
    if (! points.size())
        return false;

    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::IPoint& point = points.getReference (n);

        Colour c = Colour (point.red, point.green, point.blue).withRotatedHue (hshift);
        float sat = c.getSaturation() + saturation;
        if (sat < 0.0f)
            sat = 0.0f;
        else if (sat > 1.0f)
            sat = 1.0f;
        c = c.withSaturation (sat);
        
        float b = c.getBrightness() + brightness;
        if (b < 0.0f)
            b = 0.0f;
        else if (b > 1.0f)
            b = 1.0f;
        c = c.withBrightness (b);
        
        point.red = c.getRed();
        point.green = c.getGreen();
        point.blue = c.getBlue();
        
        if (c == Colours::black)
            point.status = Frame::BlankedPoint;
        else
            point.status = 0;
    }
    
    transformUsed = true;
    _setIldaPoints (ildaSelection, points);
    return true;
}

bool FrameEditor::scaleSketchSelected (float xScale, float yScale, bool centerOnSelected, bool constrain)
{
    Array<IPath> paths (transformPaths);
    if (! paths.size())
        return false;

    int xOffset = 32768;
    int yOffset = 32768;
    
    if (centerOnSelected)
    {
        xOffset = transformSketchCenterX;
        yOffset = transformSketchCenterY;
    }
    
    bool clipped = false;

    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            int x, y;
            a.getPosition (x, y);
            int enX, enY;
            a.getEntryPosition (enX, enY);
            int exX, exY;
            a.getExitPosition (exX, exY);
            
            double d = (double)x;
            d -= (double)xOffset;
            d *= xScale;
            d += (double)xOffset;
            x = (int)d;
            if (Frame::clipSketch (x))
                clipped = true;

            d = (double)enX;
            d -= (double)xOffset;
            d *= xScale;
            d += (double)xOffset;
            enX = (int)d;

            d = (double)exX;
            d -= (double)xOffset;
            d *= xScale;
            d += (double)xOffset;
            exX = (int)d;

            d = (double)y;
            d -= (double)yOffset;
            d *= yScale;
            d += (double)yOffset;
            y = (int)d;
            if (Frame::clipSketch (y))
                clipped = true;

            d = (double)enY;
            d -= (double)yOffset;
            d *= yScale;
            d += (double)yOffset;
            enY = (int)d;

            d = (double)exY;
            d -= (double)yOffset;
            d *= yScale;
            d += (double)yOffset;
            exY = (int)d;

            a.setPosition (x, y);
            a.setEntryPosition (enX, enY);
            a.setExitPosition (exX, exY);
            
            p->setAnchor (i, a);
        }
    }

    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setPaths (iPathSelection, paths);
    return true;
}

bool FrameEditor::rotateSketchSelected (float zAngle, bool centerOnSelection, bool constrain)
{
    Array<IPath> paths (transformPaths);
    if (! paths.size())
        return false;

    int xOffset = 32768;
    int yOffset = 32768;
    
    if (centerOnSelection)
    {
        xOffset = transformSketchCenterX;
        yOffset = transformSketchCenterY;
    }

    double rotZ = zAngle < 0 ? 360.0 + zAngle : zAngle;
    
    // Clip Z rotation
    if (rotZ > 359.9)
        rotZ = 0.0;

    // Get sin and cos
    const double pi = MathConstants<double>::pi;
    double rad = rotZ * pi / 180.0;

    AffineTransform matrix = AffineTransform::rotation((float)rad, (float)xOffset, (float)yOffset);

    bool clipped = false;

    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            int x, y;
            a.getPosition (x, y);
            int enX, enY;
            a.getEntryPosition (enX, enY);
            int exX, exY;
            a.getExitPosition (exX, exY);

            matrix.transformPoint (x,y);
            if (Frame::clipSketch (x))
                clipped = true;
            if (Frame::clipSketch (y))
                clipped = true;
            
            matrix.transformPoint (enX, enY);
            matrix.transformPoint (exX, exY);

            a.setPosition (x, y);
            a.setEntryPosition (enX, enY);
            a.setExitPosition (exX, exY);

            p->setAnchor (i, a);
        }
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setPaths (iPathSelection, paths);
    return true;
}

bool FrameEditor::reAnchorSketchSelected (int anchors, int pointsPer)
{
    if (anchors < 2)
        return false;
    
    Array<IPath> paths (transformPaths);
    if (! paths.size())
        return false;

    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        p->setExtraPointsPerAnchor ((uint16)(pointsPer - 1));
        
        if (p->getAnchorCount() != 1)
        {
            Path path = p->getPath();
            float length = path.getLength();
            if (length > 64.0f)
            {
                p->clearAllAnchors();
                Array<Anchor> newAnchors;
                for (auto i = 0; i < anchors; ++i)
                    newAnchors.add (Anchor());
                
                float offset = length / (float)(anchors - 1);
                
                for (auto i = 0; i < (anchors -1); ++i)
                {
                    CurveFit::fit (path, (float)i * offset, (float)i * offset + offset, newAnchors.getReference (i), newAnchors.getReference (i + 1));
                }
                
                for (auto i = 0; i < newAnchors.size(); ++i)
                    p->addAnchor (newAnchors[i]);
            }
        }
    }

    transformUsed = true;
    _setPaths (iPathSelection, paths);
    return true;
}

bool FrameEditor::shearSketchSelected (float shearX, float shearY,
                                       bool centerOnSelection, bool constrain)
{
    Array<IPath> paths (transformPaths);
    if (! paths.size())
        return false;

    int xOffset = 32768;
    int yOffset = 32768;
    
    if (centerOnSelection)
    {
        xOffset = transformSketchCenterX;
        yOffset = transformSketchCenterY;
    }

    AffineTransform matrix = AffineTransform::shear (-shearX, -shearY);

    bool clipped = false;

    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            int x, y;
            a.getPosition (x, y);
            int enX, enY;
            a.getEntryPosition (enX, enY);
            int exX, exY;
            a.getExitPosition (exX, exY);

            x -= xOffset;
            y -= yOffset;
            matrix.transformPoint (x,y);
            x += xOffset;
            if (Frame::clipSketch (x))
                clipped = true;
            y += yOffset;
            if (Frame::clipSketch (y))
                clipped = true;
            
            enX -= xOffset;
            enY -= yOffset;
            matrix.transformPoint (enX, enY);
            enX += xOffset;
            enY += yOffset;

            exX -= xOffset;
            exY -= yOffset;
            matrix.transformPoint (exX, exY);
            exX += xOffset;
            exY += yOffset;

            a.setPosition (x, y);
            a.setEntryPosition (enX, enY);
            a.setExitPosition (exX, exY);

            p->setAnchor (i, a);
        }
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setPaths (iPathSelection, paths);
    return true;
}

bool FrameEditor::translateSketchSelected (int xOffset, int yOffset, bool constrain)
{
    Array<IPath> paths (transformPaths);
    if (! paths.size())
        return false;
    
    bool clipped = false;
    
    for (auto n = 0; n < paths.size(); ++n)
    {
        IPath* p = &paths.getReference (n);
        
        int i = 0;
        int end = p->getAnchorCount();
        
        if (iPathSelection.getAnchor() != -1)
        {
            i = iPathSelection.getAnchor();
            end = i + 1;
        }

        for (; i < end; ++i)
        {
            Anchor a = p->getAnchor (i);
            
            if (iPathSelection.getControl() == -1)
            {
                int x = a.getX();
                x += xOffset;
                if (Frame::clipSketch (x))
                    clipped = true;
                a.setX (x);
                
                int y = a.getY();
                y += yOffset;
                if (Frame::clipSketch (y))
                    clipped = true;
                a.setY (y);
            }
            else if (iPathSelection.getControl() == 1)
            {
                int x, y;
                a.getEntryPosition (x, y);
                x += xOffset;
                y += yOffset;
                a.setEntryPosition (x, y);
            }
            else if (iPathSelection.getControl() == 2)
            {
                int x, y;
                a.getExitPosition (x, y);
                x += xOffset;
                y += yOffset;
                a.setExitPosition (x, y);
            }

            p->setAnchor (i, a);
        }
    }
    
    if (constrain && clipped)
        return false;

    transformUsed = true;
    _setPaths (iPathSelection, paths);
    return true;
}

void FrameEditor::endTransform()
{
    if (activeLayer == ilda)
    {
        // Already Transformed! So grab!
        Array<Frame::IPoint> points;
        getIldaSelectedPoints(points);

        // Restore original
        _setIldaPoints (ildaSelection, transformPoints);
        
        tranformInProgress = false;
        transformPoints.clear();
        
        // Final undoable switch
        if (transformUsed)
        {
            beginNewTransaction (transformName);
            perform (new UndoableSetIldaPoints (this, ildaSelection, points));

            refreshThumb();
        }
    }
    else
    {
        // Grab transformed version
        Array<IPath> paths;
        getSelectedIPaths (paths);
        
        // Restore original
        _setPaths (iPathSelection, transformPaths);
        
        tranformInProgress = false;
        transformPaths.clear();
    
        if (transformUsed)
        {
            beginNewTransaction (transformName);
            perform (new UndoableSetPaths (this, iPathSelection, paths));
        }
    }
 
    transformUsed = false;
    sendActionMessage (EditorActions::transformEnded);
}

void FrameEditor::setIldaSelectedX (int16 newX)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
        points.getReference (n).x.w = newX;
    
    beginNewTransaction ("Point X Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedY (int16 newY)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
        points.getReference (n).y.w = newY;
    
    beginNewTransaction ("Point Y Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedZ (int16 newZ)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
        points.getReference (n).z.w = newZ;
    
    beginNewTransaction ("Point Z Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedR (uint8 newR)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        points.getReference (n).red = newR;
        if (points[n].red == 0 && points[n].green == 0 && points[n].blue == 0)
            points.getReference (n).status = Frame::BlankedPoint;
        else
            points.getReference (n).status = 0;
    }
    
    beginNewTransaction ("Point Red Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedG (uint8 newG)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        points.getReference (n).green = newG;
        if (points[n].red == 0 && points[n].green == 0 && points[n].blue == 0)
            points.getReference (n).status = Frame::BlankedPoint;
        else
            points.getReference (n).status = 0;
    }
    
    beginNewTransaction ("Point Green Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedB (uint8 newB)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        points.getReference (n).blue = newB;
        if (points[n].red == 0 && points[n].green == 0 && points[n].blue == 0)
            points.getReference (n).status = Frame::BlankedPoint;
        else
            points.getReference (n).status = 0;
    }
    
    beginNewTransaction ("Point Blue Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}

void FrameEditor::setIldaSelectedRGB (const Colour newColor)
{
    Array<Frame::IPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        points.getReference (n).red = newColor.getRed();
        points.getReference (n).green = newColor.getGreen();
        points.getReference (n).blue = newColor.getBlue();
        
        if (points[n].red == 0 && points[n].green == 0 && points[n].blue == 0)
            points.getReference (n).status = Frame::BlankedPoint;
        else
            points.getReference (n).status = 0;
    }
    
    beginNewTransaction ("Point Color Change");
    perform (new UndoableSetIldaPoints (this, ildaSelection, points));
}


//==============================================================================
void FrameEditor::_setActiveLayer (Layer layer)
{
    if (layer != activeLayer)
    {
        activeLayer = layer;
        sendActionMessage (EditorActions::layerChanged);
        refreshThumb();
    }
}

void FrameEditor::_setActiveView (View view)
{
    if (view != activeView)
    {
        activeView = view;
        sendActionMessage (EditorActions::viewChanged);
        refreshThumb();
    }
}

void FrameEditor::_setZoomFactor (float zoom)
{
    zoom = jmin (zoom, MAX_ZOOM);
    zoom = jmax (zoom, MIN_ZOOM);
    
    if (zoom != zoomFactor)
    {
        zoomFactor = zoom;
        sendActionMessage (EditorActions::zoomFactorChanged);
    }
}

void FrameEditor::_setActiveIldaTool (IldaTool tool)
{
    if (activeIldaTool != tool)
    {
        activeIldaTool = tool;
        sendActionMessage (EditorActions::ildaToolChanged);

        refreshThumb();
    }
}

void FrameEditor::_setPointToolColor (const Colour& color)
{
    if (color != pointToolColor)
    {
        pointToolColor = color;
        if (color != Colours::black)
            lastVisiblePointToolColor = color;
        sendActionMessage (EditorActions::ildaPointToolColorChanged);
    }
}

void FrameEditor::_setActiveSketchTool (SketchTool tool)
{
    if (activeSketchTool != tool)
    {
        activeSketchTool = tool;
        sendActionMessage (EditorActions::sketchToolChanged);

        refreshThumb();
    }
}

void FrameEditor::_setSketchToolColor (const Colour& color)
{
    if (color != sketchToolColor)
    {
        sketchToolColor = color;
        if (color != Colours::black)
            lastVisibleSketchToolColor = color;
        sendActionMessage (EditorActions::sketchToolColorChanged);
    }
}

void FrameEditor::_setSketchVisible (bool visible)
{
    if (visible != sketchVisible)
    {
        sketchVisible = visible;
        sendActionMessage (EditorActions::sketchVisibilityChanged);
    }
}

void FrameEditor::_setIldaVisible (bool visible)
{
    if (visible != ildaVisible)
    {
        ildaVisible = visible;
        sendActionMessage (EditorActions::ildaVisibilityChanged);
    }
}

void FrameEditor::_setRefVisible (bool visible)
{
    if (visible != refVisible)
    {
        refVisible = visible;
        sendActionMessage (EditorActions::refVisibilityChanged);
    }
}

bool FrameEditor::_setImageData (const MemoryBlock& file)
{
    currentFrame->setImageData (file);
    sendActionMessage (EditorActions::backgroundImageChanged);
    return true;
}

void FrameEditor::_setImageOpacity (float opacity)
{
    if (opacity < 0)
        opacity = 0;
    else if (opacity > 1.0)
        opacity = 1.0;
    
    if (opacity != refOpacity)
    {
        currentFrame->setImageOpacity (opacity);
        sendActionMessage (EditorActions::refOpacityChanged);
    }
}

void FrameEditor::_setImageScale (float scale)
{
    if (scale < 0.01f)
        scale = 0.01f;
    else if (scale > 2.00f)
        scale = 2.00f;
    
    if (scale != currentFrame->getImageScale())
    {
        currentFrame->setImageScale (scale);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setImageRotation (float rot)
{
    if (rot < 0.0f)
        rot = 0.0f;
    else if (rot > 359.9f)
        rot = 359.9f;
    
    if (rot != currentFrame->getImageRotation())
    {
        currentFrame->setImageRotation (rot);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setImageXoffset (float off)
{
    if (off < -100)
        off = -100;
    else if (off > 100)
        off = 100;
    
    if (off != currentFrame->getImageXoffset())
    {
        currentFrame->setImageXoffset (off);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setImageYoffset (float off)
{
    if (off < -100)
        off = -100;
    else if (off > 100)
        off = 100;
    
    if (off != currentFrame->getImageYoffset())
    {
        currentFrame->setImageYoffset (off);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setFrames (const ReferenceCountedArray<Frame> frames)
{
    Frames = frames;
    sendActionMessage (EditorActions::framesChanged);
}

void FrameEditor::_setFrameIndex (uint16 index)
{
    if (Frames.size() <= index)
        index = 0;
    
    currentFrame = Frames[index];
    frameIndex = index;
    
    sendActionMessage (EditorActions::frameIndexChanged);
}

void FrameEditor::_deleteFrame (uint16 index)
{
    if ((Frames.size() > 1) && (index < Frames.size()))
    {
        Frames.remove (index);
        currentFrame = Frames[frameIndex];
        sendActionMessage (EditorActions::framesChanged);
    }
}

void FrameEditor::_insertFrame (uint16 index, Frame::Ptr frame)
{
    if (index <= Frames.size())
    {
        Frames.insert(index, frame);
        currentFrame = Frames[frameIndex];
        sendActionMessage (EditorActions::framesChanged);
    }
}

void FrameEditor::_newFrame()
{
    Frames.insert (getFrameIndex() + 1, new Frame());
    sendActionMessage (EditorActions::framesChanged);
}

void FrameEditor::_dupFrame()
{
    Frame::Ptr oldFrame = getFrame();
    Frame::Ptr newFrame = new Frame (*oldFrame.get());
    Frames.insert (getFrameIndex() + 1, newFrame);
    sendActionMessage (EditorActions::framesChanged);
}

void FrameEditor::_swapFrames (uint16 index1, uint16 index2)
{
    if (index1 < Frames.size() && index2 < Frames.size())
    {
        Frames.swap (index1, index2);
        
        // We could be whacking out the current index pointer
        // So reset active data just in case
        currentFrame = Frames[getFrameIndex()];
        sendActionMessage (EditorActions::framesChanged);
    }
}

void FrameEditor::_setIldaShowBlanked (bool show)
{
    if (getIldaShowBlanked() != show)
    {
        ildaShowBlanked = show;
        sendActionMessage (EditorActions::frameIndexChanged);
    }
}

void FrameEditor::_setIldaDrawLines (bool draw)
{
    if (getIldaDrawLines() != draw)
    {
        ildaDrawLines = draw;
        sendActionMessage (EditorActions::ildaDrawLinesChanged);
    }
}

void FrameEditor::_setDrawGrid (bool draw)
{
    if (getRefDrawGrid() != draw)
    {
        refDrawGrid = draw;
        sendActionMessage (EditorActions::refDrawGridChanged);
    }
}

void FrameEditor::_setIldaSelection (const SparseSet<uint16>& selection)
{
    if (selection.getTotalRange().getEnd() > getPointCount())
        return;
    
    if ((! getIldaVisible()) && (! selection.isEmpty()))
        return;
    
    if (selection != ildaSelection)
    {
        ildaSelection = selection;
        sendActionMessage (EditorActions::ildaSelectionChanged);
    }
}

void FrameEditor::_setIldaPoints (const SparseSet<uint16>& selection,
                                  const Array<Frame::IPoint>& points)
{
    auto pindex = 0;
    
    if (selection.isEmpty())
        return;
    
    for (auto n = 0; n < selection.getNumRanges(); ++n)
    {
        Range<uint16> r = selection.getRange (n);
        
        for (uint16 i = 0; i < r.getLength(); ++i)
            currentFrame->replacePoint (r.getStart() + i, points[pindex++]);
    }
    
    sendActionMessage (EditorActions::ildaPointsChanged);
}

void FrameEditor::_insertPoint (uint16 index, const Frame::IPoint& point)
{
    if (index <= currentFrame->getPointCount())
    {
        currentFrame->insertPoint (index, point);
        sendActionMessage (EditorActions::ildaPointsChanged);
    }
}

void FrameEditor::_setPoints (const Array<Frame::IPoint>& points)
{
    currentFrame->setPoints (points);
    sendActionMessage (EditorActions::ildaPointsChanged);
}

void FrameEditor::_deletePoint (uint16 index)
{
    if (index < currentFrame->getPointCount())
    {
        currentFrame->removePoint (index);
        sendActionMessage (EditorActions::ildaPointsChanged);
    }
}

void FrameEditor::_setIPathSelection (const IPathSelection& selection)
{
    if (selection.getTotalRange().getEnd() > getIPathCount())
        return;
    
    if ((! getSketchVisible()) && (! selection.isEmpty()))
        return;
    
    if (selection != iPathSelection)
    {
        iPathSelection = selection;
        sendActionMessage (EditorActions::iPathSelectionChanged);
    }
}

void FrameEditor::_deletePath (int index)
{
    if ((index >= 0) && (index < getIPathCount()))
    {
        currentFrame->deletePath (index);
        sendActionMessage (EditorActions::iPathsChanged);
    }
}

void FrameEditor::_insertPath (int index, IPath& path)
{
    if ((index >= 0) && (index <= getIPathCount()))
    {
        currentFrame->insertPath (index, path);
        sendActionMessage (EditorActions::iPathsChanged);
    }
}

void FrameEditor::_setPaths (const IPathSelection& selection,
                             const Array<IPath>& paths)
{
    int pindex = 0;
    
    for (auto n = 0; n < selection.getNumRanges(); ++n)
    {
        Range<uint16> r = selection.getRange (n);
        for (auto i = r.getStart(); i < r.getEnd(); ++i)
            currentFrame->replacePath (i, paths.getReference (pindex++));
    }
    
    sendActionMessage (EditorActions::iPathsChanged);
}

void FrameEditor::_setIPaths (const Array<IPath>& paths)
{
    currentFrame->setIPaths (paths);
    sendActionMessage (EditorActions::iPathsChanged);
}

void FrameEditor::_deleteAnchor (int pindex, int aindex)
{
    IPath path = currentFrame->getIPath (pindex);
    path.removeAnchor (aindex);
    currentFrame->replacePath (pindex, path);
    sendActionMessage (EditorActions::iPathsChanged);
}

void FrameEditor::_insertAnchor (int pindex, int aindex, const Anchor& a)
{
    IPath path = currentFrame->getIPath (pindex);
    path.insertAnchor (aindex, a);
    currentFrame->replacePath (pindex, path);
    sendActionMessage (EditorActions::iPathsChanged);
}
