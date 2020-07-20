/*
 SysTick.c
 SysTick Handler

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

#include "SysTick.h"
#include "cortexm/ExceptionHandlers.h"
#include "cmsis_device.h"
#include "stm32f7xx_hal.h"

void sysTick_Init()
{
	// Use SysTick as reference for the timer dispatcher
	SysTick_Config(SystemCoreClock / SYSTICK_FREQUENCY_HZ);
}

// Interrupt Handler
// This overrides a 'weak linkage' default handler
void SysTick_Handler()
{
	HAL_IncTick();
}

