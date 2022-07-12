/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "SensorOrientation.h"

namespace SensorOrientation
{
	std::string toString(SensorOrientation::Type sensorOrientationType)
	{
		switch (sensorOrientationType)
		{
		case Landscape:
			return "Landscape";
		case Portrait:
			return "Portrait";
		case LandscapeInverted:
			return "Landscape Inverted";
		case PortraitInverted:
			return "Portrait Inverted";
		case Indeterminate:
			return "Indeterminate";
		default:
			throw dptf_exception("SensorOrientation::Type is invalid");
		}
	}
}
