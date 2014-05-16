/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "StatusFormat.h"
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

bool ParticipantTracker::remembers(UIntN participantIndex)
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

ParticipantProxy& ParticipantTracker::operator[](UIntN participantIndex)
{
    if (remembers(participantIndex) == false)
    {
        throw dptf_exception("The participant at the given index is not valid.");
    }
    return m_trackedParticipants[participantIndex];
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

void ParticipantTracker::setPolicyServices(PolicyServicesInterfaceContainer policyServices)
{
    m_policyServices = policyServices;
}

XmlNode* ParticipantTracker::getXmlForTripPointStatistics()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("trip_point_statistics");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        allStatus->addChild(item->second.getXmlForTripPointStatistics());
    }
    return allStatus;
}

void ParticipantTracker::setTimeServiceObject(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
}