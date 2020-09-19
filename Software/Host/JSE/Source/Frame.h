/*
    Frame.h
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

#pragma once

#include <JuceHeader.h>
#include "ILDA.h"

// Reference Counted so we can keep frames around for undo and just have them
// clean up whenever those undo objects are released
class Frame : public ReferenceCountedObject
{
public:
    Frame();
    Frame (const Frame&t);
    ~Frame();
    
    const Image* getBackgroundImage() { return backgroundImage.get(); }
    void setImageData (const MemoryBlock& data);
    const MemoryBlock& getImageData() { return imageData; }
    
    float getImageOpacity()             { return imageOpacity; }
    void setImageOpacity (float opacity) { imageOpacity = opacity;}
    float getImageScale()               { return imageScale; }
    void setImageScale (float scale)    { imageScale = scale; }
    float getImageRotation()            { return imageRotation; }
    void setImageRotation (float rot)   { imageRotation = rot; }
    float getImageXoffset()             { return imageXoffset; }
    void setImageXoffset (float off)    { imageXoffset = off; }
    float getImageYoffset()             { return imageYoffset; }
    void setImageYoffset (float off)    { imageYoffset = off; }

    typedef ILDA_FORMAT_4 XYPoint;
    typedef enum {
        BlankedPoint = 0x40,
        LastPoint = 0x80
    } Status;
    
    uint16 getPointCount() { return (uint16)framePoints.size(); }
    
    bool getPoint (uint16 index, XYPoint& point);
    XYPoint getPoint (uint16 index) { return framePoints[index]; }
    
    void addPoint (XYPoint& point);
    void replacePoint (uint16 index, const XYPoint& newPoint);
    void insertPoint (uint16 index, const XYPoint& newPoint);
    void removePoint (uint16 index);
    
    void buildThumbNail (int width = 150, int height = 150, float lineSize = 1.0);
    const Image& getThumbNail() { return thumbNail; }
    
    // Make a counting pointer of our type
    using Ptr = ReferenceCountedObjectPtr<Frame>;
    
private:
    std::unique_ptr<Image> backgroundImage;
    MemoryBlock imageData;
    float imageOpacity;
    float imageScale;
    float imageRotation;
    float imageXoffset;
    float imageYoffset;
    
    Array<XYPoint> framePoints;
    
    Image thumbNail;
};
