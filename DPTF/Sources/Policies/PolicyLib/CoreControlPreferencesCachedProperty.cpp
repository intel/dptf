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

#include "CoreControlPreferencesCachedProperty.h"
using namespace std;

CoreControlPreferencesCachedProperty::CoreControlPreferencesCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_preferences(false, Constants::Invalid, Percentage::createInvalid(),
        CoreControlOffliningMode::None, CoreControlOffliningMode::None)
{
}

CoreControlPreferencesCachedProperty::~CoreControlPreferencesCachedProperty()
{
}

Bool CoreControlPreferencesCachedProperty::implementsCoreControlInterface(void)
{
    return getDomainProperties().implementsCoreControlInterface();
}

CoreControlLpoPreference CoreControlPreferencesCachedProperty::getPreferences(void)
{
    if (implementsCoreControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_preferences;
    }
    else
    {
        throw dptf_exception("Domain does not support the core control interface.");
    }
}

Bool CoreControlPreferencesCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsCoreControlInterface();
}

void CoreControlPreferencesCachedProperty::refreshData(void)
{
    if (implementsCoreControlInterface())
    {
        m_preferences =
            getPolicyServices().domainCoreControl->getCoreControlLpoPreference(getParticipantIndex(), getDomainIndex());
    }
}