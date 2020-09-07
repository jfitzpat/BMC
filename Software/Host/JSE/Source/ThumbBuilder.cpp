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
    float wScale = width / 65536.0;
    float hScale = height / 65536.0;
    
    for (auto n = 0; n < frame->getPointCount(); ++n)
    {
        Frame::XYPoint point;
        
        if (frame->getPoint (n, point))
        {
            Frame::XYPoint nextPoint;
            bool b;
            
            if (n < (frame->getPointCount() - 1))
                b = frame->getPoint (n+1, nextPoint);
            else
                b = frame->getPoint (0, nextPoint);

            if (b)
            {
                if (! (point.status & Frame::BlankedPoint))
                {
                    g.setColour (Colour (point.red, point.blue, point.green));
                    
                    // We put in the dots for beam images, etc.
                    g.fillEllipse((point.x.w + 32768) * wScale,
                                (32768 - point.y.w) * hScale,
                                  lineSize / 2.0, lineSize / 2.0);

                    g.drawLine ((float)(point.x.w + 32768) * wScale,
                                (float)(32768 - point.y.w) * hScale,
                                (float)(nextPoint.x.w + 32768) * wScale,
                                (float)(32768 - nextPoint.y.w) *hScale,
                                lineSize);
                }
            }
        }
    }
}
