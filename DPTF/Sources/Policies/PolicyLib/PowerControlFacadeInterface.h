/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "PowerStatusProperty.h"
#include "PowerControlCapabilitiesCachedProperty.h"

class dptf_export PowerControlFacadeInterface
{
public:
	virtual ~PowerControlFacadeInterface(){};

	// commands
	virtual void initializeControlsIfNeeded() = 0;
	virtual void setControlsToMax() = 0;
	virtual void setPL1PowerLimitControlToMax() = 0;
	virtual void setPL2PowerLimitControlToMax() = 0;
	virtual void refreshCapabilities() = 0;
	virtual void setCapability(const PowerControlDynamicCaps& capabilities) = 0;
	virtual void setPowerLimitMinPL1(const Power& powerLimit) = 0;
	virtual void setPowerLimitPL1(const Power& powerLimit) = 0;
	virtual void setPowerLimitPL2(const Power& powerLimit) = 0;
	virtual void setPowerLimitPL3(const Power& powerLimit) = 0;
	virtual void setPowerLimitPL4(const Power& powerLimit) = 0;
	virtual void setPowerLimitTimeWindowPL1(const TimeSpan& timeWindow) = 0;
	virtual void setPowerLimitTimeWindowPL3(const TimeSpan& timeWindow) = 0;
	virtual void setPowerLimitDutyCyclePL3(const Percentage& dutyCycle) = 0;
	virtual void setSocPowerFloorState(Bool socPowerFloorState) = 0;
	virtual void setValuesWithinCapabilities() = 0;
	virtual void lockCapabilities() = 0;
	virtual void unlockCapabilities() = 0;
	virtual void removePowerLimitPolicyRequest(PowerControlType::Type controlType) = 0;
	virtual void setPowerSharePolicyPower(const Power& powerSharePolicyPower) = 0;
	virtual void setPowerShareEffectiveBias(UInt32 powerShareEffectiveBias) = 0;
	virtual void clearPowerLimitMin() = 0;
	virtual void clearPowerLimit() = 0;

	// queries
	virtual Bool supportsPowerControls() const = 0;
	virtual Bool supportsPowerStatus() const = 0;
	virtual Bool isPowerShareControl() const = 0;
	virtual Bool isPl1PowerLimitEnabled(void) = 0;
	virtual Bool isPl2PowerLimitEnabled(void) = 0;
	virtual Bool isPl3PowerLimitEnabled(void) = 0;
	virtual Bool isPl4PowerLimitEnabled(void) = 0;
	virtual Power getCurrentPower() = 0;
	virtual Power getAveragePower() = 0;
	virtual const PowerControlDynamicCapsSet& getCapabilities() = 0;
	virtual Power getPowerLimitPL1() = 0;
	virtual Power getPowerLimitPL2() = 0;
	virtual Power getPowerLimitPL3() = 0;
	virtual Power getPowerLimitPL4() = 0;
	virtual TimeSpan getPowerLimitTimeWindowPL1() = 0;
	virtual TimeSpan getPowerLimitTimeWindowPL3() = 0;
	virtual Percentage getPowerLimitDutyCyclePL3() = 0;
	virtual Power getLivePowerLimitPL1() = 0;
	virtual Power getLivePowerLimitPL2() = 0;
	virtual Power getLivePowerLimitPL3() = 0;
	virtual Power getLivePowerLimitPL4() = 0;
	virtual TimeSpan getLivePowerLimitTimeWindowPL1() = 0;
	virtual TimeSpan getLivePowerLimitTimeWindowPL3() = 0;
	virtual Percentage getLivePowerLimitDutyCyclePL3() = 0;
	virtual Bool isSocPowerFloorEnabled() = 0;
	virtual Bool isSocPowerFloorSupported() = 0;
	virtual UInt32 getSocPowerFloorState() = 0;
	virtual void updatePowerLimitTimeWindowPL1(const TimeSpan& timeWindow) = 0;
	virtual Power getThermalDesignPower() = 0;
};
