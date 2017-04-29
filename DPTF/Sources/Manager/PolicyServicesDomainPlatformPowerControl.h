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
#include "DomainPlatformPowerControlInterface.h"

class PolicyServicesDomainPlatformPowerControl final : public PolicyServices, public DomainPlatformPowerControlInterface
{
public:
	PolicyServicesDomainPlatformPowerControl(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual Bool isPlatformPowerLimitEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual Power getPlatformPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const Power& powerLimit) override;
	virtual TimeSpan getPlatformPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getPlatformPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const Percentage& dutyCycle) override;
};
