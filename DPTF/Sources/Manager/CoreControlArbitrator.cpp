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

#include "CoreControlArbitrator.h"
#include "Utility.h"

CoreControlArbitrator::CoreControlArbitrator() :
    m_arbitratedActiveCoreCount(Constants::Invalid)
{
}

CoreControlArbitrator::~CoreControlArbitrator(void)
{
}

Bool CoreControlArbitrator::arbitrate(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
    Bool arbitratedValueChanged = false;
    UIntN artibratedCoreCount = Constants::Invalid;

    // save the CoreControlStatus at the correct location for this policy
    m_requestedActiveCoreCount[policyIndex] = coreControlStatus.getNumActiveLogicalProcessors();

    // loop through and find the request for the least number of active logical processors
    for (auto policyRequest = m_requestedActiveCoreCount.begin(); policyRequest != m_requestedActiveCoreCount.end(); policyRequest++)
    {
        UIntN currentActiveCoreCount = policyRequest->second;

        if ((currentActiveCoreCount != Constants::Invalid) &&
            ((artibratedCoreCount == Constants::Invalid) ||
                (currentActiveCoreCount < artibratedCoreCount)))
        {
            artibratedCoreCount = currentActiveCoreCount;
        }
    }

    // check to see if the CoreControlStatus is changing.
    if (artibratedCoreCount != m_arbitratedActiveCoreCount)
    {
        arbitratedValueChanged = true;
        m_arbitratedActiveCoreCount = artibratedCoreCount;
    }

    return arbitratedValueChanged;
}

CoreControlStatus CoreControlArbitrator::getArbitratedCoreControlStatus(void) const
{
    return CoreControlStatus(m_arbitratedActiveCoreCount);
}

void CoreControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    auto policyRequest = m_requestedActiveCoreCount.find(policyIndex);
    if (policyRequest != m_requestedActiveCoreCount.end())
    {
        m_requestedActiveCoreCount[policyIndex] = Constants::Invalid;
        arbitrate(policyIndex, Constants::Invalid);
    }
}
