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

#include "ActiveControlArbitrator.h"
#include "Utility.h"

ActiveControlArbitrator::ActiveControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
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

    increaseVectorSizeIfNeeded(m_requestedfanSpeedPercentage, policyIndex, maxRequestedFanSpeedPercentage);
    m_requestedfanSpeedPercentage[policyIndex] = fanSpeed;

    //
    // loop through the array and find the max requested fan speed percentage
    //
    for (UIntN i = 0; i < m_requestedfanSpeedPercentage.size(); i++)
    {
        if ((m_requestedfanSpeedPercentage[i].isValid()) &&
            ((maxRequestedFanSpeedPercentage.isValid() == false) ||
             (m_requestedfanSpeedPercentage[i] > maxRequestedFanSpeedPercentage)))
        {
            maxRequestedFanSpeedPercentage = m_requestedfanSpeedPercentage[i];
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

    increaseVectorSizeIfNeeded(m_requestedActiveControlIndex, policyIndex, Constants::Invalid);
    m_requestedActiveControlIndex[policyIndex] = activeControlIndex;

    //
    // loop through the array and find the min requested active control index
    //
    for (UIntN i = 0; i < m_requestedActiveControlIndex.size(); i++)
    {
        if ((m_requestedActiveControlIndex[i] != Constants::Invalid) &&
            ((minRequestedActiveControlIndex == Constants::Invalid) ||
             (m_requestedActiveControlIndex[i] < minRequestedActiveControlIndex)))
        {
            minRequestedActiveControlIndex = m_requestedActiveControlIndex[i];
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
    if (policyIndex < m_requestedfanSpeedPercentage.size())
    {
        m_requestedfanSpeedPercentage[policyIndex] = Percentage::createInvalid();
    }

    if (policyIndex < m_requestedActiveControlIndex.size())
    {
        m_requestedActiveControlIndex[policyIndex] = Constants::Invalid;
    }
}