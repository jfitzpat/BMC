/*
    FrameUndo.h
    Frame Editor UndoableAction classes
 
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

class UndoableSetLayer : public UndoableAction
{
public:
    UndoableSetLayer (FrameEditor* editor, FrameEditor::Layer layer)
    : newLayer(layer), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldLayer = frameEditor->getActiveLayer();
        frameEditor->_setActiveLayer (newLayer);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setActiveLayer (oldLayer);
        return true;
    }
    
private:
    FrameEditor::Layer oldLayer;
    FrameEditor::Layer newLayer;
    FrameEditor* frameEditor;
};

class UndoableSetView : public UndoableAction
{
public:
    UndoableSetView (FrameEditor* editor, FrameEditor::View view)
    : newView(view), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldView = frameEditor->getActiveView();
        frameEditor->_setActiveView (newView);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setActiveView (oldView);
        return true;
    }
    
private:
    FrameEditor::View oldView;
    FrameEditor::View newView;
    FrameEditor* frameEditor;
};

class UndoableSetIldaTool : public UndoableAction
{
public:
    UndoableSetIldaTool (FrameEditor* editor, FrameEditor::IldaTool tool)
    : newTool (tool), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldTool = frameEditor->getActiveIldaTool();
        frameEditor->_setActiveIldaTool (newTool);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setActiveIldaTool (oldTool);
        return true;
    }
    
private:
    FrameEditor::IldaTool oldTool;
    FrameEditor::IldaTool newTool;
    FrameEditor* frameEditor;
};

class UndoableSetPointToolColor : public UndoableAction
{
public:
    UndoableSetPointToolColor (FrameEditor* editor, const Colour& color)
    : newColor (color), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldColor = frameEditor->getPointToolColor();
        frameEditor->_setPointToolColor (newColor);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setPointToolColor (oldColor);
        return true;
    }
    
private:
    Colour oldColor;
    Colour newColor;
    FrameEditor* frameEditor;
};

class UndoableSetSketchTool : public UndoableAction
{
public:
    UndoableSetSketchTool (FrameEditor* editor, FrameEditor::SketchTool tool)
    : newTool (tool), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldTool = frameEditor->getActiveSketchTool();
        frameEditor->_setActiveSketchTool (newTool);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setActiveSketchTool (oldTool);
        return true;
    }
    
private:
    FrameEditor::SketchTool oldTool;
    FrameEditor::SketchTool newTool;
    FrameEditor* frameEditor;
};

class UndoableSetSketchToolColor : public UndoableAction
{
public:
    UndoableSetSketchToolColor (FrameEditor* editor, const Colour& color)
    : newColor (color), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldColor = frameEditor->getSketchToolColor();
        frameEditor->_setSketchToolColor (newColor);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setSketchToolColor (oldColor);
        return true;
    }
    
private:
    Colour oldColor;
    Colour newColor;
    FrameEditor* frameEditor;
};

class UndoableSetSketchVisibility : public UndoableAction
{
public:
    UndoableSetSketchVisibility (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getSketchVisible();
        frameEditor->_setSketchVisible (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setSketchVisible (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableSetIldaVisibility : public UndoableAction
{
public:
    UndoableSetIldaVisibility (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getIldaVisible();
        frameEditor->_setIldaVisible (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setIldaVisible (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableSetRefVisibility : public UndoableAction
{
public:
    UndoableSetRefVisibility (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getRefVisible();
        frameEditor->_setRefVisible (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setRefVisible (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableSetImage : public UndoableAction
{
public:
    UndoableSetImage (FrameEditor* editor, const MemoryBlock& file)
    : newFile (file), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldFile = frameEditor->getImageData();
        return frameEditor->_setImageData (newFile);
    }
    
    bool undo() override
    {
        frameEditor->decDirtyCounter();
        return frameEditor->_setImageData (oldFile);
    }
    
private:
    MemoryBlock oldFile;
    MemoryBlock newFile;
    FrameEditor* frameEditor;
};

class UndoableSetRefAlpha : public UndoableAction
{
public:
    UndoableSetRefAlpha (FrameEditor* editor, float alpha)
    : newAlpha (alpha), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldAlpha = frameEditor->getImageOpacity();
        frameEditor->_setImageOpacity (newAlpha);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setImageOpacity (oldAlpha);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    float oldAlpha;
    float newAlpha;
    FrameEditor* frameEditor;
};

class UndoableSetImageScale : public UndoableAction
{
public:
    UndoableSetImageScale (FrameEditor* editor, float scale)
    : newScale (scale), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldScale = frameEditor->getImageScale();
        frameEditor->_setImageScale (newScale);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setImageScale (oldScale);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    float oldScale;
    float newScale;
    FrameEditor* frameEditor;
};

class UndoableSetImageRotation : public UndoableAction
{
public:
    UndoableSetImageRotation (FrameEditor* editor, float rot)
    : newRot (rot), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldRot = frameEditor->getImageRotation();
        frameEditor->_setImageRotation(newRot);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setImageRotation (oldRot);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    float oldRot;
    float newRot;
    FrameEditor* frameEditor;
};

class UndoableSetImageXoffset : public UndoableAction
{
public:
    UndoableSetImageXoffset (FrameEditor* editor, float rot)
    : newRot (rot), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldRot = frameEditor->getImageXoffset();
        frameEditor->_setImageXoffset(newRot);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setImageXoffset (oldRot);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    float oldRot;
    float newRot;
    FrameEditor* frameEditor;
};

class UndoableSetImageYoffset : public UndoableAction
{
public:
    UndoableSetImageYoffset (FrameEditor* editor, float rot)
    : newRot (rot), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldRot = frameEditor->getImageYoffset();
        frameEditor->_setImageYoffset(newRot);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setImageYoffset (oldRot);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    float oldRot;
    float newRot;
    FrameEditor* frameEditor;
};

class UndoableLoadFile : public UndoableAction
{
public:
    UndoableLoadFile (FrameEditor* editor, const ReferenceCountedArray<Frame> frames, const File& file)
    : newFrames (frames), newFile (file), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldIndex = frameEditor->getFrameIndex();
        oldFrames = frameEditor->getFrames();
        oldFile = frameEditor->getLoadedFile();
        oldDirtyCounter = frameEditor->getDirtyCounter();
        frameEditor->_setLoadedFile (newFile);
        frameEditor->_setFrames (newFrames);
        frameEditor->_setFrameIndex (0);
        frameEditor->setDirtyCounter (0);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setLoadedFile (oldFile);
        frameEditor->_setFrames (oldFrames);
        frameEditor->_setFrameIndex (oldIndex);
        frameEditor->setDirtyCounter (oldDirtyCounter);
        return true;
    }
    
private:
    uint16 oldIndex;
    ReferenceCountedArray<Frame> oldFrames;
    uint32 oldDirtyCounter;
    File oldFile;
    ReferenceCountedArray<Frame> newFrames;
    File newFile;
    FrameEditor* frameEditor;
};

class UndoableSetIldaShowBlanked : public UndoableAction
{
public:
    UndoableSetIldaShowBlanked (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getIldaShowBlanked();
        frameEditor->_setIldaShowBlanked (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setIldaShowBlanked (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableSetIldaDrawLines : public UndoableAction
{
public:
    UndoableSetIldaDrawLines (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getIldaDrawLines();
        frameEditor->_setIldaDrawLines (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setIldaDrawLines (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableSetRefDrawGrid : public UndoableAction
{
public:
    UndoableSetRefDrawGrid (FrameEditor* editor, bool visibility)
    : newVisibility (visibility), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldVisibility = frameEditor->getRefDrawGrid();
        frameEditor->_setDrawGrid (newVisibility);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setDrawGrid (oldVisibility);
        return true;
    }
    
private:
    bool oldVisibility;
    bool newVisibility;
    FrameEditor* frameEditor;
};

class UndoableInsertPoint : public UndoableAction
{
public:
    UndoableInsertPoint (FrameEditor* editor, uint16 index, const Frame::IPoint& point)
    : pointIndex (index), newPoint (point), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        frameEditor->_insertPoint (pointIndex, newPoint);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_deletePoint (pointIndex);
        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    uint16 pointIndex;
    Frame::IPoint newPoint;
    FrameEditor* frameEditor;
};

class UndoableDeletePaths : public UndoableAction
{
public:
    UndoableDeletePaths (FrameEditor* editor,
                         const SparseSet<uint16>& select)
    : selection (select), frameEditor (editor) {;}

    bool perform() override
    {
        frameEditor->incDirtyCounter();

        // Collect the selected paths
        paths.clear();
        for (auto n = 0; n < selection.getNumRanges(); ++n)
        {
            Range<uint16> r = selection.getRange (n);
            for (auto i = r.getStart(); i < r.getEnd(); ++i)
                paths.add (frameEditor->getIPath (i));
        }

        // Loop backwards through selection to delete
        for (auto n = selection.getNumRanges() - 1; n >= 0; --n)
        {
            Range<uint16> r = selection.getRange (n);
            for (auto i = r.getEnd() - 1; i >= r.getStart(); --i)
                frameEditor->_deletePath (i);
        }
        return true;
    }
    
    bool undo() override
    {
        int pindex = 0;
        
        // Loop forwards to insert
        for (auto n = 0; n < selection.getNumRanges(); ++n)
        {
            Range<uint16> r = selection.getRange (n);
            for (auto i = r.getStart(); i < r.getEnd(); ++i)
                frameEditor->_insertPath (i, paths.getObjectPointer(pindex++));
        }

        frameEditor->decDirtyCounter();
        return true;
    }
    
private:
    ReferenceCountedArray<IPath> paths;
    SparseSet<uint16> selection;
    FrameEditor* frameEditor;
};

class UndoableDeletePoints : public UndoableAction
{
    public:
        UndoableDeletePoints (FrameEditor* editor,
                              const SparseSet<uint16>& select)
        : selection (select), frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            frameEditor->getIldaPoints (selection, oldPoints);
            
            // Loop backwards through selection to delete
            for (auto n = selection.getNumRanges() - 1; n >= 0; --n)
            {
                Range<uint16> r = selection.getRange (n);
                for (auto i = r.getEnd() - 1; i >= r.getStart(); --i)
                    frameEditor->_deletePoint ((uint16)i);
            }
            return true;
        }
        
        bool undo() override
        {
            int pindex = 0;
            
            // Loop forwards to insert
            for (auto n = 0; n < selection.getNumRanges(); ++n)
            {
                Range<uint16> r = selection.getRange (n);
                for (auto i = r.getStart(); i < r.getEnd(); ++i)
                    frameEditor->_insertPoint (i, oldPoints[pindex++]);
            }
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        SparseSet<uint16> selection;
        Array<Frame::IPoint> oldPoints;
        FrameEditor* frameEditor;
};

class UndoableSetFrameIndex : public UndoableAction
{
public:
    UndoableSetFrameIndex (FrameEditor* editor, uint16 index)
    : newIndex (index), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldIndex = frameEditor->getFrameIndex();
        frameEditor->_setFrameIndex (newIndex);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setFrameIndex (oldIndex);
        return true;
    }
    
private:
    uint16 oldIndex;
    uint16 newIndex;
    FrameEditor* frameEditor;
};

class UndoableDeleteFrame : public UndoableAction
{
    public:
        UndoableDeleteFrame (FrameEditor* editor, uint16 index)
        : delIndex (index), frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            oldFrame = frameEditor->getFrame (delIndex);
            frameEditor->_deleteFrame (delIndex);
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_insertFrame (delIndex, oldFrame);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        Frame::Ptr oldFrame;
        uint16 delIndex;
        FrameEditor* frameEditor;
};

class UndoableNewFrame : public UndoableAction
{
    public:
        UndoableNewFrame (FrameEditor* editor)
        : frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            index = frameEditor->getFrameIndex() + 1;
            frameEditor->_newFrame();
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_deleteFrame (index);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        uint16 index;
        FrameEditor* frameEditor;
};

class UndoableDupFrame : public UndoableAction
{
    public:
        UndoableDupFrame (FrameEditor* editor)
        : frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            index = frameEditor->getFrameIndex() + 1;
            frameEditor->_dupFrame();
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_deleteFrame (index);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        uint16 index;
        FrameEditor* frameEditor;
};

class UndoableSwapFrames : public UndoableAction
{
    public:
        UndoableSwapFrames (FrameEditor* editor, uint16 _index1, uint16 _index2)
        : index1 (_index1), index2 (_index2), frameEditor (editor)  {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            frameEditor->_swapFrames(index1, index2);
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_swapFrames(index1, index2);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        uint16 index1;
        uint16 index2;
        FrameEditor* frameEditor;
};

class UndoableSetIldaSelection : public UndoableAction
{
public:
    UndoableSetIldaSelection (FrameEditor* editor, const SparseSet<uint16>& select)
    : newSelect (select), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldSelect = frameEditor->getIldaSelection();
        frameEditor->_setIldaSelection (newSelect);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setIldaSelection (oldSelect);
        return true;
    }
    
private:
    SparseSet<uint16> oldSelect;
    SparseSet<uint16> newSelect;
    FrameEditor* frameEditor;
};

class UndoableSetIldaPoints : public UndoableAction
{
    public:
        UndoableSetIldaPoints (FrameEditor* editor,
                               const SparseSet<uint16>& select,
                               const Array<Frame::IPoint> points)
        : selection (select), newPoints (points), frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            frameEditor->getIldaPoints (selection, oldPoints);
            frameEditor->_setIldaPoints (selection, newPoints);
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_setIldaPoints (selection, oldPoints);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        SparseSet<uint16> selection;
        Array<Frame::IPoint> oldPoints;
        Array<Frame::IPoint> newPoints;
        FrameEditor* frameEditor;
};

class UndoableSetIPathSelection : public UndoableAction
{
public:
    UndoableSetIPathSelection (FrameEditor* editor, const SparseSet<uint16>& select)
    : newSelect (select), frameEditor (editor) {;}
    
    bool perform() override
    {
        oldSelect = frameEditor->getIPathSelection();
        frameEditor->_setIPathSelection (newSelect);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setIPathSelection (oldSelect);
        return true;
    }
    
private:
    SparseSet<uint16> oldSelect;
    SparseSet<uint16> newSelect;
    FrameEditor* frameEditor;
};

class UndoableSetPaths : public UndoableAction
{
    public:
        UndoableSetPaths (FrameEditor* editor,
                          const SparseSet<uint16>& select,
                          const ReferenceCountedArray<IPath>& paths)
        : selection (select), newPaths (paths), frameEditor (editor) {;}
        
        bool perform() override
        {
            frameEditor->incDirtyCounter();
            frameEditor->getIPaths (selection, oldPaths);
            frameEditor->_setPaths (selection, newPaths);
            return true;
        }
        
        bool undo() override
        {
            frameEditor->_setPaths (selection, oldPaths);
            frameEditor->decDirtyCounter();
            return true;
        }
        
    private:
        SparseSet<uint16> selection;
        ReferenceCountedArray<IPath> oldPaths;
        ReferenceCountedArray<IPath> newPaths;
        FrameEditor* frameEditor;
};
