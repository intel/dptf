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

#include "ParticipantTracker.h"
#include "DptfTime.h"
using namespace std;

ParticipantTracker::ParticipantTracker()
{
	m_time.reset(new DptfTime());
}

ParticipantTracker::~ParticipantTracker()
{
}

void ParticipantTracker::remember(UIntN participantIndex)
{
	ParticipantProxy participant(participantIndex, m_policyServices, m_time);
	m_trackedParticipants.insert(std::pair<UIntN, ParticipantProxy>(participantIndex, participant));
}

Bool ParticipantTracker::remembers(UIntN participantIndex)
{
	return (m_trackedParticipants.find(participantIndex) != m_trackedParticipants.end());
}

void ParticipantTracker::forget(UIntN participantIndex)
{
	if (remembers(participantIndex))
	{
		m_trackedParticipants.erase(participantIndex);
	}
}

ParticipantProxyInterface* ParticipantTracker::getParticipant(UIntN participantIndex)
{
	if (remembers(participantIndex) == false)
	{
		throw dptf_exception(
			std::string("The participant at the given index is not valid: ")
			+ std::to_string(participantIndex));
	}
	return &m_trackedParticipants[participantIndex];
}

vector<UIntN> ParticipantTracker::getAllTrackedIndexes() const
{
	vector<UIntN> allTrackedItems;
	for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
	{
		allTrackedItems.push_back(item->first);
	}
	return allTrackedItems;
}

void ParticipantTracker::setPolicyServices(const PolicyServicesInterfaceContainer& policyServices)
{
	m_policyServices = policyServices;
}

std::shared_ptr<XmlNode> ParticipantTracker::getXmlForTripPointStatistics()
{
	auto allStatus = XmlNode::createWrapperElement("trip_point_statistics");
	for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
	{
		allStatus->addChild(item->second.getXmlForTripPointStatistics());
	}
	return allStatus;
}

std::shared_ptr<DomainProxyInterface> ParticipantTracker::findDomain(DomainType::Type domainType)
{
	auto participantIndexes = getAllTrackedIndexes();
	for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
		 participantIndex++)
	{
		auto participant = getParticipant(*participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			auto domain = participant->getDomain(*domainIndex);
			if ((domain->getDomainProperties().getDomainType() == DomainType::MultiFunction))
			{
				return domain;
			}
		}
	}

	throw dptf_exception("Domain " + DomainType::ToString(domainType) + " not found.");
}

void ParticipantTracker::setTimeServiceObject(std::shared_ptr<TimeInterface> time)
{
	m_time = time;
}
