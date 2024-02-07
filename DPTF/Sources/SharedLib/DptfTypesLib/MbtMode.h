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

namespace MbtMode
{
	enum Type
	{
		DcBetterBattery = 0,
		DcBetterPerformance = 1,
		AcQuiet = 2,
		Balanced = 3,
		DcEnduranceGaming = 4,
		Collaboration = 5,
		IpAlignment = 6,
		Invalid
	};

	std::string toString(MbtMode::Type mbtMode);
	MbtMode::Type fromString(std::string mbtMode);
}
