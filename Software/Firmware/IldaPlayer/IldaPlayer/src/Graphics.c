/*
 Graphics.c
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

#include "Graphics.h"
#include "stm32f7xx_hal.h"

#define ABS(X) ((X) > 0 ? (X) : -(X))

static DMA2D_HandleTypeDef hdma2d;

static uint32_t TargetAddress = 0;
static uint32_t BrushColor = 0;
static uint32_t BackColor = 0;
static sFONT *Font;

// Internal Functions
static void FillBuffer(void *pDst, uint32_t xSize, uint32_t ySize,
		uint32_t OffLine, uint32_t ColorIndex);
static void ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize,
		uint32_t ColorMode);
static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c);

void graphics_Init()
{
	BackColor = 0xFFFFFFFF;
	BrushColor = 0xFF000000;
	Font = &Font24;
}

void graphics_SetFont(sFONT *newfont)
{
	Font = newfont;
}

sFONT* graphics_GetFont()
{
	return Font;
}

void graphics_SetTargetAddress(uint32_t newtarget)
{
	TargetAddress = newtarget;
}

uint32_t graphics_GetTargetAddress()
{
	return TargetAddress;
}

void graphics_SetBrushColor(uint32_t newcolor)
{
	BrushColor = newcolor;
}

uint32_t graphics_GetBrushColor()
{
	return BrushColor;
}

void graphics_SetBackColor(uint32_t newcolor)
{
	BackColor = newcolor;
}

uint32_t graphics_GetBackColor()
{
	return BackColor;
}

void graphics_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y,
		uint16_t xsize, uint16_t ysize)
{
	uint32_t destination = (uint32_t) pDst
			+ (y * DISPLAY_WIDTH + x) * BYTES_PER_PIXEL;
	uint32_t source = (uint32_t) pSrc;

	// #1 Configure the DMA2D Mode, Color Mode and output offset
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
	hdma2d.Init.OutputOffset = DISPLAY_WIDTH - xsize;
	hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA; // No Output Alpha Inversion
	hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR; // No Output Red & Blue swap

	// #2 DMA2D Callbacks Configuration
	hdma2d.XferCpltCallback = NULL;

	// #3 Foreground Configuration
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; // No ForeGround Red/Blue swap
	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; // No ForeGround Alpha inversion

	hdma2d.Instance = DMA2D;

	// DMA2D Initialization
	if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&hdma2d, source, destination, xsize, ysize)
					== HAL_OK)
			{
				// Polling For DMA transfer
				HAL_DMA2D_PollForTransfer(&hdma2d, 100);
			}
		}
	}
}

void graphics_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width,
		uint16_t Height)
{
	graphics_DrawHLine(Xpos, Ypos, Width);
	graphics_DrawHLine(Xpos, (Ypos + Height), Width);
	graphics_DrawVLine(Xpos, Ypos, Height);
	graphics_DrawVLine((Xpos + Width), Ypos, Height);
}

void graphics_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width,
		uint16_t Height)
{
	uint32_t Xaddress = 0;

	Xaddress = TargetAddress + BYTES_PER_PIXEL * (DISPLAY_WIDTH * Ypos + Xpos);
	FillBuffer((uint32_t*) Xaddress, Width, Height, (DISPLAY_WIDTH - Width),
			BrushColor);
}

void graphics_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
	uint32_t Xaddress = 0;

	Xaddress = TargetAddress + BYTES_PER_PIXEL * (DISPLAY_WIDTH * Ypos + Xpos);
	FillBuffer((uint32_t*) Xaddress, Length, 1, 0, BrushColor);
}

void graphics_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
	uint32_t Xaddress = 0;

	Xaddress = TargetAddress + BYTES_PER_PIXEL * (DISPLAY_WIDTH * Ypos + Xpos);
	FillBuffer((uint32_t*) Xaddress, 1, Length, (DISPLAY_WIDTH - 1),
			BrushColor);
}

void graphics_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 =
			0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
			curpixel = 0;

	deltax = ABS(x2 - x1); // The difference between the x's
	deltay = ABS(y2 - y1); // The difference between the y's
	x = x1; // Start x off at the first pixel
	y = y1; // Start y off at the first pixel

	if (x2 >= x1) // The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else // The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) // The y-values are increasing
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else // The y-values are decreasing
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) // There is at least one x-value for every y-value
	{
		xinc1 = 0; // Don't change the x when numerator >= denominator
		yinc2 = 0; // Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax; // There are more x-values than y-values
	}
	else // There is at least one y-value for every x-value
	{
		xinc2 = 0; // Don't change the x for every iteration
		yinc1 = 0; // Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay; // There are more y-values than x-values
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		// graphics_DrawPixel(x, y, BrushColor); // Draw the current pixel
		*(__IO uint32_t*) (TargetAddress
				+ (BYTES_PER_PIXEL * (y * DISPLAY_WIDTH + x))) = BrushColor;

		num += numadd; // Increase the numerator by the top of the fraction
		if (num >= den) // Check if numerator >= denominator
		{
			num -= den; // Calculate the new numerator value
			x += xinc1; // Change the x as appropriate
			y += yinc1; // Change the y as appropriate
		}
		x += xinc2; // Change the x as appropriate
		y += yinc2; // Change the y as appropriate
	}
}

void graphics_DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp)
{
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
	uint32_t Address;
	uint32_t InputColorMode = 0;

	// Get bitmap data address offset
	index = pbmp[10] + (pbmp[11] << 8) + (pbmp[12] << 16) + (pbmp[13] << 24);

	// Read bitmap width
	width = pbmp[18] + (pbmp[19] << 8) + (pbmp[20] << 16) + (pbmp[21] << 24);

	// Read bitmap height
	height = pbmp[22] + (pbmp[23] << 8) + (pbmp[24] << 16) + (pbmp[25] << 24);

	// Read bit/pixel
	bit_pixel = pbmp[28] + (pbmp[29] << 8);

	// Set the address
	Address = TargetAddress
			+ (((DISPLAY_WIDTH * Ypos) + Xpos) * (BYTES_PER_PIXEL));

	// Get the layer pixel format
	if ((bit_pixel / 8) == BYTES_PER_PIXEL)
	{
		InputColorMode = DMA2D_INPUT_ARGB8888;
	}
	else if ((bit_pixel / 8) == 2)
	{
		InputColorMode = DMA2D_INPUT_RGB565;
	}
	else
	{
		InputColorMode = DMA2D_INPUT_RGB888;
	}

	// Bypass the bitmap header
	pbmp += (index + (width * (height - 1) * (bit_pixel / 8)));

	// Convert picture to ARGB8888 pixel format
	for (index = 0; index < height; index++)
	{
		// Pixel format conversion
		ConvertLineToARGB8888((uint32_t*) pbmp, (uint32_t*) Address, width,
				InputColorMode);

		// Increment the source and destination buffers
		Address += (DISPLAY_WIDTH * BYTES_PER_PIXEL);
		pbmp -= width * (bit_pixel / 8);
	}
}

void graphics_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
	*(__IO uint32_t*) (TargetAddress
			+ (BYTES_PER_PIXEL * (Ypos * DISPLAY_WIDTH + Xpos))) = RGB_Code;
}

void graphics_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
	DrawChar(Xpos, Ypos,
			&Font->table[(Ascii - ' ') * Font->Height * ((Font->Width + 7) / 8)]);
}

void graphics_DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text,
		GRAPHICS_ALIGN_MODE Mode)
{
	uint16_t refcolumn = 1, i = 0;
	uint32_t size = 0, xsize = 0;
	uint8_t *ptr = Text;

	// Get the text size
	while (*ptr++)
		size++;

	// Characters number per line
	xsize = (DISPLAY_WIDTH / Font->Width);

	switch (Mode)
	{
	case CENTER:
	{
		refcolumn = Xpos + ((xsize - size) * Font->Width) / 2;
		break;
	}
	case LEFT:
	{
		refcolumn = Xpos;
		break;
	}
	case RIGHT:
	{
		refcolumn = -Xpos + ((xsize - size) * Font->Width);
		break;
	}
	default:
	{
		refcolumn = Xpos;
		break;
	}
	}

	// On screen?
	if ((refcolumn < 1) || (refcolumn >= 0x8000))
	{
		refcolumn = 1;
	}

	// Write 1 char at a time
	while ((*Text != 0)
			& (((DISPLAY_WIDTH - (i * Font->Width)) & 0xFFFF) >= Font->Width))
	{
		// graphics_DisplayChar(refcolumn, Ypos, *Text);
		DrawChar(refcolumn, Ypos,
				&Font->table[((*Text) - ' ') * Font->Height * ((Font->Width + 7) / 8)]);

		refcolumn += Font->Width;

		// Point on the next character
		Text++;
		i++;
	}

}

void graphics_DisplayStringAtLine(uint16_t Line, uint8_t *ptr)
{
	graphics_DisplayStringAt(0, Line * Font->Height, ptr, LEFT);
}

//
// Internal Functions
//

static void FillBuffer(void *pDst, uint32_t xSize, uint32_t ySize,
		uint32_t OffLine, uint32_t ColorIndex)
{
	// Register to memory
	hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
	hdma2d.Init.OutputOffset = OffLine;

	hdma2d.Instance = DMA2D;

	// DMA2D Initialization
	if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&hdma2d, ColorIndex, (uint32_t) pDst, xSize,
					ySize) == HAL_OK)
			{
				// Polling For DMA transfer
				HAL_DMA2D_PollForTransfer(&hdma2d, 10);
			}
		}
	}
}

static void ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize,
		uint32_t ColorMode)
{
	// Mode and offset
	hdma2d.Init.Mode = DMA2D_M2M_PFC;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
	hdma2d.Init.OutputOffset = 0;

	// Foreground
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].InputColorMode = ColorMode;
	hdma2d.LayerCfg[1].InputOffset = 0;

	hdma2d.Instance = DMA2D;

	if (HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&hdma2d, (uint32_t) pSrc, (uint32_t) pDst,
					xSize, 1) == HAL_OK)
			{
				// Polling For DMA transfer
				HAL_DMA2D_PollForTransfer(&hdma2d, 10);
			}
		}
	}
}

static void DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *c)
{
	uint32_t i = 0, j = 0;
	uint16_t height, width;
	uint8_t offset;
	uint8_t *pchar;
	uint32_t line;

	height = Font->Height;
	width = Font->Width;

	offset = 8 * ((width + 7) / 8) - width;

	for (i = 0; i < height; i++)
	{
		pchar = ((uint8_t*) c + (width + 7) / 8 * i);

		switch (((width + 7) / 8))
		{

		case 1:
			line = pchar[0];
			break;

		case 2:
			line = (pchar[0] << 8) | pchar[1];
			break;

		case 3:
		default:
			line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
			break;
		}

		for (j = 0; j < width; j++)
		{
			if (line & (1 << (width - j + offset - 1)))
			{
				// graphics_DrawPixel((Xpos + j), Ypos, BrushColor);
				*(__IO uint32_t*) (TargetAddress
						+ (BYTES_PER_PIXEL * (Ypos * DISPLAY_WIDTH + (Xpos + j)))) = BrushColor;
			}
			else
			{
				// graphics_DrawPixel((Xpos + j), Ypos, BackColor);
				*(__IO uint32_t*) (TargetAddress
						+ (BYTES_PER_PIXEL * (Ypos * DISPLAY_WIDTH + (Xpos + j)))) = BackColor;
			}
		}
		Ypos++;
	}
}
