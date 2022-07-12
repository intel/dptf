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

#include "PowerControlArbitrator.h"
#include "Utility.h"
#include "StatusFormat.h"

PowerControlArbitrator::PowerControlArbitrator()
	: m_requestedPowerLimits()
	, m_arbitratedPowerLimit()
	, m_requestedTimeWindows()
	, m_arbitratedTimeWindow()
	, m_requestedDutyCycles()
	, m_arbitratedDutyCycle()
	, m_requestedSocPowerFloorStates()
{
}

PowerControlArbitrator::~PowerControlArbitrator(void)
{
}

void PowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	updatePolicyRequest(policyIndex, controlType, powerLimit, m_requestedPowerLimits);
	Power lowestRequest = getLowestRequest(controlType, m_requestedPowerLimits);
	setArbitratedRequest(controlType, lowestRequest);
}

void PowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	updatePolicyRequest(policyIndex, controlType, timeWindow, m_requestedTimeWindows);
	TimeSpan lowestRequest = getLowestRequest(controlType, m_requestedTimeWindows);
	setArbitratedRequest(controlType, lowestRequest);
}

void PowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	updatePolicyRequest(policyIndex, controlType, dutyCycle, m_requestedDutyCycles);
	Percentage lowestRequest = getLowestRequest(controlType, m_requestedDutyCycles);
	setArbitratedRequest(controlType, lowestRequest);
}

void PowerControlArbitrator::commitPolicyRequest(UIntN policyIndex, const Bool& socPowerFloorState)
{
	updatePolicyRequest(policyIndex, socPowerFloorState, m_requestedSocPowerFloorStates);
	Bool lowestRequest = getLowestRequest(m_requestedSocPowerFloorStates);
	setArbitratedRequest(lowestRequest);
}

Bool PowerControlArbitrator::hasArbitratedPowerLimit(PowerControlType::Type controlType) const
{
	auto controlRequests = m_arbitratedPowerLimit.find(controlType);
	if (controlRequests != m_arbitratedPowerLimit.end())
	{
		return true;
	}
	return false;
}

Bool PowerControlArbitrator::hasArbitratedTimeWindow(PowerControlType::Type controlType) const
{
	auto controlRequests = m_arbitratedTimeWindow.find(controlType);
	if (controlRequests != m_arbitratedTimeWindow.end())
	{
		return true;
	}
	return false;
}

Bool PowerControlArbitrator::hasArbitratedDutyCycle(PowerControlType::Type controlType) const
{
	auto controlRequests = m_arbitratedDutyCycle.find(controlType);
	if (controlRequests != m_arbitratedDutyCycle.end())
	{
		return true;
	}
	return false;
}

Bool PowerControlArbitrator::hasArbitratedSocPowerFloorState() const
{
	if (m_arbitratedSocPowerFloorState.isValid())
	{
		return true;
	}
	return false;
}

void PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const Power& lowestRequest)
{
	auto controlRequests = m_arbitratedPowerLimit.find(controlType);
	if (controlRequests == m_arbitratedPowerLimit.end())
	{
		m_arbitratedPowerLimit[controlType] = lowestRequest;
	}
	else
	{
		if (controlRequests->second != lowestRequest)
		{
			m_arbitratedPowerLimit[controlType] = lowestRequest;
		}
	}
}

void PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const TimeSpan& lowestRequest)
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

void PowerControlArbitrator::setArbitratedRequest(PowerControlType::Type controlType, const Percentage& lowestRequest)
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

void PowerControlArbitrator::setArbitratedRequest(const Bool& lowestRequest)
{
	m_arbitratedSocPowerFloorState.set(lowestRequest);
}

void PowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit,
	std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerLimits)
{
	auto policyRequests = powerLimits.find(policyIndex);
	if (policyRequests == powerLimits.end())
	{
		powerLimits[policyIndex] = std::map<PowerControlType::Type, Power>();
	}
	powerLimits[policyIndex][controlType] = powerLimit;
}

void PowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow,
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& timeWindows)
{
	auto policyRequests = timeWindows.find(policyIndex);
	if (policyRequests == timeWindows.end())
	{
		timeWindows[policyIndex] = std::map<PowerControlType::Type, TimeSpan>();
	}
	timeWindows[policyIndex][controlType] = timeWindow;
}

void PowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle,
	std::map<UIntN, std::map<PowerControlType::Type, Percentage>>& dutyCycles)
{
	auto policyRequests = dutyCycles.find(policyIndex);
	if (policyRequests == dutyCycles.end())
	{
		dutyCycles[policyIndex] = std::map<PowerControlType::Type, Percentage>();
	}
	dutyCycles[policyIndex][controlType] = dutyCycle;
}

void PowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	const Bool& socPowerFloorState,
	std::map<UIntN, Bool>& socPowerFloorStates)
{
	auto policyRequests = socPowerFloorStates.find(policyIndex);
	if (policyRequests == socPowerFloorStates.end())
	{
		socPowerFloorStates[policyIndex] = Bool();
	}
	socPowerFloorStates[policyIndex] = socPowerFloorState;
}

Power PowerControlArbitrator::getLowestRequest(
	PowerControlType::Type controlType,
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
		throw dptf_exception(
			"There were no power limit requests to pick from when choosing the lowest for \
							 arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

TimeSpan PowerControlArbitrator::getLowestRequest(
	PowerControlType::Type controlType,
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
		throw dptf_exception(
			"There were no power time window requests to pick from when choosing the lowest for \
							  arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

Percentage PowerControlArbitrator::getLowestRequest(
	PowerControlType::Type controlType,
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
		throw dptf_exception(
			"There were no power duty cycle requests to pick from when choosing the lowest for \
							  arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

Bool PowerControlArbitrator::getLowestRequest(const std::map<UIntN, Bool>& socPowerFloorStates)
{
	Bool lowestRequest = false;
	Bool lowestRequestSet(false);
	for (auto policy = socPowerFloorStates.begin(); policy != socPowerFloorStates.end(); policy++)
	{
		auto& socPowerFloorState = policy->second;
		if (lowestRequestSet == false)
		{
			lowestRequestSet = true;
			lowestRequest = socPowerFloorState;
		}
		else
		{
			if (lowestRequest == false && socPowerFloorState == true)
			{
				lowestRequest = socPowerFloorState;
			}
		}
	}

	if (lowestRequestSet == false)
	{
		throw dptf_exception(
			"There were no soc power floor states requests to pick from when choosing the lowest for \
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
		throw dptf_exception(
			"No power limit has been set for control type " + PowerControlType::ToString(controlType) + ".");
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
		throw dptf_exception(
			"No power limit time window has been set for control type " + PowerControlType::ToString(controlType)
			+ ".");
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
		throw dptf_exception(
			"No power limit duty cycle has been set for control type " + PowerControlType::ToString(controlType) + ".");
	}
	else
	{
		return controlDutyCycle->second;
	}
}

Bool PowerControlArbitrator::getArbitratedSocPowerFloorState() const
{
	if (m_arbitratedSocPowerFloorState.isInvalid())
	{
		throw dptf_exception("No soc power floor state has been set.");
	}
	else
	{
		return m_arbitratedSocPowerFloorState.get();
	}
}

Power PowerControlArbitrator::arbitrate(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit)
{
	auto tempPolicyRequests = m_requestedPowerLimits;
	updatePolicyRequest(policyIndex, controlType, powerLimit, tempPolicyRequests);
	Power lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

TimeSpan PowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	auto tempPolicyRequests = m_requestedTimeWindows;
	updatePolicyRequest(policyIndex, controlType, timeWindow, tempPolicyRequests);
	TimeSpan lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

Percentage PowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	auto tempPolicyRequests = m_requestedDutyCycles;
	updatePolicyRequest(policyIndex, controlType, dutyCycle, tempPolicyRequests);
	Percentage lowestRequest = getLowestRequest(controlType, tempPolicyRequests);
	return lowestRequest;
}

Bool PowerControlArbitrator::arbitrate(UIntN policyIndex, const Bool& socPowerFloorState)
{
	auto tempPolicyRequests = m_requestedSocPowerFloorStates;
	updatePolicyRequest(policyIndex, socPowerFloorState, tempPolicyRequests);
	Bool lowestRequest = getLowestRequest(tempPolicyRequests);
	return lowestRequest;
}

void PowerControlArbitrator::removeRequestsForPolicy(UIntN policyIndex)
{
	removePowerLimitRequest(policyIndex);
	removeTimeWindowRequest(policyIndex);
	removeDutyCycleRequest(policyIndex);
	removeSocPowerFloorStateRequest(policyIndex);
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

void PowerControlArbitrator::removeSocPowerFloorStateRequest(UIntN policyIndex)
{
	auto policyRequests = m_requestedSocPowerFloorStates.find(policyIndex);
	if (policyRequests != m_requestedSocPowerFloorStates.end())
	{
		m_requestedSocPowerFloorStates.erase(policyRequests);
		if (m_requestedSocPowerFloorStates.size() > 0)
		{
			Bool lowestRequest = getLowestRequest(m_requestedSocPowerFloorStates);
			setArbitratedRequest(lowestRequest);
		}
		else
		{
			m_arbitratedSocPowerFloorState.invalidate();
		}
	}
}

std::shared_ptr<XmlNode> PowerControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("power_control_arbitrator_status");
	auto policyPowerLimitRequests = m_requestedPowerLimits.find(policyIndex);
	if (policyPowerLimitRequests != m_requestedPowerLimits.end())
	{
		auto powerLimits = policyPowerLimitRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
		{
			auto powerLimit = Power::createInvalid();
			auto controlRequest = powerLimits.find((PowerControlType::Type)controlType);
			if (controlRequest != powerLimits.end())
			{
				powerLimit = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"power_limit_" + PowerControlType::ToString((PowerControlType::Type)controlType),
				powerLimit.toString()));
		}
	}

	auto policyTimeWindowRequests = m_requestedTimeWindows.find(policyIndex);
	if (policyTimeWindowRequests != m_requestedTimeWindows.end())
	{
		auto timeWindows = policyTimeWindowRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
		{
			auto timeWindow = TimeSpan::createInvalid();
			auto controlRequest = timeWindows.find((PowerControlType::Type)controlType);
			if (controlRequest != timeWindows.end())
			{
				timeWindow = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"time_window_" + PowerControlType::ToString((PowerControlType::Type)controlType),
				timeWindow.toStringMilliseconds()));
		}
	}

	auto policyDutyCycleRequests = m_requestedDutyCycles.find(policyIndex);
	if (policyDutyCycleRequests != m_requestedDutyCycles.end())
	{
		auto dutyCycles = policyDutyCycleRequests->second;
		for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
		{
			auto dutyCycle = Percentage::createInvalid();
			auto controlRequest = dutyCycles.find((PowerControlType::Type)controlType);
			if (controlRequest != dutyCycles.end())
			{
				dutyCycle = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"duty_cycle_" + PowerControlType::ToString((PowerControlType::Type)controlType), dutyCycle.toString()));
		}
	}

	auto policySocPowerFloorStateRequests = m_requestedSocPowerFloorStates.find(policyIndex);
	if (policySocPowerFloorStateRequests != m_requestedSocPowerFloorStates.end())
	{
		auto socPowerFloorState = policySocPowerFloorStateRequests->second;
		requestRoot->addChild(
			XmlNode::createDataElement("soc_power_floor_state", StatusFormat::friendlyValue(socPowerFloorState)));
	}

	return requestRoot;
}

void PowerControlArbitrator::setArbitratedPowerLimitForControlTypes(
	const std::vector<PowerControlType::Type>& controlTypes)
{
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
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
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
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
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
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

void PowerControlArbitrator::removePowerLimitRequestForPolicy(UIntN policyIndex, PowerControlType::Type controlType)
{
	auto policyPowerLimitRequests = m_requestedPowerLimits.find(policyIndex);
	if (policyPowerLimitRequests != m_requestedPowerLimits.end())
	{
		auto request = policyPowerLimitRequests->second.find(controlType);
		if (request != policyPowerLimitRequests->second.end())
		{
			policyPowerLimitRequests->second.erase(request);
		}
	}
}

std::vector<PowerControlType::Type> PowerControlArbitrator::findControlTypesSetForPolicy(
	const std::map<PowerControlType::Type, Power>& controlRequests) const
{
	std::vector<PowerControlType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
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
	const std::map<PowerControlType::Type, TimeSpan>& controlRequests) const
{
	std::vector<PowerControlType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
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
	const std::map<PowerControlType::Type, Percentage>& controlRequests) const
{
	std::vector<PowerControlType::Type> controlTypes;
	for (UIntN controlType = 0; controlType < (UIntN)PowerControlType::max; ++controlType)
	{
		auto controlRequest = controlRequests.find((PowerControlType::Type)controlType);
		if (controlRequest != controlRequests.end())
		{
			controlTypes.push_back((PowerControlType::Type)controlType);
		}
	}
	return controlTypes;
}
