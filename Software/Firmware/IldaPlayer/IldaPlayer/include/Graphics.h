/*
 Graphics.h
 Simple Graphics Routines for UI

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

 Portions of this code were copied and modified from the file
 stm32f769i_discovery_ldc.c. See that file for additional
 licensing and copyright information
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "stdint.h"
#include "fonts.h"

#define DISPLAY_WIDTH		800
#define DISPLAY_HEIGHT      480
#define BYTES_PER_PIXEL 	(4)

typedef enum
{
	CENTER = 0x01, RIGHT = 0x02, LEFT = 0x03
} GRAPHICS_ALIGN_MODE;

void graphics_Init();

void graphics_SetTargetAddress(uint32_t newtarget);
uint32_t graphics_GetTargetAddress();

void graphics_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y,
		uint16_t xsize, uint16_t ysize);

void graphics_SetBrushColor(uint32_t newcolor);
uint32_t graphics_GetBrushColor();
void graphics_SetBackColor(uint32_t newcolor);
uint32_t graphics_GetBackColor();

void graphics_SetFont(sFONT *fonts);
sFONT* graphics_GetFont();

void graphics_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);
void graphics_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
		GRAPHICS_ALIGN_MODE Mode);
void graphics_DisplayStringAtLine(uint16_t Line, uint8_t *ptr);

void graphics_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code);

void graphics_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width,
		uint16_t Height);
void graphics_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width,
		uint16_t Height);

void graphics_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void graphics_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void graphics_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void graphics_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);

#endif

