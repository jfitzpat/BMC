/*
 SDCard.h
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

#ifndef SDCARD_H
#define SDCARD_H

// For ILDA file loading
#include "ILDA.h"

typedef struct
{
	uint32_t numPoints;
	ILDA_FORMAT_4 points;
} SD_FRAME;

typedef struct
{
	uint8_t fname[256];
	uint8_t altname[13];
	uint32_t frameCount;
	SD_FRAME *frames[];
} SD_FRAME_TABLE;

void sdCard_Init();

uint32_t sdCard_GetFileCount();
uint8_t sdCard_LoadIldaFile(uint32_t index, SD_FRAME_TABLE *table,
		SD_FRAME *frames);

#endif
