/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

UInt64 PolicyServicesDomainActivityStatus::getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreActivityCounter(domainIndex);
}

UInt32 PolicyServicesDomainActivityStatus::getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreActivityCounterWidth(domainIndex);
}

UInt64 PolicyServicesDomainActivityStatus::getTimestampCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getTimestampCounter(domainIndex);
}

UInt32 PolicyServicesDomainActivityStatus::getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getTimestampCounterWidth(domainIndex);
}

CoreActivityInfo PolicyServicesDomainActivityStatus::getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreActivityInfo(domainIndex);
}

void PolicyServicesDomainActivityStatus::setPowerShareEffectiveBias(UIntN participantIndex, UIntN domainIndex, UInt32 powerShareEffectiveBias)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setPowerShareEffectiveBias(domainIndex, powerShareEffectiveBias);
}