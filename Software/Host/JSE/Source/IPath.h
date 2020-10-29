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
        : color (c), startZ (0), endZ (0),
          pointDensity (1200), extraPointsPerAnchor(0),
          extraPointsAtStart (0), extraPointsAtEnd (0),
          blankedPointsBeforeStart (3), blankedPointsAfterEnd (3) {;}
    ~IPath() {;}
    
    int getAnchorCount() const { return anchors.size(); }
    const Anchor& getAnchor (int index) const { return anchors.getReference (index); }
    
    void addAnchor (const Anchor& a);
    void insertAnchor (int index, const Anchor& a);
    void removeAnchor (int index);
    void setAnchor (int index, const Anchor& a);
    void clearAllAnchors();
    
    const Colour& getColor() { return color; }
    void setColor (Colour c) { color = c; }
    
    uint16 getExtraPointsAtStart () const { return extraPointsAtStart; }
    void setExtraPointsAtStart (uint16 p) { extraPointsAtStart = p; }
    uint16 getExtraPointsAtEnd () const { return extraPointsAtEnd; }
    void setExtraPointsAtEnd (uint16 p) { extraPointsAtEnd = p; }
    int getStartZ() const { return startZ; }
    void setStartZ (int z) { startZ = z; }
    int getEndZ() const { return anchors.size() > 1 ? endZ : startZ; }
    void setEndZ (int z) { endZ = z; }
    uint16 getPointDensity() const { return pointDensity; }
    void setPointDensity (uint16 d) { pointDensity = d ? d : 1; }
    uint16 getExtraPointsPerAnchor() const { return extraPointsPerAnchor; }
    void setExtraPointsPerAnchor (uint16 p) { extraPointsPerAnchor = p; }
    uint16 getBlankedPointsBeforeStart() const { return blankedPointsBeforeStart; }
    void setBlankedPointsBeforeStart (uint16 p) { blankedPointsBeforeStart = p; }
    uint16 getBlankedPointsAfterEnd() const { return blankedPointsAfterEnd; }
    void setBlankedPointsAfterEnd (uint16 p) { blankedPointsAfterEnd = p; }

    const Path& getPath() const { return path; }
    
    IPath reversed();

private:
    void buildPath();
    
    Array<Anchor> anchors;
    Colour color;
    int startZ;
    int endZ;
    uint16 pointDensity;
    uint16 extraPointsPerAnchor;
    uint16 extraPointsAtStart;
    uint16 extraPointsAtEnd;
    uint16 blankedPointsBeforeStart;
    uint16 blankedPointsAfterEnd;
    
    Path path;
};
