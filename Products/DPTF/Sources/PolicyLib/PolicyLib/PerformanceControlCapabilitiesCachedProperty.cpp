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

#include "PerformanceControlCapabilitiesCachedProperty.h"
using namespace std;

PerformanceControlCapabilitiesCachedProperty::PerformanceControlCapabilitiesCachedProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : CachedProperty(), DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_performanceControlDynamicCaps(Constants::Invalid, Constants::Invalid),
    m_performanceControlStaticCaps(false)
{
}

PerformanceControlCapabilitiesCachedProperty::~PerformanceControlCapabilitiesCachedProperty()
{
}

Bool PerformanceControlCapabilitiesCachedProperty::implementsPerformanceControlInterface(void)
{
    return getDomainProperties().implementsPerformanceControlInterface();
}

const PerformanceControlDynamicCaps& PerformanceControlCapabilitiesCachedProperty::getDynamicCaps(void)
{
    if (implementsPerformanceControlInterface())
    {
        if (isCacheValid() == false)
        {
            refresh();
        }
        return m_performanceControlDynamicCaps;
    }
    else
    {
        throw dptf_exception("Domain does not support the performance control interface.");
    }
}

Bool PerformanceControlCapabilitiesCachedProperty::supportsProperty(void)
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return implementsPerformanceControlInterface();
}

void PerformanceControlCapabilitiesCachedProperty::refreshData(void)
{
    if (implementsPerformanceControlInterface())
    {
        m_performanceControlStaticCaps =
            getPolicyServices().domainPerformanceControl->getPerformanceControlStaticCaps(
                getParticipantIndex(), getDomainIndex());
        m_performanceControlDynamicCaps =
            getPolicyServices().domainPerformanceControl->getPerformanceControlDynamicCaps(
                getParticipantIndex(), getDomainIndex());
    }
}