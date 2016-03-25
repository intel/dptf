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

#include "PowerControlArbitrator.h"
#include "Utility.h"

PowerControlArbitrator::PowerControlArbitrator()
{

}

PowerControlArbitrator::~PowerControlArbitrator(void)
{

}

Bool PowerControlArbitrator::arbitrate(UIntN policyIndex, PowerControlType::Type controlType, 
    const Power& powerLimit)
{
    updatePolicyRequest(policyIndex, controlType, powerLimit);
    Power lowestRequest = getLowestRequest(controlType, m_requestedPowerLimits);
    Bool changed = setArbitratedRequest(controlType, lowestRequest);
    return changed;
}

Bool PowerControlArbitrator::arbitrate(UIntN policyIndex, PowerControlType::Type controlType, 
    const TimeSpan& timeWindow)
{
    updatePolicyRequest(policyIndex, controlType, timeWindow);
    TimeSpan lowestRequest = getLowestRequest(controlType, m_requestedTimeWindows);
    Bool changed = setArbitratedRequest(controlType, lowestRequest);
    return changed;
}

Bool PowerControlArbitrator::arbitrate(UIntN policyIndex, PowerControlType::Type controlType, 
    const Percentage& dutyCycle)
{
    updatePolicyRequest(policyIndex, controlType, dutyCycle);
    Percentage lowestRequest = getLowestRequest(controlType, m_requestedDutyCycles);
    Bool changed = setArbitratedRequest(controlType, lowestRequest);
    return changed;
}

Bool PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const Power& lowestRequest)
{
    Bool changed(false);
    auto controlRequests = m_arbitratedPowerLimit.find(controlType);
    if (controlRequests == m_arbitratedPowerLimit.end())
    {
        changed = true;
        m_arbitratedPowerLimit[controlType] = lowestRequest;
    }
    else
    {
        if (controlRequests->second != lowestRequest)
        {
            changed = true;
            m_arbitratedPowerLimit[controlType] = lowestRequest;
        }
    }
    return changed;
}

Bool PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const TimeSpan& lowestRequest)
{
    Bool changed(false);
    auto controlRequests = m_arbitratedTimeWindow.find(controlType);
    if (controlRequests == m_arbitratedTimeWindow.end())
    {
        changed = true;
        m_arbitratedTimeWindow[controlType] = lowestRequest;
    }
    else
    {
        if (controlRequests->second != lowestRequest)
        {
            changed = true;
            m_arbitratedTimeWindow[controlType] = lowestRequest;
        }
    }
    return changed;
}

Bool PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const Percentage& lowestRequest)
{
    Bool changed(false);
    auto controlRequests = m_arbitratedDutyCycle.find(controlType);
    if (controlRequests == m_arbitratedDutyCycle.end())
    {
        changed = true;
        m_arbitratedDutyCycle[controlType] = lowestRequest;
    }
    else
    {
        if (controlRequests->second != lowestRequest)
        {
            changed = true;
            m_arbitratedDutyCycle[controlType] = lowestRequest;
        }
    }
    return changed;
}

void PowerControlArbitrator::updatePolicyRequest(UIntN policyIndex, PowerControlType::Type controlType,
    const Power& powerLimit)
{
    auto policyRequests = m_requestedPowerLimits.find(policyIndex);
    if (policyRequests == m_requestedPowerLimits.end())
    {
        m_requestedPowerLimits[policyIndex] = std::map<PowerControlType::Type, Power>();
    }
    m_requestedPowerLimits[policyIndex][controlType] = powerLimit;
}

void PowerControlArbitrator::updatePolicyRequest(UIntN policyIndex, PowerControlType::Type controlType, 
    const TimeSpan& timeWindow)
{
    auto policyRequests = m_requestedTimeWindows.find(policyIndex);
    if (policyRequests == m_requestedTimeWindows.end())
    {
        m_requestedTimeWindows[policyIndex] = std::map<PowerControlType::Type, TimeSpan>();
    }
    m_requestedTimeWindows[policyIndex][controlType] = timeWindow;
}

void PowerControlArbitrator::updatePolicyRequest(UIntN policyIndex, PowerControlType::Type controlType, 
    const Percentage& dutyCycle)
{
    auto policyRequests = m_requestedDutyCycles.find(policyIndex);
    if (policyRequests == m_requestedDutyCycles.end())
    {
        m_requestedDutyCycles[policyIndex] = std::map<PowerControlType::Type, Percentage>();
    }
    m_requestedDutyCycles[policyIndex][controlType] = dutyCycle;
}

