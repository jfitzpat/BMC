/*
	UI.C
	Manage user input

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

#include "UI.h"
#include "TimerCallback.h"
#include "Display.h"
#include "stm32f769i_discovery_ts.h"

static void UiCallback();

void ui_Init()
{
	// If we can't initialize the TouchScreen, bail
	if (BSP_TS_Init(DISPLAY_WIDTH, DISPLAY_HEIGHT) != TS_OK)
		return;

	timerCallback_Add(&UiCallback, 50);
}

void UiCallback()
{
	TS_StateTypeDef ts;

	if (BSP_TS_GetState(&ts) == TS_OK)
	{
		if (ts.touchDetected)
		{
			display_DrawCursor(ts.touchX[0], ts.touchY[0]);
//			trace_printf("ts: %d %d %d\n", ts.touchDetected, ts.touchX[0], ts.touchY[0]);
		}
	}

}
