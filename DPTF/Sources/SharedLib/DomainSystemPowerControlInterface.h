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
#include "PsysPowerLimitType.h"
#include "TimeSpan.h"

class DomainSystemPowerControlInterface
{
public:
	virtual ~DomainSystemPowerControlInterface(){};

	virtual Bool isSystemPowerLimitEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) = 0;

	virtual Power getSystemPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) = 0;
	virtual void setSystemPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Power& powerLimit) = 0;

	virtual TimeSpan getSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) = 0;
	virtual void setSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const TimeSpan& timeWindow) = 0;

	virtual Percentage getSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) = 0;
	virtual void setSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Percentage& dutyCycle) = 0;
};
