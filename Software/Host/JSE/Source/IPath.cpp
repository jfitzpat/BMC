/*
    IPath.cpp
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

#include "IPath.h"

//==========================================================================================

void IPath::addAnchor (const Anchor& a)
{
    anchors.add (a);
    buildPath();
}

void IPath::insertAnchor (int index, const Anchor& a)
{
    anchors.insert (index, a);
    buildPath();
}

void IPath::removeAnchor (int index)
{
    anchors.remove (index);
    buildPath();
}

void IPath::setAnchor (int index, const Anchor& a)
{
    anchors.set (index, a);
    buildPath();
}

void IPath::clearAllAnchors()
{
    anchors.clear();
    buildPath();
}

void IPath::buildPath()
{
    path.clear();
    
    if (anchors.size())
    {
        Anchor last = getAnchor (0);
        path.startNewSubPath ((float)last.getX(), (float)last.getY());
        
        for (auto i = 1; i < anchors.size(); ++i)
        {
            Anchor next = getAnchor (i);

            if (last.getExitXDelta() == 0 && last.getExitYDelta() == 0 &&
                next.getExitXDelta() == 0 && next.getExitYDelta() == 0)
                path.lineTo ((float)next.getX(), (float)next.getY());
            else
            {
                int exitX, exitY;
                int entryX, entryY;

                last.getExitPosition (exitX, exitY);
                next.getEntryPosition (entryX, entryY);
                
                path.cubicTo((float)exitX, (float)exitY, (float)entryX, (float)entryY, (float)next.getX(), (float)next.getY());
            }
            
            last = next;
        }
    }
}
