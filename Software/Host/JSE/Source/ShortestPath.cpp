/*
    ShortestPath.h
    Find the shortest render path
 
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

#include "ShortestPath.h"

class PElement
{
public:
    PElement () : index (-1), reversed (false) {;}
    PElement (const Array<IPath>& paths, int idx)
    : index (idx), reversed (false)
    {
        Anchor startA = paths[idx].getAnchor (0);
        Anchor endA = paths[idx].getAnchor (paths[idx].getAnchorCount() - 1);
        
        start = Point<int>(startA.getX(), startA.getY());
        end = Point<int>(endA.getX(), endA.getY());
    }
    
public:
    int index;
    Point<int> start;
    Point<int> end;
    bool reversed;
};

static int getTotalLength (const Array<PElement>& elements)
{
    Point<int> start;
    Point<int> end;
    
    int length = 0;
    
    for (auto n = 0; n < (elements.size() - 1); ++n)
    {
        if (elements[n].reversed)
            start = elements[n].start;
        else
            start = elements[n].end;
        
        if (elements[n+1].reversed)
            end = elements[n+1].end;
        else
            end = elements[n+1].start;
        
        length += start.getDistanceFrom (end);
    }
    
    if (elements[elements.size() -1].reversed)
        start = elements[elements.size() -1].start;
    else
        start = elements[elements.size() -1].end;
    
    if (elements[0].reversed)
        end = elements[0].end;
    else
        end = elements[0].start;

    length += start.getDistanceFrom (end);
    return length;
}

void findAndMoveShortest (Array<PElement>& dst, Array<PElement>& src)
{
    PElement e = dst[dst.size() -1];
    Point<int> start;
    
    if (e.reversed)
        start = e.start;
    else
        start = e.end;

    int index = 0;
    int shortest = 1000000;
    bool reversed = false;

    for (auto n = 0; n < src.size(); ++n)
    {
        e = src[n];
        int distance = start.getDistanceFrom (e.start);
        if (distance < shortest)
        {
            shortest = distance;
            index = n;
            reversed = false;
        }
        
        distance = start.getDistanceFrom (e.end);
        if (distance < shortest)
        {
            shortest = distance;
            index = n;
            reversed = true;
        }
    }
    
    e = src[index];
    src.remove (index);
    e.reversed = reversed;
    dst.add (e);
}

void ShortestPath::find (const Array<IPath>& original, Array<IPath>& shortest)
{
    Array<PElement> base;
    
    // Collect the starts and ends
    for (auto n = 0; n < original.size(); ++n)
        base.add (PElement (original, n));
    
    // The original is the one to beat
    Array<PElement> minPath = base;
    int minLength = getTotalLength (minPath);
    Logger::outputDebugString (String (minLength));
    
    for (auto n = 0; n < base.size(); ++n)
    {
        Array<PElement> src = base;
        Array<PElement> dst;

        PElement e = src[n];
        dst.add (e);
        src.remove (n);
        
        while (src.size())
            findAndMoveShortest (dst, src);
        
        int length = getTotalLength (dst);
        if (length < minLength)
        {
            minLength = length;
            minPath = dst;
        }
        
        src = base;
        dst.clear();
        e.reversed = true;
        dst.add (e);
        src.remove (n);
        
        while (src.size())
            findAndMoveShortest (dst, src);
        
        length = getTotalLength (dst);
        if (length < minLength)
        {
            minLength = length;
            minPath = dst;
        }
    }
    
    Logger::outputDebugString (String (minLength));
    
    for (auto n = 0; n < minPath.size(); ++n)
    {
        String s = String (minPath[n].index) + " " + String ((int)minPath[n].reversed);
        Logger::outputDebugString (s);
        
        if (minPath[n].reversed)
            shortest.add (original[minPath[n].index].reversed());
        else
            shortest.add (original[minPath[n].index]);
    }
    
//    shortest = original;
}
