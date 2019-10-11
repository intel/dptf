/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#include "Dptf.h"
#include "esif_sdk_data.h"

struct EsifDataBinaryClpoPackage
{
	union esif_data_variant lpoEnable; // ULONG - If 0, LPO control not supported (even if available).
	union esif_data_variant startPstateIndex; // ULONG - When the policy should initiate active control (P state index!)
	union esif_data_variant stepSize; // ULONG - Percentage indicating how many processors to offline
	// E.g. 25% on a Dual core system with 4 logical processors, then policy would take away 1 logical processor at a
	// time
	union esif_data_variant powerControlSetting; // ULONG - If P0 (power limiting), 1 = SMT Control, 2 = Core off lining
	union esif_data_variant
		performanceControlSetting; // ULONG - If P1 (performance limiting), 1 = SMT Control, 2 = Core off lining
};
