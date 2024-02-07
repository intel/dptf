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
#include "Power.h"
#include "ChargerType.h"

class dptf_export BatteryStatusFacadeInterface
{
public:
	virtual ~BatteryStatusFacadeInterface(){};

	virtual Power getMaxBatteryPower(void) = 0;
	virtual DptfBuffer getBatteryStatus(void) = 0;
	virtual DptfBuffer getBatteryInformation(void) = 0;
	virtual ChargerType::Type getChargerType(void) = 0;
	virtual Power getPlatformBatterySteadyState(void) = 0;
	virtual UInt32 getBatteryHighFrequencyImpedance(void) = 0;
	virtual UInt32 getBatteryNoLoadVoltage(void) = 0;
	virtual UInt32 getBatteryMaxPeakCurrent(void) = 0;
	virtual Percentage getBatteryPercentage(void) = 0;
	virtual void setBatteryPercentage(Percentage batteryPercentage) = 0;
};
