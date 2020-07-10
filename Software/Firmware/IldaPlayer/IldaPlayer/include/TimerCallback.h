/*
	TimerCallback.h
	Foreground Dispatcher based on SysTick

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

#ifndef TIMERCALLBACK_H_
#define TIMERCALLBACK_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stm32f7xx_hal.h"

// Callback function typedef
typedef void (*func_TimerCallback)();


// Module Interface
void timerCallback_Init();

void timerCallback_Add (func_TimerCallback, int32_t rate);
void timerCallback_Remove (func_TimerCallback function);

// Dispatch Loop
// Should be called periodically from main loop
void timerCallback_Dispatch();


#endif /* TIMERCALLBACK_H_ */
