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

#include "DomainSystemPowerControl_000.h"

DomainSystemPowerControl_000::DomainSystemPowerControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainSystemPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

void DomainSystemPowerControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainSystemPowerControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainSystemPowerControl_000::isSystemPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throw not_implemented();
}

Power DomainSystemPowerControl_000::getSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throw not_implemented();
}

void DomainSystemPowerControl_000::setSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Power& limit)
{
	throw not_implemented();
}

TimeSpan DomainSystemPowerControl_000::getSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throw not_implemented();
}

void DomainSystemPowerControl_000::setSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throw not_implemented();
}

Percentage DomainSystemPowerControl_000::getSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throw not_implemented();
}

void DomainSystemPowerControl_000::setSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throw not_implemented();
}

std::string DomainSystemPowerControl_000::getName(void)
{
	return "System Power Control (Version 0)";
}

void DomainSystemPowerControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}
