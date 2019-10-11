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

#include "DomainSocWorkloadClassification_000.h"

DomainSocWorkloadClassification_000::DomainSocWorkloadClassification_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainSocWorkloadClassificationBase(participantIndex, domainIndex, participantServicesInterface)
{
}

UInt32 DomainSocWorkloadClassification_000::getSocWorkloadClassification()
{
	throw not_implemented();
}

std::string DomainSocWorkloadClassification_000::getName(void)
{
	return "Soc Workload Classification (Version 0)";
}

std::shared_ptr<XmlNode> DomainSocWorkloadClassification_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}