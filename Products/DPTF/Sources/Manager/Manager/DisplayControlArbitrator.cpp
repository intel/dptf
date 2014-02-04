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

#include "DisplayControlArbitrator.h"
#include "Utility.h"

DisplayControlArbitrator::DisplayControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_arbitratedDisplayControlIndex(Constants::Invalid)
{
}

DisplayControlArbitrator::~DisplayControlArbitrator(void)
{
}

Bool DisplayControlArbitrator::arbitrate(UIntN policyIndex, UIntN displayControlIndex)
{
    Bool arbitratedValueChanged = false;
    UIntN maxRequestedDisplayControlIndex = Constants::Invalid;

    increaseVectorSizeIfNeeded(m_requestedDisplayControlIndex, policyIndex, Constants::Invalid);
    m_requestedDisplayControlIndex[policyIndex] = displayControlIndex;

    //
    // loop through the array and find the max requested display control index
    //
    for (UIntN i = 0; i < m_requestedDisplayControlIndex.size(); i++)
    {
        if ((m_requestedDisplayControlIndex[i] != Constants::Invalid) &&
            ((maxRequestedDisplayControlIndex == Constants::Invalid) ||
             (m_requestedDisplayControlIndex[i] > maxRequestedDisplayControlIndex)))
        {
            maxRequestedDisplayControlIndex = m_requestedDisplayControlIndex[i];
        }
    }

    //
    // check to see if the performance control index is changing.
    //
    if (maxRequestedDisplayControlIndex != m_arbitratedDisplayControlIndex)
    {
        arbitratedValueChanged = true;
        m_arbitratedDisplayControlIndex = maxRequestedDisplayControlIndex;
    }

    return arbitratedValueChanged;
}

UIntN DisplayControlArbitrator::getArbitratedDisplayControlIndex(void) const
{
    return m_arbitratedDisplayControlIndex;
}

void DisplayControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedDisplayControlIndex.size())
    {
        m_requestedDisplayControlIndex[policyIndex] = Constants::Invalid;
    }
}