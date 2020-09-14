/*
    IldaExporter.cpp
    Export all compatible frame object to an ILDA file
 
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

#include "IldaExporter.h"

bool IldaExporter::save (ReferenceCountedArray<Frame>& frameArray, File& file)
{
    if (file.exists())
        file.deleteFile();
    
    FileOutputStream output (file);
    
    if (! output.openedOk())
        return false;
 
    ILDA_HEADER header;
    zerostruct (header);
    
    header.ilda[0] = 'I'; header.ilda[1] = 'L';
    header.ilda[2] = 'D'; header.ilda[3] =  'A';
    memcpy (header.name, "Scrootch", 8);
    memcpy (header.company, ".me! JSE", 8);
    header.format = 4;
    int s = frameArray.size();
    header.totalFrames.b[0] = (uint8)(s >> 8);
    header.totalFrames.b[1] = (uint8)(s & 0xFF);
    header.projector = 1;
    
    for (auto n = 0; n < frameArray.size(); ++n)
    {
        // Update header
        header.frameNumber.b[0] = (uint8)(n >> 8);
        header.frameNumber.b[1] = (uint8)(n & 0xFF);
        
        if (frameArray[n]->getPointCount())
        {
            header.numRecords.b[0] = (uint8)(frameArray[n]->getPointCount() >> 8);
            header.numRecords.b[1] = (uint8)(frameArray[n]->getPointCount() & 0xFF);
        }
        else
        {
            header.numRecords.b[0] = 0;
            header.numRecords.b[1] = 4;
        }

        // Write Header
        output.write (&header, sizeof(header));
        
        Frame::XYPoint point;
        
        // Now records
        if (frameArray[n]->getPointCount())
        {
            for (uint16 i = 0; i < frameArray[n]->getPointCount(); ++i)
            {
                Frame::XYPoint in;
                
                frameArray[n]->getPoint (i, in);
                
                // Swap endian order of X, Y, and Z
                point.x.b[0] = in.x.b[1];
                point.x.b[1] = in.x.b[0];
                point.y.b[0] = in.y.b[1];
                point.y.b[1] = in.y.b[0];
                point.z.b[0] = in.z.b[1];
                point.z.b[1] = in.z.b[0];
                point.red = in.red;
                point.green = in.green;
                point.blue = in.blue;
                point.status = in.status;
                if (i == (frameArray[n]->getPointCount() - 1))
                    point.status |= ILDA_LAST;

                output.write (&point, sizeof(point));
            }
        }
        else
        {
            // Write out 4 blanked points
            zerostruct (point);
            point.status = ILDA_BLANK;
            output.write (&point, sizeof(point));
            output.write (&point, sizeof(point));
            output.write (&point, sizeof(point));
            point.status = ILDA_BLANK | ILDA_LAST;
            output.write (&point, sizeof(point));
        }
    }
    
    // One more header without records
    // We don't care about endian swap for this
    header.frameNumber.w = header.totalFrames.w;
    header.numRecords.w = 0;
    
    output.write (&header, sizeof (header));
    output.flush();
    
    return true;
}
