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

#include "PolicyServicesDomainDisplayControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainDisplayControl::PolicyServicesDomainDisplayControl(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

DisplayControlDynamicCaps PolicyServicesDomainDisplayControl::getDisplayControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getDisplayControlDynamicCaps(domainIndex);
}

DisplayControlStatus PolicyServicesDomainDisplayControl::getDisplayControlStatus(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getDisplayControlStatus(domainIndex);
}

UIntN PolicyServicesDomainDisplayControl::getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getUserPreferredDisplayIndex(domainIndex);
}

UIntN PolicyServicesDomainDisplayControl::getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getUserPreferredSoftBrightnessIndex(domainIndex);
}

Bool PolicyServicesDomainDisplayControl::isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->isUserPreferredIndexModified(domainIndex);
}

UIntN PolicyServicesDomainDisplayControl::getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getSoftBrightnessIndex(domainIndex);
}

DisplayControlSet PolicyServicesDomainDisplayControl::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getDisplayControlSet(domainIndex);
}

void PolicyServicesDomainDisplayControl::setDisplayControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN displayControlIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setDisplayControl(domainIndex, getPolicyIndex(), displayControlIndex);
}

void PolicyServicesDomainDisplayControl::setSoftBrightness(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN displayControlIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setSoftBrightness(domainIndex, getPolicyIndex(), displayControlIndex);
}

void PolicyServicesDomainDisplayControl::updateUserPreferredSoftBrightnessIndex(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->updateUserPreferredSoftBrightnessIndex(domainIndex);
}

void PolicyServicesDomainDisplayControl::restoreUserPreferredSoftBrightness(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->restoreUserPreferredSoftBrightness(domainIndex);
}

void PolicyServicesDomainDisplayControl::setDisplayControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	DisplayControlDynamicCaps newCapabilities)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setDisplayControlDynamicCaps(domainIndex, getPolicyIndex(), newCapabilities);
}

void PolicyServicesDomainDisplayControl::setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setDisplayCapsLock(domainIndex, getPolicyIndex(), lock);
}
