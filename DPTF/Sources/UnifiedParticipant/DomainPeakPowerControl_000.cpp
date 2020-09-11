/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "DomainPeakPowerControl_000.h"

DomainPeakPowerControl_000::DomainPeakPowerControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPeakPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

Power DomainPeakPowerControl_000::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPeakPowerControl_000::setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower)
{
	throw not_implemented();
}

Power DomainPeakPowerControl_000::getDCPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPeakPowerControl_000::setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower)
{
	throw not_implemented();
}

void DomainPeakPowerControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPeakPowerControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::string DomainPeakPowerControl_000::getName(void)
{
	return "No Peak Power Control (Version 0)";
}

std::shared_ptr<XmlNode> DomainPeakPowerControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}
