/*
	Display.c
	Display Management Routines

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

#include "display.h"

#include "stm32f769i_discovery_lcd.h"
#include "UIGraphics.h"

#define LCD_SCREEN_WIDTH              800
#define LCD_SCREEN_HEIGHT             480
#define ARGB8888_BYTE_PER_PIXEL       4

/* LTDC foreground layer address 800x480 in ARGB8888 */
#define LCD_FG_LAYER_ADDRESS          LCD_FB_START_ADDRESS

/* LTDC background layer address 800x480 in ARGB8888 following the foreground layer */
#define LCD_BG_LAYER_ADDRESS          LCD_FG_LAYER_ADDRESS + (LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT * ARGB8888_BYTE_PER_PIXEL)

#define INTERNAL_BUFFER_START_ADDRESS LCD_BG_LAYER_ADDRESS + (LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT * ARGB8888_BYTE_PER_PIXEL)

void display_Init()
{
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

	// Splash time!
	BSP_LCD_DrawBitmap(0, 0, (uint8_t *)uiGraphics_splash_bmp);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAtLine(12, (uint8_t *)" BMC");
	BSP_LCD_DisplayStringAtLine(13, (uint8_t *)" ILDA Player");
	BSP_LCD_DisplayStringAtLine(14, (uint8_t *)" v: 1.0");
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAtLine(29, (uint8_t *)" http://Scrootch.Me/bmc");
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_DisplayOn();

	// Adjust brightness
	// BSP_LCD_SetBrightness(100);

	// Background transparent, foreground opaque
	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_BACKGROUND, 0);
	BSP_LCD_SetTransparency(LTDC_ACTIVE_LAYER_FOREGROUND, 255);
}

