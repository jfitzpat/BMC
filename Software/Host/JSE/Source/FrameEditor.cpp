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
#include "FrameEditor.h"

#include "FrameUndo.h"      // UndoableTask classes

//==============================================================================
FrameEditor::FrameEditor()
    : activeLayer (sketch),
      sketchVisible (true),
      ildaVisible (true),
      refVisible (true),
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
void FrameEditor::setActiveLayer (Layer layer)
{
    if (layer != activeLayer)
    {
        beginNewTransaction ("Layer Change");
        perform(new UndoableSetLayer (this, layer));
    }
}

//==============================================================================
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

File FrameEditor::getImageFile()
{
    if (currentFrame == nullptr)
        return File();
    
    return currentFrame->getImageFile();
}

void FrameEditor::selectImage()
{
    if (currentFrame == nullptr)
        return;
    
    FileChooser myChooser ("Choose Image to Load...",
                           File::getSpecialLocation (File::userDocumentsDirectory),
                           "*.png,*.jpg,*.jpeg,*.gif");
 
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
            perform(new UndoableSetImage (this, f));
        }
    }
}

const Image* FrameEditor::getImage()
{
    if (currentFrame == nullptr)
        return nullptr;
    else
        return currentFrame->getBackgroundImage();
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

float FrameEditor::getImageScale()
{
    if (currentFrame == nullptr)
        return 1.0;
    
    return currentFrame->getImageScale();
}

void FrameEditor::setImageScale (float scale)
{
    if (currentFrame == nullptr)
        return;
    
    if (scale < 0.01)
        scale = 0.01;
    else if (scale > 2.00)
        scale = 2.00;
    
    if (scale != currentFrame->getImageScale())
    {
        if (getCurrentTransactionName() == "Background Scale")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Scale");
        perform(new UndoableSetImageScale (this, scale));
    }
}

float FrameEditor::getImageRotation()
{
    if (currentFrame == nullptr)
        return 0.0;
    
    return currentFrame->getImageRotation();
}

void FrameEditor::setImageRotation (float rot)
{
    if (currentFrame == nullptr)
        return;
    
    if (rot < 0)
        rot = 0;
    else if (rot > 359.9)
        rot = 359.9;
    
    if (rot != currentFrame->getImageRotation())
    {
        if (getCurrentTransactionName() == "Background Rotation")
            undoCurrentTransactionOnly();
        
        beginNewTransaction ("Background Rotation");
        perform(new UndoableSetImageRotation (this, rot));
    }
}

float FrameEditor::getImageXoffset()
{
    if (currentFrame == nullptr)
        return 0.0;
    
    return currentFrame->getImageXoffset();
}

void FrameEditor::setImageXoffset (float off)
{
    if (currentFrame == nullptr)
        return;
    
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

float FrameEditor::getImageYoffset()
{
    if (currentFrame == nullptr)
        return 0.0;
    
    return currentFrame->getImageYoffset();
}

void FrameEditor::setImageYoffset (float off)
{
    if (currentFrame == nullptr)
        return;
    
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
   FileChooser myChooser ("ILDA file to Load...",
                          File::getSpecialLocation (File::userDocumentsDirectory),
                          "*.ild");

   if (myChooser.browseForFileToOpen())
   {
       File f = myChooser.getResult();

       ReferenceCountedArray<Frame> frames;
       if (! IldaLoader::load (frames, f))
       {
           AlertWindow::showMessageBox(AlertWindow::WarningIcon, "File Error",
                                       "An error occurred loading the selected ILDA file.", "ok");
       }
       else
       {
           beginNewTransaction ("Load File");
           perform(new UndoableLoadFile (this, frames));
       }
   }
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

bool FrameEditor::_setImage (File& file)
{
    if (currentFrame == nullptr)
        return false;
    
    // A little sneaky, if the file went invalid, fail
    // But if it was an empty file to begin with, use it
    Image i = ImageFileFormat::loadFrom (file);
    if (i.isValid() || file.getFullPathName().length() == 0)
    {
        currentFrame->setImageFile (file);
        currentFrame->setOwnedBackgroundImage (new Image(i));
        sendActionMessage (EditorActions::backgroundImageChanged);
        return true;
    }
    
    return false;
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
    if (currentFrame == nullptr)
        return;
    
    if (scale < 0.01)
        scale = 0.01;
    else if (scale > 2.00)
        scale = 2.00;
    
    if (scale != currentFrame->getImageScale())
    {
        currentFrame->setImageScale (scale);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setImageRotation (float rot)
{
    if (currentFrame == nullptr)
        return;
    
    if (rot < 0)
        rot = 0;
    else if (rot > 359.9)
        rot = 359.9;
    
    if (rot != currentFrame->getImageRotation())
    {
        currentFrame->setImageRotation (rot);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}

void FrameEditor::_setImageXoffset (float off)
{
    if (currentFrame == nullptr)
        return;
    
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
    if (currentFrame == nullptr)
        return;
    
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
    sendActionMessage (EditorActions::frameIndexChanged);
}
