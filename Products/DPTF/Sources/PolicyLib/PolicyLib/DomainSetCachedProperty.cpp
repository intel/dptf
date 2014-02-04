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

#include "DomainSetCachedProperty.h"
using namespace std;

DomainSetCachedProperty::DomainSetCachedProperty(PolicyServicesInterfaceContainer policyServices, UIntN participantIndex)
    : CachedProperty(), ParticipantProperty(participantIndex, policyServices),
    m_domainProperties(vector<DomainProperties>())
{
}

DomainSetCachedProperty::~DomainSetCachedProperty(void)
{
}

void DomainSetCachedProperty::refreshData(void)
{
    m_domainProperties = getPolicyServices().participantProperties->getDomainPropertiesSet(getParticipantIndex());
}

const DomainPropertiesSet& DomainSetCachedProperty::getDomainPropertiesSet()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_domainProperties;
}

Bool DomainSetCachedProperty::supportsProperty(void)
{
    return (getDomainPropertiesSet().getDomainCount() > 0);
}

DomainProperties DomainSetCachedProperty::getDomainProperties(UIntN domainIndex)
{
    const DomainPropertiesSet& set = getDomainPropertiesSet();
    for (UIntN setIndex = 0; setIndex < set.getDomainCount(); setIndex++)
    {
        if (set[setIndex].getDomainIndex() == domainIndex)
        {
            return set[setIndex];
        }
    }

    throw dptf_exception("Domain " + to_string(domainIndex) + " not in domain set for participant.");
}