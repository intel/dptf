/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "SystemPowerControlArbitrator.h"
#include "Utility.h"

SystemPowerControlArbitrator::SystemPowerControlArbitrator()
	: m_requestedSystemPowerLimits()
	, m_arbitratedSystemPowerLimit()
	, m_requestedTimeWindows()
	, m_arbitratedTimeWindow()
	, m_requestedDutyCycles()
	, m_arbitratedDutyCycle()
{
}

SystemPowerControlArbitrator::~SystemPowerControlArbitrator(void)
{
}

void SystemPowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Power& powerLimit)
{
	updatePolicyRequest(policyIndex, controlType, powerLimit, m_requestedSystemPowerLimits);
	Power lowestRequest = getLowestRequest(controlType, m_requestedSystemPowerLimits);
	setArbitratedRequest(controlType, lowestRequest);
}

void SystemPowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const TimeSpan& timeWindow)
{
	updatePolicyRequest(policyIndex, controlType, timeWindow, m_requestedTimeWindows);
	TimeSpan lowestRequest = getLowestRequest(controlType, m_requestedTimeWindows);
	setArbitratedRequest(controlType, lowestRequest);
}

void SystemPowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Percentage& dutyCycle)
{
	updatePolicyRequest(policyIndex, controlType, dutyCycle, m_requestedDutyCycles);
	Percentage lowestRequest = getLowestRequest(controlType, m_requestedDutyCycles);
	setArbitratedRequest(controlType, lowestRequest);
}

Bool SystemPowerControlArbitrator::hasArbitratedSystemPowerLimit(PsysPowerLimitType::Type controlType) const
{
	auto controlRequests = m_arbitratedSystemPowerLimit.find(controlType);
	if (controlRequests != m_arbitratedSystemPowerLimit.end())
	{
		return true;
	}
	return false;
}

Bool SystemPowerControlArbitrator::hasArbitratedTimeWindow(PsysPowerLimitType::Type controlType) const
{
	auto controlRequests = m_arbitratedTimeWindow.find(controlType);
	if (controlRequests != m_arbitratedTimeWindow.end())
	{
		return true;
	}
	return false;
}

Bool SystemPowerControlArbitrator::hasArbitratedDutyCycle(PsysPowerLimitType::Type controlType) const
{
	auto controlRequests = m_arbitratedDutyCycle.find(controlType);
	if (controlRequests != m_arbitratedDutyCycle.end())
	{
		return true;
	}
	return false;
}

void SystemPowerControlArbitrator::setArbitratedRequest(
	PsysPowerLimitType::Type controlType,
	const Power& lowestRequest)
{
	auto controlRequests = m_arbitratedSystemPowerLimit.find(controlType);
	if (controlRequests == m_arbitratedSystemPowerLimit.end())
	{
		m_arbitratedSystemPowerLimit[controlType] = lowestRequest;
	}
	else
	{
		if (controlRequests->second != lowestRequest)
		{
			m_arbitratedSystemPowerLimit[controlType] = lowestRequest;
		}
	}
}

void SystemPowerControlArbitrator::setArbitratedRequest(
	PsysPowerLimitType::Type controlType,
	const TimeSpan& lowestRequest)
{
	auto controlRequests = m_arbitratedTimeWindow.find(controlType);
	if (controlRequests == m_arbitratedTimeWindow.end())
	{
		m_arbitratedTimeWindow[controlType] = lowestRequest;
	}
	else
	{
		if (controlRequests->second != lowestRequest)
		{
			m_arbitratedTimeWindow[controlType] = lowestRequest;
		}
	}
}

void SystemPowerControlArbitrator::setArbitratedRequest(
	PsysPowerLimitType::Type controlType,
	const Percentage& lowestRequest)
{
	auto controlRequests = m_arbitratedDutyCycle.find(controlType);
	if (controlRequests == m_arbitratedDutyCycle.end())
	{
		m_arbitratedDutyCycle[controlType] = lowestRequest;
	}
	else
	{
		if (controlRequests->second != lowestRequest)
		{
			m_arbitratedDutyCycle[controlType] = lowestRequest;
		}
	}
}

void SystemPowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Power& powerLimit,
	std::map<UIntN, std::map<PsysPowerLimitType::Type, Power>>& powerLimits)
{
	auto policyRequests = powerLimits.find(policyIndex);
	if (policyRequests == powerLimits.end())
	{
		powerLimits[policyIndex] = std::map<PsysPowerLimitType::Type, Power>();
	}
	powerLimits[policyIndex][controlType] = powerLimit;
}

void SystemPowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const TimeSpan& timeWindow,
	std::map<UIntN, std::map<PsysPowerLimitType::Type, TimeSpan>>& timeWindows)
{
	auto policyRequests = timeWindows.find(policyIndex);
	if (policyRequests == timeWindows.end())
	{
		timeWindows[policyIndex] = std::map<PsysPowerLimitType::Type, TimeSpan>();
	}
	timeWindows[policyIndex][controlType] = timeWindow;
}

void SystemPowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Percentage& dutyCycle,
	std::map<UIntN, std::map<PsysPowerLimitType::Type, Percentage>>& dutyCycles)
{
	auto policyRequests = dutyCycles.find(policyIndex);
	if (policyRequests == dutyCycles.end())
	{
		dutyCycles[policyIndex] = std::map<PsysPowerLimitType::Type, Percentage>();
	}
	dutyCycles[policyIndex][controlType] = dutyCycle;
}

Power SystemPowerControlArbitrator::getLowestRequest(
	PsysPowerLimitType::Type controlType,
	const std::map<UIntN, std::map<PsysPowerLimitType::Type, Power>>& powerLimits)
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
		throw dptf_exception(
			"There were no power limit requests to pick from when choosing the lowest for \
							 arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

TimeSpan SystemPowerControlArbitrator::getLowestRequest(
	PsysPowerLimitType::Type controlType,
	const std::map<UIntN, std::map<PsysPowerLimitType::Type, TimeSpan>>& timeWindows)
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
		throw dptf_exception(
			"There were no power time window requests to pick from when choosing the lowest for \
							  arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

Percentage SystemPowerControlArbitrator::getLowestRequest(
	PsysPowerLimitType::Type controlType,
	const std::map<UIntN, std::map<PsysPowerLimitType::Type, Percentage>>& dutyCycles)
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
		throw dptf_exception(
			"There were no power duty cycle requests to pick from when choosing the lowest for \
							  arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

Power SystemPowerControlArbitrator::getArbitratedSystemPowerLimit(PsysPowerLimitType::Type controlType) const
{
	auto controlPowerLimit = m_arbitratedSystemPowerLimit.find(controlType);
	if (controlPowerLimit == m_arbitratedSystemPowerLimit.end())
	{
		throw dptf_exception(
			"No power limit has been set for control type " + PsysPowerLimitType::ToString(controlType) + ".");
	}
	else
	{
		return controlPowerLimit->second;
	}
}

TimeSpan SystemPowerControlArbitrator::getArbitratedTimeWindow(PsysPowerLimitType::Type controlType) const
{
	auto controlTimeWindow = m_arbitratedTimeWindow.find(controlType);
	if (controlTimeWindow == m_arbitratedTimeWindow.end())
	{
		throw dptf_exception(
			"No power limit time window has been set for control type " + PsysPowerLimitType::ToString(controlType)
			+ ".");
	}
	else
	{
		return controlTimeWindow->second;
	}
}

Percentage SystemPowerControlArbitrator::getArbitratedDutyCycle(PsysPowerLimitType::Type controlType) const
{
	auto controlDutyCycle = m_arbitratedDutyCycle.find(controlType);
	if (controlDutyCycle == m_arbitratedDutyCycle.end())
	{
		throw dptf_exception(
			"No power limit duty cycle has been set for control type " + PsysPowerLimitType::ToString(controlType)
			+ ".");
	}
	else
	{
		return controlDutyCycle->second;
	}
}

Power SystemPowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Power& powerLimit)
{
	auto tempPolicyRequests = m_requestedSystemPowerLimits;
	updatePolicyRequest(policyIndex, controlType, powerLimit, tempPolicyRequests);
	Power lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

TimeSpan SystemPowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const TimeSpan& timeWindow)
{
	auto tempPolicyRequests = m_requestedTimeWindows;
	updatePolicyRequest(policyIndex, controlType, timeWindow, tempPolicyRequests);
	TimeSpan lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

Percentage SystemPowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PsysPowerLimitType::Type controlType,
	const Percentage& dutyCycle)
{
	auto tempPolicyRequests = m_requestedDutyCycles;
	updatePolicyRequest(policyIndex, controlType, dutyCycle, tempPolicyRequests);
	Percentage lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

void SystemPowerControlArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
	removeSystemPowerLimitRequest(policyIndex);
	removeTimeWindowRequest(policyIndex);
	removeDutyCycleRequest(policyIndex);
}

std::shared_ptr<XmlNode> SystemPowerControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("system_power_control_arbitrator_status");
	auto policyPowerLimitRequests = m_requestedSystemPowerLimits.find(policyIndex);
	if (policyPowerLimitRequests != m_requestedSystemPowerLimits.end())
	{
		auto powerLimits = policyPowerLimitRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; ++controlType)
		{
			auto powerLimit = Power::createInvalid();
			auto controlRequest = powerLimits.find((PsysPowerLimitType::Type)controlType);
			if (controlRequest != powerLimits.end())
			{
				powerLimit = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"power_limit_" + PsysPowerLimitType::ToXmlString((PsysPowerLimitType::Type)controlType),
				powerLimit.toString()));
		}
	}

	auto policyTimeWindowRequests = m_requestedTimeWindows.find(policyIndex);
	if (policyTimeWindowRequests != m_requestedTimeWindows.end())
	{
		auto timeWindows = policyTimeWindowRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; ++controlType)
		{
			auto timeWindow = TimeSpan::createInvalid();
			auto controlRequest = timeWindows.find((PsysPowerLimitType::Type)controlType);
			if (controlRequest != timeWindows.end())
			{
				timeWindow = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"time_window_" + PsysPowerLimitType::ToXmlString((PsysPowerLimitType::Type)controlType),
				timeWindow.toStringMilliseconds()));
		}
	}

	auto policyDutyCycleRequests = m_requestedDutyCycles.find(policyIndex);
	if (policyDutyCycleRequests != m_requestedDutyCycles.end())
	{
		auto dutyCycles = policyDutyCycleRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; ++controlType)
		{
			auto dutyCycle = Percentage::createInvalid();
			auto controlRequest = dutyCycles.find((PsysPowerLimitType::Type)controlType);
			if (controlRequest != dutyCycles.end())
			{
				dutyCycle = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"duty_cycle_" + PsysPowerLimitType::ToXmlString((PsysPowerLimitType::Type)controlType),
				dutyCycle.toString()));
		}
	}

	return requestRoot;
}

