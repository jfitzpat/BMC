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
      activeIldaTool (selectTool),
      pointToolColor (Colours::red),
      sketchVisible (true),
      ildaVisible (true),
      ildaShowBlanked (true),
      ildaDrawLines (true),
      refVisible (true),
      refDrawGrid (true),
      refOpacity (1.0),
      frameIndex (0)
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
        sendActionMessage (EditorActions::dirtyStatusChanged);
}

void FrameEditor::decDirtyCounter()
{
    if (dirtyCounter)
    {
        dirtyCounter--;
        
        if (! dirtyCounter)
            sendActionMessage (EditorActions::dirtyStatusChanged);
    }
}

//==============================================================================
void FrameEditor::fileSave()
{
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

void FrameEditor::cancelRequest()
{
    sendActionMessage (EditorActions::cancelRequest);
}

void FrameEditor::deleteRequest()
{
    sendActionMessage (EditorActions::deleteRequest);
}

//==============================================================================
const Point<int16> FrameEditor::getCenterOfIldaSelection()
{
    // Don't bother if there is no selection visible
    if (ildaSelection.isEmpty() || activeLayer != ilda)
        return Point<int16> (0, 0);
    
    bool first = true;
    int16 minx, maxx, miny, maxy;
    minx = maxx = miny = maxy = 0;  // For Visual Studio warning
    
    for (auto n = 0; n < ildaSelection.getNumRanges(); ++n)
    {
        Range<uint16> r = ildaSelection.getRange (n);
        for (auto i=0; i < r.getLength(); ++i)
        {
            Frame::XYPoint point;
            currentFrame->getPoint (r.getStart() + i, point);
            
            if (first)
            {
                minx = maxx = point.x.w;
                miny = maxy = point.y.w;
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
            }
        }
    }
    
    return Point<int16> ((minx + maxx) / 2, (miny + maxy) / 2);
}

const Point<int> FrameEditor::getComponentCenterOfIldaSelection()
{
    Point<int16> c = getCenterOfIldaSelection();
    return Point<int> ((int)c.getX() + 32768, 32767 - (int)c.getY());
}

void FrameEditor::getComponentCenterOfIldaSelection (int&x, int&y)
{
    Point<int16> c = getCenterOfIldaSelection();
    
    x = (int)c.getX() + 32768;
    y = 32767 - (int)c.getY();
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
        perform(new UndoableSetLayer (this, layer));
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
    
    if (selection != ildaSelection)
    {
        beginNewTransaction ("Selection Change");
        perform (new UndoableSetIldaSelection (this, selection));
    }
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
    }
}

void FrameEditor::_setPointToolColor (const Colour& color)
{
    if (color != pointToolColor)
    {
        pointToolColor = color;
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
    if (index < Frames.size())
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

