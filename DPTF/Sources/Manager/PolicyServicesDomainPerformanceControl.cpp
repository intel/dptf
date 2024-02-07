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

#include "PolicyServicesDomainPerformanceControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainPerformanceControl::PolicyServicesDomainPerformanceControl(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

PerformanceControlStaticCaps PolicyServicesDomainPerformanceControl::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPerformanceControlStaticCaps(domainIndex);
}

PerformanceControlDynamicCaps PolicyServicesDomainPerformanceControl::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPerformanceControlDynamicCaps(domainIndex);
}

PerformanceControlStatus PolicyServicesDomainPerformanceControl::getPerformanceControlStatus(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPerformanceControlStatus(domainIndex);
}

PerformanceControlSet PolicyServicesDomainPerformanceControl::getPerformanceControlSet(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPerformanceControlSet(domainIndex);
}

void PolicyServicesDomainPerformanceControl::setPerformanceControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setPerformanceControl(domainIndex, getPolicyIndex(), performanceControlIndex);
}

void PolicyServicesDomainPerformanceControl::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PerformanceControlDynamicCaps newCapabilities)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setPerformanceControlDynamicCaps(domainIndex, getPolicyIndex(), newCapabilities);
}

void PolicyServicesDomainPerformanceControl::setPerformanceCapsLock(
	UIntN participantIndex,
	UIntN domainIndex,
	Bool lock)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setPerformanceCapsLock(domainIndex, getPolicyIndex(), lock);
}
