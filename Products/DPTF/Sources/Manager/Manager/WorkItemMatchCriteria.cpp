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

#include "WorkItemMatchCriteria.h"

WorkItemMatchCriteria::WorkItemMatchCriteria(void) : m_matchCriteriaAdded(false),
    m_testFrameworkEventType(false), m_frameworkEventType(FrameworkEvent::Max),
    m_testUniqueId(false), m_uniqueId(Constants::Invalid),
    m_testParticipantIndex(false), m_participantIndex(Constants::Invalid),
    m_testDomainIndex(false), m_domainIndex(Constants::Invalid),
    m_testPolicyIndex(false), m_policyIndex(Constants::Invalid)
{
}

void WorkItemMatchCriteria::addFrameworkEventTypeToMatchList(FrameworkEvent::Type frameworkEventType)
{
    m_matchCriteriaAdded = true;
    m_testFrameworkEventType = true;
    m_frameworkEventType = frameworkEventType;
}

void WorkItemMatchCriteria::addUniqueIdToMatchList(UInt64 uniqueId)
{
    m_matchCriteriaAdded = true;
    m_testUniqueId = true;
    m_uniqueId = uniqueId;
}

void WorkItemMatchCriteria::addParticipantIndexToMatchList(UIntN participantIndex)
{
    if (participantIndex != Constants::Invalid)
    {
        m_matchCriteriaAdded = true;
        m_testParticipantIndex = true;
        m_participantIndex = participantIndex;
    }
}

void WorkItemMatchCriteria::addDomainIndexToMatchList(UIntN domainIndex)
{
    if (domainIndex != Constants::Invalid)
    {
        m_matchCriteriaAdded = true;
        m_testDomainIndex = true;
        m_domainIndex = domainIndex;
    }
}

void WorkItemMatchCriteria::addPolicyIndexToMatchList(UIntN policyIndex)
{
    if (policyIndex != Constants::Invalid)
    {
        m_matchCriteriaAdded = true;
        m_testPolicyIndex = true;
        m_policyIndex = policyIndex;
    }
}

Bool WorkItemMatchCriteria::testAgainstMatchList(FrameworkEvent::Type frameworkEventType, UInt64 uniqueId,
    UIntN participantIndex, UIntN domainIndex, UIntN policyIndex) const
{
    if (m_matchCriteriaAdded == false)
    {
        return false;
    }

    if ((m_testFrameworkEventType == true) && (m_frameworkEventType != frameworkEventType))
    {
        return false;
    }

    if ((m_testUniqueId == true) && (m_uniqueId != uniqueId))
    {
        return false;
    }

    if ((m_testParticipantIndex == true) &&
        ((participantIndex == Constants::Invalid) || (m_participantIndex != participantIndex)))
    {
        return false;
    }

    if ((m_testDomainIndex == true) &&
        ((domainIndex == Constants::Invalid) || (m_domainIndex != domainIndex)))
    {
        return false;
    }

    if ((m_testPolicyIndex == true) &&
        ((policyIndex == Constants::Invalid) || (m_policyIndex != policyIndex)))
    {
        return false;
    }

    return true;
}