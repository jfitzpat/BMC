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
#include "IPath.h"

#define MIN_ZOOM (1.0f)
#define MAX_ZOOM (16.0f)

//==============================================================================
// Keep the broadcast messages short and unique
namespace EditorActions
{
    const String dirtyStatusChanged         ("DSC");
    const String selectionCopied            ("SCP");
    const String layerChanged               ("LC");
    const String viewChanged                ("VC");
    const String zoomFactorChanged          ("ZFC");
    const String sketchVisibilityChanged    ("SVC");
    const String ildaVisibilityChanged      ("IVC");
    const String refVisibilityChanged       ("RVC");
    const String backgroundImageChanged     ("BIC");
    const String refOpacityChanged          ("ROC");
    const String refDrawGridChanged         ("RGC");
    const String backgroundImageAdjusted    ("BIA");
    const String framesChanged              ("FC");
    const String frameThumbsChanged         ("FTC");
    const String frameIndexChanged          ("FIC");
    const String ildaShowBlankChanged       ("SBC");
    const String ildaDrawLinesChanged       ("DLC");
    const String ildaSelectionChanged       ("ISC");
    const String ildaPointsChanged          ("IPC");
    const String ildaToolChanged            ("ITC");
    const String ildaPointToolColorChanged  ("PTC");
    const String sketchToolChanged          ("STC");
    const String sketchToolColorChanged     ("SCC");
    const String iPathsChanged              ("PAC");
    const String iPathSelectionChanged      ("PSC");
    const String cancelRequest              ("CR");
    const String deleteRequest              ("DR");
    const String upRequest                  ("UPR");
    const String downRequest                ("DNR");
    const String leftRequest                ("LER");
    const String rightRequest               ("RIR");
    const String smallUpRequest             ("uPR");
    const String smallDownRequest           ("dNR");
    const String smallLeftRequest           ("lER");
    const String smallRightRequest          ("rIR");
    const String transformStarted           ("TS");
    const String transformEnded             ("TE");
}


// Class to hold selected iPaths
class IPathSelection : public SparseSet<uint16>
{
public:
    IPathSelection() : anchor (-1), control (-1) {;}
    
    int getAnchor() { return anchor; }
    void setAnchor (int a) { anchor = a; }
    int getControl() { return control; }
    void setControl (int c) { control = c; }

    bool operator== (const IPathSelection& other) const noexcept
    {
        if (! SparseSet<uint16>::operator== (other))
            return false;
        
        if (! (anchor == other.anchor))
            return false;
        
        return control == other.control;
    }
    
