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

#include "PerformanceControlArbitrator.h"
#include "Utility.h"

PerformanceControlArbitrator::PerformanceControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_arbitratedPerformanceControlIndex(Constants::Invalid)
{
}

PerformanceControlArbitrator::~PerformanceControlArbitrator(void)
{
}

Bool PerformanceControlArbitrator::arbitrate(UIntN policyIndex, UIntN performanceControlIndex)
{
    Bool arbitratedValueChanged = false;
    UIntN maxRequestedPerformanceControlIndex = Constants::Invalid;

    increaseVectorSizeIfNeeded(m_requestedPerformanceControlIndex, policyIndex, Constants::Invalid);
    m_requestedPerformanceControlIndex[policyIndex] = performanceControlIndex;

    //
    // loop through the array and find the max requested performance control index (which is the lowest p-state performance)
    //
    for (UIntN i = 0; i < m_requestedPerformanceControlIndex.size(); i++)
    {
        if ((m_requestedPerformanceControlIndex[i] != Constants::Invalid) &&
            ((maxRequestedPerformanceControlIndex == Constants::Invalid) ||
             (m_requestedPerformanceControlIndex[i] > maxRequestedPerformanceControlIndex)))
        {
            maxRequestedPerformanceControlIndex = m_requestedPerformanceControlIndex[i];
        }
    }

    //
    // check to see if the performance control index is changing.
    //
    if (maxRequestedPerformanceControlIndex != m_arbitratedPerformanceControlIndex)
    {
        arbitratedValueChanged = true;
        m_arbitratedPerformanceControlIndex = maxRequestedPerformanceControlIndex;
    }

    return arbitratedValueChanged;
}

UIntN PerformanceControlArbitrator::getArbitratedPerformanceControlIndex(void) const
{
    return m_arbitratedPerformanceControlIndex;
}

void PerformanceControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedPerformanceControlIndex.size())
    {
        m_requestedPerformanceControlIndex[policyIndex] = Constants::Invalid;
    }
}