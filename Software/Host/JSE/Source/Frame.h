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
#include "IPath.h"
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

    typedef ILDA_FORMAT_4 IPoint;
    typedef enum {
        BlankedPoint = 0x40,
        LastPoint = 0x80
    } Status;
    
    uint16 getPointCount() { return (uint16)framePoints.size(); }
    
    bool getPoint (uint16 index, IPoint& point);
    IPoint getPoint (uint16 index) { return framePoints[index]; }
    
    void addPoint (IPoint& point);
    void replacePoint (uint16 index, const IPoint& newPoint);
    void insertPoint (uint16 index, const IPoint& newPoint);
    void removePoint (uint16 index);
  
    int getIPathCount() { return iPaths.size(); }
    const IPath getIPath (int index) { return iPaths[index]; }
    void addPath (IPath& p) { iPaths.add (p); }
    void deletePath (int index) { iPaths.remove (index); }
    void insertPath (int index, const IPath& p) { iPaths.insert (index, p); }
    void replacePath (int index, const IPath& p) { iPaths.set (index, p); }
    
    void buildThumbNail (int width = 150, int height = 150, float lineSize = 1.0);
    const Image& getThumbNail() { return thumbNail; }
    
    // Make a counting pointer of our type
    using Ptr = ReferenceCountedObjectPtr<Frame>;
    
    // Conversion helpers
    typedef enum {
        front = 0,
        bottom = 1,
        left = 2
    } ViewAngle;
    
    static float getCompX (const IPoint& point, ViewAngle view = front)
    {
        if (view == left)
            return (float)((float)point.z.w + 32768.0f);
        else
            return (float)((float)point.x.w + 32768.0f);
    }
    static float getCompY (const IPoint& point, ViewAngle view = front)
    {
        if (view == bottom)
            return (float)(32767.0f - (float)point.z.w);
        else
            return (float)(32767.0f - (float)point.y.w);
    }
    static int getCompXInt (const IPoint& point, ViewAngle view = front)
    {
        if (view == left)
            return (int)((int)point.z.w + 32768);
        else
            return (int)((int)point.x.w + 32768);
    }
    static int getCompYInt (const IPoint& point, ViewAngle view = front)
    {
        if (view == bottom)
            return (int)(32767 - (int)point.z.w);
        else
            return (int)(32767 - (int)point.y.w);
    }
    static int16 getIldaX (const Point<int>& point) { return (int16)(point.getX() - 32768); }
    static int16 getIldaY (const Point<int>& point) { return (int16)(32767 - point.getY()); }
    static int getIldaX (const Rectangle<int>& rect) { return rect.getX() - 32768; }
    static int getIldaY (const Rectangle<int>& rect) { return 32767 - (rect.getY() + rect.getHeight()); }
    static Rectangle<int> getIldaRect (Rectangle<int>& r)
    {
        return Rectangle<int> (getIldaX(r), getIldaY(r), r.getWidth(), r.getHeight());
    }
    static Point<int> getCompPoint (const IPoint& point, ViewAngle view = front)
    {
        return Point<int>(getCompXInt (point, view), getCompYInt (point, view));
    }
    static int toIldaX (int x) { return x - 32768; }
    static int toIldaY (int y) { return 32767 - y; }
    static int toCompX (int x) { return x + 32768; }
    static int toCompY (int y) { return 32767 - y; }
    static void blankPoint (IPoint& point)
    {
        point.status = Frame::BlankedPoint;
        point.red = point.green = point.blue = 0;
    }
    static bool clipIlda (int& val)
    {
        bool clipped = false;
        if (val < -32768)
        {
            val = -32768;
            clipped = true;
        }
        else if (val > 32767)
        {
            val = 32767;
            clipped = true;
        }
        
        return clipped;
    }
    
    static bool clipSketch (int& val)
    {
        bool clipped = false;
        if (val < 0)
        {
            val = 0;
            clipped = true;
        }
        else if (val > 0xFFFF)
        {
            val = 0xFFFF;
            clipped = true;
        }
        
        return clipped;
    }

private:
    std::unique_ptr<Image> backgroundImage;
    MemoryBlock imageData;
    float imageOpacity;
    float imageScale;
    float imageRotation;
    float imageXoffset;
    float imageYoffset;
    
    Array<IPoint> framePoints;
    Array<IPath> iPaths;

    Image thumbNail;
};
