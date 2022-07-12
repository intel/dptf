/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "Participant.h"
#include "esif_sdk_iface_app.h"

class dptf_export ParticipantManagerInterface
{
public:
	virtual ~ParticipantManagerInterface(){};

	virtual UIntN allocateNextParticipantIndex(void) = 0;
	virtual void createParticipant(
		UIntN participantIndex,
		const AppParticipantDataPtr participantDataPtr,
		Bool participantEnabled) = 0;

	virtual void destroyAllParticipants(void) = 0;
	virtual void destroyParticipant(UIntN participantIndex) = 0;

	virtual std::set<UIntN> getParticipantIndexes(void) const = 0;
	virtual Participant* getParticipantPtr(UIntN participantIndex) const = 0;
	virtual void clearAllParticipantCachedData() = 0;
	virtual Bool participantExists(const std::string& participantName) const = 0;
	virtual std::shared_ptr<IParticipant> getParticipant(const std::string& participantName) const = 0;

	virtual std::string GetStatusAsXml(void) = 0;
};
