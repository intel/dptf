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

#include "ParticipantTracker.h"
#include "DptfTime.h"
using namespace std;

ParticipantTracker::ParticipantTracker()
	: m_trackedParticipants(map<UIntN, ParticipantProxy>())
{
	m_time.reset(new DptfTime());
}

void ParticipantTracker::remember(UIntN participantIndex)
{
	ParticipantProxy participant(participantIndex, m_policyServices, m_time);
	m_trackedParticipants.insert(pair<UIntN, ParticipantProxy>(participantIndex, participant));
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
			string("The participant at the given index is not valid: ") + to_string(participantIndex));
	}
	return &m_trackedParticipants.at(participantIndex);
}

vector<UIntN> ParticipantTracker::getAllTrackedIndexes() const
{
	vector<UIntN> allTrackedItems;
	allTrackedItems.reserve(m_trackedParticipants.size());
	for (const auto& m_trackedParticipant : m_trackedParticipants)
	{
		allTrackedItems.emplace_back(m_trackedParticipant.first);
	}
	return allTrackedItems;
}

void ParticipantTracker::setPolicyServices(const PolicyServicesInterfaceContainer& policyServices)
{
	m_policyServices = policyServices;
}

shared_ptr<XmlNode> ParticipantTracker::getXmlForTripPointStatistics()
{
	auto allStatus = XmlNode::createWrapperElement("trip_point_statistics");
	for (auto& m_trackedParticipant : m_trackedParticipants)
	{
		allStatus->addChild(m_trackedParticipant.second.getXmlForTripPointStatistics());
	}
	return allStatus;
}

shared_ptr<DomainProxyInterface> ParticipantTracker::findDomain(DomainType::Type domainType)
{
	const auto participantIndexes = getAllTrackedIndexes();
	for (const unsigned int& participantIndex : participantIndexes)
	{
		const auto participant = getParticipant(participantIndex);
		const auto domainIndexes = participant->getDomainIndexes();
		for (const unsigned int& domainIndex : domainIndexes)
		{
			auto domain = participant->getDomain(domainIndex);
			if ((domain->getDomainProperties().getDomainType() == DomainType::MultiFunction))
			{
				return domain;
			}
		}
	}

	throw dptf_exception("Domain " + DomainType::toString(domainType) + " not found.");
}

vector<shared_ptr<DomainProxyInterface>> ParticipantTracker::getAllDomains()
{
	vector<shared_ptr<DomainProxyInterface>> result;

	const auto participantIndexes = getAllTrackedIndexes();
	for (const unsigned int& participantIndex : participantIndexes)
	{
		const auto participant = getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (const unsigned int& domainIndex : domainIndexes)
		{
			const auto domain = participant->getDomain(domainIndex);
			result.emplace_back(domain);
		}
	}

	return result;
}

void ParticipantTracker::setTimeServiceObject(shared_ptr<TimeInterface> time)
{
	m_time = time;
}
