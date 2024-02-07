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
#include "EnvironmentProfileUpdater.h"
#include "EnvironmentProfileGenerator.h"
#include "EsifServicesInterface.h"

class EventPayloadParticipantDomainId;
class WorkItemInterface;

class dptf_export RealEnvironmentProfileUpdater : public EnvironmentProfileUpdater
{
public:
	RealEnvironmentProfileUpdater(DptfManagerInterface* manager);
	~RealEnvironmentProfileUpdater() override = default;
	void update(FrameworkEvent::Type event, const DptfBuffer& eventPayload) override;
	EnvironmentProfile getLastUpdatedProfile() const override;

private:
	DptfManagerInterface* m_manager;
	EnvironmentProfile m_environmentProfile;
	DomainType::Type getDomainType(const EventPayloadParticipantDomainId& id) const;
};
