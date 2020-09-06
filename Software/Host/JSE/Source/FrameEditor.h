/*
    FrameEditor.h
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

#pragma once

#include <JuceHeader.h>
#include "Frame.h"

//==============================================================================
class FrameEditor  : public ActionBroadcaster,
                     public UndoManager
{
public:
    FrameEditor();
    ~FrameEditor() override;

    typedef enum {
        sketch = 0,
        ilda,
        reference
    } Layer;

    // Polling
    Layer getActiveLayer() { return activeLayer; }
    bool getSketchVisible() { return sketchVisible; }
    bool getIldaVisible() { return ildaVisible; }
    bool getRefVisible() { return refVisible; }
    
    File getImageFile();
    const Image* getImage();
    float getRefOpacity() { return refOpacity; }
    bool getRefDrawGrid() { return refDrawGrid; }
    float getImageScale();
    float getImageRotation();
    float getImageXoffset();
    float getImageYoffset();
    
    const ReferenceCountedArray<Frame>& getFrames() { return Frames; }
    uint16 getFrameIndex() { return frameIndex; }
    
    uint16 getPointCount() { return currentFrame->getPointCount(); }
    bool getPoint (uint16 index, Frame::XYPoint& point)
    {
        return currentFrame->getPoint (index, point);
    }

    bool getIldaShowBlanked() { return ildaShowBlanked; }
    bool getIldaDrawLines() { return ildaDrawLines; }
    
    // Undoable Commands
    void setActiveLayer (Layer layer);
    void setSketchVisible (bool visible);
    void setIldaVisible (bool visible);
    void setRefVisible (bool visible);
    
    void selectImage();
    void clearImage();
    void setRefOpacity (float opacity);
    void setDrawGrid (bool draw);
    void setImageScale (float scale);
    void setImageRotation (float rot);
    void setImageXoffset (float off);
    void setImageYoffset (float off);

    void loadFile();
    
    void setIldaShowBlanked (bool show);
    void setIldaDrawLines (bool show);
    
    // Destructive Version (invoked by UndoManager)
    void _setActiveLayer (Layer layer);
    void _setSketchVisible (bool visible);
    void _setIldaVisible (bool visible);
    void _setRefVisible (bool visible);

    bool _setImage (File& file);
    void _setRefOpacity (float opacity);
    void _setDrawGrid (bool draw);
    void _setImageScale (float scale);
    void _setImageRotation (float rot);
    void _setImageXoffset (float off);
    void _setImageYoffset (float off);

    void _setFrames (const ReferenceCountedArray<Frame> frames);
    void _setFrameIndex (uint16 index);

    void _setIldaShowBlanked (bool show);
    void _setIldaDrawLines (bool draw);

private:
    Layer activeLayer;
    bool sketchVisible;
    bool ildaVisible;
    bool ildaShowBlanked;
    bool ildaDrawLines;
    bool refVisible;
    bool refDrawGrid;
    float refOpacity;
    
    uint16 frameIndex;
    ReferenceCountedArray<Frame> Frames;
    Frame::Ptr currentFrame;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameEditor)
};

namespace EditorActions
{
    const String layerChanged ("LC");
    const String sketchVisibilityChanged ("SVC");
    const String ildaVisibilityChanged ("IVC");
    const String refVisibilityChanged ("RVC");
    const String backgroundImageChanged ("BIC");
    const String refOpacityChanged("ROC");
    const String refDrawGridChanged("RGC");
    const String backgroundImageAdjusted ("BIA");
    const String framesChanged ("FC");
    const String frameIndexChanged ("FIC");
    const String ildaShowBlankChanged ("SBC");
    const String ildaDrawLinesChanged ("DLC");
}
