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

#include "DisplayControlSetCachedProperty.h"
using namespace std;

DisplayControlSetCachedProperty::DisplayControlSetCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_displayControlSet(std::vector<DisplayControl>())
{
}

DisplayControlSetCachedProperty::~DisplayControlSetCachedProperty(void)
{
}

void DisplayControlSetCachedProperty::refreshData(void)
{
    m_displayControlSet =
        getPolicyServices().domainDisplayControl->getDisplayControlSet(getParticipantIndex(), getDomainIndex());
}

const DisplayControlSet& DisplayControlSetCachedProperty::getControls()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_displayControlSet;
}

Bool DisplayControlSetCachedProperty::supportsProperty(void)
{
    if (getDomainProperties().implementsDisplayControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return (m_displayControlSet.getCount() > 0);
    }
    else
    {
        return false;
    }
}
