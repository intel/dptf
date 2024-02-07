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

#include "PolicyServicesDomainSystemPowerControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainSystemPowerControl::PolicyServicesDomainSystemPowerControl(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

Bool PolicyServicesDomainSystemPowerControl::isSystemPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->isSystemPowerLimitEnabled(domainIndex, limitType);
}

Power PolicyServicesDomainSystemPowerControl::getSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getSystemPowerLimit(domainIndex, limitType);
}

void PolicyServicesDomainSystemPowerControl::setSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setSystemPowerLimit(domainIndex, getPolicyIndex(), limitType, powerLimit);
}

TimeSpan PolicyServicesDomainSystemPowerControl::getSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getSystemPowerLimitTimeWindow(domainIndex, limitType);
}

void PolicyServicesDomainSystemPowerControl::setSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setSystemPowerLimitTimeWindow(domainIndex, getPolicyIndex(), limitType, timeWindow);
}

Percentage PolicyServicesDomainSystemPowerControl::getSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getSystemPowerLimitDutyCycle(domainIndex, limitType);
}

void PolicyServicesDomainSystemPowerControl::setSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setSystemPowerLimitDutyCycle(domainIndex, getPolicyIndex(), limitType, dutyCycle);
}
