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

#include "CoreControlCapabilitiesCachedProperty.h"
using namespace std;

CoreControlCapabilitiesCachedProperty::CoreControlCapabilitiesCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_dynamicCaps(Constants::Invalid, Constants::Invalid),
    m_staticCaps(Constants::Invalid)
{
}

CoreControlCapabilitiesCachedProperty::~CoreControlCapabilitiesCachedProperty()
{
}

Bool CoreControlCapabilitiesCachedProperty::implementsCoreControlInterface(void)
{
    return getDomainProperties().implementsCoreControlInterface();
}

const CoreControlDynamicCaps& CoreControlCapabilitiesCachedProperty::getDynamicCaps(void)
{
    if (implementsCoreControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_dynamicCaps;
    }
    else
    {
        throw dptf_exception("Domain does not support the core control interface.");
    }
}

const CoreControlStaticCaps& CoreControlCapabilitiesCachedProperty::getStaticCaps(void)
{
    if (implementsCoreControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_staticCaps;
    }
    else
    {
        throw dptf_exception("Domain does not support the core control interface.");
    }
}

Bool CoreControlCapabilitiesCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsCoreControlInterface();
}

void CoreControlCapabilitiesCachedProperty::refreshData(void)
{
    if (implementsCoreControlInterface())
    {
        m_staticCaps = getPolicyServices().domainCoreControl->getCoreControlStaticCaps(
            getParticipantIndex(), getDomainIndex());
        m_dynamicCaps = getPolicyServices().domainCoreControl->getCoreControlDynamicCaps(
            getParticipantIndex(), getDomainIndex());
    }
}