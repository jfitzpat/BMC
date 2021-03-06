/*
    JSEFileSaver.h
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

#pragma once

#include <JuceHeader.h>
#include "FrameEditor.h"

class JSEFileSaver
{
public:
    static bool save (FrameEditor* editor, File& file);
    
private:
    static FrameEditor* frameEditor;
    static var frameToObj (uint16 frameIndex);
    static var pointToObj (uint16 frameIndex, uint16 pointIndex);
    static var ipathToObj (uint16 frameIndex, uint16 pathIndex);
    static var anchorToObj (Anchor& a);
};
