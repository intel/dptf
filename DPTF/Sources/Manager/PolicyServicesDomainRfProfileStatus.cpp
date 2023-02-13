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

#include "PolicyServicesDomainRfProfileStatus.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainRfProfileStatus::PolicyServicesDomainRfProfileStatus(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

RfProfileDataSet PolicyServicesDomainRfProfileStatus::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRfProfileDataSet(domainIndex);
}

UInt32 PolicyServicesDomainRfProfileStatus::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getWifiCapabilities(domainIndex);
}

UInt32 PolicyServicesDomainRfProfileStatus::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRfiDisable(domainIndex);
}

UInt64 PolicyServicesDomainRfProfileStatus::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getDvfsPoints(domainIndex);
}

void PolicyServicesDomainRfProfileStatus::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->setDdrRfiTable(domainIndex, ddrRfiStruct);
}

void PolicyServicesDomainRfProfileStatus::setProtectRequest(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt64 frequencyRate)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->setProtectRequest(domainIndex, frequencyRate);
}

void PolicyServicesDomainRfProfileStatus::setRfProfileOverride(
	UIntN participantIndex,
	UIntN domainIndex,
	const DptfBuffer& rfProfileBufferData)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setRfProfileOverride(domainIndex, domainIndex, rfProfileBufferData);
}
