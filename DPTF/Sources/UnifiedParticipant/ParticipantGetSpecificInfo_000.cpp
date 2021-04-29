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

#include "ParticipantGetSpecificInfo_000.h"

ParticipantGetSpecificInfo_000::ParticipantGetSpecificInfo_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ParticipantGetSpecificInfoBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

std::map<ParticipantSpecificInfoKey::Type, Temperature> ParticipantGetSpecificInfo_000::getParticipantSpecificInfo(
	UIntN participantIndex,
	const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
	throw not_implemented();
}

void ParticipantGetSpecificInfo_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> ParticipantGetSpecificInfo_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string ParticipantGetSpecificInfo_000::getName(void)
{
	return "Get Specific Info Control (Version 0)";
}
