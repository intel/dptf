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

#include "PolicyServicesDomainPlatformPowerControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainPlatformPowerControl::PolicyServicesDomainPlatformPowerControl(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

Bool PolicyServicesDomainPlatformPowerControl::isPlatformPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->isPlatformPowerLimitEnabled(domainIndex, limitType);
}

Power PolicyServicesDomainPlatformPowerControl::getPlatformPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformPowerLimit(domainIndex, limitType);
}

void PolicyServicesDomainPlatformPowerControl::setPlatformPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setPlatformPowerLimit(domainIndex, getPolicyIndex(), limitType, powerLimit);
}

TimeSpan PolicyServicesDomainPlatformPowerControl::getPlatformPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformPowerLimitTimeWindow(domainIndex, limitType);
}

void PolicyServicesDomainPlatformPowerControl::setPlatformPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setPlatformPowerLimitTimeWindow(domainIndex, getPolicyIndex(), limitType, timeWindow);
}

Percentage PolicyServicesDomainPlatformPowerControl::getPlatformPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformPowerLimitDutyCycle(domainIndex, limitType);
}

void PolicyServicesDomainPlatformPowerControl::setPlatformPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	participant->setPlatformPowerLimitDutyCycle(domainIndex, getPolicyIndex(), limitType, dutyCycle);
}
