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

class IPath : public ReferenceCountedObject
{
public:
    IPath (Colour c = Colours::white)
        : color (c) {;}
    ~IPath() {;}
    
    int getAnchorCount() { return anchors.size(); }
    const Anchor& getAnchor (int index) { return anchors.getReference (index); }
    
    void addAnchor (const Anchor& a);
    void insertAnchor (int index, const Anchor& a);
    void removeAnchor (int index);
    void clearAllAnchors();
    
    const Colour& getColor() { return color; }
    void setColor (Colour c) { color = c; }
    
    const Path& getPath() { return path; }

    // Make a counting pointer of our type
    using Ptr = ReferenceCountedObjectPtr<IPath>;

private:
    void buildPath();
    
    Array<Anchor> anchors;
    Colour color;
    Path path;
};
