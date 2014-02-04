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

#pragma once

#include "BasicTypes.h"
#include "FrameworkEvent.h"
#include "Constants.h"

class WorkItemMatchCriteria
{
public:

    WorkItemMatchCriteria(void);

    void addFrameworkEventTypeToMatchList(FrameworkEvent::Type frameworkEventType);
    void addUniqueIdToMatchList(UInt64 uniqueId);
    void addParticipantIndexToMatchList(UIntN participantIndex);
    void addDomainIndexToMatchList(UIntN domainIndex);
    void addPolicyIndexToMatchList(UIntN policyIndex);

    Bool testAgainstMatchList(FrameworkEvent::Type frameworkEventType, UInt64 uniqueId,
        UIntN participantIndex = Constants::Invalid, UIntN domainIndex = Constants::Invalid,
        UIntN policyIndex = Constants::Invalid) const;

private:

    Bool m_matchCriteriaAdded;

    Bool m_testFrameworkEventType;
    FrameworkEvent::Type m_frameworkEventType;

    Bool m_testUniqueId;
    UInt64 m_uniqueId;

    Bool m_testParticipantIndex;
    UIntN m_participantIndex;

    Bool m_testDomainIndex;
    UIntN m_domainIndex;

    Bool m_testPolicyIndex;
    UIntN m_policyIndex;
};