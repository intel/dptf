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
#include "ParticipantProxy.h"
#include "PolicyServicesInterfaceContainer.h"
#include "TimeInterface.h"
#include "ParticipantTrackerInterface.h"

// keeps track of participants as they come and go on the system.
class dptf_export ParticipantTracker : public ParticipantTrackerInterface
{
public:
	ParticipantTracker();
	~ParticipantTracker() override = default;

	void remember(UIntN participantIndex) override;
	Bool remembers(UIntN participantIndex) override;
	void forget(UIntN participantIndex) override;
	ParticipantProxyInterface* getParticipant(UIntN participantIndex) override;
	std::vector<UIntN> getAllTrackedIndexes() const override;
	void setPolicyServices(const PolicyServicesInterfaceContainer& policyServices) override;
	void setTimeServiceObject(std::shared_ptr<TimeInterface> time) override;
	std::shared_ptr<XmlNode> getXmlForTripPointStatistics() override;
	std::shared_ptr<DomainProxyInterface> findDomain(DomainType::Type domainType) override;
	std::vector<std::shared_ptr<DomainProxyInterface>> getAllDomains() override;

protected:
	std::map<UIntN, ParticipantProxy> m_trackedParticipants;
	PolicyServicesInterfaceContainer m_policyServices;
	std::shared_ptr<TimeInterface> m_time;
};
