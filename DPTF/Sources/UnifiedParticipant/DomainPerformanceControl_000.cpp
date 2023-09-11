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

#include "DomainPerformanceControl_000.h"

DomainPerformanceControl_000::DomainPerformanceControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPerformanceControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

PerformanceControlStaticCaps DomainPerformanceControl_000::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

PerformanceControlDynamicCaps DomainPerformanceControl_000::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

PerformanceControlStatus DomainPerformanceControl_000::getPerformanceControlStatus(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

PerformanceControlSet DomainPerformanceControl_000::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::setPerformanceControl(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN performanceControlIndex)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PerformanceControlDynamicCaps newCapabilities)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throw not_implemented();
}

UIntN DomainPerformanceControl_000::getCurrentPerformanceControlIndex(UIntN ParticipantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainPerformanceControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::capture(void)
{
	throw not_implemented();
}

void DomainPerformanceControl_000::restore(void)
{
	throw not_implemented();
}

std::string DomainPerformanceControl_000::getName(void)
{
	return "Performance Control (Version 0)";
}
