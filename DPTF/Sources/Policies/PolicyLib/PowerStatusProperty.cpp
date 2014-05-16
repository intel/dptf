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

#include "PowerStatusProperty.h"
using namespace std;

PowerStatusProperty::PowerStatusProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : DomainProperty(participantIndex, domainIndex, domainProperties, policyServices)
{
}

PowerStatusProperty::~PowerStatusProperty()
{
}

Bool PowerStatusProperty::implementsPowerStatusInterface(void)
{
    return getDomainProperties().implementsPowerStatusInterface();
}

PowerStatus PowerStatusProperty::getStatus()
{
    if (implementsPowerStatusInterface())
    {
        return getPolicyServices().domainPowerStatus->getPowerStatus(
            getParticipantIndex(), getDomainIndex());
    }
    else
    {
        throw dptf_exception("Domain does not support the power status interface.");
    }
}

Bool PowerStatusProperty::supportsProperty(void)
{
    return implementsPowerStatusInterface();
}