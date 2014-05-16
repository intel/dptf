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

#include "DomainWorkItem.h"
#include "Participant.h"

DomainWorkItem::DomainWorkItem(DptfManager* dptfManager, FrameworkEvent::Type frameworkEventType,
    UIntN participantIndex, UIntN domainIndex) :
    ParticipantWorkItem(dptfManager, frameworkEventType, participantIndex),
    m_domainIndex(domainIndex)
{
}

DomainWorkItem::~DomainWorkItem(void)
{
}

Bool DomainWorkItem::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return matchCriteria.testAgainstMatchList(getFrameworkEventType(), getUniqueId(),
        getParticipantIndex(), getDomainIndex());
}

std::string DomainWorkItem::toXml(void) const
{
    throw implement_me();
}

UIntN DomainWorkItem::getDomainIndex(void) const
{
    return m_domainIndex;
}