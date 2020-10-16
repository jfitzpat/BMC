/*
    Frame.cpp
    Editable Frame Container
 
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

#include "ThumbBuilder.h"
#include "Frame.h"

//==============================================================================
Frame::Frame()
: imageOpacity (1.0),
  imageScale (1.0),
  imageRotation (0.0),
  imageXoffset (0.0),
  imageYoffset (0.0)
{
}

Frame::Frame (const Frame& frame)
{
    imageOpacity = frame.imageOpacity;
    imageScale = frame.imageScale;
    imageRotation = frame.imageRotation;
    imageXoffset = frame.imageXoffset;
    imageYoffset = frame.imageYoffset;
    framePoints = frame.framePoints;
    thumbNail = frame.thumbNail;
    imageData = frame.imageData;

    for (auto n = 0; n < frame.iPaths.size(); ++n)
    iPaths.add (new IPath (*frame.iPaths.getObjectPointer (n)));
    
    Image i = ImageFileFormat::loadFrom(imageData.getData(), imageData.getSize());
    backgroundImage.reset (new Image(i));
}

Frame::~Frame()
{
}

void Frame::setImageData (const MemoryBlock& data)
{
    imageData = data;
    Image i = ImageFileFormat::loadFrom(imageData.getData(), imageData.getSize());
    backgroundImage.reset (new Image(i));
}

//==============================================================================
bool Frame::getPoint (uint16 index, IPoint& point)
{
    if (index >= framePoints.size())
        return false;
    
    point = framePoints[index];
    return true;
}

void Frame::addPoint (IPoint& point)
{
    framePoints.add (point);
}

void Frame::replacePoint (uint16 index, const IPoint& newPoint)
{
    if (index >= framePoints.size())
        return;
    
    framePoints.remove (index);
    framePoints.insert (index, newPoint);
}

void Frame::insertPoint (uint16 index, const IPoint& newPoint)
{
    if (index > framePoints.size())
        return;
    
    framePoints.insert (index, newPoint);
}

void Frame::removePoint (uint16 index)
{
    if (index >= framePoints.size())
        return;
    
    framePoints.remove (index);
}

void Frame::buildThumbNail (int width, int height, float lineSize)
{
    ThumbBuilder::build (this, thumbNail, width, height, lineSize);
}
