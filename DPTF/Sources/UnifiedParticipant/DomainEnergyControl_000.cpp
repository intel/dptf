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

#include "DomainEnergyControl_000.h"

DomainEnergyControl_000::DomainEnergyControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainEnergyControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

void DomainEnergyControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainEnergyControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainEnergyControl_000::getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

EnergyCounterInfo DomainEnergyControl_000::getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

double DomainEnergyControl_000::getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainEnergyControl_000::getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Power DomainEnergyControl_000::getInstantaneousPower(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainEnergyControl_000::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainEnergyControl_000::setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold)
{
	throw not_implemented();
}

void DomainEnergyControl_000::setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainEnergyControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainEnergyControl_000::getName(void)
{
	return "Energy Control (Version 0)";
}
