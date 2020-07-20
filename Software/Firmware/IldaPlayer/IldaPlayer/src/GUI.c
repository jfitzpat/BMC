/*
 GUI.h
 Functions to manage the graphical interface

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

#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"

#include "GUI.h"
#include "UIGraphics.h"
#include "TimerCallback.h"
#include "SDCard.h"
#include "Scan.h"
#include "Graphics.h"

static void TouchCallback();
static void DrawMainBackground();
static void _DrawFrame();
static void DrawFrame();

// LTDC foreground layer address 800x480 in ARGB8888
#define LCD_FG_LAYER_ADDRESS          (LCD_FB_START_ADDRESS)
// LTDC background layer address 800x480 in ARGB8888 following the foreground layer
#define LCD_BG_LAYER_ADDRESS          (LCD_FG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * BYTES_PER_PIXEL))
// Free SDRAM Start
#define INTERNAL_BUFFER_START_ADDRESS (LCD_BG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * BYTES_PER_PIXEL))

extern LTDC_HandleTypeDef hltdc_discovery;
static __IO int32_t front_buffer = 0;
static __IO int32_t pend_buffer = -1;

static const uint32_t Buffers[] =
{
LCD_FG_LAYER_ADDRESS,
LCD_BG_LAYER_ADDRESS };

SD_FRAME_TABLE *FrameTable = (SD_FRAME_TABLE*) INTERNAL_BUFFER_START_ADDRESS;

uint32_t CurrentFile = 1;
uint32_t FrameIdx = 0;
uint8_t Animate = 1;

void gui_Init()
{
	// Initalize Touch
	BSP_TS_Init(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	// Initialize in Video Burst Mode, bail if we rail
	if (BSP_LCD_Init() != LCD_OK) return;

	// Initialize our display layer, using SDRAM
	BSP_LCD_LayerDefaultInit(0, LCD_FG_LAYER_ADDRESS);
	BSP_LCD_SelectLayer(0);
	BSP_LCD_SetTransparency(0, 0);

	// Setup a line event
	HAL_LTDC_ProgramLineEvent(&hltdc_discovery, 0);

	// Splash time!
	graphics_SetTargetAddress(Buffers[0]);
	graphics_DrawBitmap(0, 0, (uint8_t*) uiGraphics_splash_bmp);
	graphics_SetBrushColor(LCD_COLOR_BLACK);
	graphics_SetBackColor(LCD_COLOR_WHITE);
	graphics_SetFont(&Font24);
	graphics_DisplayStringAtLine(12, (uint8_t*) " BMC");
	graphics_DisplayStringAtLine(13, (uint8_t*) " ILDA Player");
	graphics_DisplayStringAtLine(14, (uint8_t*) " v: 1.0");
	graphics_SetFont(&Font16);
	graphics_DisplayStringAt(10, (29 * 16 - 2),
			(uint8_t*) "http://Scrootch.Me/bmc", LEFT_MODE);

	// Adjust brightness
	// BSP_LCD_SetBrightness(100);

	// Fade in splash screen
	for (int n = 0; n < 255; ++n)
	{
		BSP_LCD_SetTransparency(0, n);
		HAL_Delay(5);
	}

	// Load the first ILDA
	FrameTable->frameCount = 0;

	if (sdCard_GetFileCount())
	{
		sdCard_LoadIldaFile(CurrentFile, FrameTable,
				(SD_FRAME*) (INTERNAL_BUFFER_START_ADDRESS + 0x1000));
	}

	HAL_Delay(1500);

	// Draw the main screen
	graphics_SetTargetAddress(Buffers[1 - front_buffer]);
	DrawMainBackground();
	_DrawFrame();
	pend_buffer = 1 - front_buffer;

	// If anything loaded, setup the frame and start the scanner
	if (FrameTable->frameCount)
	{
		scan_SetCurrentFrame(FrameTable->frames[0]);
		scan_SetEnable(1);
	}

	// Wait for the display to flips
	while (pend_buffer >= 0)
		;
	// Make a copy of the screen
	graphics_CopyBuffer((uint32_t*) Buffers[front_buffer],
			(uint32_t*) Buffers[1 - front_buffer], 0, 0, DISPLAY_WIDTH,
			DISPLAY_HEIGHT);

	// Start our callback for touch input
	timerCallback_Add(&TouchCallback, 50);
}

void* gui_GetFreeSDRAMBase()
{
	return (void*) INTERNAL_BUFFER_START_ADDRESS;
}

void DrawMainBackground()
{
	// Draw main screen...
	graphics_SetBrushColor(LCD_COLOR_BLUE);
	graphics_FillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	graphics_SetBrushColor(LCD_COLOR_BLACK);
	graphics_FillRect(DISPLAY_WIDTH - 472 - 4, 4, 472, 472);
	graphics_SetBrushColor(LCD_COLOR_WHITE);
	graphics_DrawRect(DISPLAY_WIDTH - 472 - 4, 4, 472, 472);
	graphics_DrawRect(DISPLAY_WIDTH - 472 - 5, 3, 474, 474);

	graphics_SetBackColor(LCD_COLOR_BLUE);
	graphics_SetFont(&Font16);
	graphics_DisplayStringAt(10, (29 * 16 - 2),
			(uint8_t*) "http://Scrootch.Me/bmc", LEFT);
}

static void DrawFrame()
{
	// Draw is pending!
	if (pend_buffer >= 0) return;

	graphics_SetTargetAddress(Buffers[1 - front_buffer]);
	graphics_SetBrushColor(LCD_COLOR_BLUE);
	graphics_FillRect(0, 0, 323, 168);
	graphics_SetBrushColor(LCD_COLOR_BLACK);
	graphics_FillRect(DISPLAY_WIDTH - 472 - 4 + 1, 5, 470, 470);
	_DrawFrame();
	pend_buffer = 1 - front_buffer;
}

static void _DrawFrame()
{
	int x = DISPLAY_WIDTH - 472 - 4 + 1;
	int y = 4 + 1;
	int w = 470;
	int h = 470;

	graphics_SetBrushColor(LCD_COLOR_WHITE);
	graphics_SetBackColor(LCD_COLOR_BLUE);
	graphics_SetFont(&Font24);

	char outstr[30];
	if (!sdCard_GetFileCount())
		sprintf(outstr, " Files: %d", (int) sdCard_GetFileCount());
	else
		sprintf(outstr, " File: %ld of %ld", CurrentFile,
				sdCard_GetFileCount());

	graphics_DisplayStringAtLine(1, (uint8_t*) outstr);
	if (FrameTable->frameCount)
	{
		sprintf(outstr, " %s", FrameTable->altname);
		graphics_DisplayStringAtLine(5, (uint8_t*) outstr);
		sprintf(outstr, " Frame: %ld of %ld", FrameIdx + 1,
				FrameTable->frameCount);
		graphics_DisplayStringAtLine(6, (uint8_t*) outstr);
	}

	if (FrameTable->frameCount)
	{
		SD_FRAME *frame = FrameTable->frames[FrameIdx];
		ILDA_FORMAT_4 *pntData = &(frame->points);

		for (uint32_t n = 0; n < (frame->numPoints - 1); ++n)
		{
			if (!(pntData[n].status & 0x40))
			{
				uint32_t color = 0xFF000000 | (pntData[n].red << 16)
						| (pntData[n].green << 8) | (pntData[n].blue);

				graphics_SetBrushColor(color);

				// Get our coordinates
				int32_t x1, y1, x2, y2;
				x1 = pntData[n].x.w;
				y1 = pntData[n].y.w;
				x2 = pntData[n + 1].x.w;
				y2 = pntData[n + 1].y.w;

				// Convert from center origin to top left origin
				x1 += 32768;
				x2 += 32768;
				y1 += 32768;
				y1 = 65535 - y1;
				y2 += 32768;
				y2 = 65535 - y2;

				// Scale to width/height
				x1 = x1 * w / 65536 + x;
				x2 = x2 * w / 65536 + x;
				y1 = y1 * h / 65536 + y;
				y2 = y2 * h / 65536 + y;

				graphics_DrawLine(x1, y1, x2, y2);
			}
		}
	}
}

static void IncFile()
{
	if (sdCard_GetFileCount() > 1)
	{
		scan_SetEnable(0);
		++CurrentFile;
		if (CurrentFile > sdCard_GetFileCount()) CurrentFile = 1;

		sdCard_LoadIldaFile(CurrentFile, FrameTable,
				(SD_FRAME*) (INTERNAL_BUFFER_START_ADDRESS + 0x1000));

		FrameIdx = 0;
		if (FrameTable->frameCount)
		{
			scan_SetCurrentFrame(FrameTable->frames[0]);
			scan_SetEnable(1);
		}

		DrawFrame();
	}
}

static void DecFile()
{
	if (sdCard_GetFileCount() > 1)
	{
		scan_SetEnable(0);

		if (CurrentFile > 1)
			--CurrentFile;
		else
			CurrentFile = sdCard_GetFileCount();

		sdCard_LoadIldaFile(CurrentFile, FrameTable,
				(SD_FRAME*) (INTERNAL_BUFFER_START_ADDRESS + 0x1000));

		FrameIdx = 0;
		if (FrameTable->frameCount)
		{
			scan_SetCurrentFrame(FrameTable->frames[0]);
			scan_SetEnable(1);
		}

		DrawFrame();
	}
}

static void IncFrame()
{
	if (FrameTable->frameCount > 1)
	{
		++FrameIdx;
		if (FrameIdx >= FrameTable->frameCount) FrameIdx = 0;

		scan_SetCurrentFrame(FrameTable->frames[FrameIdx]);
		DrawFrame();
	}
}

static void DecFrame()
{
	if (FrameTable->frameCount > 1)
	{
		if (FrameIdx > 0)
			--FrameIdx;
		else
			FrameIdx = FrameTable->frameCount - 1;

		scan_SetCurrentFrame(FrameTable->frames[FrameIdx]);
		DrawFrame();
	}
}

void TouchCallback()
{
	static uint8_t toggle = 0;
	static uint8_t last = 0;

	TS_StateTypeDef ts;

	if (Animate)
	{
		toggle++;
		if (toggle & 1) IncFrame();
	}

	if (BSP_TS_GetState(&ts) == TS_OK)
	{
		if (ts.touchDetected == 1)
		{
			if (last == 0)
			{
				// Control zone?
				if (ts.touchX[0] < 324)
				{
					if (ts.touchY[0] < 240)
					{
						if (ts.touchY[0] < 120)
						{
							if (ts.touchX[0] < 170)
								DecFile();
							else
								IncFile();
						}
						else
						{
							if (ts.touchX[0] < 170)
								DecFrame();
							else
								IncFrame();
						}
					}
					else
					{
						if (Animate)
							Animate = 0;
						else
							Animate = 1;
					}
				}
			}
		}

		last = ts.touchDetected;
	}
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc)
{
	if (pend_buffer >= 0)
	{
		LTDC_LAYER(hltdc, 0)->CFBAR = ((uint32_t) Buffers[pend_buffer]);
		__HAL_LTDC_RELOAD_CONFIG(hltdc);

		front_buffer = pend_buffer;
		pend_buffer = -1;
	}

	HAL_LTDC_ProgramLineEvent(hltdc, 0);
}

void LTDC_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&hltdc_discovery);
}
