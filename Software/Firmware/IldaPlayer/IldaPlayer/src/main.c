/*
 main.c
 Let's get this party started

 Initializes the modules and then loops forever dispatching
 foreground timer events

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

#include "stm32f7xx_hal.h"

#include "cmsis_os.h"

#include "SysTick.h"
#include "Led.h"
#include "UIGraphics.h"
#include "Graphics.h"
#include "SDCard.h"
#include "GUI.h"
#include "Scan.h"
#include "Network.h"

static void StartThread(void const * argument);


// At this point clocks should be setup and HAL should be initialized

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc, char *argv[])
{
	// Send a greeting to the trace device (skipped on Release).
	trace_puts("Hello ARM World!");

	// At this stage the system clock should have already been configured
	// at high speed.
	trace_printf("System clock: %u Hz\n", SystemCoreClock);

	// Initialize all the modules in their dependant order
	sysTick_Init();

	osThreadDef(Start, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 4);
	osThreadCreate (osThread(Start), NULL);

	osKernelStart();

	// Loop forever
	for( ;; );
}

static void StartThread(void const * argument)
{
	// Initialize the non timer tick modules in dependency order
	led_Init();
	sdCard_Init();
	scan_Init();
	uiGraphics_Init();
	graphics_Init();
	gui_Init();
	network_Init();
	gui_Start();

	// We're done initializing
	// kill, don't return
	while (1)
	{
		osThreadTerminate(NULL);
	}
}


#pragma GCC diagnostic pop
