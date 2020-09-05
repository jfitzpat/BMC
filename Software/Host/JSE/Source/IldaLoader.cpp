/*
    IldaLoader.cpp
    Load and ILDA file as an array of Frame objects
 
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

#include "IldaLoader.h"

static const ILDA_FORMAT_2 IldaColors[] =
{
  #include "ildacolors.inc"
};

bool IldaLoader::load (ReferenceCountedArray<Frame>& frameArray, File& file)
{
    FileInputStream input (file);
    if (input.failedToOpen())
        return false;

    frameArray.clear();
    
    // Loop until we are out of frames
    do
    {
        Frame::Ptr frame = new Frame;
        
        ILDA_HEADER header;

        // Read the header
        if (input.read (&header, sizeof(header)) != sizeof(header)) break;

        // Valid?
        if (header.ilda[0] != 'I' || header.ilda[1] != 'L'
                || header.ilda[2] != 'D' || header.ilda[3] != 'A') break;

        uint16 rCount;
        rCount = header.numRecords.b[0];
        rCount <<= 8;
        rCount += header.numRecords.b[1];

        // 0 records marks end
        if (!rCount) break;

        int n;
        for (n = 0; n < rCount; ++n)
        {
            Frame::XYPoint newPoint;

            // We have 5 different handlers for the 5 different
            // ILDA data formats (ugh)
            if (header.format == 0)
            {
                ILDA_FORMAT_0 in;

                // Try to read the next point
                if (input.read (&in, sizeof(in)) != sizeof(in)) break;

                // Change endian and store X, Y and Z
                newPoint.x.b[1] = in.x.b[0];
                newPoint.x.b[0] = in.x.b[1];
                newPoint.y.b[1] = in.y.b[0];
                newPoint.y.b[0] = in.y.b[1];
                newPoint.z.b[1] = in.z.b[0];
                newPoint.z.b[0] = in.z.b[1];

                // Store status (minus last frame indicator!)
                newPoint.status = (in.status & 0x7F);

                // Lookup and store colors
                newPoint.red = IldaColors[in.colorIdx].red;
                newPoint.green = IldaColors[in.colorIdx].green;
                newPoint.blue = IldaColors[in.colorIdx].blue;
            }
            else if (header.format == 1)
            {
                ILDA_FORMAT_1 in1;

                // Try to read the next point
                if (input.read (&in1, sizeof(in1)) != sizeof(in1)) break;

                // Change endian and store X, Y and Z
                newPoint.x.b[1] = in1.x.b[0];
                newPoint.x.b[0] = in1.x.b[1];
                newPoint.y.b[1] = in1.y.b[0];
                newPoint.y.b[0] = in1.y.b[1];

                newPoint.z.w = 0;

                // Store status
                newPoint.status = in1.status;

                // Lookup and store colors
                newPoint.red = IldaColors[in1.colorIdx].red;
                newPoint.green = IldaColors[in1.colorIdx].green;
                newPoint.blue = IldaColors[in1.colorIdx].blue;
            }
            else if (header.format == 2)
            {
                // Color Palette
                ILDA_FORMAT_2 in2;
                // Try to read the next point
                if (input.read (&in2, sizeof(in2)) != sizeof(in2)) break;
            }
            else if (header.format == 4)
            {
                ILDA_FORMAT_4 in4;

                // Try to read the next point
                if (input.read (&in4, sizeof(in4)) != sizeof(in4)) break;

                // Change endian and store X, Y and Z
                newPoint.x.b[1] = in4.x.b[0];
                newPoint.x.b[0] = in4.x.b[1];
                newPoint.y.b[1] = in4.y.b[0];
                newPoint.y.b[0] = in4.y.b[1];
                newPoint.z.b[1] = in4.z.b[0];
                newPoint.z.b[0] = in4.z.b[1];

                // Store status
                newPoint.status = in4.status;

                // Store colors
                newPoint.red = in4.red;
                newPoint.green = in4.green;
                newPoint.blue = in4.blue;
            }
            else if (header.format == 5)
            {
                ILDA_FORMAT_5 in5;

                // Try to read the next point
                if (input.read (&in5, sizeof(in5)) != sizeof(in5)) break;

                // Change endian and store X, Y and Z
                newPoint.x.b[1] = in5.x.b[0];
                newPoint.x.b[0] = in5.x.b[1];
                newPoint.y.b[1] = in5.y.b[0];
                newPoint.y.b[0] = in5.y.b[1];

                newPoint.z.w = 0;

                // Store status
                newPoint.status = in5.status;

                // Store colors
                newPoint.red = in5.red;
                newPoint.green = in5.green;
                newPoint.blue = in5.blue;
            }
            else
                break;
            
            // Store the point
            frame->addPoint (newPoint);
        }

        if (n != rCount) break;

        // Don't store palletes!
        if (header.format != 2)
            frameArray.add (frame);
    }
    while (1);

    if (frameArray.size())
        return true;
    
    return false;
}
