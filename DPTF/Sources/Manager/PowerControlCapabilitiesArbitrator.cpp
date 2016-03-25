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

#include "PowerControlCapabilitiesArbitrator.h"
#include "Utility.h"

PowerControlCapabilitiesArbitrator::PowerControlCapabilitiesArbitrator()
{

}

PowerControlCapabilitiesArbitrator::~PowerControlCapabilitiesArbitrator(void)
{

}

Bool PowerControlCapabilitiesArbitrator::arbitrate(
    UIntN policyIndex, const PowerControlDynamicCapsSet& capSet)
{
    auto prevArbitratedCaps = getArbitratedPowerControlCapabilities();
    updatePolicyRequest(capSet, policyIndex);
    auto nextArbitratedCaps = getArbitratedPowerControlCapabilities();
    return (prevArbitratedCaps != nextArbitratedCaps);
}

void PowerControlCapabilitiesArbitrator::updatePolicyRequest(
    const PowerControlDynamicCapsSet &capSet, UIntN policyIndex)
{
    auto controlTypes = capSet.getControlTypes();
    for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
    {
        auto capability = capSet.getCapability(*controlType);
        m_requestedMaxPowerLimit[policyIndex][*controlType] = capability.getMaxPowerLimit();
        m_requestedMinPowerLimit[policyIndex][*controlType] = capability.getMinPowerLimit();
        m_requestedPowerLimitStep[policyIndex][*controlType] = capability.getPowerStepSize();
        m_requestedMaxTimeWindow[policyIndex][*controlType] = capability.getMaxTimeWindow();
        m_requestedMinTimeWindow[policyIndex][*controlType] = capability.getMinTimeWindow();
    }
}

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::getArbitratedPowerControlCapabilities() const
{
    std::vector<PowerControlDynamicCaps> allCaps;
    auto controlTypes = getControlTypes();
    for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
    {
        Power maxPowerLimit = getLowestMaxPowerLimit(*controlType);
        Power minPowerLimit = getHighestMinPowerLimit(*controlType);
        Power stepSize = getHighestPowerLimitStep(*controlType);
        TimeSpan maxTimeWindow = getLowestMaxTimeWindow(*controlType);
        TimeSpan minTimeWindow = getHighestMinTimeWindow(*controlType);
        PowerControlDynamicCaps caps(*controlType,
            minPowerLimit, maxPowerLimit, stepSize,
            minTimeWindow, maxTimeWindow,
            Percentage(0.0), Percentage(0.0));
        allCaps.push_back(caps);
    }
    return PowerControlDynamicCapsSet(allCaps);
}

void PowerControlCapabilitiesArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
    m_requestedMaxPowerLimit.erase(policyIndex);
    m_requestedMinPowerLimit.erase(policyIndex);
    m_requestedPowerLimitStep.erase(policyIndex);
    m_requestedMaxTimeWindow.erase(policyIndex);
    m_requestedMinTimeWindow.erase(policyIndex);
}

Power PowerControlCapabilitiesArbitrator::getLowestMaxPowerLimit(PowerControlType::Type controlType) const
{
    Power lowestMaxPower(Power::createInvalid());
    for (auto policy = m_requestedMaxPowerLimit.begin(); policy != m_requestedMaxPowerLimit.end(); policy++)
    {
        auto request = policy->second.find(controlType);
        if (request != policy->second.end())
        {
            if (lowestMaxPower.isValid() == false)
            {
                lowestMaxPower = request->second;
            }
            else
            {
                lowestMaxPower = std::min(lowestMaxPower, request->second);
            }
        }
    }
    return lowestMaxPower;
}

Power PowerControlCapabilitiesArbitrator::getHighestMinPowerLimit(PowerControlType::Type controlType) const
{
    Power highestMinPower(Power::createInvalid());
    for (auto policy = m_requestedMinPowerLimit.begin(); policy != m_requestedMinPowerLimit.end(); policy++)
    {
        auto request = policy->second.find(controlType);
        if (request != policy->second.end())
        {
            if (highestMinPower.isValid() == false)
            {
                highestMinPower = request->second;
            }
            else
            {
                highestMinPower = std::max(highestMinPower, request->second);
            }
        }
    }
    return highestMinPower;
}

