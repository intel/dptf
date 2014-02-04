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

#include "PowerStatusCachedProperty.h"
using namespace std;

PowerStatusCachedProperty::PowerStatusCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_powerStatus(Power::createInvalid())
{
}

PowerStatusCachedProperty::~PowerStatusCachedProperty()
{
}

Bool PowerStatusCachedProperty::implementsPowerStatusInterface(void)
{
    return getDomainProperties().implementsPowerStatusInterface();
}

const PowerStatus& PowerStatusCachedProperty::getStatus(void)
{
    if (implementsPowerStatusInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_powerStatus;
    }
    else
    {
        throw dptf_exception("Domain does not support the power control interface.");
    }
}

Bool PowerStatusCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsPowerStatusInterface();
}

void PowerStatusCachedProperty::refreshData(void)
{
    if (implementsPowerStatusInterface())
    {
        m_powerStatus =
            getPolicyServices().domainPowerStatus->getPowerStatus(
                getParticipantIndex(), getDomainIndex());
    }
}