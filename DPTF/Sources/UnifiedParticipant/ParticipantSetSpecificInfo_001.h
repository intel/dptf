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

#pragma once

#include "Dptf.h"
#include "ParticipantSetSpecificInfoBase.h"

class ParticipantSetSpecificInfo_001 : public ParticipantSetSpecificInfoBase
{
public:
	ParticipantSetSpecificInfo_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~ParticipantSetSpecificInfo_001();

	// ParticipantSetSpecificInfoInterface
	virtual void setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature)
		override;
	virtual void setParticipantSpecificInfo(
		UIntN participantIndex,
		ParticipantSpecificInfoKey::Type tripPoint,
		const Temperature& tripValue) override;

	// ComponentExtendedInterface
	virtual void clearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	ParticipantSetSpecificInfo_001(const ParticipantSetSpecificInfo_001& rhs);
	ParticipantSetSpecificInfo_001& operator=(const ParticipantSetSpecificInfo_001& rhs);
};
