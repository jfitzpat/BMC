/*
	SDCard.c
	Basic FatFs SD Card access

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/Trace.h"

#include "SDCard.h"
#include "stm32f769i_discovery_sd.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "ILDA.h"

static FATFS FileSystem;
static char SD_Path[4];

static uint32_t FileCount = 0;

static const ILDA_FORMAT_2 IldaColors[] = {
#include "ildacolors.inc"
};

void sdCard_Init()
{
	// Try to attach the driver
	if(FATFS_LinkDriver(&SD_Driver, SD_Path) == FR_OK)
	{
		trace_puts("SD Driver linked");

		if (f_mount(&FileSystem, (TCHAR const*)"", 0) == FR_OK)
		{
			trace_puts("SD Mounted.");
		}
	}
}

uint32_t sdCard_GetFileCount()
{
	static uint8_t firstCount = 0;

	if (! firstCount)
	{
		FRESULT res;
		FileCount = 0;
		FILINFO fno;
		DIR dir;

		FileCount = 0;

		res = f_findfirst(&dir, &fno, "/Graphics", "*.ild");

		while (fno.fname[0])
		{
			if (res == FR_OK)
		    {
				// Visible and not a subdirectory or volume
				if ((fno.fattrib & 0xE) == 0)
				{
					++FileCount;
//					trace_printf("File %d: %s\n", FileCount, fno.fname);
				}
				res = f_findnext(&dir, &fno);
		    }
		    else
		    {
		    	FileCount = 0;
		    	break;
		    }
		}

		f_closedir(&dir);
		trace_printf("Files found: %d\n", FileCount);
		firstCount = 1;
	}

	return FileCount;
}

uint8_t sdCard_LoadIldaFile (uint32_t index, SD_FRAME_TABLE *table, SD_FRAME *frames)
{
	SD_FRAME* nextFrame = frames;
	ILDA_FORMAT_4* nextPoint = &(nextFrame->points);

	// Valid index?
	if (index == 0 || index > FileCount)
		return 0;

	// Walk the directory until we find the index;
	FRESULT res;
	FILINFO fno;
	DIR dir;
	uint32_t idx = 0;
	res = f_findfirst(&dir, &fno, "/Graphics", "*.ild");

	while (fno.fname[0])
	{
		if (res == FR_OK)
	    {
			// Visible and not a subdirectory or volume
			if ((fno.fattrib & 0xE) == 0)
			{
				++idx;
				if (idx == index)
					break;
			}
			res = f_findnext(&dir, &fno);
	    }
	    else
	    {
	    	idx = 0;
	    	break;
	    }
	}

	f_closedir(&dir);

	if (idx != index)
		return 0;

	trace_printf("Load ILDA %d (%s) found\n", index, fno.fname);

	char outstr[280];
	strcpy (outstr, "/Graphics/");
	strcat (outstr, fno.fname);

	FIL fil;
	if (f_open (&fil, outstr, FA_READ) != FR_OK)
		return 0;

	// Set the frame count to 0;
	table->frameCount = 0;
	// Save the name
	strcpy((char *)table->fname, (char *)fno.fname);
	strcpy((char *)table->altname, (char *)fno.altname);

	// Loop until we are out of frames
	do
	{
		UINT b;
		ILDA_HEADER header;

		// Read the header
		if (f_read(&fil, &header, sizeof(header), &b) != FR_OK)
			break;

		if (b != sizeof(header))
			break;

		// Valid?
		if (header.ilda[0] != 'I' ||
			header.ilda[1] != 'L' ||
			header.ilda[2] != 'D' ||
			header.ilda[3] != 'A')
			break;

		uint16_t rCount;
		rCount = header.numRecords.b[0];
		rCount <<= 8;
		rCount += header.numRecords.b[1];

		// 0 records marks end
		if (! rCount)
			break;

		int n;
		for (n = 0; n < rCount; ++n)
		{
			// We have 5 different handlers for the 5 different
			// ILDA data formats (ugh)
			if (header.format == 0)
			{
				ILDA_FORMAT_0 in;

				// Try to read the next point
				if (f_read(&fil, &in, sizeof(in), &b) != FR_OK)
					break;
				if (b != sizeof(in))
					break;

				// Change endian and store X, Y and Z
				nextPoint[n].x.b[1] = in.x.b[0];
				nextPoint[n].x.b[0] = in.x.b[1];
				nextPoint[n].y.b[1] = in.y.b[0];
				nextPoint[n].y.b[0] = in.y.b[1];
				nextPoint[n].z.b[1] = in.z.b[0];
				nextPoint[n].z.b[0] = in.z.b[1];

				// Store status
				nextPoint[n].status = in.status;

				// Lookup and store colors
				nextPoint[n].red = IldaColors[in.colorIdx].red;
				nextPoint[n].green = IldaColors[in.colorIdx].green;
				nextPoint[n].blue = IldaColors[in.colorIdx].blue;
			}
			else if (header.format == 1)
			{
				ILDA_FORMAT_1 in1;

				// Try to read the next point
				if (f_read(&fil, &in1, sizeof(in1), &b) != FR_OK)
					break;
				if (b != sizeof(in1))
					break;

				// Change endian and store X, Y and Z
				nextPoint[n].x.b[1] = in1.x.b[0];
				nextPoint[n].x.b[0] = in1.x.b[1];
				nextPoint[n].y.b[1] = in1.y.b[0];
				nextPoint[n].y.b[0] = in1.y.b[1];

				nextPoint[n].z.w = 0;

				// Store status
				nextPoint[n].status = in1.status;

				// Lookup and store colors
				nextPoint[n].red = IldaColors[in1.colorIdx].red;
				nextPoint[n].green = IldaColors[in1.colorIdx].green;
				nextPoint[n].blue = IldaColors[in1.colorIdx].blue;
			}
			else if (header.format == 2)
			{
				// Color Palette
				ILDA_FORMAT_2 in2;
				if (f_read(&fil, &in2, sizeof(in2), &b) != FR_OK)
					break;
				if (b != sizeof(in2))
					break;
			}
			else if (header.format == 4)
			{
				ILDA_FORMAT_4 in4;

				// Try to read the next point
				if (f_read(&fil, &in4, sizeof(in4), &b) != FR_OK)
					break;
				if (b != sizeof(in4))
					break;

				// Change endian and store X, Y and Z
				nextPoint[n].x.b[1] = in4.x.b[0];
				nextPoint[n].x.b[0] = in4.x.b[1];
				nextPoint[n].y.b[1] = in4.y.b[0];
				nextPoint[n].y.b[0] = in4.y.b[1];
				nextPoint[n].z.b[1] = in4.z.b[0];
				nextPoint[n].z.b[0] = in4.z.b[1];

				// Store status
				nextPoint[n].status = in4.status;

				// Store colors
				nextPoint[n].red = in4.red;
				nextPoint[n].green = in4.green;
				nextPoint[n].blue = in4.blue;
			}
			else if (header.format == 5)
			{
				ILDA_FORMAT_5 in5;

				// Try to read the next point
				if (f_read(&fil, &in5, sizeof(in5), &b) != FR_OK)
					break;
				if (b != sizeof(in5))
					break;

				// Change endian and store X, Y and Z
				nextPoint[n].x.b[1] = in5.x.b[0];
				nextPoint[n].x.b[0] = in5.x.b[1];
				nextPoint[n].y.b[1] = in5.y.b[0];
				nextPoint[n].y.b[0] = in5.y.b[1];

				nextPoint[n].z.w = 0;

				// Store status
				nextPoint[n].status = in5.status;

				// Store colors
				nextPoint[n].red = in5.red;
				nextPoint[n].green = in5.green;
				nextPoint[n].blue = in5.blue;
			}
			else
				break;
		}

		if (n != rCount)
			break;

		// Don't store palletes!
		if (header.format != 2)
		{
			// Update the count
			nextFrame->numPoints = rCount;

			// Store the frame pointer in the table
			table->frameCount++;
			table->frames[table->frameCount -1] = nextFrame;

			// Advance the frame pointer
			// DWORD align
			uint32_t p = (uint32_t)(&(nextPoint[rCount]));
			if (p & 3)
			{
				p += 3;
				p &= 0xFFFFFFFC;
			}
			nextFrame = (SD_FRAME *)p;

			// Reset the point data pointer
			nextPoint = &(nextFrame->points);
		}

	} while (1);

	// Close the file
	f_close(&fil);

	// Nothing read?
	if (! table->frameCount)
		return 0;

//	trace_printf("Loaded %d frames at:\n", table->frameCount);
//	for (uint32_t i = 0; i < table->frameCount; ++i)
//		trace_printf("  %08x\n", table->frames[i]);

	return 1;
}
