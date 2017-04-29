/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "DomainTccOffsetControl_000.h"

DomainTccOffsetControl_000::DomainTccOffsetControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainTccOffsetControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

Temperature DomainTccOffsetControl_000::getTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainTccOffsetControl_000::setTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& tccOffset)
{
	throw not_implemented();
}

Temperature DomainTccOffsetControl_000::getMaxTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Temperature DomainTccOffsetControl_000::getMinTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainTccOffsetControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainTccOffsetControl_000::clearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::string DomainTccOffsetControl_000::getName(void)
{
	return "No TCC Offset Control (Version 0)";
}

std::shared_ptr<XmlNode> DomainTccOffsetControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}
