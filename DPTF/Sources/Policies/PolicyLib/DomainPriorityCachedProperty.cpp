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

#include "DomainPriorityCachedProperty.h"
using namespace std;

DomainPriorityCachedProperty::DomainPriorityCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_domainPriority(0)
{
}

DomainPriorityCachedProperty::~DomainPriorityCachedProperty(void)
{
}

void DomainPriorityCachedProperty::refreshData(void)
{
    m_domainPriority = getPolicyServices().domainPriority->getDomainPriority(getParticipantIndex(), getDomainIndex());
}

const DomainPriority& DomainPriorityCachedProperty::getDomainPriority()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_domainPriority;
}

Bool DomainPriorityCachedProperty::supportsProperty(void)
{
    return (getDomainPriority().getCurrentPriority() != Constants::Invalid);
}