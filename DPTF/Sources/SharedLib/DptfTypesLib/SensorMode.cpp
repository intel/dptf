/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "SensorMode.h"

namespace SensorMode
{
	std::string toString(SensorMode::Type sensorUserPresence)
	{
		switch (sensorUserPresence)
		{
		case Off:
			return "Off";
		case Disabled:
			return "Disabled";
		case Wof:
			return "Wof";
		case Vision:
			return "Vision";
		case Passthru:
			return "Passthru";
		case Privacy:
			return "Privacy";
		case Invalid:
			return "Invalid";
		default:
			throw dptf_exception("SensorMode::Type is invalid");
		}
	}

	SensorMode::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return SensorMode::Off;
		}
		else if (value == 1)
		{
			return SensorMode::Disabled;
		}
		else if (value == 2)
		{
			return SensorMode::Wof;
		}
		else if (value == 3)
		{
			return SensorMode::Vision;
		}
		else if (value == 4)
		{
			return SensorMode::Passthru;
		}
		else if (value == 5)
		{
			return SensorMode::Privacy;
		}
		else if (value == Constants::Invalid)
		{
			return SensorMode::Invalid;
		}
		else
		{
			throw dptf_exception("Sensor mode value is invalid");
		}
	}
}
