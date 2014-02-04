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

#include "RfStatusCachedProperty.h"
using namespace std;

RfStatusCachedProperty::RfStatusCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_profileData(0, 0, 0, RfProfileSupplementalData(0, 0, 0, 0, RadioConnectionStatus::NotConnected, 0))
{
}

RfStatusCachedProperty::~RfStatusCachedProperty()
{
}

Bool RfStatusCachedProperty::implementsRfStatusInterface(void)
{
    return getDomainProperties().implementsRfProfileStatusInterface();
}

RfProfileData RfStatusCachedProperty::getProfileData(void)
{
    if (implementsRfStatusInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_profileData;
    }
    else
    {
        throw dptf_exception("Domain does not support the radio frequency status interface.");
    }
}

Bool RfStatusCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsRfStatusInterface();
}

void RfStatusCachedProperty::refreshData(void)
{
    if (implementsRfStatusInterface())
    {
        m_profileData = getPolicyServices().domainRfProfileStatus->getRfProfileData(
            getParticipantIndex(), getDomainIndex());
    }
}