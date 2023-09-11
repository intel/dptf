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

#pragma once

#include "ParticipantManagerInterface.h"

class dptf_export ParticipantManager : public ParticipantManagerInterface
{
public:
	ParticipantManager(DptfManagerInterface* dptfManager);
	~ParticipantManager() override;

	// remove the copy constructor and assignment operator.
	ParticipantManager(const ParticipantManager& rhs) = delete;
	ParticipantManager& operator=(const ParticipantManager& rhs) = delete;

	UIntN allocateNextParticipantIndex() override;
	void createParticipant(
		UIntN participantIndex,
		const AppParticipantDataPtr participantDataPtr,
		Bool participantEnabled) override;

	void destroyAllParticipants() override;
	void destroyParticipant(UIntN participantIndex) override;

	std::set<UIntN> getParticipantIndexes() const override;
	Participant* getParticipantPtr(UIntN participantIndex) const override;

	// This will clear the cached data stored within all participants *within* the framework.  It will not ask the
	// actual participants to clear their caches.
	void clearAllParticipantCachedData() override;
	Bool participantExists(const std::string& participantName) const override;
	std::shared_ptr<IParticipant> getParticipant(const std::string& participantName) const override;
	std::string GetStatusAsXml() override;

private:
	DptfManagerInterface* m_dptfManager;
	EsifServicesInterface* getEsifServices() const;
	std::map<UIntN, std::shared_ptr<Participant>> m_participants;
};
