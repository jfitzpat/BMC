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

static const SD_FRAME EmptyFrame = {
		1,			// 1 point
		{{0}, {0}, {0}, 	// xyz at origin
		0xC0,		// Blanked, last point
		0, 0, 0}		// All colors at 0
};

static SPI_HandleTypeDef Spi_Handle;
static DMA_HandleTypeDef Dma_Handle;

SD_FRAME* CurrentFrame;
int32_t curPoint = 0;
uint8_t DacOut[17];

static void spi_Read (void *bout, uint16_t bcount);
static void spi_Write (void *bout, uint16_t bcount);

void scan_Init()
{
	// Default to lone dot
	CurrentFrame = (SD_FRAME*)(&EmptyFrame);

	// Lots of clocks to turn on
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	// Configure GPIO and SPI pins
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init (GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init (GPIOB, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init (GPIOA, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init (GPIOH, &GPIO_InitStructure);

	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = GPIO_PIN_4;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Alternate = 0;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init (GPIOJ, &GPIO_InitStructure);

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
    Dma_Handle.Instance                 = DMA1_Stream4;
    Dma_Handle.Init.Channel             = DMA_CHANNEL_0;
    Dma_Handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    Dma_Handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    Dma_Handle.Init.MemBurst            = DMA_MBURST_INC4;
    Dma_Handle.Init.PeriphBurst         = DMA_PBURST_INC4;
    Dma_Handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    Dma_Handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    Dma_Handle.Init.MemInc              = DMA_MINC_ENABLE;
    Dma_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    Dma_Handle.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    Dma_Handle.Init.Mode                = DMA_NORMAL;
    Dma_Handle.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&Dma_Handle);

    // Associate the two handles
    __HAL_LINKDMA(&Spi_Handle, hdmatx, Dma_Handle);

    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

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
	bout[0] = 0xC;
	bout[1] = 0x99;
	bout[2] = 0x99;
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x4;
	bout[1] = 0x00;
	bout[2] = 0x0C;
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x83;
	spi_Write(bout, sizeof(bout));
	spi_Read(spi, sizeof(spi));

	bout[0] = 0x3;
	bout[1] = spi[1];
	bout[2] = spi[2];

	bout[2] &= (~0x20);
	bout[2] |= 0x8;
	spi_Write(bout, sizeof(bout));

	bout[0] = 0x9;
	bout[1] = 0xF0;
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

	ILDA_FORMAT_4* pntData;
	pntData = &(CurrentFrame->points);

	int32_t val;

	val = pntData[curPoint].x.w;
	val += 32768;
	DacOut[1] = val >> 8;
	DacOut[2] = val & 0xFF;

	val = pntData[curPoint].y.w;
	val += 32768;
	DacOut[5] = val >> 8;
	DacOut[6] = val & 0xFF;

	if (pntData[curPoint].status & 0x40)
		DacOut[9] = DacOut[10] = 0;
	else
		DacOut[9] = DacOut[10] = 0xFF;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&Spi_Handle, (uint8_t *)DacOut, sizeof(DacOut), 100000);
	// CS will get released in timer interrupt

	if (pntData[curPoint].status & 0x80)
		curPoint = 0;
	else
		++curPoint;

	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	// Timer 2 is our master controller
	TIM2->PSC = 0;
	// Initialize the PWM period to get 50 Hz as frequency from 1MHz
	TIM2->ARR = ((SystemCoreClock / 2) / 28000) - 1;
	// Select Clock Divison of 1
	TIM2->CR1 &= ~ TIM_CR1_CKD;
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


void scan_SetEnable(uint8_t enable)
{
	if (enable)
		TIM2->CR1 |= TIM_CR1_CEN;
	else
		TIM2->CR1 &= ~TIM_CR1_CEN;
}

// This should really be a request handled at the interrupt
// so graphics complete by default !!!!
void scan_SetCurrentFrame (SD_FRAME* newFrame)
{
	uint32_t reg = TIM2->CR1;

	scan_SetEnable(0);
	CurrentFrame = newFrame;
	curPoint = 0;

	if (reg & TIM_CR1_CEN)
		scan_SetEnable(1);
}

// Read from Temp chip
void spi_Read (void *bout, uint16_t bcount)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Receive(&Spi_Handle, (uint8_t *)bout, bcount, 100000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
}

// Write to DAC
void spi_Write (void *bout, uint16_t bcount)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&Spi_Handle, (uint8_t *)bout, bcount, 100000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

}

void TIM2_IRQHandler()
{
	if (TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR &= ~TIM_SR_UIF;

		// Latch the previous values
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

		ILDA_FORMAT_4* pntData;
		pntData = &(CurrentFrame->points);

		int32_t val;
		val = pntData[curPoint].x.w;
		val += 32768;
		DacOut[1] = val >> 8;
		DacOut[2] = val & 0xFF;

		val = pntData[curPoint].y.w;
		val += 32768;
		DacOut[5] = val >> 8;
		DacOut[6] = val & 0xFF;

		int16_t idx;
		if (CurrentFrame->numPoints > 4)
		{
			idx = curPoint - 4;
			if (idx < 0)
				idx += CurrentFrame->numPoints;
		}
		else
			idx = curPoint;

		if (pntData[idx].status & 0x40)
			DacOut[9] = DacOut[10] = 0;
		else
			DacOut[9] = DacOut[10] = 0xFF;

		if (pntData[curPoint].status & 0x80)
			curPoint = 0;
		else
			++curPoint;

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_SPI_Transmit_DMA(&Spi_Handle, (uint8_t *)DacOut, sizeof(DacOut));
	}
}

void DMA1_Stream4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&Dma_Handle);
}
