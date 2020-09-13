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
      activeLayer (sketch),
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

//==============================================================================
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

void FrameEditor::setRefOpacity (float opacity)
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
        sendActionMessage(EditorActions::layerChanged);
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

void FrameEditor::_setRefOpacity (float opacity)
{
    if (opacity < 0)
        opacity = 0;
    else if (opacity > 1.0)
        opacity = 1.0;
    
    if (opacity != refOpacity)
    {
        refOpacity = opacity;
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

