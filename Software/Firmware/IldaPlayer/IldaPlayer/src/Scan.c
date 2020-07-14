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
#include "stm32f7xx_hal.h"

// Temp stuff
const int16_t xpig[] = {
#include "xpig.inc"
};

const int16_t ypig[] = {
#include "ypig.inc"
};

const uint8_t spig[] = {
#include "spig.inc"
};


static SPI_HandleTypeDef Spi_Handle;
static DMA_HandleTypeDef hdma_tx;

int32_t pointCount = 0;
uint8_t zout[17];
// volatile uint8_t scan = 0;

static void spi_Read (void *bout, uint16_t bcount);
static void spi_Write (void *bout, uint16_t bcount);

void scan_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;

	// Configure pin in output push/pull mode
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

    hdma_tx.Instance                 = DMA1_Stream4;
    hdma_tx.Init.Channel             = DMA_CHANNEL_0;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_tx);

    // Associate the two handles
    __HAL_LINKDMA(&Spi_Handle, hdmatx, hdma_tx);

    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

	uint8_t bout[3];
	uint8_t bin[3];
	uint8_t spi[3];

	bout[0] = 0x81;
	bout[1] = 0;
	bout[2] = 0;

	spi_Write(bout, sizeof(bout));
	spi_Read(bin, sizeof(bin));

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

	zout[0] = 0x14;
	zout[3] = 0;
	zout[4] = 0;
	zout[7] = 0;
	zout[8] = 0;
	zout[11] = 0;
	zout[12] = 0;
	zout[13] = 0;
	zout[14] = 0;
	zout[15] = 0;
	zout[16] = 0;

	int32_t val;

	val = xpig[pointCount];
	val += 32768;
	zout[1] = val >> 8;
	zout[2] = val & 0xFF;

	val = ypig[pointCount];
	val += 32768;
	zout[5] = val >> 8;
	zout[6] = val & 0xFF;

	if (spig[pointCount] & 0x40)
		zout[9] = zout[10] = 0;
	else
		zout[9] = zout[10] = 0xFF;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&Spi_Handle, (uint8_t *)zout, sizeof(zout), 100000);
//	spi_Write(zout, sizeof(zout));
	// CS will get released in timer interrupt

	if (spig[pointCount] & 0x80)
		pointCount = 0;
	else
		++pointCount;

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
	TIM2->CR1 |= TIM_CR1_CEN;

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

		// Latch Values
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);

		int32_t val;
		val = xpig[pointCount];
		val += 32768;
		zout[1] = val >> 8;
		zout[2] = val & 0xFF;

		val = ypig[pointCount];
		val += 32768;
		zout[5] = val >> 8;
		zout[6] = val & 0xFF;

		int16_t idx;
		idx = pointCount - 4;
		if (pointCount < 0)
			idx += sizeof(spig);

		if (spig[idx] & 0x40)
			zout[9] = zout[10] = 0;
		else
			zout[9] = zout[10] = 0xFF;

		if (spig[pointCount] & 0x80)
			pointCount = 0;
		else
			++pointCount;

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
//		HAL_SPI_Transmit(&Spi_Handle, (uint8_t *)zout, sizeof(zout), 100000);
		HAL_SPI_Transmit_DMA(&Spi_Handle, (uint8_t *)zout, sizeof(zout));
	}
}

void DMA1_Stream4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_tx);
}
