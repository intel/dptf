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

XmlNode* ParticipantTracker::getXmlForCriticalTripPoints()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("critical_trip_point_status");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        if (item->second.getCriticalTripPointProperty().supportsProperty())
        {
            allStatus->addChild(item->second.getXmlForCriticalTripPoints());
        }
    }
    return allStatus;
}

XmlNode* ParticipantTracker::getXmlForActiveTripPoints()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("active_trip_point_status");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        if (item->second.getActiveTripPointProperty().supportsProperty())
        {
            allStatus->addChild(item->second.getXmlForActiveTripPoints());
        }
    }
    return allStatus;
}

XmlNode* ParticipantTracker::getXmlForPassiveTripPoints()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("passive_trip_point_status");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        if (item->second.getPassiveTripPointProperty().supportsProperty())
        {
            allStatus->addChild(item->second.getXmlForPassiveTripPoints());
        }
    }
    return allStatus;
}

XmlNode* ParticipantTracker::getXmlForActiveCoolingControls()
{
    XmlNode* fanStatus = XmlNode::createWrapperElement("fan_status");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        auto domainIndexes = item->second.getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            if (item->second[*domainIndex].getActiveCoolingControl().supportsActiveCoolingControls())
            {
                fanStatus->addChild(item->second[*domainIndex].getActiveCoolingControl().getXml());
            }
        }
    }
    return fanStatus;
}

XmlNode* ParticipantTracker::getXmlForPassiveControlKnobs()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("passive_control_status");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        allStatus->addChild(item->second.getXmlForPassiveControlKnobs());
    }
    return allStatus;
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

XmlNode* ParticipantTracker::getXmlForConfigTdpLevels()
{
    XmlNode* allStatus = XmlNode::createWrapperElement("config_tdp_levels");
    for (auto item = m_trackedParticipants.begin(); item != m_trackedParticipants.end(); item++)
    {
        allStatus->addChild(item->second.getXmlForConfigTdpLevel());
    }
    return allStatus;
}

void ParticipantTracker::setTimeServiceObject(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
}