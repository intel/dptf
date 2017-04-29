/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

void PowerControlCapabilitiesArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	const PowerControlDynamicCapsSet& capSet)
{
	updatePolicyRequest(capSet, policyIndex);
}

Bool PowerControlCapabilitiesArbitrator::hasArbitratedPowerControlCapabilities() const
{
	if (m_requestedMaxPowerLimit.empty() && m_requestedMinPowerLimit.empty() && m_requestedPowerLimitStep.empty()
		&& m_requestedMaxTimeWindow.empty()
		&& m_requestedMinTimeWindow.empty())
	{
		return false;
	}
	return true;
}

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::arbitrate(
	UIntN policyIndex,
	const PowerControlDynamicCapsSet& capSet)
{
	auto newControlTypes = capSet.getControlTypes();
	auto tempRequestedMaxPowerLimit = m_requestedMaxPowerLimit;
	auto tempRequestedMinPowerLimit = m_requestedMinPowerLimit;
	auto tempRequestedPowerLimitStep = m_requestedPowerLimitStep;
	auto tempRequestedMaxTimeWindow = m_requestedMaxTimeWindow;
	auto tempRequestedMinTimeWindow = m_requestedMinTimeWindow;
	for (auto controlType = newControlTypes.begin(); controlType != newControlTypes.end(); ++controlType)
	{
		auto capability = capSet.getCapability(*controlType);
		tempRequestedMaxPowerLimit[policyIndex][*controlType] = capability.getMaxPowerLimit();
		tempRequestedMinPowerLimit[policyIndex][*controlType] = capability.getMinPowerLimit();
		tempRequestedPowerLimitStep[policyIndex][*controlType] = capability.getPowerStepSize();
		tempRequestedMaxTimeWindow[policyIndex][*controlType] = capability.getMaxTimeWindow();
		tempRequestedMinTimeWindow[policyIndex][*controlType] = capability.getMinTimeWindow();
	}

	std::vector<PowerControlDynamicCaps> allCaps;
	auto controlTypes = getControlTypes(
		tempRequestedMaxPowerLimit,
		tempRequestedMinPowerLimit,
		tempRequestedPowerLimitStep,
		tempRequestedMaxTimeWindow,
		tempRequestedMinTimeWindow);
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		Power maxPowerLimit = getLowestMaxPowerLimit(*controlType, tempRequestedMaxPowerLimit);
		Power minPowerLimit = getHighestMinPowerLimit(*controlType, tempRequestedMinPowerLimit);
		if (maxPowerLimit < minPowerLimit)
		{
			minPowerLimit = maxPowerLimit;
		}

		Power stepSize = getHighestPowerLimitStep(*controlType, tempRequestedPowerLimitStep);
		TimeSpan maxTimeWindow = getLowestMaxTimeWindow(*controlType, tempRequestedMaxTimeWindow);
		TimeSpan minTimeWindow = getHighestMinTimeWindow(*controlType, tempRequestedMinTimeWindow);
		if (maxTimeWindow < minTimeWindow)
		{
			minTimeWindow = maxTimeWindow;
		}

		PowerControlDynamicCaps caps(
			*controlType,
			minPowerLimit,
			maxPowerLimit,
			stepSize,
			minTimeWindow,
			maxTimeWindow,
			Percentage(0.0),
			Percentage(0.0));
		allCaps.push_back(caps);
	}
	return PowerControlDynamicCapsSet(allCaps);
}

Bool PowerControlCapabilitiesArbitrator::arbitrateLockRequests(UIntN policyIndex, Bool lock)
{
	Bool previousLock = getArbitratedLock();
	updatePolicyLockRequest(lock, policyIndex);
	Bool newLock = getArbitratedLock();
	return (previousLock != newLock);
}

void PowerControlCapabilitiesArbitrator::updatePolicyRequest(
	const PowerControlDynamicCapsSet& capSet,
	UIntN policyIndex)
{
	auto controlTypes = capSet.getControlTypes();
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		auto capability = capSet.getCapability(*controlType);
		m_requestedMaxPowerLimit[policyIndex][*controlType] = capability.getMaxPowerLimit();
		m_requestedMinPowerLimit[policyIndex][*controlType] = capability.getMinPowerLimit();
		m_requestedPowerLimitStep[policyIndex][*controlType] = capability.getPowerStepSize();
		m_requestedMaxTimeWindow[policyIndex][*controlType] = capability.getMaxTimeWindow();
		m_requestedMinTimeWindow[policyIndex][*controlType] = capability.getMinTimeWindow();
	}
}

