/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "PolicyServices.h"
#include "DomainPowerControlInterface.h"

class PolicyServicesDomainPowerControl : public PolicyServices, public DomainPowerControlInterface
{
public:
	PolicyServicesDomainPowerControl(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
		override final;
	virtual void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) override final;
	virtual Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
		override;
	virtual Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	virtual Power getPowerLimitWithoutCache(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	virtual void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) override;
	virtual void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override final;
	virtual Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) override final;
	virtual double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) override final;
	virtual double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) override final;
	virtual TimeSpan getAlpha(UIntN participantIndex, UIntN domainIndex) override final;
	virtual TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) override final;
	virtual TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) override final;
	virtual TimeSpan getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) override final;
};
