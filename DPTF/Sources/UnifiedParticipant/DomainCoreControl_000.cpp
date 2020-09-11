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

#include "DomainCoreControl_000.h"

DomainCoreControl_000::DomainCoreControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainCoreControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

CoreControlStaticCaps DomainCoreControl_000::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

CoreControlDynamicCaps DomainCoreControl_000::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

CoreControlLpoPreference DomainCoreControl_000::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

CoreControlStatus DomainCoreControl_000::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainCoreControl_000::setActiveCoreControl(
	UIntN participantIndex,
	UIntN domainIndex,
	const CoreControlStatus& coreControlStatus)
{
	throw not_implemented();
}

void DomainCoreControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainCoreControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainCoreControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainCoreControl_000::getName(void)
{
	return "Core Control (Version 0)";
}
