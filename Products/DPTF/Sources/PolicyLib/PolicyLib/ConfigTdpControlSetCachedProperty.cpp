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

#include "ConfigTdpControlSetCachedProperty.h"
using namespace std;

ConfigTdpControlSetCachedProperty::ConfigTdpControlSetCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_configTdpControlSet(std::vector<ConfigTdpControl>(1, ConfigTdpControl(0, 0, 0, 0)))
{
}

ConfigTdpControlSetCachedProperty::~ConfigTdpControlSetCachedProperty()
{
}

Bool ConfigTdpControlSetCachedProperty::implementsConfigTdpControlInterface(void)
{
    return getDomainProperties().implementsConfigTdpControlInterface();
}

const ConfigTdpControlSet& ConfigTdpControlSetCachedProperty::getControlSet(void)
{
    if (implementsConfigTdpControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_configTdpControlSet;
    }
    else
    {
        throw dptf_exception("Domain does not support the ConfigTDP control interface.");
    }
}

Bool ConfigTdpControlSetCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsConfigTdpControlInterface();
}

void ConfigTdpControlSetCachedProperty::refreshData(void)
{
    if (implementsConfigTdpControlInterface())
    {
        m_configTdpControlSet = getPolicyServices().domainConfigTdpControl->getConfigTdpControlSet(
                getParticipantIndex(), getDomainIndex());
    }
}