void PowerControlCapabilitiesArbitrator::updatePolicyLockRequest(Bool lock, UIntN policyIndex)
{
	m_requestedLocks[policyIndex] = lock;
}

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::getArbitratedPowerControlCapabilities()
{
	std::vector<PowerControlDynamicCaps> allCaps;
	auto controlTypes = getControlTypes(
		m_requestedMaxPowerLimit,
		m_requestedMinPowerLimit,
		m_requestedPowerLimitStep,
		m_requestedMaxTimeWindow,
		m_requestedMinTimeWindow);
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		Power maxPowerLimit = getLowestMaxPowerLimit(*controlType, m_requestedMaxPowerLimit);
		Power minPowerLimit = getHighestMinPowerLimit(*controlType, m_requestedMinPowerLimit);
		if (maxPowerLimit < minPowerLimit)
		{
			minPowerLimit = maxPowerLimit;
		}

		Power stepSize = getHighestPowerLimitStep(*controlType, m_requestedPowerLimitStep);
		TimeSpan maxTimeWindow = getLowestMaxTimeWindow(*controlType, m_requestedMaxTimeWindow);
		TimeSpan minTimeWindow = getHighestMinTimeWindow(*controlType, m_requestedMinTimeWindow);
		if (maxTimeWindow < minTimeWindow)
		{
			minTimeWindow = maxTimeWindow;
		}

		PowerControlDynamicCaps caps(
			*controlType,
			minPowerLimit,
			maxPowerLimit,
			stepSize,
			minTimeWindow,
			maxTimeWindow,
			Percentage(0.0),
			Percentage(0.0));
		allCaps.push_back(caps);
	}
	return PowerControlDynamicCapsSet(allCaps);
}

Bool PowerControlCapabilitiesArbitrator::getArbitratedLock() const
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

void PowerControlCapabilitiesArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
	m_requestedMaxPowerLimit.erase(policyIndex);
	m_requestedMinPowerLimit.erase(policyIndex);
	m_requestedPowerLimitStep.erase(policyIndex);
	m_requestedMaxTimeWindow.erase(policyIndex);
	m_requestedMinTimeWindow.erase(policyIndex);
	m_requestedLocks.erase(policyIndex);
}

Power PowerControlCapabilitiesArbitrator::getLowestMaxPowerLimit(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const
{
	Power lowestMaxPower(Power::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
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

Power PowerControlCapabilitiesArbitrator::getHighestMinPowerLimit(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const
{
	Power highestMinPower(Power::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
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

Power PowerControlCapabilitiesArbitrator::getHighestPowerLimitStep(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const
{
	Power highestStepPower(Power::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
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

TimeSpan PowerControlCapabilitiesArbitrator::getLowestMaxTimeWindow(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& requests) const
{
	TimeSpan lowestMaxTimeWindow(TimeSpan::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
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

TimeSpan PowerControlCapabilitiesArbitrator::getHighestMinTimeWindow(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& requests) const
{
	TimeSpan highestMinTimeWindow(TimeSpan::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
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

std::set<PowerControlType::Type> PowerControlCapabilitiesArbitrator::getControlTypes(
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& maxPowerRequests,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& minPowerRequests,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerStepRequests,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& maxTimeRequests,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& minTimeRequests) const
{
	std::set<PowerControlType::Type> controlTypes;
	for (auto request = maxPowerRequests.begin(); request != maxPowerRequests.end(); ++request)
	{
		for (auto controlType = request->second.begin(); controlType != request->second.end(); ++controlType)
		{
			controlTypes.insert(controlType->first);
		}
	}

	for (auto request = minPowerRequests.begin(); request != minPowerRequests.end(); ++request)
	{
		for (auto controlType = request->second.begin(); controlType != request->second.end(); ++controlType)
		{
			controlTypes.insert(controlType->first);
		}
	}

	for (auto request = powerStepRequests.begin(); request != powerStepRequests.end(); ++request)
	{
		for (auto controlType = request->second.begin(); controlType != request->second.end(); ++controlType)
		{
			controlTypes.insert(controlType->first);
		}
	}

	for (auto request = maxTimeRequests.begin(); request != maxTimeRequests.end(); ++request)
	{
		for (auto controlType = request->second.begin(); controlType != request->second.end(); ++controlType)
		{
			controlTypes.insert(controlType->first);
		}
	}

	for (auto request = minTimeRequests.begin(); request != minTimeRequests.end(); ++request)
	{
		for (auto controlType = request->second.begin(); controlType != request->second.end(); ++controlType)
		{
			controlTypes.insert(controlType->first);
		}
	}

	return controlTypes;
}
