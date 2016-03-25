/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "DisplayControlStatusProperty.h"
using namespace std;

DisplayControlStatusProperty::DisplayControlStatusProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_displayControlStatus(Constants::Invalid)
{
}

DisplayControlStatusProperty::~DisplayControlStatusProperty()
{
}

Bool DisplayControlStatusProperty::implementsDisplayControlInterface(void)
{
    return getDomainProperties().implementsDisplayControlInterface();
}

const DisplayControlStatus& DisplayControlStatusProperty::getStatus(void)
{
    if (implementsDisplayControlInterface())
    {
        m_displayControlStatus = getPolicyServices().domainDisplayControl->getDisplayControlStatus(
            getParticipantIndex(), getDomainIndex());  // we need to re-read the status in case the user changed it
        return m_displayControlStatus;
    }
    else
    {
        throw dptf_exception("Domain does not support the display control interface.");
    }
}

Bool DisplayControlStatusProperty::supportsProperty(void)
{
    return implementsDisplayControlInterface();
}