Power PowerControlArbitrator::getLowestRequest(PowerControlType::Type controlType, 
    const std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerLimits)
{
    Power lowestRequest;
    Bool lowestRequestSet(false);
    for (auto policy = powerLimits.begin(); policy != powerLimits.end(); policy++)
    {
        auto& controls = policy->second;
        auto control = controls.find(controlType);
        if (control != controls.end())
        {
            if (lowestRequestSet == false)
            {
                lowestRequestSet = true;
                lowestRequest = control->second;
            }
            else
            {
                if (control->second < lowestRequest)
                {
                    lowestRequest = control->second;
                }
            }
        }
    }

    if (lowestRequestSet == false)
    {
        throw dptf_exception("There were no power limit requests to pick from when choosing the lowest for \
                             arbitration.");
    }
    else
    {
        return lowestRequest;
    }
}

TimeSpan PowerControlArbitrator::getLowestRequest(PowerControlType::Type controlType, 
    const std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& timeWindows)
{
    TimeSpan lowestRequest;
    Bool lowestRequestSet(false);
    for (auto policy = timeWindows.begin(); policy != timeWindows.end(); policy++)
    {
        auto& controls = policy->second;
        auto control = controls.find(controlType);
        if (control != controls.end())
        {
            if (lowestRequestSet == false)
            {
                lowestRequestSet = true;
                lowestRequest = control->second;
            }
            else
            {
                if (control->second < lowestRequest)
                {
                    lowestRequest = control->second;
                }
            }
        }
    }

    if (lowestRequestSet == false)
    {
        throw dptf_exception("There were no power time window requests to pick from when choosing the lowest for \
                              arbitration.");
    }
    else
    {
        return lowestRequest;
    }
}

Percentage PowerControlArbitrator::getLowestRequest(PowerControlType::Type controlType, 
    const std::map<UIntN, std::map<PowerControlType::Type, Percentage>>& dutyCycles)
{
    Percentage lowestRequest;
    Bool lowestRequestSet(false);
    for (auto policy = dutyCycles.begin(); policy != dutyCycles.end(); policy++)
    {
        auto& controls = policy->second;
        auto control = controls.find(controlType);
        if (control != controls.end())
        {
            if (lowestRequestSet == false)
            {
                lowestRequestSet = true;
                lowestRequest = control->second;
            }
            else
            {
                if (control->second < lowestRequest)
                {
                    lowestRequest = control->second;
                }
            }
        }
    }

    if (lowestRequestSet == false)
    {
        throw dptf_exception("There were no power duty cycle requests to pick from when choosing the lowest for \
                              arbitration.");
    }
    else
    {
        return lowestRequest;
    }
}

Power PowerControlArbitrator::getArbitratedPowerLimit(PowerControlType::Type controlType) const
{
    auto controlPowerLimit = m_arbitratedPowerLimit.find(controlType);
    if (controlPowerLimit == m_arbitratedPowerLimit.end())
    {
        throw dptf_exception("No power limit has been set for control type " + 
            PowerControlType::ToString(controlType) + ".");
    }
    else
    {
        return controlPowerLimit->second;
    }
}

TimeSpan PowerControlArbitrator::getArbitratedTimeWindow(PowerControlType::Type controlType) const
{
    auto controlTimeWindow = m_arbitratedTimeWindow.find(controlType);
    if (controlTimeWindow == m_arbitratedTimeWindow.end())
    {
        throw dptf_exception("No power limit time window has been set for control type " +
            PowerControlType::ToString(controlType) + ".");
    }
    else
    {
        return controlTimeWindow->second;
    }
}

Percentage PowerControlArbitrator::getArbitratedDutyCycle(PowerControlType::Type controlType) const
{
    auto controlDutyCycle = m_arbitratedDutyCycle.find(controlType);
    if (controlDutyCycle == m_arbitratedDutyCycle.end())
    {
        throw dptf_exception("No power limit duty cycle has been set for control type " +
            PowerControlType::ToString(controlType) + ".");
    }
    else
    {
        return controlDutyCycle->second;
    }
}

void PowerControlArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
    removePowerLimitRequest(policyIndex);
    removeTimeWindowRequest(policyIndex);
    removeDutyCycleRequest(policyIndex);
}

