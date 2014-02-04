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

#include "ConfigTdpControlArbitrator.h"
#include "Utility.h"

ConfigTdpControlArbitrator::ConfigTdpControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_arbitratedConfigTdpControlIndex(Constants::Invalid)
{
}

ConfigTdpControlArbitrator::~ConfigTdpControlArbitrator(void)
{
}

Bool ConfigTdpControlArbitrator::arbitrate(UIntN policyIndex, UIntN configTdpControlIndex)
{
    Bool arbitratedValueChanged = false;
    UIntN maxRequestedConfigTdpControlIndex = Constants::Invalid;

    increaseVectorSizeIfNeeded(m_requestedConfigTdpControlIndex, policyIndex, Constants::Invalid);
    m_requestedConfigTdpControlIndex[policyIndex] = configTdpControlIndex;

    //
    // loop through the array and find the lowest config tdp level requested (this is the max index)
    //
    for (UIntN i = 0; i < m_requestedConfigTdpControlIndex.size(); i++)
    {
        if ((m_requestedConfigTdpControlIndex[i] != Constants::Invalid) &&
            ((maxRequestedConfigTdpControlIndex == Constants::Invalid) ||
             (m_requestedConfigTdpControlIndex[i] > maxRequestedConfigTdpControlIndex)))
        {
            maxRequestedConfigTdpControlIndex = m_requestedConfigTdpControlIndex[i];
        }
    }

    //
    // check to see if the config tdp control index is changing.
    //
    if (maxRequestedConfigTdpControlIndex != m_arbitratedConfigTdpControlIndex)
    {
        arbitratedValueChanged = true;
        m_arbitratedConfigTdpControlIndex = maxRequestedConfigTdpControlIndex;
    }

    return arbitratedValueChanged;
}

UIntN ConfigTdpControlArbitrator::getArbitratedConfigTdpControlIndex(void) const
{
    return m_arbitratedConfigTdpControlIndex;
}

void ConfigTdpControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedConfigTdpControlIndex.size())
    {
        m_requestedConfigTdpControlIndex[policyIndex] = Constants::Invalid;
    }
}