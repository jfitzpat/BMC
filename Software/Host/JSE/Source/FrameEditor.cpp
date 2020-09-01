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
#include "FrameEditor.h"

//==============================================================================
FrameEditor::FrameEditor()
    : activeLayer (sketch),
      sketchVisible (true),
      ildaVisible (true),
      refVisible (true),
      refOpacity(1.0)
{
    currentFrame.reset (new Frame());
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
        activeLayer = layer;
        sendActionMessage(EditorActions::layerChanged);
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
        ildaVisible = visible;
        sendActionMessage (EditorActions::ildaVisibilityChanged);
    }
}

void FrameEditor::setRefVisible (bool visible)
{
    if (visible != refVisible)
    {
        refVisible = visible;
        sendActionMessage (EditorActions::refVisibilityChanged);
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
            currentFrame->setImageFile (f);
            currentFrame->setOwnedBackgroundImage (new Image(i));
            sendActionMessage (EditorActions::backgroundImageChanged);
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
        refOpacity = opacity;
        sendActionMessage (EditorActions::refOpacityChanged);
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
        currentFrame->setImageScale (scale);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
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
        currentFrame->setImageRotation (rot);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
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
        currentFrame->setImageXoffset (off);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
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
        currentFrame->setImageYoffset (off);
        sendActionMessage (EditorActions::backgroundImageAdjusted);
    }
}
