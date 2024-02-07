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

#pragma once

#include "Dptf.h"
#include "ParticipantGetSpecificInfoBase.h"
class PrimitiveAndInstance;

class ParticipantGetSpecificInfo_001 : public ParticipantGetSpecificInfoBase
{
public:
	ParticipantGetSpecificInfo_001(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~ParticipantGetSpecificInfo_001() override = default;

	// ParticipantGetSpecificInfoInterface
	std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		UIntN participantIndex,
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) override;

	// ComponentExtendedInterface
	void onClearCachedData(void) override;
	std::string getName(void) override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	ParticipantGetSpecificInfo_001(const ParticipantGetSpecificInfo_001& rhs);
	ParticipantGetSpecificInfo_001& operator=(const ParticipantGetSpecificInfo_001& rhs);

	std::map<ParticipantSpecificInfoKey::Type, Temperature> m_cachedData;

	Temperature readSpecificInfo(PrimitiveAndInstance primitiveAndInstance) const;
	PrimitiveAndInstance getPrimitiveAndInstanceForSpecificInfoKey(ParticipantSpecificInfoKey::Type request) const;
};
