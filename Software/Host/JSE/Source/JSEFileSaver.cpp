/*
    JSEFileSaver.cpp
    Save a JSE JSON file
 
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
#include "JSEFileSaver.h"

FrameEditor* JSEFileSaver::frameEditor;

bool JSEFileSaver::save (FrameEditor* editor, File& file)
{
    frameEditor = editor;
    DynamicObject::Ptr fileObj = new DynamicObject();
    
    fileObj->setProperty (JSEFile::AppVersion, ProjectInfo::versionString);
    fileObj->setProperty (JSEFile::FileVersion, JSE_FILE_VERSION);
    fileObj->setProperty (JSEFile::FrameCount, frameEditor->getFrameCount());
    
    var frames;
    for (uint16 n = 0; n < frameEditor->getFrameCount(); ++n)
        frames.append (frameToObj (n));
    
    fileObj->setProperty (JSEFile::Frames, frames);
    
    var json (fileObj.get());
    
    if (file.exists())
        file.deleteFile();
    
    FileOutputStream output(file);
    if (output.openedOk())
    {
        GZIPCompressorOutputStream z (output);
        JSON::writeToStream(z, json);
        z.flush();
        output.flush();
        return true;
    }
    
    return false;
}

var JSEFileSaver::frameToObj (uint16 frameIndex)
{
    DynamicObject* obj = new DynamicObject();

    obj->setProperty (JSEFile::ImageFileSize,
                      (int)frameEditor->getFrames()[frameIndex]->getImageData().getSize());
    
    obj->setProperty (JSEFile::ImageFile, frameEditor->getFrames()[frameIndex]->getImageData().toBase64Encoding());

    obj->setProperty (JSEFile::ImageOpacity, frameEditor->getFrames()[frameIndex]->getImageOpacity());
    obj->setProperty (JSEFile::ImageScale, frameEditor->getFrames()[frameIndex]->getImageScale());
    obj->setProperty (JSEFile::ImageRotation, frameEditor->getFrames()[frameIndex]->getImageRotation());
    obj->setProperty (JSEFile::ImageXOffset, frameEditor->getFrames()[frameIndex]->getImageXoffset());
    obj->setProperty (JSEFile::ImageYOffset, frameEditor->getFrames()[frameIndex]->getImageYoffset());
  
    obj->setProperty (JSEFile::PointCount, frameEditor->getFrames()[frameIndex]->getPointCount());
    
    var points;
    for (uint16 i = 0; i < frameEditor->getFrames()[frameIndex]->getPointCount(); ++i)
        points.append (pointToObj (frameIndex, i));
    
    obj->setProperty (JSEFile::Points, points);
    
    var paths;
    for (uint16 i = 0; i < frameEditor->getFrames()[frameIndex]->getIPathCount(); ++i)
        paths.append (ipathToObj (frameIndex, i));
    
    obj->setProperty (JSEFile::iPaths, paths);
    return var (obj);
}

var JSEFileSaver::pointToObj (uint16 frameIndex, uint16 pointIndex)
{
    Frame::IPoint point;
    
    frameEditor->getFrames()[frameIndex]->getPoint(pointIndex, point);
    
    DynamicObject* obj = new DynamicObject();
    obj->setProperty (JSEFile::PointX,      point.x.w);
    obj->setProperty (JSEFile::PointY,      point.y.w);
    obj->setProperty (JSEFile::PointZ,      point.z.w);
    obj->setProperty (JSEFile::PointRed,    point.red);
    obj->setProperty (JSEFile::PointGreen,  point.green);
    obj->setProperty (JSEFile::PointBlue,   point.blue);
    obj->setProperty (JSEFile::PointStatus, point.status);

    return var (obj);
}

var JSEFileSaver::ipathToObj (uint16 frameIndex, uint16 pathIndex)
{
    IPath path = frameEditor->getFrames()[frameIndex]->getIPath (pathIndex);
    DynamicObject* obj = new DynamicObject();

    Colour c = path.getColor();
    obj->setProperty (JSEFile::iPathRed, c.getRed());
    obj->setProperty (JSEFile::iPathGreen, c.getGreen());
    obj->setProperty (JSEFile::iPathBlue, c.getBlue());

    obj->setProperty (JSEFile::iPathDensity, path.getPointDensity());
    obj->setProperty (JSEFile::ExtraPerAnchor, path.getExtraPointsPerAnchor());
    obj->setProperty (JSEFile::BlanksBefore, path.getBlankedPointsBeforeStart());
    obj->setProperty (JSEFile::BlanksAfter, path.getBlankedPointsAfterEnd());

    var anchors;
    for (uint16 i = 0; i < path.getAnchorCount(); ++i)
    {
        Anchor a = path.getAnchor (i);
        anchors.append (anchorToObj (a));
    }
    obj->setProperty (JSEFile::Anchors, anchors);
    return var (obj);
}

var JSEFileSaver::anchorToObj (Anchor& a)
{
    DynamicObject* obj = new DynamicObject();

    obj->setProperty (JSEFile::AnchorX, a.getX());
    obj->setProperty (JSEFile::AnchorY, a.getY());
    obj->setProperty (JSEFile::AnchorEntryX, a.getEntryXDelta());
    obj->setProperty (JSEFile::AnchorEntryY, a.getEntryYDelta());
    obj->setProperty (JSEFile::AnchorExitX, a.getExitXDelta());
    obj->setProperty (JSEFile::AnchorExitY, a.getExitYDelta());

    return var (obj);
}
