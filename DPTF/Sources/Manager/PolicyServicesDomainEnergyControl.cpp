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

#include "PolicyServicesDomainEnergyControl.h"
#include "ParticipantManagerInterface.h"

PolicyServicesDomainEnergyControl::PolicyServicesDomainEnergyControl(DptfManagerInterface* dptfManager, UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

UInt32 PolicyServicesDomainEnergyControl::getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRaplEnergyCounter(domainIndex);
}

EnergyCounterInfo PolicyServicesDomainEnergyControl::getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRaplEnergyCounterInfo(domainIndex);
}

double PolicyServicesDomainEnergyControl::getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRaplEnergyUnit(domainIndex);
}

UInt32 PolicyServicesDomainEnergyControl::getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getRaplEnergyCounterWidth(domainIndex);
}

Power PolicyServicesDomainEnergyControl::getInstantaneousPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getInstantaneousPower(domainIndex);
}

UInt32 PolicyServicesDomainEnergyControl::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getParticipantManager()->getParticipantPtr(participantIndex)->getEnergyThreshold(domainIndex);
}

void PolicyServicesDomainEnergyControl::setEnergyThreshold(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 energyThreshold)
{
	throwIfNotWorkItemThread();
	getParticipantManager()->getParticipantPtr(participantIndex)->setEnergyThreshold(domainIndex, energyThreshold);
}

void PolicyServicesDomainEnergyControl::setEnergyThresholdInterruptDisable(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	getParticipantManager()
		->getParticipantPtr(participantIndex)
		->setEnergyThresholdInterruptDisable(domainIndex);
}