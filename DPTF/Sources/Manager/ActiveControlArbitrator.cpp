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

#include "ActiveControlArbitrator.h"
#include "Utility.h"

ActiveControlArbitrator::ActiveControlArbitrator() :
    m_arbitratedFanSpeedPercentage(Percentage::createInvalid()),
    m_arbitratedActiveControlIndex(Constants::Invalid)
{
}

ActiveControlArbitrator::~ActiveControlArbitrator(void)
{
}

Bool ActiveControlArbitrator::arbitrate(UIntN policyIndex, const Percentage& fanSpeed)
{
    Bool arbitratedValueChanged = false;
    Percentage maxRequestedFanSpeedPercentage = Percentage::createInvalid();

    m_requestedfanSpeedPercentage[policyIndex] = fanSpeed;

    //
    // loop through and find the max requested fan speed percentage
    //
    for (auto policyRequest = m_requestedfanSpeedPercentage.begin(); policyRequest != m_requestedfanSpeedPercentage.end(); policyRequest++)
    {
        if ((policyRequest->second.isValid()) &&
            ((maxRequestedFanSpeedPercentage.isValid() == false) ||
             (policyRequest->second > maxRequestedFanSpeedPercentage)))
        {
            maxRequestedFanSpeedPercentage = policyRequest->second;
        }
    }

    //
    // check to see if the fan speed percentage is changing
    //
    if (maxRequestedFanSpeedPercentage != m_arbitratedFanSpeedPercentage)
    {
        arbitratedValueChanged = true;
        m_arbitratedFanSpeedPercentage = maxRequestedFanSpeedPercentage;
    }

    return arbitratedValueChanged;
}

Bool ActiveControlArbitrator::arbitrate(UIntN policyIndex, UIntN activeControlIndex)
{
    Bool arbitratedValueChanged = false;
    UIntN minRequestedActiveControlIndex = Constants::Invalid;

    m_requestedActiveControlIndex[policyIndex] = activeControlIndex;

    //
    // loop through and find the min requested active control index
    //
    for (auto policyRequest = m_requestedActiveControlIndex.begin(); policyRequest != m_requestedActiveControlIndex.end(); policyRequest++)
    {
        if ((policyRequest->second != Constants::Invalid) &&
            ((minRequestedActiveControlIndex == Constants::Invalid) ||
             (policyRequest->second < minRequestedActiveControlIndex)))
        {
            minRequestedActiveControlIndex = policyRequest->second;
        }
    }

    //
    // check to see if the active control index is changing.
    //
    if (minRequestedActiveControlIndex != m_arbitratedActiveControlIndex)
    {
        arbitratedValueChanged = true;
        m_arbitratedActiveControlIndex = minRequestedActiveControlIndex;
    }

    return arbitratedValueChanged;
}

Percentage ActiveControlArbitrator::getArbitratedFanSpeedPercentage(void) const
{
    return m_arbitratedFanSpeedPercentage;
}

UIntN ActiveControlArbitrator::getArbitratedActiveControlIndex(void) const
{
    return m_arbitratedActiveControlIndex;
}

void ActiveControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    auto fanSpeedRequest = m_requestedfanSpeedPercentage.find(policyIndex);
    if (fanSpeedRequest != m_requestedfanSpeedPercentage.end())
    {
        m_requestedfanSpeedPercentage[policyIndex] = Percentage::createInvalid();
        arbitrate(policyIndex, Percentage::createInvalid());
    }

    auto activeIndexRequest = m_requestedActiveControlIndex.find(policyIndex);
    if (activeIndexRequest != m_requestedActiveControlIndex.end())
    {
        m_requestedActiveControlIndex[policyIndex] = Constants::Invalid;
        arbitrate(policyIndex, Constants::Invalid);
    }
}