void SystemPowerControlArbitrator::removeSystemPowerLimitRequest(UIntN policyIndex)
{
	auto policyRequests = m_requestedSystemPowerLimits.find(policyIndex);
	if (policyRequests != m_requestedSystemPowerLimits.end())
	{
		auto controlTypes = findControlTypesSetForPolicy(policyRequests->second);
		m_requestedSystemPowerLimits.erase(policyRequests);
		if (m_requestedSystemPowerLimits.size() > 0)
		{
			setArbitratedSystemPowerLimitForControlTypes(controlTypes);
		}
		else
		{
			m_arbitratedSystemPowerLimit.clear();
		}
	}
}

void SystemPowerControlArbitrator::removeTimeWindowRequest(UIntN policyIndex)
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

void SystemPowerControlArbitrator::removeDutyCycleRequest(UIntN policyIndex)
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

void SystemPowerControlArbitrator::setArbitratedSystemPowerLimitForControlTypes(
	const std::vector<PsysPowerLimitType::Type>& controlTypes)
{
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
	{
		try
		{
			Power lowestRequest = getLowestRequest(*controlType, m_requestedSystemPowerLimits);
			setArbitratedRequest(*controlType, lowestRequest);
		}
		catch (...)
		{
		}
	}
}

void SystemPowerControlArbitrator::setArbitratedTimeWindowsForControlTypes(
	const std::vector<PsysPowerLimitType::Type>& controlTypes)
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

void SystemPowerControlArbitrator::setArbitratedDutyCyclesForControlTypes(
	const std::vector<PsysPowerLimitType::Type>& controlTypes)
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

std::vector<PsysPowerLimitType::Type> SystemPowerControlArbitrator::findControlTypesSetForPolicy(
	const std::map<PsysPowerLimitType::Type, Power>& controlRequests)
{
	std::vector<PsysPowerLimitType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; controlType++)
	{
		auto controlRequest = controlRequests.find((PsysPowerLimitType::Type)controlType);
		if (controlRequest != controlRequests.end())
		{
			controlTypes.push_back((PsysPowerLimitType::Type)controlType);
		}
	}
	return controlTypes;
}

std::vector<PsysPowerLimitType::Type> SystemPowerControlArbitrator::findControlTypesSetForPolicy(
	const std::map<PsysPowerLimitType::Type, TimeSpan>& controlRequests)
{
	std::vector<PsysPowerLimitType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; controlType++)
	{
		auto controlRequest = controlRequests.find((PsysPowerLimitType::Type)controlType);
		if (controlRequest != controlRequests.end())
		{
			controlTypes.push_back((PsysPowerLimitType::Type)controlType);
		}
	}
	return controlTypes;
}

std::vector<PsysPowerLimitType::Type> SystemPowerControlArbitrator::findControlTypesSetForPolicy(
	const std::map<PsysPowerLimitType::Type, Percentage>& controlRequests)
{
	std::vector<PsysPowerLimitType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PsysPowerLimitType::MAX; controlType++)
	{
		auto controlRequest = controlRequests.find((PsysPowerLimitType::Type)controlType);
		if (controlRequest != controlRequests.end())
		{
			controlTypes.push_back((PsysPowerLimitType::Type)controlType);
		}
	}
	return controlTypes;
}
