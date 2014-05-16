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

#include "DisplayControlStatusCachedProperty.h"
using namespace std;

DisplayControlStatusCachedProperty::DisplayControlStatusCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_displayControlStatus(Constants::Invalid)
{
}

DisplayControlStatusCachedProperty::~DisplayControlStatusCachedProperty()
{
}

Bool DisplayControlStatusCachedProperty::implementsDisplayControlInterface(void)
{
    return getDomainProperties().implementsDisplayControlInterface();
}

const DisplayControlStatus& DisplayControlStatusCachedProperty::getStatus(void)
{
    if (implementsDisplayControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_displayControlStatus;
    }
    else
    {
        throw dptf_exception("Domain does not support the display control interface.");
    }
}

Bool DisplayControlStatusCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsDisplayControlInterface();
}

void DisplayControlStatusCachedProperty::refreshData(void)
{
    m_displayControlStatus = getPolicyServices().domainDisplayControl->getDisplayControlStatus(
        getParticipantIndex(), getDomainIndex());
}