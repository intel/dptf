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

#include "RealEnvironmentProfileUpdater.h"
#include "EventPayloadParticipantDomainId.h"
#include "ParticipantManagerInterface.h"
#include "WIEnvironmentProfileChanged.h"
#include "WorkItemQueueManagerInterface.h"
using namespace std;

RealEnvironmentProfileUpdater::RealEnvironmentProfileUpdater(DptfManagerInterface* manager)
	: m_manager(manager)
{
	m_environmentProfile = m_manager->getEnvironmentProfileGenerator()->generateWithCpuIdOnly();
}

void RealEnvironmentProfileUpdater::update(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	if (event == FrameworkEvent::DomainCreate)
	{
		const auto id = EventPayloadParticipantDomainId(eventPayload);
		const auto domainType = getDomainType(id);
		if (domainType == DomainType::MultiFunction)
		{
			m_environmentProfile = m_manager->getEnvironmentProfileGenerator()->generateWithFullProfile(id.participantId, id.domainId);
			const auto workItem = make_shared<WIEnvironmentProfileChanged>(m_manager, m_environmentProfile);
			m_manager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(workItem);
		}
	}
}

EnvironmentProfile RealEnvironmentProfileUpdater::getLastUpdatedProfile() const
{
	return m_environmentProfile;
}

DomainType::Type RealEnvironmentProfileUpdater::getDomainType(const EventPayloadParticipantDomainId& id) const
{
	try
	{
		const auto participant = m_manager->getParticipantManager()->getParticipantPtr(id.participantId);
		const auto domainSet = participant->getDomainPropertiesSet();
		const auto domainProperties = domainSet.getDomainProperties(id.domainId);
		return domainProperties.getDomainType();
	}
	catch (...)
	{
		return DomainType::Invalid;
	}
}
