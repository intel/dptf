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

#include "PerformanceControlCapabilitiesArbitrator.h"
#include "Utility.h"

PerformanceControlCapabilitiesArbitrator::PerformanceControlCapabilitiesArbitrator()
{
}

PerformanceControlCapabilitiesArbitrator::~PerformanceControlCapabilitiesArbitrator()
{
}

Bool PerformanceControlCapabilitiesArbitrator::arbitrate(UIntN policyIndex, const PerformanceControlDynamicCaps& caps)
{
    auto prevArbitratedCaps = getArbitratedPerformanceControlCapabilities();
    updatePolicyRequest(caps, policyIndex);
    auto nextArbitratedCaps = getArbitratedPerformanceControlCapabilities();
    return (prevArbitratedCaps != nextArbitratedCaps);
}

Bool PerformanceControlCapabilitiesArbitrator::arbitrateLockRequests(UIntN policyIndex, Bool lock)
{
    Bool previousLock = getArbitratedLock();
    updatePolicyLockRequest(lock, policyIndex);
    Bool newLock = getArbitratedLock();
    return (previousLock != newLock);
}

PerformanceControlDynamicCaps PerformanceControlCapabilitiesArbitrator::getArbitratedPerformanceControlCapabilities() const
{
    auto lowerPStateIndex = getSmallestLowerPStateIndex();
    auto upperPStateIndex = getBiggestUpperPStateIndex();
    if (upperPStateIndex > lowerPStateIndex)
    {
        upperPStateIndex = lowerPStateIndex;
    }
    return PerformanceControlDynamicCaps(lowerPStateIndex, upperPStateIndex);
}

Bool PerformanceControlCapabilitiesArbitrator::getArbitratedLock() const
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

void PerformanceControlCapabilitiesArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
    m_requestedUpperPState.erase(policyIndex);
    m_requestedLowerPState.erase(policyIndex);
    m_requestedLocks.erase(policyIndex);
}

void PerformanceControlCapabilitiesArbitrator::updatePolicyRequest(const PerformanceControlDynamicCaps &caps, UIntN policyIndex)
{
    auto newMax = caps.getCurrentUpperLimitIndex();
    auto newMin = caps.getCurrentLowerLimitIndex();

    if (newMax == Constants::Invalid && newMin == Constants::Invalid)
    {
        m_requestedUpperPState.erase(policyIndex);
        m_requestedLowerPState.erase(policyIndex);
    }
    else
    {
        m_requestedUpperPState[policyIndex] = newMax;
        m_requestedLowerPState[policyIndex] = newMin;
    }
}

void PerformanceControlCapabilitiesArbitrator::updatePolicyLockRequest(Bool lock, UIntN policyIndex)
{
    m_requestedLocks[policyIndex] = lock;
}

UIntN PerformanceControlCapabilitiesArbitrator::getBiggestUpperPStateIndex() const
{
    UIntN biggestMaxPerfIndex = Constants::Invalid;
    for (auto request = m_requestedUpperPState.begin(); request != m_requestedUpperPState.end(); request++)
    {
        if (biggestMaxPerfIndex == Constants::Invalid)
        {
            biggestMaxPerfIndex = request->second;
        }
        else
        {
            biggestMaxPerfIndex = std::max(biggestMaxPerfIndex, request->second);
        }
    }
    return biggestMaxPerfIndex;
}

UIntN PerformanceControlCapabilitiesArbitrator::getSmallestLowerPStateIndex() const
{
    UIntN smallestLowPerfIndex = Constants::Invalid;
    for (auto request = m_requestedLowerPState.begin(); request != m_requestedLowerPState.end(); request++)
    {
        if (smallestLowPerfIndex == Constants::Invalid)
        {
            smallestLowPerfIndex = request->second;
        }
        else
        {
            smallestLowPerfIndex = std::min(smallestLowPerfIndex, request->second);
        }
    }
    return smallestLowPerfIndex;
}