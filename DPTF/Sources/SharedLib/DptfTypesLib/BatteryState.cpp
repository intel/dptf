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

#include "BatteryState.h"

namespace BatteryState
{
	std::string ToString(BatteryState::State state)
	{
		switch (state)
		{
		case BatteryState::NotCharging:
			return "Not Charging";
		case BatteryState::Discharging:
			return "Discharging";
		case BatteryState::Charging:
			return "Charging";
		case BatteryState::NotChargingCritical:
			return "Not Charging Critical";
		case BatteryState::DischargingCritical:
			return "Discharging Critical";
		case BatteryState::ChargingCritical:
			return "Charging Critical";
		default:
			return Constants::InvalidString;
		}
	}
}
