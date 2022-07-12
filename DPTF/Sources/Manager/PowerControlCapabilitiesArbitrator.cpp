/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include <StatusFormat.h>

PowerControlCapabilitiesArbitrator::PowerControlCapabilitiesArbitrator()
	: m_requestedMaxPowerLimit()
	, m_requestedMinPowerLimit()
	, m_requestedPowerLimitStep()
	, m_requestedMaxTimeWindow()
	, m_requestedMinTimeWindow()
	, m_requestedLocks()
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
		&& m_requestedMaxTimeWindow.empty() && m_requestedMinTimeWindow.empty())
	{
		return false;
	}
	return true;
}

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::arbitrate(
	UIntN policyIndex,
	const PowerControlDynamicCapsSet& requestedCapSet,
	const PowerControlDynamicCapsSet& currentCapSet)
{
	auto newControlTypes = requestedCapSet.getControlTypes();
	auto tempRequestedMaxPowerLimit = m_requestedMaxPowerLimit;
	auto tempRequestedMinPowerLimit = m_requestedMinPowerLimit;
	auto tempRequestedPowerLimitStep = m_requestedPowerLimitStep;
	auto tempRequestedMaxTimeWindow = m_requestedMaxTimeWindow;
	auto tempRequestedMinTimeWindow = m_requestedMinTimeWindow;
	for (auto controlType = newControlTypes.begin(); controlType != newControlTypes.end(); ++controlType)
	{
		auto capability = requestedCapSet.getCapability(*controlType);
		tempRequestedMaxPowerLimit[policyIndex][*controlType] = capability.getMaxPowerLimit();
		tempRequestedMinPowerLimit[policyIndex][*controlType] = capability.getMinPowerLimit();
		tempRequestedPowerLimitStep[policyIndex][*controlType] = capability.getPowerStepSize();
		tempRequestedMaxTimeWindow[policyIndex][*controlType] = capability.getMaxTimeWindow();
		tempRequestedMinTimeWindow[policyIndex][*controlType] = capability.getMinTimeWindow();
	}

	auto arbitratedCapabilities = createNewArbitratedCapabilitites(
		tempRequestedMaxPowerLimit,
		tempRequestedMinPowerLimit,
		tempRequestedPowerLimitStep,
		tempRequestedMaxTimeWindow,
		tempRequestedMinTimeWindow,
		currentCapSet);
	return arbitratedCapabilities;
}

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::createNewArbitratedCapabilitites(
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& maxPowerRequests,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& minPowerRequests,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerStepRequests,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& maxTimeRequests,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& minTimeRequests,
	const PowerControlDynamicCapsSet& currentCapSet)
{
	std::vector<PowerControlDynamicCaps> allCaps;
	auto controlTypes =
		getControlTypes(maxPowerRequests, minPowerRequests, powerStepRequests, maxTimeRequests, minTimeRequests);
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		auto currentPowerMax = Power::createInvalid();
		auto currentPowerMin = Power::createInvalid();
		auto currentPowerStep = Power::createInvalid();
		auto currentTimeMax = TimeSpan::createInvalid();
		auto currentTimeMin = TimeSpan::createInvalid();

		try
		{
			auto currentCapability = currentCapSet.getCapability(*controlType);
			currentPowerMax = currentCapability.getMaxPowerLimit();
			currentPowerMin = currentCapability.getMinPowerLimit();
			currentPowerStep = currentCapability.getPowerStepSize();
			currentTimeMax = currentCapability.getMaxTimeWindow();
			currentTimeMin = currentCapability.getMinTimeWindow();
		}
		catch (dptf_exception&)
		{
			// there is no current capability control
		}

		Power maxPowerLimit = getLowestMaxPowerLimit(*controlType, maxPowerRequests);
		if (!maxPowerLimit.isValid() && currentPowerMax.isValid())
		{
			maxPowerLimit = currentPowerMax;
		}

		Power minPowerLimit = getLowestMinPowerLimit(*controlType, minPowerRequests);
		if (!minPowerLimit.isValid() && currentPowerMin.isValid())
		{
			minPowerLimit = currentPowerMin;
		}

		if (maxPowerLimit.isValid() && minPowerLimit.isValid() && maxPowerLimit < minPowerLimit)
		{
			minPowerLimit = maxPowerLimit; // set both min and max to the most limited of the two values
		}

		Power stepSize = getHighestPowerLimitStep(*controlType, powerStepRequests);
		if (!stepSize.isValid() && currentPowerStep.isValid())
		{
			stepSize = currentPowerStep;
		}

		TimeSpan maxTimeWindow = getLowestMaxTimeWindow(*controlType, maxTimeRequests);
		if (!maxTimeWindow.isValid() && currentTimeMax.isValid())
		{
			maxTimeWindow = currentTimeMax;
		}

		TimeSpan minTimeWindow = getHighestMinTimeWindow(*controlType, minTimeRequests);
		if (!minTimeWindow.isValid() && currentTimeMin.isValid())
		{
			minTimeWindow = currentTimeMin;
		}

		if (maxTimeWindow.isValid() && minTimeWindow.isValid() && maxTimeWindow < minTimeWindow)
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

PowerControlDynamicCapsSet PowerControlCapabilitiesArbitrator::getArbitratedPowerControlCapabilities(
	const PowerControlDynamicCapsSet& currentCapSet)
{
	auto arbitratedCapabilities = createNewArbitratedCapabilitites(
		m_requestedMaxPowerLimit,
		m_requestedMinPowerLimit,
		m_requestedPowerLimitStep,
		m_requestedMaxTimeWindow,
		m_requestedMinTimeWindow,
		currentCapSet);
	return arbitratedCapabilities;
}

Bool PowerControlCapabilitiesArbitrator::getArbitratedLock() const
{
	for (auto lockRequest = m_requestedLocks.begin(); lockRequest != m_requestedLocks.end(); ++lockRequest)
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

std::shared_ptr<XmlNode> PowerControlCapabilitiesArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("power_control_capabilities_arbitrator_status");
	auto tempRequestedMaxPowerLimit = m_requestedMaxPowerLimit;
	auto tempRequestedMinPowerLimit = m_requestedMinPowerLimit;
	auto tempRequestedPowerLimitStep = m_requestedPowerLimitStep;
	auto tempRequestedMaxTimeWindow = m_requestedMaxTimeWindow;
	auto tempRequestedMinTimeWindow = m_requestedMinTimeWindow;
	auto controlTypes = getControlTypes(
		tempRequestedMaxPowerLimit,
		tempRequestedMinPowerLimit,
		tempRequestedPowerLimitStep,
		tempRequestedMaxTimeWindow,
		tempRequestedMinTimeWindow);

	auto ppcc = PowerControlDynamicCapsSet();
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		PowerControlDynamicCaps caps = PowerControlDynamicCaps(
			*controlType,
			Power::createInvalid(),
			Power::createInvalid(),
			Power::createInvalid(),
			TimeSpan::createInvalid(),
			TimeSpan::createInvalid(),
			Percentage::createInvalid(),
			Percentage::createInvalid());
		auto policyPowerRequest = m_requestedMaxPowerLimit.find(policyIndex);
		if (policyPowerRequest != m_requestedMaxPowerLimit.end())
		{
			auto policyMaxPower = policyPowerRequest->second.find(*controlType);
			if (policyMaxPower != policyPowerRequest->second.end())
			{
				caps.setMaxPowerLimit(policyMaxPower->second);
			}
		}

		policyPowerRequest = m_requestedMinPowerLimit.find(policyIndex);
		if (policyPowerRequest != m_requestedMinPowerLimit.end())
		{
			auto policyMinPower = policyPowerRequest->second.find(*controlType);
			if (policyMinPower != policyPowerRequest->second.end())
			{
				caps.setMinPowerLimit(policyMinPower->second);
			}
		}

		policyPowerRequest = m_requestedPowerLimitStep.find(policyIndex);
		if (policyPowerRequest != m_requestedPowerLimitStep.end())
		{
			auto policyStep = policyPowerRequest->second.find(*controlType);
			if (policyStep != policyPowerRequest->second.end())
			{
				caps.setPowerStepSize(policyStep->second);
			}
		}

		auto policyTimeRequest = m_requestedMaxTimeWindow.find(policyIndex);
		if (policyTimeRequest != m_requestedMaxTimeWindow.end())
		{
			auto policyMaxTime = policyTimeRequest->second.find(*controlType);
			if (policyMaxTime != policyTimeRequest->second.end())
			{
				caps.setMaxTimeWindow(policyMaxTime->second);
			}
		}

		policyTimeRequest = m_requestedMinTimeWindow.find(policyIndex);
		if (policyTimeRequest != m_requestedMinTimeWindow.end())
		{
			auto policyMinTime = policyTimeRequest->second.find(*controlType);
			if (policyMinTime != policyTimeRequest->second.end())
			{
				caps.setMinTimeWindow(policyMinTime->second);
			}
		}
		requestRoot->addChild(caps.getXml());
	}

	auto policyLockRequest = m_requestedLocks.find(policyIndex);
	Bool lockRequested = false;
	if (policyLockRequest != m_requestedLocks.end())
	{
		lockRequested = policyLockRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("requested_lock", StatusFormat::friendlyValue(lockRequested)));
	return requestRoot;
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
			else if (request->second.isValid())
			{
				lowestMaxPower = std::min(lowestMaxPower, request->second);
			}
		}
	}
	return lowestMaxPower;
}

Power PowerControlCapabilitiesArbitrator::getLowestMinPowerLimit(
	PowerControlType::Type controlType,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const
{
	Power lowestMinPower(Power::createInvalid());
	for (auto policy = requests.begin(); policy != requests.end(); ++policy)
	{
		auto request = policy->second.find(controlType);
		if (request != policy->second.end())
		{
			if (lowestMinPower.isValid() == false)
			{
				lowestMinPower = request->second;
			}
			else if (request->second.isValid())
			{
				lowestMinPower = std::min(lowestMinPower, request->second);
			}
		}
	}
	return lowestMinPower;
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
			else if (request->second.isValid())
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
			else if (request->second.isValid())
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
			else if (request->second.isValid())
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
