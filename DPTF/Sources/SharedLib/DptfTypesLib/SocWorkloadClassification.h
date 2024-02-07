/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

namespace SocWorkloadClassification
{
	enum Type
	{
		Idle = 1,
		SemiActive = 2,
		Bursty = 3,
		Sustained = 4,
		BatteryLife = 5,
		Invalid = 6
	};

	enum HardwareHintType
	{
		HardwareHintIdle = 0,
		HardwareHintBatteryLife = 1,
		HardwareHintSustained = 2,
		HardwareHintBursty = 3,
		HardwareHintInvalid = 4
	};

	SocWorkloadClassification::Type toSocWorkloadClassificationHint(HardwareHintType socWorkloadHint);
	std::string toString(Type socWorkloadClassification);
	bool isValid (UInt32 workload);
}
