/*
    ThumbBuilder.cpp
    Create a Frame thumbnail
 
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

#include "ThumbBuilder.h"

void ThumbBuilder::build (Frame* frame, Image& thumb, int width, int height, float lineSize)
{
    thumb = Image(Image::ARGB, width, height, true);
    
    Graphics g (thumb);
    float wScale = width / 65536.0f;
    float hScale = height / 65536.0f;
    
    for (uint16 n = 0; n < frame->getPointCount(); ++n)
    {
        Frame::IPoint point;
        
        if (frame->getPoint (n, point))
        {
            Frame::IPoint nextPoint;
            bool b;
            
            if (n < (frame->getPointCount() - 1))
                b = frame->getPoint (n+1, nextPoint);
            else
                b = frame->getPoint (0, nextPoint);

            if (b)
            {
                if (! (point.status & Frame::BlankedPoint))
                {
                    g.setColour (Colour (point.red, point.green, point.blue));
                    
                    // We put in the dots for beam images, etc.
                    g.fillEllipse (Frame::getCompX(point) * wScale,
                                   Frame::getCompY(point) * hScale,
                                   lineSize / 2.0f, lineSize / 2.0f);

                    g.drawLine (Frame::getCompX(point) * wScale,
                                Frame::getCompY(point) * hScale,
                                Frame::getCompX(nextPoint) * wScale,
                                Frame::getCompY(nextPoint) * hScale,
                                lineSize);
                }
            }
        }
    }
}
