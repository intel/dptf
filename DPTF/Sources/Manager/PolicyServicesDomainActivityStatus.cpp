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

#include "PolicyServicesDomainActivityStatus.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainActivityStatus::PolicyServicesDomainActivityStatus(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

UInt32 PolicyServicesDomainActivityStatus::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getEnergyThreshold(domainIndex);
}

void PolicyServicesDomainActivityStatus::setEnergyThreshold(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 energyThreshold)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->setEnergyThreshold(domainIndex, energyThreshold);
}

Temperature PolicyServicesDomainActivityStatus::getPowerShareTemperatureThreshold(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPowerShareTemperatureThreshold(domainIndex);
}

Percentage PolicyServicesDomainActivityStatus::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getUtilizationThreshold(domainIndex);
}

Percentage PolicyServicesDomainActivityStatus::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getResidencyUtilization(domainIndex);
}

void PolicyServicesDomainActivityStatus::setEnergyThresholdInterruptFlag(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 energyThresholdInterruptFlag)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setEnergyThresholdInterruptFlag(domainIndex, energyThresholdInterruptFlag);
}