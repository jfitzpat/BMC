/*
 DMX.c
 DMX in and out

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
#include "diag/Trace.h"

#include "DMX.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

#if 0
static void DmxTimerCallback(void const* argument);
#endif

static UART_HandleTypeDef huart;

void dmx_Init()
{
	__HAL_RCC_USART6_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// Setup PC6 and 6 and UART
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	huart.Instance = USART6;
	huart.Init.BaudRate = 250000;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.WordLength = UART_WORDLENGTH_8B;

	HAL_UART_Init(&huart);

	HAL_NVIC_SetPriority(USART6_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USART6_IRQn);

#if 0
	// Echo test
	osTimerDef(DMXtimer, DmxTimerCallback);
	osTimerId osTimer = osTimerCreate(osTimer(DMXtimer), osTimerPeriodic, NULL);
	osTimerStart(osTimer, 1000);
#endif
}

#if 0
void DmxTimerCallback(void const* argument)
{
	static uint8_t inbuf[10];
	(void)argument;

	HAL_UART_Receive_IT(&huart, inbuf, sizeof(inbuf));
	HAL_UART_Transmit(&huart, (uint8_t *)"Hello UART", 10, 1000);
}
#endif

void USART6_IRQHandler()
{
	HAL_UART_IRQHandler(&huart);
}
