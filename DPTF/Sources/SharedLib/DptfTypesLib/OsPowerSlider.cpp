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

#include "OsPowerSlider.h"

namespace OsPowerSlider
{
	std::string toString(OsPowerSlider::Type osPowerSlider)
	{
		switch (osPowerSlider)
		{
		case BatterySaver:
			return "Battery Saver";
		case BetterBattery:
			return "Better Battery";
		case BetterPerformance:
			return "Better Performance";
		case BestPerformance:
			return "Best Performance";
		default:
			throw dptf_exception("OsPowerSlider::Type is invalid");
		}
	}

	OsPowerSlider::Type toType(UIntN value)
	{
		if (value == 25)
		{
			return OsPowerSlider::BatterySaver;
		}
		else if (value == 50)
		{
			return OsPowerSlider::BetterBattery;
		}
		else if (value == 75)
		{
			return OsPowerSlider::BetterPerformance;
		}
		else if (value == 100)
		{
			return OsPowerSlider::BestPerformance;
		}

		throw dptf_exception("Value is invalid");
	}
}
