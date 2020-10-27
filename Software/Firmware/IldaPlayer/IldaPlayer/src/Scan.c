/*
 Scan.c
 Scan Engine for ILDA images

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

#include "Scan.h"
#include "GUI.h"
#include "SDCard.h"
#include "stm32f7xx_hal.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SCAN_MAX_RATE (22000)
#define SCAN_MIN_ARR (((SystemCoreClock / 2) / SCAN_MAX_RATE) - 1)

#define DEG_IN_RAD (0.01745329252)

static const SD_FRAME EmptyFrame =
{ 1,			// 1 point
		{
		{ 0 },
		{ 0 },
		{ 0 }, 	// xyz at origin
				0xC0,		// Blanked, last point
				0, 0, 0 }		// All colors at 0
};

static SPI_HandleTypeDef Spi_Handle;
static DMA_HandleTypeDef Dma_Handle;

volatile uint8_t NewFrameRequest = 0;
SD_FRAME *NewFrame;
SD_FRAME *CurrentFrame;
int32_t curPoint = 0;
uint8_t DacOut[17];

// Sintable in .1 degree steps
static const double SinTable[] = {
#include "sintable.inc"
};

typedef struct {
	int32_t posX;			// X, Y position
	int32_t posY;
	int32_t roX;			// Rotation offset
	int32_t roY;
	int32_t roZ;
	int32_t blankOffset;	// Blanking/Color offset
	double intensity;		// Intensity (0-1)
	double scaleX;			// X,Y, and Z scale (0-2)
	double scaleY;
	double scaleZ;
	double matrix11;		// 3x3 Rotation Matrix
	double matrix12;
	double matrix13;
	double matrix21;
	double matrix22;
	double matrix23;
	double matrix31;
	double matrix32;
	double matrix33;
} TRANSFORM;

TRANSFORM currentTransform = { 0, 0,
							   0, 0, 0,
							   0,
							   1.0,
							   0.5, 0.5, 0.5,
							   1.0, 0, 0,
							   0, 1.0, 0,
							   0, 0, 1.0 };

TRANSFORM pendingTransform = { 0, 0,
		   	   	   	   	   	   0, 0, 0,
							   0,
							   1.0,
							   0.5, 0.5, 0.5,
							   1.0, 0, 0,
							   0, 1.0, 0,
							   0, 0, 1.0 };

uint8_t UpdateTransform = 0;


static void spi_Read(void *bout, uint16_t bcount);
static void spi_Write(void *bout, uint16_t bcount);

void scan_Init()
{
	// Default to lone dot
	CurrentFrame = (SD_FRAME*) (&EmptyFrame);

	// Lots of clocks to turn on
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	// Configure GPIO and SPI pins
	GPIO_InitTypeDef GPIO_InitStructure;

	// SPI CLK
	GPIO_InitStructure.Pin = GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	// SPI MISO and MOSI
	GPIO_InitStructure.Pin = GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	// SPI NSS (GPIO, not SPI controlled)
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	// /LDAC Line
	HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

	// Shutter
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_3, GPIO_PIN_RESET);

	GPIO_InitStructure.Pin = GPIO_PIN_3;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOJ, &GPIO_InitStructure);

	// Blanking
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);

	GPIO_InitStructure.Pin = GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

	// Setup the SPI peripheral
	Spi_Handle.Instance = SPI2;
	Spi_Handle.Init.Mode = SPI_MODE_MASTER;
	Spi_Handle.Init.Direction = SPI_DIRECTION_2LINES;
	Spi_Handle.Init.DataSize = SPI_DATASIZE_8BIT;
	Spi_Handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
	Spi_Handle.Init.CLKPhase = SPI_PHASE_1EDGE;
	Spi_Handle.Init.NSS = SPI_NSS_SOFT;
	Spi_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	Spi_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
	Spi_Handle.Init.TIMode = SPI_TIMODE_DISABLE;
	Spi_Handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	Spi_Handle.Init.CRCPolynomial = 0;

	HAL_SPI_Init(&Spi_Handle);

	// Setup a DMA channel to use
	Dma_Handle.Instance = DMA1_Stream4;
	Dma_Handle.Init.Channel = DMA_CHANNEL_0;
	Dma_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	Dma_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	Dma_Handle.Init.MemBurst = DMA_MBURST_INC4;
	Dma_Handle.Init.PeriphBurst = DMA_PBURST_INC4;
	Dma_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	Dma_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	Dma_Handle.Init.MemInc = DMA_MINC_ENABLE;
	Dma_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	Dma_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	Dma_Handle.Init.Mode = DMA_NORMAL;
	Dma_Handle.Init.Priority = DMA_PRIORITY_LOW;

	HAL_DMA_Init(&Dma_Handle);

	// Associate the two handles
	__HAL_LINKDMA(&Spi_Handle, hdmatx, Dma_Handle);

	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	// Timer 2 is our master controller
	TIM2->PSC = 0;
	// Initialize the period to get 28 kHz as frequency from 96MHz
	TIM2->ARR = SCAN_MIN_ARR;
	// Select Clock Divison of 1
	TIM2->CR1 &= ~ TIM_CR1_CKD;
	// buffer ARR
	TIM2->CR1 |= TIM_CR1_ARPE;
	// CMS 00 is edge aligned up/down counter
	// DIR 0 = up
	TIM2->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
	//* Trigger of TIM2 Update into TIM1 Slave
	TIM2->CR2 &= ~TIM_CR2_MMS;
	TIM2->CR2 |= TIM_CR2_MMS_1; // 010 = Update

	// Configure the repetition counter
	TIM2->RCR = 0;

	// Enable Interrupt
	TIM2->DIER |= TIM_DIER_UIE;

	// Start the scanner
//	TIM2->CR1 |= TIM_CR1_CEN;
}

static void SetupDAC()
{
	// Setup the DAC
	uint8_t bout[3];
	uint8_t bin[3];
	uint8_t spi[3];

	// Read the Chip ID
	// Probably should allow 12 and 10 bit variants !!!!
	bout[0] = 0x81;
	bout[1] = 0;
	bout[2] = 0;

	spi_Write(bout, sizeof(bout));
	spi_Read(bin, sizeof(bin));

	// Setup the rest of the DAC registers
	bout[0] = 0xC;		// DACRANGE1
	bout[1] = 0x99;		// 0 and 1 to 0-5
	bout[2] = 0x0; 		// 2 and 3 to +/-5V
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x4;		// GENCONFIG
	bout[1] = 0x00;		// Power up internal reference
	bout[2] = 0x00;
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x83;		// SPICONFIG
	spi_Write(bout, sizeof(bout));
	spi_Read(spi, sizeof(spi));

	bout[0] = 0x3;
	bout[1] = spi[1];
	bout[2] = spi[2];

	bout[2] &= (~0x20);	// Active mode
	bout[2] |= 0x8;		// Enable streaming mode
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x9;		// DACPWDWN
	bout[1] = 0xF0;		// Turn on all 8 DACs
	bout[2] = 0x0F;
	spi_Write(bout, sizeof(bout));

	// Initialize our DAC output packet
	// We'll use a stream write staring at DAC0
	DacOut[0] = 0x14;
	DacOut[3] = 0;
	DacOut[4] = 0;
	DacOut[7] = 0;
	DacOut[8] = 0;
	DacOut[11] = 0;
	DacOut[12] = 0;
	DacOut[13] = 0;
	DacOut[14] = 0;
	DacOut[15] = 0;
	DacOut[16] = 0;
}

void scan_SetEnable(uint8_t enable)
{
	static uint8_t firstEnable = 0;
	if (enable)
	{
		// We defer setting up the DAC for two reasons
		// First, it gives the DAC itself time to initialize
		// after reset.
		// Second, it makes sure we start at the first point of the
		// first frame we loaded
		if (!firstEnable)
		{
			// Configure DAC
			SetupDAC();

			// Open Shutter
			HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_3, GPIO_PIN_SET);

			// Only once
			firstEnable = 1;
		}

		TIM2->CR1 |= TIM_CR1_CEN;
	}
	else
		TIM2->CR1 &= ~TIM_CR1_CEN;
}

// If not scanning, just stick it in, otherwise
// Make a request for the IRQ to do it
void scan_SetCurrentFrame(SD_FRAME *newFrame)
{
	if (! TIM2->CR1 & TIM_CR1_CEN)
	{
		CurrentFrame = newFrame;
		curPoint = 0;
		NewFrameRequest = 0;
	}
	else
	{
		// Wait for any pending request to finish
//		while (NewFrameRequest)
//			;
		NewFrame = newFrame;
		NewFrameRequest = 1;
	}
}

uint32_t scan_GetScanRate()
{
	return TIM2->ARR;
}

void scan_SetScanRate(uint32_t newrate)
{
	if (newrate < SCAN_MIN_ARR)
		newrate = SCAN_MIN_ARR;

	TIM2->ARR = newrate;
}

static void Multiply3by3(double in1[3][3], double in2[3][3], double out[3][3])
{
	for (int col = 0 ; col < 3; ++col)
	{
		for (int row = 0; row < 3; ++row)
		{
			double d = 0;
			d += in1[row][0] * in2[0][col];
			d += in1[row][1] * in2[1][col];
			d += in1[row][2] * in2[2][col];
			out[row][col] = d;
		}
	}
}

void scan_UpdateTransform (int32_t posX,
						   int32_t posY,
						   int32_t roX,
						   int32_t roY,
						   int32_t roZ,
						   int32_t blankOffset,
						   double intensity,
						   double scaleX,
						   double scaleY,
						   double scaleZ,
						   uint16_t rotX,
						   uint16_t rotY,
						   uint16_t rotZ)
{
	// Keep some rotation matrixes around
	static double rx[3][3] = {{1, 0, 0},
							  {0, 1, 0},
							  {0, 0, 1}};
	static double ry[3][3] = {{1, 0, 0},
			  	  	  	  	  {0, 1, 0},
							  {0, 0, 1}};
	static double rz[3][3] = {{1, 0, 0},
			  	  	  	  	  {0, 1, 0},
							  {0, 0, 1}};
	// Update already peding !!!!
	// Ideally we should just update to the latest, but the structure
	// isn't an automic operation
	if (UpdateTransform)
		return;

	// Position and center of rotation are just copies
	pendingTransform.posX = posX;
	pendingTransform.posY = posY;
	pendingTransform.roX = roX;
	pendingTransform.roY = roY;
	pendingTransform.roZ = roZ;
	pendingTransform.blankOffset = blankOffset;
	pendingTransform.intensity = intensity;
	pendingTransform.scaleX = scaleX;
	pendingTransform.scaleY = scaleY;
	pendingTransform.scaleZ = scaleZ;

	// Scale and rotations we convert to a affine transform matrix
	double sin, cos;

	// Clip rotation
	if (rotX > 3599)
		rotX = 0;

	// Get sin and cos, sin is straight lookup,
	// cos is 90 degres offset from sin
	sin = SinTable[rotX];
	rotX += 900;
	if (rotX > 3599)
		rotX -= 3600;
	cos = SinTable[rotX];

	rx[1][1] = cos;
	rx[2][2] = cos;
	rx[1][2] = 0 - sin;
	rx[2][1] = sin;

	// Repeat for Y
	if (rotY > 3599)
		rotY = 0;

	sin = SinTable[rotY];
	rotY += 900;
	if (rotY > 3599)
		rotY -= 3600;
	cos = SinTable[rotY];

	ry[0][0] = cos;
	ry[2][2] = cos;
	ry[2][0] = 0 - sin;
	ry[0][2] = sin;

	// And Z
	if (rotZ > 3599)
		rotZ = 0;

	sin = SinTable[rotZ];
	rotZ += 900;
	if (rotZ > 3599)
		rotZ -= 3600;
	cos = SinTable[rotZ];

	rz[0][0] = cos;
	rz[1][1] = cos;
	rz[0][1] = 0 - sin;
	rz[1][0] = sin;

	double out1[3][3];
	Multiply3by3(rx, ry, out1);
	double out2[3][3];
	Multiply3by3(out1, rz, out2);

	pendingTransform.matrix11 = out2[0][0];
	pendingTransform.matrix12 = out2[1][0];
	pendingTransform.matrix13 = out2[2][0];
	pendingTransform.matrix21 = out2[0][1];
	pendingTransform.matrix22 = out2[1][1];
	pendingTransform.matrix23 = out2[2][1];
	pendingTransform.matrix31 = out2[0][2];
	pendingTransform.matrix32 = out2[1][2];
	pendingTransform.matrix33 = out2[2][2];

	UpdateTransform = 1;
}

// Read from Temp chip
void spi_Read(void *bout, uint16_t bcount)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Receive(&Spi_Handle, (uint8_t*) bout, bcount, 100000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
}

// Write to DAC
void spi_Write(void *bout, uint16_t bcount)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&Spi_Handle, (uint8_t*) bout, bcount, 100000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

}

void TIM2_IRQHandler()
{
	if (TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;

		// Latch the previous values
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

		ILDA_FORMAT_4 *pntData;
		pntData = &(CurrentFrame->points);

		int32_t val;
		double d;
		double dx, dy, dz;
		uint8_t clip = 0;

		dx = pntData[curPoint].x.w;
		dx *= currentTransform.scaleX;
		dx += currentTransform.roX;
		dy = pntData[curPoint].y.w;
		dy *= currentTransform.scaleY;
		dy += currentTransform.roY;
		dz = pntData[curPoint].z.w;
		dz *= currentTransform.scaleZ;
		dz += currentTransform.roZ;

		d = dx * currentTransform.matrix11 + dy * currentTransform.matrix12 + dz * currentTransform.matrix13;
		val = (int32_t)d;
		val += currentTransform.posX;
		if (val < -32768)
		{
			val = -32768;
			clip = 1;
		}
		if (val > 32767)
		{
			val = 32767;
			clip = 1;
		}

		val += 32768;
		DacOut[7] = val >> 8;
		DacOut[8] = val & 0xFF;

		d = dx * currentTransform.matrix21 + dy * currentTransform.matrix22 + dz * currentTransform.matrix23;
		val = (int32_t)d;
		val += currentTransform.posY;
		if (val < -32768)
		{
			val = -32768;
			clip = 1;
		}
		if (val > 32767)
		{
			val = 32767;
			clip = 1;
		}

		val += 32768;
		DacOut[5] = val >> 8;
		DacOut[6] = val & 0xFF;

		int16_t idx;
		if (CurrentFrame->numPoints > (uint32_t)abs(currentTransform.blankOffset))
		{
			idx = curPoint + currentTransform.blankOffset;
			if (idx < 0)
				idx += CurrentFrame->numPoints;
			else if (idx >= (int16_t)CurrentFrame->numPoints)
				idx -= CurrentFrame->numPoints;
		}
		else
			idx = curPoint;

		if ((pntData[idx].status & 0x40) || clip)
		{
			DacOut[3] = DacOut[4] = 0;
			DacOut[1] = DacOut[2] = 0;
			DacOut[15] = DacOut[16] = 0;
		}
		else
		{
			double i;
			i = pntData[idx].red * currentTransform.intensity;
			DacOut[3] = (uint8_t)i;
			i = pntData[idx].green * currentTransform.intensity;
			DacOut[1] = (uint8_t)i;
			i = pntData[idx].blue * currentTransform.intensity;
			DacOut[15] = (uint8_t)i;
		}

		if (pntData[curPoint].status & 0x80)
		{
			curPoint = 0;

			// Is it time for a new frame?
			if (NewFrameRequest)
			{
				CurrentFrame = NewFrame;
				NewFrameRequest = 0;
			}

			// Update transform?
			if (UpdateTransform)
			{
				memcpy (&currentTransform, &pendingTransform, sizeof(TRANSFORM));
				UpdateTransform = 0;
			}
		}
		else
			++curPoint;

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_SPI_Transmit_DMA(&Spi_Handle, (uint8_t*) DacOut, sizeof(DacOut));
	}
}

void DMA1_Stream4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&Dma_Handle);
}
