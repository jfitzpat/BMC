/*
 Scan.h
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

#ifndef SCAN_H
#define SCAN_H

#include "SDCard.h"

void scan_Init();

void scan_SetEnable(uint8_t enable);
void scan_SetCurrentFrame(SD_FRAME *newFrame);

void scan_UpdateTransform (int32_t posX,
						   int32_t posY,
						   int32_t corX,
						   int32_t corY,
						   int32_t corZ,
						   int32_t blankOffset,
						   double intensity,
						   double scaleX,
						   double scaleY,
						   double scaleZ,
						   uint16_t rotX,
						   uint16_t rotY,
						   uint16_t rotZ);

uint32_t scan_GetScanRate();
void scan_SetScanRate(uint32_t newrate);

#endif
