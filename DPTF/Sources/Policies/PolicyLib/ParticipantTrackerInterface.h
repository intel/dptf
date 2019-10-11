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

#pragma once

#include "Dptf.h"
#include "ParticipantProxy.h"
#include "PolicyServicesInterfaceContainer.h"
#include "TimeInterface.h"
#include "ParticipantProxyInterface.h"

class dptf_export ParticipantTrackerInterface
{
public:
	virtual ~ParticipantTrackerInterface(){};

	virtual void remember(UIntN participantIndex) = 0;
	virtual Bool remembers(UIntN participantIndex) = 0;
	virtual void forget(UIntN participantIndex) = 0;
	virtual ParticipantProxyInterface* getParticipant(UIntN participantIndex) = 0;
	virtual std::shared_ptr<DomainProxyInterface> findDomain(DomainType::Type domainType) = 0;
	virtual std::vector<UIntN> getAllTrackedIndexes() const = 0;
	virtual void setPolicyServices(const PolicyServicesInterfaceContainer& policyServices) = 0;
	virtual void setTimeServiceObject(std::shared_ptr<TimeInterface> time) = 0;
	virtual std::shared_ptr<XmlNode> getXmlForTripPointStatistics() = 0;
};