void PowerControlArbitrator::removePowerLimitRequest(UIntN policyIndex)
{
    auto policyRequests = m_requestedPowerLimits.find(policyIndex);
    if (policyRequests != m_requestedPowerLimits.end())
    {
        auto controlTypes = findControlTypesSetForPolicy(policyRequests->second);
        m_requestedPowerLimits.erase(policyRequests);
        if (m_requestedPowerLimits.size() > 0)
        {
            setArbitratedPowerLimitForControlTypes(controlTypes);
        }
        else
        {
            m_arbitratedPowerLimit.clear();
        }
    }
}

void PowerControlArbitrator::removeTimeWindowRequest(UIntN policyIndex)
{
    auto policyRequests = m_requestedTimeWindows.find(policyIndex);
    if (policyRequests != m_requestedTimeWindows.end())
    {
        auto controlTypes = findControlTypesSetForPolicy(policyRequests->second);
        m_requestedTimeWindows.erase(policyRequests);
        if (m_requestedTimeWindows.size() > 0)
        {
            setArbitratedTimeWindowsForControlTypes(controlTypes);
        }
        else
        {
            m_arbitratedTimeWindow.clear();
        }
    }
}

void PowerControlArbitrator::removeDutyCycleRequest(UIntN policyIndex)
{
    auto policyRequests = m_requestedDutyCycles.find(policyIndex);
    if (policyRequests != m_requestedDutyCycles.end())
    {
        auto controlTypes = findControlTypesSetForPolicy(policyRequests->second);
        m_requestedDutyCycles.erase(policyRequests);
        if (m_requestedDutyCycles.size() > 0)
        {
            setArbitratedDutyCyclesForControlTypes(controlTypes);
        }
        else
        {
            m_arbitratedDutyCycle.clear();
        }
    }
}

void PowerControlArbitrator::setArbitratedPowerLimitForControlTypes(
    const std::vector<PowerControlType::Type>& controlTypes)
{
    for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
    {
        try
        {
            Power lowestRequest = getLowestRequest(*controlType, m_requestedPowerLimits);
            setArbitratedRequest(*controlType, lowestRequest);
        }
        catch (...)
        {
        }
    }
}

void PowerControlArbitrator::setArbitratedTimeWindowsForControlTypes(
    const std::vector<PowerControlType::Type>& controlTypes)
{
    for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
    {
        try
        {
            TimeSpan lowestRequest = getLowestRequest(*controlType, m_requestedTimeWindows);
            setArbitratedRequest(*controlType, lowestRequest);
        }
        catch (...)
        {
        }
    }
}

void PowerControlArbitrator::setArbitratedDutyCyclesForControlTypes(
    const std::vector<PowerControlType::Type>& controlTypes)
{
    for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
    {
        try
        {
            Percentage lowestRequest = getLowestRequest(*controlType, m_requestedDutyCycles);
            setArbitratedRequest(*controlType, lowestRequest);
        }
        catch (...)
        {
        }
    }
}

std::vector<PowerControlType::Type> PowerControlArbitrator::findControlTypesSetForPolicy(
    const std::map<PowerControlType::Type, Power>& controlRequests)
{
    std::vector<PowerControlType::Type> controlTypes;
    for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; controlType++)
    {
        auto controlRequest = controlRequests.find((PowerControlType::Type)controlType);
        if (controlRequest != controlRequests.end())
        {
            controlTypes.push_back((PowerControlType::Type)controlType);
        }
    }
    return controlTypes;
}

std::vector<PowerControlType::Type> PowerControlArbitrator::findControlTypesSetForPolicy(
    const std::map<PowerControlType::Type, TimeSpan>& controlRequests)
{
    std::vector<PowerControlType::Type> controlTypes;
    for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; controlType++)
    {
        auto controlRequest = controlRequests.find((PowerControlType::Type)controlType);
        if (controlRequest != controlRequests.end())
        {
            controlTypes.push_back((PowerControlType::Type)controlType);
        }
    }
    return controlTypes;
}

std::vector<PowerControlType::Type> PowerControlArbitrator::findControlTypesSetForPolicy(
    const std::map<PowerControlType::Type, Percentage>& controlRequests)
{
    std::vector<PowerControlType::Type> controlTypes;
    for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; controlType++)
    {
        auto controlRequest = controlRequests.find((PowerControlType::Type)controlType);
        if (controlRequest != controlRequests.end())
        {
            controlTypes.push_back((PowerControlType::Type)controlType);
        }
    }
    return controlTypes;
}