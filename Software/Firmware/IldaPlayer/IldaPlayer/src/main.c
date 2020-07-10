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

#include "SysTick.h"
#include "TimerCallback.h"
#include "Led.h"
#include "UIGraphics.h"
#include "Display.h"

// At this point clocks should be setup and HAL should be initialized

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
{
  // Send a greeting to the trace device (skipped on Release).
  trace_puts("Hello ARM World!");

  // At this stage the system clock should have already been configured
  // at high speed.
  trace_printf("System clock: %u Hz\n", SystemCoreClock);

  // Initialize all the modules in their dependant order
  sysTick_Init();
  timerCallback_Init();
  led_Init();
  uiGraphics_Init();
  display_Init();

  // Infinite loop
  while (1)
  {
	  timerCallback_Dispatch();
  }
  // Infinite loop, never return.
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
