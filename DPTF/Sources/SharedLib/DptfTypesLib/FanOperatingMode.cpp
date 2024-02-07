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

#include "FanOperatingMode.h"
#include "StringConverter.h"

namespace FanOperatingMode
{
	std::string toString(FanOperatingMode::Type type)
	{
		switch (type)
		{
		case FanOperatingMode::DefaultFanOperatingMode:
			return "Default";
		case FanOperatingMode::PerformanceFanOperatingMode:
			return "Performance";
		case FanOperatingMode::BalancedFanOperatingMode:
			return "Balanced";
		case FanOperatingMode::QuietFanOperatingMode:
			return "Quiet";
		case FanOperatingMode::MaintenanceFanOperatingMode:
			return "Maintenance";
		default:
			return Constants::InvalidString;
		}
	}

	FanOperatingMode::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return FanOperatingMode::DefaultFanOperatingMode;
		}
		else if (value == 1)
		{
			return FanOperatingMode::PerformanceFanOperatingMode;
		}
		else if (value == 2)
		{
			return FanOperatingMode::BalancedFanOperatingMode;
		}
		else if (value == 3)
		{
			return FanOperatingMode::QuietFanOperatingMode;
		}
		else if (value == 15)
		{
			return FanOperatingMode::MaintenanceFanOperatingMode;
		}
		throw dptf_exception("type is invalid");
	}
}