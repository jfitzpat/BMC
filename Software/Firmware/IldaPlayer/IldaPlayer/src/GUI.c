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

static void TouchCallback();
static void DrawMainBackground();
static void DrawFrame();

#define ARGB8888_BYTE_PER_PIXEL       (4)

// LTDC foreground layer address 800x480 in ARGB8888
#define LCD_FG_LAYER_ADDRESS          (LCD_FB_START_ADDRESS)
// LTDC background layer address 800x480 in ARGB8888 following the foreground layer
#define LCD_BG_LAYER_ADDRESS          (LCD_FG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * ARGB8888_BYTE_PER_PIXEL))
// Free SDRAM Start
#define INTERNAL_BUFFER_START_ADDRESS (LCD_BG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * ARGB8888_BYTE_PER_PIXEL))

SD_FRAME_TABLE* FrameTable = (SD_FRAME_TABLE *)INTERNAL_BUFFER_START_ADDRESS;

uint32_t CurrentFile = 1;
uint32_t FrameIdx = 0;

void gui_Init()
{
	// Initalize Touch
	BSP_TS_Init(DISPLAY_WIDTH, DISPLAY_HEIGHT);

	// Initialize in Video Burst Mode, bail if we rail
	if (BSP_LCD_Init() != LCD_OK)
		return;

	// Initialize our background layer, using SDRAM
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER_BACKGROUND, LCD_FB_START_ADDRESS);

	// Select and clear to black
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_BACKGROUND);
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	// Now do the same for the foreground layer
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER_FOREGROUND, LCD_BG_LAYER_ADDRESS);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_FOREGROUND);
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_BACKGROUND, 255);
	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, 0);

	// Splash time!
	BSP_LCD_DrawBitmap(0, 0, (uint8_t *)uiGraphics_splash_bmp);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAtLine(12, (uint8_t *)" BMC");
	BSP_LCD_DisplayStringAtLine(13, (uint8_t *)" ILDA Player");
	BSP_LCD_DisplayStringAtLine(14, (uint8_t *)" v: 1.0");
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(10, (29*16-2), (uint8_t *)"http://Scrootch.Me/bmc", LEFT_MODE);
	BSP_LCD_DisplayOn();

	// Adjust brightness
	// BSP_LCD_SetBrightness(100);

	// Fade in splash screen
	for (int n = 0; n <255; ++n )
	{
		BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, n);
		HAL_Delay(5);
	}

	// Load the first ILDA
	FrameTable->frameCount = 0;

	if (sdCard_GetFileCount())
	{
		sdCard_LoadIldaFile (CurrentFile, FrameTable,
				(SD_FRAME *)(INTERNAL_BUFFER_START_ADDRESS + 0x1000));
	}

	// Draw the main screen
	DrawMainBackground();

	HAL_Delay(1500);

	for (int n = 255; n > 0; --n )
	{
		BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, n);
		HAL_Delay(5);
	}

	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, DISABLE);
	HAL_Delay(50);
	DrawFrame();
	HAL_Delay(50);
	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, 255);

	// If anything loaded, setup the frame and start the scanner
	if (FrameTable->frameCount)
	{
		scan_SetCurrentFrame (FrameTable->frames[0]);
		scan_SetEnable(1);
	}

	// Start our callback for touch input
	timerCallback_Add(&TouchCallback, 50);
}

void* gui_GetFreeSDRAMBase()
{
	return (void*)INTERNAL_BUFFER_START_ADDRESS;
}

void DrawMainBackground()
{
	// Draw main screen...
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_BACKGROUND, DISABLE);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_BACKGROUND);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(DISPLAY_WIDTH - 472 - 4, 4, 472, 472);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawRect(DISPLAY_WIDTH - 472 - 4, 4, 472, 472);
	BSP_LCD_DrawRect(DISPLAY_WIDTH - 472 - 5, 3, 474, 474);

	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(10, (29*16-2), (uint8_t *)"http://Scrootch.Me/bmc", LEFT_MODE);
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_BACKGROUND, ENABLE);
}

void DrawFrame()
{
	int x = DISPLAY_WIDTH - 472 - 4;
	int y = 4;
	int w = 472;
	int h = 472;

	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, DISABLE);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_FOREGROUND);
	BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
	BSP_LCD_SetFont(&Font24);

	char outstr[30];
	if (! sdCard_GetFileCount())
		sprintf(outstr, " Files: %d", (int)sdCard_GetFileCount());
	else
		sprintf(outstr, " File: %ld of %ld", CurrentFile, sdCard_GetFileCount());

	BSP_LCD_DisplayStringAtLine(1, (uint8_t *)outstr);
	if (FrameTable->frameCount)
	{
		sprintf(outstr, " %s", FrameTable->altname);
		BSP_LCD_DisplayStringAtLine(5, (uint8_t*)outstr);
		sprintf(outstr, " Frame: %ld of %ld", FrameIdx + 1, FrameTable->frameCount);
		BSP_LCD_DisplayStringAtLine(6, (uint8_t*)outstr);
	}


	if (FrameTable->frameCount)
	{
		SD_FRAME* frame = FrameTable->frames[FrameIdx];
		ILDA_FORMAT_4* pntData = &(frame->points);

		for (uint32_t n=0; n < (frame->numPoints - 1); ++n)
		{
			if (! (pntData[n].status & 0x40))
			{
				uint32_t color = 0xFF000000 |
								 (pntData[n].red << 16) |
								 (pntData[n].green << 8) |
								 (pntData[n].blue);

				BSP_LCD_SetTextColor(color);

				// Get our coordinates
				int32_t x1, y1, x2, y2;
				x1 = pntData[n].x.w;
				y1 = pntData[n].y.w;
				x2 = pntData[n+1].x.w;
				y2 = pntData[n+1].y.w;

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

				BSP_LCD_DrawLine(x1, y1, x2, y2);
			}
		}
	}
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, ENABLE);
}

static void IncFile()
{
	if (sdCard_GetFileCount() > 1)
	{
		scan_SetEnable(0);
		++CurrentFile;
		if (CurrentFile > sdCard_GetFileCount())
			CurrentFile = 1;

		sdCard_LoadIldaFile (CurrentFile, FrameTable,
				(SD_FRAME *)(INTERNAL_BUFFER_START_ADDRESS + 0x1000));

		FrameIdx = 0;
		if (FrameTable->frameCount)
		{
			scan_SetCurrentFrame (FrameTable->frames[0]);
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

		sdCard_LoadIldaFile (CurrentFile, FrameTable,
				(SD_FRAME *)(INTERNAL_BUFFER_START_ADDRESS + 0x1000));

		FrameIdx = 0;
		if (FrameTable->frameCount)
		{
			scan_SetCurrentFrame (FrameTable->frames[0]);
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
		if (FrameIdx >= FrameTable->frameCount)
			FrameIdx = 0;

		scan_SetCurrentFrame (FrameTable->frames[FrameIdx]);
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

		scan_SetCurrentFrame (FrameTable->frames[FrameIdx]);
		DrawFrame();
	}
}

void TouchCallback()
{
	static uint8_t last = 0;

	TS_StateTypeDef ts;

	if (BSP_TS_GetState(&ts) == TS_OK)
	{
		if (ts.touchDetected == 1)
		{
			if (last == 0)
			{
				// Control zone?
				if (ts.touchX[0] < 324 && ts.touchY[0] < 240)
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
			}
		}

		last = ts.touchDetected;
	}

}