    bool operator!= (const IPathSelection& other) const noexcept
    {
        if (SparseSet<uint16>::operator!= (other))
            return true;
        
        if (anchor != other.anchor)
            return true;
        
        return control != other.control;
    }

private:
    int anchor;
    int control;
};

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

    typedef Frame::ViewAngle View;
    
    typedef enum {
        selectTool = 0,
        moveTool,
        pointTool
    } IldaTool;
    
    typedef enum {
        sketchSelectTool = 0,
        sketchMoveTool,
        sketchLineTool,
        sketchRectTool,
        sketchEllipseTool,
        sketchPenTool
    } SketchTool;
    
    // Dirty/Clean mechanism
    uint32 getDirtyCounter() { return dirtyCounter; }
    void setDirtyCounter (uint32 count);
    void incDirtyCounter();
    void decDirtyCounter();
    void refreshThumb();
    
    // Save/Export
    void fileSave();
    void fileSaveAs();
    void fileIldaExport();
    
    // Edit helpers
    bool canCopy();
    bool canPaste();
    void copy();
    
    // Sketch helpers
    void connectIPaths (const Array<IPath>& src, Array<IPath>& dst);
    bool isClosedIPath (const IPath& path);
    void pointToIPointXYZ (Point<int> a, Frame::IPoint& point);
    void anchorToPointXYZ (const Anchor& a, Frame::IPoint& point);
    void generatePointsFromPaths (const Array<IPath>& paths, Array<Frame::IPoint>& points, bool appendPoints = false);


    // Tool helpers
    void cancelRequest()
    {
        sendActionMessage (EditorActions::cancelRequest);
        refreshThumb();
    }
    void deleteRequest()    { sendActionMessage (EditorActions::deleteRequest); }
    void upRequest()        { sendActionMessage (EditorActions::upRequest); }
    void downRequest()      { sendActionMessage (EditorActions::downRequest); }
    void leftRequest()      { sendActionMessage (EditorActions::leftRequest); }
    void rightRequest()     { sendActionMessage (EditorActions::rightRequest); }
    void smallUpRequest()   { sendActionMessage (EditorActions::smallUpRequest); }
    void smallDownRequest() { sendActionMessage (EditorActions::smallDownRequest); }
    void smallLeftRequest() { sendActionMessage (EditorActions::smallLeftRequest); }
    void smallRightRequest(){ sendActionMessage (EditorActions::smallRightRequest); }
    bool hasSelection();
    bool hasMovableSelection();
    void toggleBlanking();
    void cycleColors();
    
    // Polling
    uint32 getScanRate() { return scanRate; }
    const File& getLoadedFile() { return loadedFile; }
    
    float getZoomFactor() { return zoomFactor; }
    Layer getActiveLayer() { return activeLayer; }
    View getActiveView() { return activeView; }
    bool getSketchVisible() { return sketchVisible; }
    bool getIldaVisible() { return ildaVisible; }
    bool getRefVisible() { return refVisible; }
    
    IldaTool getActiveIldaTool() { return activeIldaTool; }
    Colour getPointToolColor() { return pointToolColor; }
    SketchTool getActiveSketchTool() { return activeSketchTool; }
    Colour getSketchToolColor() { return sketchToolColor; }
    
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
    bool getPoint (uint16 index, Frame::IPoint& point)
    {
        return currentFrame->getPoint (index, point);
    }
    const Array<Frame::IPoint>& getPoints() { return currentFrame->getPoints(); }

    bool getIldaShowBlanked() { return ildaShowBlanked; }
    bool getIldaDrawLines() { return ildaDrawLines; }
    const SparseSet<uint16>& getIldaSelection() { return ildaSelection; }
    void getCenterOfIldaSelection (int16& x, int16& y, int16& z);
    const Point<int> getComponentCenterOfIldaSelection();
    void getComponentCenterOfIldaSelection (int& x, int& y);
    
    void getIldaSelectedPoints (Array<Frame::IPoint>& points);
    void getIldaPoints (const SparseSet<uint16>& selection, Array<Frame::IPoint>& points);

    const Image& getCurrentThumbNail() { return currentFrame->getThumbNail(); }
    const Image& getThumbNail (uint16 index) { return Frames[index]->getThumbNail(); }
    
    int getIPathCount() { return currentFrame->getIPathCount(); }
    const IPath getIPath (int index) { return currentFrame->getIPath (index); }
    const Array<IPath>& getIPaths() { return currentFrame->getIPaths(); }
    void getSelectedIPaths (Array<IPath>& paths)
            { getIPaths (iPathSelection, paths); }
    void getIPaths (const IPathSelection& selection, Array<IPath>& paths);
    IPathSelection& getIPathSelection() { return iPathSelection; }
    void getCenterOfIPathSelection (int& x, int& y);
    
    // Undoable Commands
    // Transform operaitons must be proceeded with startTransform
    // and ended with endTransform
    void startTransform (const String& name);
    bool isTransforming() { return tranformInProgress; }
    
    // ILDA Transforms
    bool scaleIldaSelected (float xScale, float yScale, float zScale, bool centerOnSelection, bool constrain = true);
    bool rotateIldaSelected (float xAngle, float yAngle, float zAngle, bool centerOnSelection, bool constrain = true);
    bool shearIldaSelected (float shearX, float shearY, bool centerOnSelection, bool constrain = true);
    bool translateIldaSelected (int xOffset, int yOffset, int zOffset, bool constrain = true);
    bool barberPoleIldaSelected (float radius, float skew, float zAngle, bool centeredOnSelection, bool constrain = true);
    bool bulgeIldaSelected (float radius, float gain, bool centeredOnSelection, bool constrain = true);
    bool spiralIldaSelected (float angle, int size, bool centerOnSelection, bool constrain = true);
    bool sphereIldaSelected (double xScale, double yScale, double rScale, bool centerOnSelection, bool constrain = true);
    bool gradientIldaSelected (const Colour& color1, const Colour& color2, float angle, float length, bool radial, bool centerOnSelection, const Colour& color3 = Colours::transparentBlack);
    bool adjustHueIldaSelected (float hshift, float saturation, float brightness);
    
    // Sketch Transforms
    bool scaleSketchSelected (float xScale, float yScale, bool centerOnSelection, bool constrain = true);
    bool rotateSketchSelected (float zAngle, bool centerOnSelection, bool constrain = true);
    bool shearSketchSelected (float shearX, float shearY, bool centerOnSelection, bool constrain = true);
    bool translateSketchSelected (int xOffset, int yOffset, bool constrain = true);
    void endTransform();

    // Non transform (atomic) undoable operations
    void setActiveLayer (Layer layer);
    void setActiveView (View view);
    void setSketchVisible (bool visible);
    void setIldaVisible (bool visible);
    void setRefVisible (bool visible);
    
    void setActiveIldaTool (IldaTool tool);
    void setPointToolColor (const Colour& color);
    void togglePointToolBlank();
    void cyclePointToolColors();

    void cut();
    void paste();

    void setActiveSketchTool (SketchTool tool);
    void setSketchToolColor (const Colour& color);
    void toggleSketchToolBlank();
    void cycleSketchToolColors();

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
    void adjustIldaSelection (int offset);

    bool moveIldaSelected (int xOffset, int yOffset, bool constrain = true)
        { return moveIldaSelected (xOffset, yOffset, 0, constrain); }
    bool moveIldaSelected (int xOffset, int yOffset, int zOffset, bool constrain = true);
    bool centerIldaSelected (bool doX = true, bool doY = true, bool doZ = true, bool constrain = true);
    void duplicateIldaSelected();
    void anchorIldaSelected();
    
    void setIldaSelectedX (int16 newX);
    void setIldaSelectedY (int16 newY);
    void setIldaSelectedZ (int16 newZ);
    void setIldaSelectedR (uint8 newR);
    void setIldaSelectedG (uint8 newR);
    void setIldaSelectedB (uint8 newR);
    void setIldaSelectedRGB (const Colour newColor);
    
    void insertPoint (const Frame::IPoint& point);
    void deletePoints();

    void setIPathSelection (const IPathSelection& selection);
    void deletePaths();
    void deleteAnchor();
    int insertEllipsePath (const Rectangle<int>& rect);
    int insertRectPath (const Rectangle<int>& rect);
    int insertPath (Point<int> firstAnchor);
    int insertAnchor (Point<int> location);
    void insertControls (Point<int> location);
    void forceAnchorCurved();
    void forceAnchorStraight();
    void selectEntry();
    void selectExit();
    void zeroExitControl();
    bool moveSketchSelected (int xOffset, int yOffset, bool constrain = true);
    void renderSketch (bool shortestPath, bool updateSketch = false);
    
    void setSketchSelectedSpacing (uint16 newSpacing);
    void setSketchSelectedExtraPerAnchor (uint16 extra);
    void setSketchSelectedBlankingBefore (uint16 points);
    void setSketchSelectedBlankingAfter (uint16 points);
    void setSketchSelectedColor (const Colour& color);
    
    bool centerSketchSelected (bool doX = true, bool doY = true, bool constrain = true);

    // Destructive Version (invoked by UndoManager)
    void _setLoadedFile (const File& file) { loadedFile = file; }
    void _setZoomFactor (float zoom);
    void _setActiveLayer (Layer layer);
    void _setActiveView (View view);
    void _setSketchVisible (bool visible);
    void _setIldaVisible (bool visible);
    void _setRefVisible (bool visible);

    void _setActiveIldaTool (IldaTool tool);
    void _setPointToolColor (const Colour& color);

    void _setActiveSketchTool (SketchTool tool);
    void _setSketchToolColor (const Colour& color);

    void _insertPoint (uint16 index, const Frame::IPoint& point);
    void _setPoints (const Array<Frame::IPoint>& points);
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
                         const Array<Frame::IPoint>& points);

    void _setIPathSelection (const IPathSelection& selection);
    void _deletePath (int index);
    void _insertPath (int index, IPath& path);
    void _setPaths (const IPathSelection& selection, const Array<IPath>& paths);
    void _setIPaths (const Array<IPath>& paths);
    void _deleteAnchor (int pindex, int aindex);
    void _insertAnchor (int pindex, int aindex, const Anchor& a);
    
private:
    File loadedFile;
    uint32 dirtyCounter;
    uint32 scanRate;
    float zoomFactor;
    Layer activeLayer;
    View activeView;
    IldaTool activeIldaTool;
    Colour pointToolColor;
    Colour lastVisiblePointToolColor;
    SketchTool activeSketchTool;
    Colour sketchToolColor;
    Colour lastVisibleSketchToolColor;
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
    
    bool tranformInProgress;
    bool transformUsed;
    Array<Frame::IPoint> transformPoints;
    Array<IPath> transformPaths;
    int16 transformCenterX;
    int16 transformCenterY;
    int16 transformCenterZ;
    int transformSketchCenterX;
    int transformSketchCenterY;
    String transformName;
    
    IPathSelection iPathSelection;
    Array<IPath> iPathCopy;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameEditor)
};

