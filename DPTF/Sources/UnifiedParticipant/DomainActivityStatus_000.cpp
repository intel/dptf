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

#include "DomainActivityStatus_000.h"

DomainActivityStatus_000::DomainActivityStatus_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActivityStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

Percentage DomainActivityStatus_000::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Percentage DomainActivityStatus_000::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainActivityStatus_000::getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainActivityStatus_000::getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainActivityStatus_000::getTimestampCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainActivityStatus_000::getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

CoreActivityInfo DomainActivityStatus_000::getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainActivityStatus_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// do nothing
}

void DomainActivityStatus_000::onClearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainActivityStatus_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainActivityStatus_000::getName(void)
{
	return "Activity Status (Version 0)";
}

UInt32 DomainActivityStatus_000::getSocDgpuPerformanceHintPoints(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

