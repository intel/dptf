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

#include "CoreControlArbitrator.h"
#include "Utility.h"

CoreControlArbitrator::CoreControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_arbitratedCoreControlStatus(Constants::Invalid)
{
}

CoreControlArbitrator::~CoreControlArbitrator(void)
{
    for (UIntN i = 0; i < m_requestedCoreControlStatus.size(); i++)
    {
        DELETE_MEMORY_TC(m_requestedCoreControlStatus[i]);
    }
}

Bool CoreControlArbitrator::arbitrate(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
    Bool arbitratedValueChanged = false;
    CoreControlStatus arbitratedCoreControlStatus(Constants::Invalid);

    increaseVectorSizeIfNeeded(m_requestedCoreControlStatus, policyIndex);

    // save the CoreControlStatus at the correct location for this policy
    if (m_requestedCoreControlStatus[policyIndex] == nullptr)
    {
        m_requestedCoreControlStatus[policyIndex] = new CoreControlStatus(coreControlStatus);
    }
    else
    {
        *(m_requestedCoreControlStatus[policyIndex]) = coreControlStatus;
    }

    // loop through and find the request for the least number of active logical processors
    for (UIntN i = 0; i < m_requestedCoreControlStatus.size(); i++)
    {
        CoreControlStatus* currentCoreControlStatus = m_requestedCoreControlStatus[i];

        if (currentCoreControlStatus != nullptr)
        {
            UIntN currentNumActiveLogicalProcessors = currentCoreControlStatus->getNumActiveLogicalProcessors();
            UIntN arbitratedNumActiveLogicalProcessors = arbitratedCoreControlStatus.getNumActiveLogicalProcessors();

            if ((currentNumActiveLogicalProcessors != Constants::Invalid) &&
                ((arbitratedNumActiveLogicalProcessors == Constants::Invalid) ||
                 (currentNumActiveLogicalProcessors < arbitratedNumActiveLogicalProcessors)))
            {
                arbitratedCoreControlStatus = *currentCoreControlStatus;
            }
        }
    }

    // check to see if the CoreControlStatus is changing.
    if (arbitratedCoreControlStatus != m_arbitratedCoreControlStatus)
    {
        arbitratedValueChanged = true;
        m_arbitratedCoreControlStatus = arbitratedCoreControlStatus;
    }

    return arbitratedValueChanged;
}

CoreControlStatus CoreControlArbitrator::getArbitratedCoreControlStatus(void) const
{
    return m_arbitratedCoreControlStatus;
}

void CoreControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedCoreControlStatus.size())
    {
        DELETE_MEMORY_TC(m_requestedCoreControlStatus[policyIndex]);
    }
}