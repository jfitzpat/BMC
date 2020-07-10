/*
	TimerCallback.c
	A simple foreground task dispatcher based on SysTick

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

#include "TimerCallback.h"


// We keep a a dynamic array of registered callbacks
typedef struct {
	func_TimerCallback func;
	int32_t rate;
	int32_t counter;
} TimerCallback;


static uint32_t TimerElements = 0;
static TimerCallback *TimerCallbacks = 0;


void timerCallback_Init()
{

}



void timerCallback_Add (func_TimerCallback function, int32_t rate)
{
	int memSize = (TimerElements + 1) * sizeof (TimerCallback);
	TimerCallback *newArray = malloc(memSize);

	if (!newArray)
		return;

	if (TimerCallbacks)
	{
		memcpy(newArray, TimerCallbacks, memSize - sizeof (TimerCallback));
		free(TimerCallbacks);
	}

	TimerCallbacks = newArray;

	TimerCallbacks[TimerElements].func = function;
	TimerCallbacks[TimerElements].rate = rate;
	TimerCallbacks[TimerElements].counter = rate;

	++TimerElements;
}


void timerCallback_Remove (func_TimerCallback function)
{
	int memSize = (TimerElements - 1) * sizeof(TimerCallback);
	TimerCallback *newArray = malloc(memSize);

	if (!newArray)
		return;

	if (TimerCallbacks)
	{
		int pos = 0;

		for (uint32_t i = 0; i < TimerElements; i++)
		{
			if (TimerCallbacks[i].func != function)
			{
				memcpy(&newArray[pos], &TimerCallbacks[i], sizeof(TimerCallback));
				++pos;
			}
		}
		free(TimerCallbacks);
	}

	TimerCallbacks = newArray;

	--TimerElements;
}



void timerCallback_Dispatch()
{
	static uint32_t lastTick = 0;

	uint32_t currentTime = HAL_GetTick();

	if (lastTick != currentTime)
	{
		int32_t elapsed = currentTime - lastTick;
		lastTick = currentTime;

		for (uint32_t idx = 0; idx < TimerElements; ++idx)
		{
			TimerCallbacks[idx].counter -= elapsed;
			if (TimerCallbacks[idx].counter <= 0)
			{
				TimerCallbacks[idx].counter = TimerCallbacks[idx].rate;
				(*(TimerCallbacks[idx].func))();
			}
		}
	}
}
