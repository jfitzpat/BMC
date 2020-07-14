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

static void TouchCallback();


#define ARGB8888_BYTE_PER_PIXEL       4

/* LTDC foreground layer address 800x480 in ARGB8888 */
#define LCD_FG_LAYER_ADDRESS          LCD_FB_START_ADDRESS

/* LTDC background layer address 800x480 in ARGB8888 following the foreground layer */
#define LCD_BG_LAYER_ADDRESS          LCD_FG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * ARGB8888_BYTE_PER_PIXEL)

#define INTERNAL_BUFFER_START_ADDRESS LCD_BG_LAYER_ADDRESS + (DISPLAY_WIDTH * DISPLAY_HEIGHT * ARGB8888_BYTE_PER_PIXEL)

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
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_BLUE);

	char outstr[30];
	sprintf(outstr, " Files: %d", (int)sdCard_GetFileCount());
	BSP_LCD_DisplayStringAtLine(1, (uint8_t *)outstr);

	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(10, (29*16-2), (uint8_t *)"http://Scrootch.Me/bmc", LEFT_MODE);
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_BACKGROUND, ENABLE);

	HAL_Delay(1000);

	for (int n = 255; n > 0; --n )
	{
		BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, n);
		HAL_Delay(5);
	}

	HAL_Delay(100);
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, DISABLE);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_FOREGROUND);
	BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, ENABLE);
	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, 255);

	timerCallback_Add(&TouchCallback, 50);
}

static void DrawCursor (uint16_t x, uint16_t y)
{
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, DISABLE);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_FOREGROUND);
	BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawRect(x, y, 30, 30);
	BSP_LCD_SetLayerVisible(LTDC_ACTIVE_LAYER_FOREGROUND, ENABLE);
}


void TouchCallback()
{
	TS_StateTypeDef ts;

	if (BSP_TS_GetState(&ts) == TS_OK)
	{
		if (ts.touchDetected)
		{
			DrawCursor(ts.touchX[0], ts.touchY[0]);
		}
	}

}

