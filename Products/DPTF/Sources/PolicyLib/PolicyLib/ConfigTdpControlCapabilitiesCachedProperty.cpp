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

#include "ConfigTdpControlCapabilitiesCachedProperty.h"
using namespace std;

ConfigTdpControlCapabilitiesCachedProperty::ConfigTdpControlCapabilitiesCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_configTdpControlDynamicCaps(Constants::Invalid, Constants::Invalid)
{
}

ConfigTdpControlCapabilitiesCachedProperty::~ConfigTdpControlCapabilitiesCachedProperty()
{
}

Bool ConfigTdpControlCapabilitiesCachedProperty::implementsConfigTdpControlInterface(void)
{
    return getDomainProperties().implementsConfigTdpControlInterface();
}

const ConfigTdpControlDynamicCaps& ConfigTdpControlCapabilitiesCachedProperty::getDynamicCaps(void)
{
    if (implementsConfigTdpControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_configTdpControlDynamicCaps;
    }
    else
    {
        throw dptf_exception("Domain does not support the ConfigTDP control interface.");
    }
}

Bool ConfigTdpControlCapabilitiesCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsConfigTdpControlInterface();
}

void ConfigTdpControlCapabilitiesCachedProperty::refreshData(void)
{
    if (implementsConfigTdpControlInterface())
    {
        m_configTdpControlDynamicCaps =
            getPolicyServices().domainConfigTdpControl->getConfigTdpControlDynamicCaps(
                getParticipantIndex(), getDomainIndex());
    }
}