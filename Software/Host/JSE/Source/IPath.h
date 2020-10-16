/*
    IPath.h
    A Path for the sketch layer
 
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

#include "Anchor.h"
#include <JuceHeader.h>

class IPath
{
public:
    IPath (Colour c = Colours::white)
        : color (c), pointDensity (1200), extraPointsPerAnchor(0),
          blankedPointsBeforeStart (1), blankedPointsAfterEnd (1) {;}
    ~IPath() {;}
    
    int getAnchorCount() { return anchors.size(); }
    const Anchor& getAnchor (int index) { return anchors.getReference (index); }
    
    void addAnchor (const Anchor& a);
    void insertAnchor (int index, const Anchor& a);
    void removeAnchor (int index);
    void setAnchor (int index, const Anchor& a);
    void clearAllAnchors();
    
    const Colour& getColor() { return color; }
    void setColor (Colour c) { color = c; }
    
    uint16 getPointDensity() { return pointDensity; }
    void setPointDensity (uint16 d) { pointDensity = d ? d : 1; }
    uint16 getExtraPointsPerAnchor() { return extraPointsPerAnchor; }
    void setExtraPointsPerAnchor (uint16 p) { extraPointsPerAnchor = p; }
    uint16 getBlankedPointsBeforeStart() { return blankedPointsBeforeStart; }
    void setBlankedPointsBeforeStart (uint16 p) { blankedPointsBeforeStart = p ? p : 1; }
    uint16 getBlankedPointsAfterEnd() { return blankedPointsAfterEnd; }
    void setBlankedPointsAfterEnd (uint16 p) { blankedPointsAfterEnd = p ? p : 1; }

    const Path& getPath() { return path; }

private:
    void buildPath();
    
    Array<Anchor> anchors;
    Colour color;
    uint16 pointDensity;
    uint16 extraPointsPerAnchor;
    uint16 blankedPointsBeforeStart;
    uint16 blankedPointsAfterEnd;
    
    Path path;
};
