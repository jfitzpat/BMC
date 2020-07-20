/*
 ILDA.h
 Some basic defines for dealing with ILDA files

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

#ifndef ILDA_H
#define ILDA_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct
{
	uint8_t ilda[4];
	uint8_t rsvd[3];
	uint8_t format;
	uint8_t name[8];
	uint8_t company[8];
	union
	{
		uint16_t w;
		uint8_t b[2];
	} numRecords;
	union
	{
		uint16_t w;
		uint8_t b[2];
	} frameNumber;
	union
	{
		uint16_t w;
		uint8_t b[2];
	} totalFrames;
	uint8_t projector;
	uint8_t dummy;
} ILDA_HEADER;

typedef struct
{
	union
	{
		int16_t w;
		uint8_t b[2];
	} x;
	union
	{
		int16_t w;
		uint8_t b[2];
	} y;
	union
	{
		int16_t w;
		uint8_t b[2];
	} z;
	uint8_t status;
	uint8_t colorIdx;
} ILDA_FORMAT_0;

typedef struct
{
	union
	{
		int16_t w;
		uint8_t b[2];
	} x;
	union
	{
		int16_t w;
		uint8_t b[2];
	} y;
	uint8_t status;
	uint8_t colorIdx;
} ILDA_FORMAT_1;

typedef struct
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} ILDA_FORMAT_2;

typedef struct
{
	union
	{
		int16_t w;
		uint8_t b[2];
	} x;
	union
	{
		int16_t w;
		uint8_t b[2];
	} y;
	union
	{
		int16_t w;
		uint8_t b[2];
	} z;
	uint8_t status;
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} ILDA_FORMAT_4;

typedef struct
{
	union
	{
		int16_t w;
		uint8_t b[2];
	} x;
	union
	{
		int16_t w;
		uint8_t b[2];
	} y;
	uint8_t status;
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} ILDA_FORMAT_5;

#pragma pack(pop)

#endif
