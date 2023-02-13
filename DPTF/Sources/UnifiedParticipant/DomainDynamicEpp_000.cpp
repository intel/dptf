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

#include "DomainDynamicEpp_000.h"

DomainDynamicEpp_000::DomainDynamicEpp_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainDynamicEppBase(participantIndex, domainIndex, participantServicesInterface)
{
}

UInt32 DomainDynamicEpp_000::getEppSensitivityHint()
{
	throw not_implemented();
}

void DomainDynamicEpp_000::updateEppSensitivityHint(UInt32 eppSensitivityHint)
{
	throw not_implemented();
}

void DomainDynamicEpp_000::setDynamicEppSupport(UInt32 dynamicEppSupport)
{
	throw not_implemented();
}

std::string DomainDynamicEpp_000::getName(void)
{
	return "Dynamic EPP (Version 0)";
}

std::shared_ptr<XmlNode> DomainDynamicEpp_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}