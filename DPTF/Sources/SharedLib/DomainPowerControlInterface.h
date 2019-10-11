/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "PowerControlDynamicCapsSet.h"
#include "PowerControlType.h"
#include "TimeSpan.h"

class DomainPowerControlInterface
{
public:
	virtual ~DomainPowerControlInterface(){};

	virtual Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) = 0;

	virtual Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) = 0;
	virtual Power getPowerLimitWithoutCache(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) = 0;
	virtual Bool isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Bool isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) = 0;
	virtual void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) = 0;

	virtual TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) = 0;
	virtual void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) = 0;
	virtual void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) = 0;

	virtual Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) = 0;
	virtual void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) = 0;
	virtual void setSocPowerFloorState(UIntN participantIndex, UIntN domainIndex, Bool socPowerFloorState) = 0;

	virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) = 0;
	virtual void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) = 0;

	virtual Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual TimeSpan getAlpha(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual TimeSpan getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void removePowerLimitPolicyRequest(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) = 0;
};
