/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "PolicyServicesDomainRfProfileControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainRfProfileControl::PolicyServicesDomainRfProfileControl(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

RfProfileCapabilities PolicyServicesDomainRfProfileControl::getRfProfileCapabilities(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRfProfileCapabilities(domainIndex);
}

void PolicyServicesDomainRfProfileControl::setRfProfileCenterFrequency(
	UIntN participantIndex,
	UIntN domainIndex,
	const Frequency& centerFrequency)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setRfProfileCenterFrequency(domainIndex, getPolicyIndex(), centerFrequency);
}

Percentage PolicyServicesDomainRfProfileControl::getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getSscBaselineSpreadValue(domainIndex);
}

Percentage PolicyServicesDomainRfProfileControl::getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getSscBaselineThreshold(domainIndex);
}

Percentage PolicyServicesDomainRfProfileControl::getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getSscBaselineGuardBand(domainIndex);
}