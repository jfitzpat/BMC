/*
    FrameEditor.h
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

#pragma once

#include <JuceHeader.h>
#include "Frame.h"

#define MIN_ZOOM (1.0f)
#define MAX_ZOOM (16.0f)

//==============================================================================
class FrameEditor  : public ActionBroadcaster,
                     public UndoManager
{
public:
    FrameEditor();
    ~FrameEditor() override;

    typedef enum {
        sketch = 0,
        ilda,
        reference
    } Layer;

    typedef enum {
        selectTool,
        pointTool
    } IldaTool;
    
    // Dirty/Clean mechanism
    uint32 getDirtyCounter() { return dirtyCounter; }
    void setDirtyCounter (uint32 count);
    void incDirtyCounter();
    void decDirtyCounter();
    
    // Save/Export
    void fileSave();
    void fileSaveAs();
    void fileIldaExport();
    
    // Tool helpers
    void cancelRequest();
    
    // Polling
    uint32 getScanRate() { return scanRate; }
    const File& getLoadedFile() { return loadedFile; }
    
    float getZoomFactor() { return zoomFactor; }
    Layer getActiveLayer() { return activeLayer; }
    bool getSketchVisible() { return sketchVisible; }
    bool getIldaVisible() { return ildaVisible; }
    bool getRefVisible() { return refVisible; }
    
    IldaTool getActiveIldaTool() { return activeIldaTool; }
    Colour getPointToolColor() { return pointToolColor; }
    
    const MemoryBlock& getImageData() {return currentFrame->getImageData(); }
    
    const Image* getImage() { return currentFrame->getBackgroundImage(); }
    bool getRefDrawGrid() { return refDrawGrid; }
    float getImageOpacity() { return currentFrame->getImageOpacity(); }
    float getImageScale() { return currentFrame->getImageScale(); }
    float getImageRotation() { return currentFrame->getImageRotation(); }
    float getImageXoffset() { return currentFrame->getImageXoffset(); }
    float getImageYoffset() { return currentFrame->getImageYoffset(); }
    
    const ReferenceCountedArray<Frame>& getFrames() { return Frames; }
    uint16 getFrameCount() { return (uint16)Frames.size(); }
    uint16 getFrameIndex() { return frameIndex; }
    Frame::Ptr getFrame ( uint16 index ) { return Frames[index]; };
    Frame::Ptr getFrame () { return getFrame (getFrameIndex()); }
    
    uint16 getPointCount() { return currentFrame->getPointCount(); }
    bool getPoint (uint16 index, Frame::XYPoint& point)
    {
        return currentFrame->getPoint (index, point);
    }

    bool getIldaShowBlanked() { return ildaShowBlanked; }
    bool getIldaDrawLines() { return ildaDrawLines; }
    const SparseSet<uint16>& getIldaSelection() { return ildaSelection; }
    const Point<int16> getCenterOfIldaSelection();
    const Point<int> getComponentCenterOfIldaSelection();
    void getComponentCenterOfIldaSelection (int&x, int&y);
    
    void getIldaSelectedPoints (Array<Frame::XYPoint>& points);
    void getIldaPoints (const SparseSet<uint16>& selection, Array<Frame::XYPoint>& points);

    const Image& getCurrentThumbNail() { return currentFrame->getThumbNail(); }
    const Image& getThumbNail (uint16 index) { return Frames[index]->getThumbNail(); }
    
    // Undoable Commands
    void setActiveLayer (Layer layer);
    void setSketchVisible (bool visible);
    void setIldaVisible (bool visible);
    void setRefVisible (bool visible);
    
    void setActiveIldaTool (IldaTool tool);
    void setPointToolColor (const Colour& color);

    void selectImage();
    void clearImage();
    void setDrawGrid (bool draw);
    void setImageOpacity (float opacity);
    void setImageScale (float scale);
    void setImageRotation (float rot);
    void setImageXoffset (float off);
    void setImageYoffset (float off);

    void loadFile();
    void loadFile (File& file);
    
    void newFile();
    void selectAll();
    void clearSelection();
    
    void setIldaShowBlanked (bool show);
    void setIldaDrawLines (bool show);

    void setFrameIndex (uint16 index);
    void deleteFrame ();
    void newFrame();
    void dupFrame();
    void moveFrameUp();
    void moveFrameDown();
    
    void setIldaSelection (const SparseSet<uint16>& selection);

    void setIldaSelectedX (int16 newX);
    void setIldaSelectedY (int16 newY);
    void setIldaSelectedZ (int16 newZ);
    void setIldaSelectedR (uint8 newR);
    void setIldaSelectedG (uint8 newR);
    void setIldaSelectedB (uint8 newR);
    void setIldaSelectedRGB (const Colour newColor);
    
    void insertPoint (const Frame::XYPoint& point);
    
    // Destructive Version (invoked by UndoManager)
    void _setLoadedFile (const File& file) { loadedFile = file; }
    void _setZoomFactor (float zoom);
    void _setActiveLayer (Layer layer);
    void _setSketchVisible (bool visible);
    void _setIldaVisible (bool visible);
    void _setRefVisible (bool visible);

    void _setActiveIldaTool (IldaTool tool);
    void _setPointToolColor (const Colour& color);

    void _insertPoint (uint16 index, const Frame::XYPoint& point);
    void _deletePoint (uint16 index);
    
    bool _setImageData (const MemoryBlock& file);
    void _setDrawGrid (bool draw);
    void _setImageOpacity (float opacity);
    void _setImageScale (float scale);
    void _setImageRotation (float rot);
    void _setImageXoffset (float off);
    void _setImageYoffset (float off);

    void _setFrames (const ReferenceCountedArray<Frame> frames);
    void _setFrameIndex (uint16 index);
    void _deleteFrame (uint16 index);
    void _insertFrame (uint16 index, Frame::Ptr frame);
    void _newFrame();
    void _dupFrame();
    void _swapFrames (uint16 index1, uint16 index2);
    
    void _setIldaShowBlanked (bool show);
    void _setIldaDrawLines (bool draw);
    void _setIldaSelection (const SparseSet<uint16>& selection);

    void _setIldaPoints (const SparseSet<uint16>& selection,
                         const Array<Frame::XYPoint>& points);

private:
    File loadedFile;
    uint32 dirtyCounter;
    uint32 scanRate;
    float zoomFactor;
    Layer activeLayer;
    IldaTool activeIldaTool;
    Colour pointToolColor;
    bool sketchVisible;
    bool ildaVisible;
    bool ildaShowBlanked;
    bool ildaDrawLines;
    bool refVisible;
    bool refDrawGrid;
    float refOpacity;
    
    uint16 frameIndex;
    ReferenceCountedArray<Frame> Frames;
    Frame::Ptr currentFrame;

    SparseSet<uint16> ildaSelection;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameEditor)
};

// Keep the broadcast messages short and unique
namespace EditorActions
{
    const String dirtyStatusChanged         ("DSC");
    const String layerChanged               ("LC");
    const String zoomFactorChanged          ("ZFC");
    const String sketchVisibilityChanged    ("SVC");
    const String ildaVisibilityChanged      ("IVC");
    const String refVisibilityChanged       ("RVC");
    const String backgroundImageChanged     ("BIC");
    const String refOpacityChanged          ("ROC");
    const String refDrawGridChanged         ("RGC");
    const String backgroundImageAdjusted    ("BIA");
    const String framesChanged              ("FC");
    const String frameIndexChanged          ("FIC");
    const String ildaShowBlankChanged       ("SBC");
    const String ildaDrawLinesChanged       ("DLC");
    const String ildaSelectionChanged       ("ISC");
    const String ildaPointsChanged          ("IPC");
    const String ildaToolChanged            ("ITC");
    const String ildaPointToolColorChanged  ("PTC");
    const String cancelRequest              ("CR");
}
