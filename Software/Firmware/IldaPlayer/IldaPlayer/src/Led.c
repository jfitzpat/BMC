/*
 Led.c
 Blink a stupid light

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

#include "Led.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"

static void LedTimerCallback(void const* argument);

void led_Init()
{
	// Enable clock for GPIOJ
	__HAL_RCC_GPIOJ_CLK_ENABLE();

	// Setup PJ5 as PP Output
	GPIO_InitTypeDef GPIO_InitStruct;

	// Led off
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_5, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

	osTimerDef(LEDtimer, LedTimerCallback);
	osTimerId osTimer = osTimerCreate(osTimer(LEDtimer), osTimerPeriodic, NULL);
	osTimerStart(osTimer, 500);
}

void LedTimerCallback(void const* argument)
{
	(void) argument;

	HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_5);
}
