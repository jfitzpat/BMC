/*
    ILDA.h
    Some typedefs to help with ILDA files
 
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

#define ILDA_BLANK (0x40)
#define ILDA_LAST (0x80)

#pragma pack(push, 1)

typedef struct
{
	uint8 ilda[4];
	uint8 rsvd[3];
	uint8 format;
	uint8 name[8];
	uint8 company[8];
	union
	{
		int16 w;
		uint8 b[2];
	} numRecords;
	union
	{
		int16 w;
		uint8 b[2];
	} frameNumber;
	union
	{
		int16 w;
		uint8 b[2];
	} totalFrames;
	uint8 projector;
	uint8 dummy;
} ILDA_HEADER;

typedef struct
{
	union
	{
		int16_t w;
		uint8 b[2];
	} x;
	union
	{
		int16_t w;
		uint8 b[2];
	} y;
	union
	{
		int16_t w;
		uint8 b[2];
	} z;
	uint8 status;
	uint8 colorIdx;
} ILDA_FORMAT_0;

typedef struct
{
	union
	{
		int16_t w;
		uint8 b[2];
	} x;
	union
	{
		int16_t w;
		uint8 b[2];
	} y;
	uint8 status;
	uint8 colorIdx;
} ILDA_FORMAT_1;

typedef struct
{
	uint8 red;
	uint8 green;
	uint8 blue;
} ILDA_FORMAT_2;

typedef struct
{
	union
	{
		int16_t w;
		uint8 b[2];
	} x;
	union
	{
		int16_t w;
		uint8 b[2];
	} y;
	union
	{
		int16_t w;
		uint8 b[2];
	} z;
	uint8 status;
	uint8 blue;
	uint8 green;
	uint8 red;
} ILDA_FORMAT_4;

typedef struct
{
	union
	{
		int16_t w;
		uint8 b[2];
	} x;
	union
	{
		int16_t w;
		uint8 b[2];
	} y;
	uint8 status;
	uint8 blue;
	uint8 green;
	uint8 red;
} ILDA_FORMAT_5;

#pragma pack(pop)

