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

#include "PolicyServicesDomainPowerStatus.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainPowerStatus::PolicyServicesDomainPowerStatus(DptfManagerInterface* dptfManager, UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

PowerStatus PolicyServicesDomainPowerStatus::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPowerStatus(domainIndex);
}

Power PolicyServicesDomainPowerStatus::getAveragePower(
	UIntN participantIndex,
	UIntN domainIndex,
	const PowerControlDynamicCaps& capabilities)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getAveragePower(domainIndex, capabilities);
}

Power PolicyServicesDomainPowerStatus::getPowerValue(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getPowerValue(domainIndex);
}

void PolicyServicesDomainPowerStatus::setCalculatedAveragePower(
	UIntN participantIndex,
	UIntN domainIndex,
	Power powerValue)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->setCalculatedAveragePower(domainIndex, powerValue);
}