Power PowerControlCapabilitiesArbitrator::getHighestPowerLimitStep(PowerControlType::Type controlType) const
{
    Power highestStepPower(Power::createInvalid());
    for (auto policy = m_requestedPowerLimitStep.begin(); policy != m_requestedPowerLimitStep.end(); policy++)
    {
        auto request = policy->second.find(controlType);
        if (request != policy->second.end())
        {
            if (highestStepPower.isValid() == false)
            {
                highestStepPower = request->second;
            }
            else
            {
                highestStepPower = std::max(highestStepPower, request->second);
            }
        }
    }
    return highestStepPower;
}

TimeSpan PowerControlCapabilitiesArbitrator::getLowestMaxTimeWindow(PowerControlType::Type controlType) const
{
    TimeSpan lowestMaxTimeWindow(TimeSpan::createInvalid());
    for (auto policy = m_requestedMaxTimeWindow.begin(); policy != m_requestedMaxTimeWindow.end(); policy++)
    {
        auto request = policy->second.find(controlType);
        if (request != policy->second.end())
        {
            if (lowestMaxTimeWindow.isValid() == false)
            {
                lowestMaxTimeWindow = request->second;
            }
            else
            {
                lowestMaxTimeWindow = std::min(lowestMaxTimeWindow, request->second);
            }
        }
    }
    return lowestMaxTimeWindow;
}

TimeSpan PowerControlCapabilitiesArbitrator::getHighestMinTimeWindow(PowerControlType::Type controlType) const
{
    TimeSpan highestMinTimeWindow(TimeSpan::createInvalid());
    for (auto policy = m_requestedMinTimeWindow.begin(); policy != m_requestedMinTimeWindow.end(); policy++)
    {
        auto request = policy->second.find(controlType);
        if (request != policy->second.end())
        {
            if (highestMinTimeWindow.isValid() == false)
            {
                highestMinTimeWindow = request->second;
            }
            else
            {
                highestMinTimeWindow = std::max(highestMinTimeWindow, request->second);
            }
        }
    }
    return highestMinTimeWindow;
}

std::set<PowerControlType::Type> PowerControlCapabilitiesArbitrator::getControlTypes() const
{
    std::set<PowerControlType::Type> controlTypes;
    for (auto request = m_requestedMaxPowerLimit.begin(); request != m_requestedMaxPowerLimit.end(); request++)
    {
        for (auto controlType = request->second.begin(); controlType != request->second.end(); controlType++)
        {
            controlTypes.insert(controlType->first);
        }
    }

    for (auto request = m_requestedMinPowerLimit.begin(); request != m_requestedMinPowerLimit.end(); request++)
    {
        for (auto controlType = request->second.begin(); controlType != request->second.end(); controlType++)
        {
            controlTypes.insert(controlType->first);
        }
    }

    for (auto request = m_requestedPowerLimitStep.begin(); request != m_requestedPowerLimitStep.end(); request++)
    {
        for (auto controlType = request->second.begin(); controlType != request->second.end(); controlType++)
        {
            controlTypes.insert(controlType->first);
        }
    }

    for (auto request = m_requestedMaxTimeWindow.begin(); request != m_requestedMaxTimeWindow.end(); request++)
    {
        for (auto controlType = request->second.begin(); controlType != request->second.end(); controlType++)
        {
            controlTypes.insert(controlType->first);
        }
    }

    for (auto request = m_requestedMinTimeWindow.begin(); request != m_requestedMinTimeWindow.end(); request++)
    {
        for (auto controlType = request->second.begin(); controlType != request->second.end(); controlType++)
        {
            controlTypes.insert(controlType->first);
        }
    }

    return controlTypes;
}