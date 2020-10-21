/*
    Anchor.h
    Anchor Point in an IPath
 
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

class Anchor
{
public:
    Anchor (int x = 0, int y = 0, int enX = 0, int enY = 0, int exX = 0, int exY = 0)
     : xPosition (x),
       yPosition (y),
       entryXDelta (enX),
       entryYDelta (enY),
       exitXDelta (exX),
       exitYDelta (exY) {;}
    
    ~Anchor() {;}
    
    int getX() const { return xPosition; }
    void setX (int x) { xPosition = x; }
    int getY() const { return yPosition; }
    void setY (int y) { yPosition = y; }
    void getPosition  (int& x, int& y) const { x = xPosition; y = yPosition; }
    void setPosition (int x, int y) { xPosition = x; yPosition = y; }
    int getEntryXDelta() const { return entryXDelta; }
    void setEntryXDelta (int x) { entryXDelta = x; }
    int getEntryYDelta() const { return entryYDelta; }
    void setEntryYDelta (int y) { entryYDelta = y; }
    void getEntryPosition  (int& x, int& y) const { x = xPosition + entryXDelta;
                                                   y = yPosition + entryYDelta; }
    void setEntryPosition (int x, int y) { entryXDelta = x - xPosition;
                                           entryYDelta = y - yPosition; }
    int getExitXDelta() const { return exitXDelta; }
    void setExitXDelta (int x) { exitXDelta = x; }
    int getExitYDelta()  const { return exitYDelta; }
    void setExitYDelta (int y) { exitYDelta = y;}
    void getExitPosition (int& x, int& y) const { x = xPosition + exitXDelta;
                                                  y = yPosition + exitYDelta; }
    void setExitPosition (int x, int y) { exitXDelta = x - xPosition;
                                          exitYDelta = y - yPosition; }
    
private:
    int xPosition;
    int yPosition;
    int entryXDelta;
    int entryYDelta;
    int exitXDelta;
    int exitYDelta;
};
