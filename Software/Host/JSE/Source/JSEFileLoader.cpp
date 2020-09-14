/*
    JSEFileLoader.cpp
    Load a jse file as an array of Frame objects
 
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

#include "JSEFile.h"
#include "JSEFileLoader.h"

bool JSEFileLoader::load (ReferenceCountedArray<Frame>& frameArray, File& file)
{
    frameArray.clear();

    FileInputStream input (file);
    if (input.failedToOpen())
        return false;
    
    GZIPDecompressorInputStream z(input);

    var fileObj = JSON::parse (z);
    
    // JSE version?
    String s = fileObj.getDynamicObject()->getProperty (JSEFile::AppVersion);
    if (! s.length())
        return false;
    
    // !!!! Message?
    int fv = fileObj.getDynamicObject()->getProperty (JSEFile::FileVersion);
    if (fv != JSE_FILE_VERSION)
        return false;
    
    var frames = fileObj.getDynamicObject()->getProperty (JSEFile::Frames);
    if (! frames.isArray())
        return false;

    // Loop until we are out of frames
    for (auto n = 0; n < frames.getArray()->size(); ++n)
    {
        // New frame to store too
        Frame::Ptr frame = new Frame;

        // Frame data from file
        DynamicObject* frameData = frames[n].getDynamicObject();
        if (frameData == nullptr)
            return false;
        
        // Is there a reference image?
        if (frameData->getProperty (JSEFile::ImageFileSize))
        {
            MemoryBlock b;
            b.fromBase64Encoding (frameData->getProperty (JSEFile::ImageFile).toString());
            frame->setImageData (b);
        }
        
        frame->setImageOpacity (frameData->getProperty (JSEFile::ImageOpacity));
        frame->setImageScale (frameData->getProperty (JSEFile::ImageScale));
        frame->setImageRotation (frameData->getProperty (JSEFile::ImageRotation));
        frame->setImageXoffset (frameData->getProperty (JSEFile::ImageXOffset));
        frame->setImageYoffset (frameData->getProperty (JSEFile::ImageYOffset));

        // Copy Points for this frame
        var points = frameData->getProperty (JSEFile::Points);
        if (points.isArray())
        {
            for (auto i = 0; i < points.getArray()->size(); ++i)
            {
                Frame::XYPoint point;
                DynamicObject* pointData = points[i].getDynamicObject();
                if (pointData == nullptr)
                    return false;
                
                point.x.w = (uint16)(int)pointData->getProperty (JSEFile::PointX);
                point.y.w = (uint16)(int)pointData->getProperty (JSEFile::PointY);
                point.z.w = (uint16)(int)pointData->getProperty (JSEFile::PointZ);
                point.red = (uint8)(int)pointData->getProperty (JSEFile::PointRed);
                point.green = (uint8)(int)pointData->getProperty (JSEFile::PointGreen);
                point.blue = (uint8)(int)pointData->getProperty (JSEFile::PointBlue);
                point.status= (uint8)(int)pointData->getProperty (JSEFile::PointStatus);
                
                // Store the point
                frame->addPoint (point);
            }
        }

        // Build the thumbnail and add the frame
        frame->buildThumbNail();
        frameArray.add (frame);
    }

    if (frameArray.size())
        return true;
    
    return false;
}
