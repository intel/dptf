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

#include "DomainUtilization_000.h"

DomainUtilization_000::DomainUtilization_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainUtilizationBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

UtilizationStatus DomainUtilization_000::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Percentage DomainUtilization_000::getMaxCoreUtilization(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainUtilization_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainUtilization_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainUtilization_000::getName(void)
{
	return "Utilization Status (Version 0)";
}
