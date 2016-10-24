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

#include "DisplayControlCapabilitiesArbitrator.h"
#include "Utility.h"

DisplayControlCapabilitiesArbitrator::DisplayControlCapabilitiesArbitrator()
{
}

DisplayControlCapabilitiesArbitrator::~DisplayControlCapabilitiesArbitrator()
{
}

Bool DisplayControlCapabilitiesArbitrator::arbitrate(UIntN policyIndex, const DisplayControlDynamicCaps& caps)
{
    auto prevArbitratedCaps = getArbitratedDisplayControlCapabilities();
    updatePolicyRequest(caps, policyIndex);
    auto nextArbitratedCaps = getArbitratedDisplayControlCapabilities();
    return (prevArbitratedCaps != nextArbitratedCaps);
}

Bool DisplayControlCapabilitiesArbitrator::arbitrateLockRequests(UIntN policyIndex, Bool lock)
{
    Bool previousLock = getArbitratedLock();
    updatePolicyLockRequest(lock, policyIndex);
    Bool newLock = getArbitratedLock();
    return (previousLock != newLock);
}

DisplayControlDynamicCaps DisplayControlCapabilitiesArbitrator::getArbitratedDisplayControlCapabilities() const
{
    auto minDisplayIndex = getHighestMinDisplayIndex();
    auto maxDisplayIndex = getLowestMaxDisplayIndex();
    if (maxDisplayIndex > minDisplayIndex)
    {
        maxDisplayIndex = minDisplayIndex;
    }
    return DisplayControlDynamicCaps(maxDisplayIndex, minDisplayIndex);
}

Bool DisplayControlCapabilitiesArbitrator::getArbitratedLock() const
{
    for (auto lockRequest = m_requestedLocks.begin(); lockRequest != m_requestedLocks.end(); lockRequest++)
    {
        if (lockRequest->second == true)
        {
            return true;
        }
    }

    return false;
}

void DisplayControlCapabilitiesArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
    m_requestedMaxDisplayIndex.erase(policyIndex);
    m_requestedMinDisplayIndex.erase(policyIndex);
    m_requestedLocks.erase(policyIndex);
}

void DisplayControlCapabilitiesArbitrator::updatePolicyRequest(const DisplayControlDynamicCaps &caps, UIntN policyIndex)
{
    auto newMax = caps.getCurrentUpperLimit();
    auto newMin = caps.getCurrentLowerLimit();

    if (newMax == Constants::Invalid && newMin == Constants::Invalid)
    {
        m_requestedMaxDisplayIndex.erase(policyIndex);
        m_requestedMinDisplayIndex.erase(policyIndex);
    }
    else
    {
        m_requestedMaxDisplayIndex[policyIndex] = newMax;
        m_requestedMinDisplayIndex[policyIndex] = newMin;
    }
}

void DisplayControlCapabilitiesArbitrator::updatePolicyLockRequest(Bool lock, UIntN policyIndex)
{
    m_requestedLocks[policyIndex] = lock;
}

UIntN DisplayControlCapabilitiesArbitrator::getLowestMaxDisplayIndex() const
{
    UIntN lowestMaxDisplayIndex = Constants::Invalid;
    for (auto request = m_requestedMaxDisplayIndex.begin(); request != m_requestedMaxDisplayIndex.end(); request++)
    {
        if (lowestMaxDisplayIndex == Constants::Invalid)
        {
            lowestMaxDisplayIndex = request->second;
        }
        else
        {
            lowestMaxDisplayIndex = std::max(lowestMaxDisplayIndex, request->second);
        }
    }
    return lowestMaxDisplayIndex;
}

UIntN DisplayControlCapabilitiesArbitrator::getHighestMinDisplayIndex() const
{
    UIntN highestMinDisplayIndex = Constants::Invalid;
    for (auto request = m_requestedMinDisplayIndex.begin(); request != m_requestedMinDisplayIndex.end(); request++)
    {
        if (highestMinDisplayIndex == Constants::Invalid)
        {
            highestMinDisplayIndex = request->second;
        }
        else
        {
            highestMinDisplayIndex = std::min(highestMinDisplayIndex, request->second);
        }
    }
    return highestMinDisplayIndex;
}