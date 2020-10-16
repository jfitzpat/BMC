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
                Frame::IPoint point;
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
        
        // Copy Paths for this frame
        var paths = frameData->getProperty (JSEFile::iPaths);
        if (paths.isArray())
        {
            for (auto i = 0; i < paths.getArray()->size(); ++i)
            {
                IPath path;
                DynamicObject* pathData = paths[i].getDynamicObject();
                if (pathData == nullptr)
                    return false;
                
                Colour c = Colour ((uint8)(int)pathData->getProperty (JSEFile::iPathRed),
                                   (uint8)(int)pathData->getProperty (JSEFile::iPathGreen),
                                   (uint8)(int)pathData->getProperty (JSEFile::iPathBlue));
                
                path.setColor (c);
                path.setPointDensity ((uint16)(int)pathData->getProperty (JSEFile::iPathDensity));
                path.setExtraPointsPerAnchor ((uint16)(int)pathData->getProperty (JSEFile::ExtraPerAnchor));
                path.setBlankedPointsBeforeStart ((uint16)(int)pathData->getProperty (JSEFile::BlanksBefore));
                path.setBlankedPointsAfterEnd ((uint16)(int)pathData->getProperty (JSEFile::BlanksAfter));
                
                var anchors = pathData->getProperty (JSEFile::Anchors);
                if (anchors.isArray())
                {
                    for (auto j = 0; j < anchors.getArray()->size(); ++j)
                    {
                        DynamicObject* anchorData = anchors[j].getDynamicObject();
                        if (anchorData == nullptr)
                            return false;
                        
                        int x = anchorData->getProperty (JSEFile::AnchorX);
                        int y = anchorData->getProperty (JSEFile::AnchorY);
                        int enX = anchorData->getProperty (JSEFile::AnchorEntryX);
                        int enY = anchorData->getProperty (JSEFile::AnchorEntryY);
                        int exX = anchorData->getProperty (JSEFile::AnchorExitX);
                        int exY = anchorData->getProperty (JSEFile::AnchorExitY);

                        path.addAnchor (Anchor (x, y, enX, enY, exX, exY));
                    }
                }
                
                frame->addPath (path);
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
