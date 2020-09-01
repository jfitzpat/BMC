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
class FrameEditor  : public ActionBroadcaster
{
public:
    FrameEditor();
    ~FrameEditor() override;

    typedef enum {
        sketch = 0,
        ilda,
        reference
    } Layer;
    
    void setActiveLayer (Layer layer);
    Layer getActiveLayer() { return activeLayer; }
    
    void setSketchVisible (bool visible);
    void setIldaVisible (bool visible);
    void setRefVisible (bool visible);
    bool getSketchVisible() { return sketchVisible; }
    bool getIldaVisible() { return ildaVisible; }
    bool getRefVisible() { return refVisible; }
    
    File getImageFile();
    void selectImage();
    const Image* getImage();
    
    float getRefOpacity() { return refOpacity; }
    void setRefOpacity (float opacity);
    float getImageScale();
    void setImageScale (float scale);
    float getImageRotation();
    void setImageRotation (float rot);
    float getImageXoffset();
    void setImageXoffset (float off);
    float getImageYoffset();
    void setImageYoffset (float off);

private:
    Layer activeLayer;
    bool sketchVisible;
    bool ildaVisible;
    bool refVisible;
    float refOpacity;
    
    std::unique_ptr<Frame> currentFrame;
    
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
    const String backgroundImageAdjusted ("BIA");
}
