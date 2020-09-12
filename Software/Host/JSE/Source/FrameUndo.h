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
    UndoableSetImage (FrameEditor* editor, File& file)
    : newFile (file), frameEditor (editor) {;}
    
    bool perform() override
    {
        frameEditor->incDirtyCounter();
        oldFile = frameEditor->getImageFile();
        return frameEditor->_setImage (newFile);
    }
    
    bool undo() override
    {
        frameEditor->decDirtyCounter();
        return frameEditor->_setImage (oldFile);
    }
    
private:
    File oldFile;
    File newFile;
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
        oldAlpha = frameEditor->getRefOpacity();
        frameEditor->_setRefOpacity (newAlpha);
        return true;
    }
    
    bool undo() override
    {
        frameEditor->_setRefOpacity (oldAlpha);
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
                               const Array<Frame::XYPoint> points)
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
        Array<Frame::XYPoint> oldPoints;
        Array<Frame::XYPoint> newPoints;
        FrameEditor* frameEditor;
};
