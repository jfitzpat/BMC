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
#include "FrameEditor.h"

#include "FrameUndo.h"      // UndoableTask classes

//==============================================================================
FrameEditor::FrameEditor()
    : dirtyCounter (0),
      scanRate (30000),
      zoomFactor (1.0),
      activeLayer (sketch),
      activeView (Frame::front),
      activeIldaTool (selectTool),
      pointToolColor (Colours::white),
      lastVisiblePointToolColor (Colours::white),
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

bool FrameEditor::hasSelection()
{
    if (activeLayer == ilda && (! ildaSelection.isEmpty()))
        return true;
        
    return false;
}

bool FrameEditor::hasMovableSelection()
{
    return hasSelection();
}

void FrameEditor::toggleBlanking()
{
    if ((activeLayer == ilda) && (activeIldaTool == pointTool))
        togglePointToolBlank();
}

void FrameEditor::cycleColors()
{
    if ((activeLayer == ilda) && (activeIldaTool == pointTool))
        cyclePointToolColors();
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
            Frame::XYPoint point;
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

void FrameEditor::getIldaSelectedPoints (Array<Frame::XYPoint>& points)
{
    points.clear();
    
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        
        for (uint16 i = 0; i < r.getLength(); ++i)
        {
            Frame::XYPoint point;
            
            getPoint (r.getStart() + i, point);
            points.add (point);
        }
    }
}

void FrameEditor::getIldaPoints (const SparseSet<uint16>& selection,
                                 Array<Frame::XYPoint>& points)
{
    points.clear();
    
    for (auto n = 0; n < selection.getNumRanges(); ++n)
    {
        Range<uint16> r = selection.getRange (n);
        
        for (uint16 i = 0; i < r.getLength(); ++i)
        {
            Frame::XYPoint point;
            
            getPoint (r.getStart() + i, point);
            points.add (point);
        }
    }
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

void FrameEditor::setSketchVisible (bool visible)
{
    if (visible != sketchVisible)
    {
        sketchVisible = visible;
        sendActionMessage (EditorActions::sketchVisibilityChanged);
    }
}

void FrameEditor::setIldaVisible (bool visible)
{
    if (visible != ildaVisible)
    {
        beginNewTransaction ("Ilda Visible Change");
        perform(new UndoableSetIldaVisibility (this, visible));
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
                    Frame::XYPoint point;
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
}

void FrameEditor::clearSelection()
{
    if (getActiveLayer() == ilda)
        setIldaSelection (SparseSet<uint16>());
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
    
    if (file.getFileExtension() == ".ild")
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
        
        if (f.getFileExtension() == ".ild")
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

void FrameEditor::insertPoint (const Frame::XYPoint& point)
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
    
    if (! getIldaVisible())
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
    Array<Frame::XYPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return false;
    
    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points;
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
        Frame::XYPoint& point = points.getReference (n);
        
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
    getIldaSelectedPoints (points);
    if (! points.size())
        return;
    
    int pIndex = points.size() - 1;
    
    Frame::XYPoint point;
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

void FrameEditor::startTransform (const String& name)
{
    getIldaSelectedPoints (transformPoints);
    getCenterOfIldaSelection (transformCenterX, transformCenterY, transformCenterZ);
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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
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
        
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
    if (! points.size())
        return false;

    bool clipped = false;
    
    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);
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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);
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
    Array<Frame::XYPoint> points = transformPoints;
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
        Frame::XYPoint& point = points.getReference (n);

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
    Array<Frame::XYPoint> points = transformPoints;
    if (! points.size())
        return false;

    for (auto n = 0; n < points.size(); ++n)
    {
        Frame::XYPoint& point = points.getReference (n);

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

void FrameEditor::endTransform()
{
    // Already Transformed! So grab!
    Array<Frame::XYPoint> points;
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
        transformUsed = false;

        refreshThumb();
    }
    sendActionMessage (EditorActions::transformEnded);
}

void FrameEditor::setIldaSelectedX (int16 newX)
{
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
    Array<Frame::XYPoint> points;
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
        sendActionMessage (EditorActions::framesChanged);
    }
}

void FrameEditor::_insertFrame (uint16 index, Frame::Ptr frame)
{
    if (index <= Frames.size())
    {
        Frames.insert(index, frame);
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
    
    if (! getIldaVisible())
        return;
    
    if (selection != ildaSelection)
    {
        ildaSelection = selection;
        sendActionMessage (EditorActions::ildaSelectionChanged);
    }
}

void FrameEditor::_setIldaPoints (const SparseSet<uint16>& selection,
                                  const Array<Frame::XYPoint>& points)
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

void FrameEditor::_insertPoint (uint16 index, const Frame::XYPoint& point)
{
    if (index <= currentFrame->getPointCount())
    {
        currentFrame->insertPoint (index, point);
        sendActionMessage (EditorActions::ildaPointsChanged);
    }
}

void FrameEditor::_deletePoint (uint16 index)
{
    if (index < currentFrame->getPointCount())
    {
        currentFrame->removePoint (index);
        sendActionMessage (EditorActions::ildaPointsChanged);
    }
}

