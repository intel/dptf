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

#include "PolicyServicesDomainPlatformPowerStatus.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainPlatformPowerStatus::PolicyServicesDomainPlatformPowerStatus(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

Power PolicyServicesDomainPlatformPowerStatus::getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getMaxBatteryPower(domainIndex);
}

Power PolicyServicesDomainPlatformPowerStatus::getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformRestOfPower(domainIndex);
}

Power PolicyServicesDomainPlatformPowerStatus::getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getAdapterPowerRating(domainIndex);
}

DptfBuffer PolicyServicesDomainPlatformPowerStatus::getBatteryStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getBatteryStatus(domainIndex);
}

DptfBuffer PolicyServicesDomainPlatformPowerStatus::getBatteryInformation(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getBatteryInformation(domainIndex);
}

PlatformPowerSource::Type PolicyServicesDomainPlatformPowerStatus::getPlatformPowerSource(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformPowerSource(domainIndex);
}

ChargerType::Type PolicyServicesDomainPlatformPowerStatus::getChargerType(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getChargerType(domainIndex);
}

Power PolicyServicesDomainPlatformPowerStatus::getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getPlatformBatterySteadyState(domainIndex);
}

UInt32 PolicyServicesDomainPlatformPowerStatus::getACNominalVoltage(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getACNominalVoltage(domainIndex);
}

UInt32 PolicyServicesDomainPlatformPowerStatus::getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getACOperationalCurrent(domainIndex);
}

Percentage PolicyServicesDomainPlatformPowerStatus::getAC1msPercentageOverload(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getAC1msPercentageOverload(domainIndex);
}

Percentage PolicyServicesDomainPlatformPowerStatus::getAC2msPercentageOverload(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getAC2msPercentageOverload(domainIndex);
}

Percentage PolicyServicesDomainPlatformPowerStatus::getAC10msPercentageOverload(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	auto participant = getParticipantManager()->getParticipantPtr(participantIndex);
	return participant->getAC10msPercentageOverload(domainIndex);
}
