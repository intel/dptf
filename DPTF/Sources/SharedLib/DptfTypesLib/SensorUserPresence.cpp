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

#include "SensorUserPresence.h"

namespace SensorUserPresence
{
	std::string toString(SensorUserPresence::Type sensorUserPresence)
	{
		switch (sensorUserPresence)
		{
		case NotPresent:
			return "Not Present";
		case Disengaged:
			return "Disengaged";
		case Engaged:
			return "Engaged";
		case FaceEngaged:
			return "Face Engaged";
		case Invalid:
			return "Invalid";
		default:
			throw dptf_exception("SensorUserPresence::Type is invalid");
		}
	}

	SensorUserPresence::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return SensorUserPresence::NotPresent;
		}
		else if (value == 1)
		{
			return SensorUserPresence::Disengaged;
		}
		else if (value == 2)
		{
			return SensorUserPresence::Engaged;
		}
		else if (value == 3)
		{
			return SensorUserPresence::FaceEngaged;
		}
		else if (value == 99)
		{
			return SensorUserPresence::Invalid;
		}
		else
		{
			throw dptf_exception("sensor value is invalid");
		}
	}
}
