/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "DomainPlatformPowerControl_000.h"

DomainPlatformPowerControl_000::DomainPlatformPowerControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPlatformPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

void DomainPlatformPowerControl_000::setPortPowerLimit(const UInt32 portNumber, const Power& powerLimit)
{
	throw not_implemented();
}

void DomainPlatformPowerControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPlatformPowerControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainPlatformPowerControl_000::getName(void)
{
	return "Platform Power Control (Version 0)";
}