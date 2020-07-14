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
#include "diag/Trace.h"

#include "SDCard.h"
#include "stm32f769i_discovery_sd.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

static FATFS FileSystem;
static char SD_Path[4];

static uint32_t FileCount = 0;

void sdCard_Init()
{
	// Try to attach the driver
	if(FATFS_LinkDriver(&SD_Driver, SD_Path) == FR_OK)
	{
		trace_puts("SD Driver linked");

		if (f_mount(&FileSystem, (TCHAR const*)"", 0) == FR_OK)
		{
			trace_puts("SD Mounted.");

			FRESULT res;
			FileCount = 0;
			FILINFO fno;
			DIR dir;

			res = f_findfirst(&dir, &fno, "/Graphics", "*.ild");

			while (fno.fname[0])
			{
				if (res == FR_OK)
			    {
					// Visible and not a subdirectory or volume
					if ((fno.fattrib & 0xE) == 0)
					{
						++FileCount;
//						trace_printf("File %d: %s\n", FileCount, fno.fname);
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
		}
	}
}

uint32_t sdCard_GetFileCount()
{
	return FileCount;
}
