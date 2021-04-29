/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "ArbitratorPlatformPowerControl.h"

ArbitratorPlatformPowerControl::ArbitratorPlatformPowerControl()
	: m_arbitratedPowerLimits(std::map<UInt32, Power>())
	, m_requestedPowerLimits(std::map<UIntN, std::map<UInt32, Power>>())
	, m_arbitratedValueChangedSinceLastSet(false)
{
}

ArbitratorPlatformPowerControl::~ArbitratorPlatformPowerControl(void)
{
}

Power ArbitratorPlatformPowerControl::arbitrate(UIntN policyIndex, const UInt32 portNumber, const Power& powerLimit)
{
	auto tempPolicyRequests = m_requestedPowerLimits;
	auto policyRequests = tempPolicyRequests.find(policyIndex);
	if (policyRequests == tempPolicyRequests.end())
	{
		tempPolicyRequests[policyIndex] = std::map<UInt32, Power>();
	}
	tempPolicyRequests[policyIndex][portNumber] = powerLimit;
	auto arbitratedValue = getLowestRequest(portNumber, tempPolicyRequests);
	return arbitratedValue;
}

void ArbitratorPlatformPowerControl::commitPolicyRequest(UIntN policyIndex, const UInt32 portNumber, const Power& powerLimit)
{
	auto policyRequests = m_requestedPowerLimits.find(policyIndex);
	if (policyRequests == m_requestedPowerLimits.end())
	{
		m_requestedPowerLimits[policyIndex] = std::map<UInt32, Power>();
	}
	m_requestedPowerLimits[policyIndex][portNumber] = powerLimit;

	auto oldArbitratedValue = m_arbitratedPowerLimits[portNumber];
	m_arbitratedPowerLimits[portNumber] = getLowestRequest(portNumber, m_requestedPowerLimits);
	if (oldArbitratedValue.isValid())
	{
		m_arbitratedValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedPowerLimits[portNumber]);
	}
	else
	{
		m_arbitratedValueChangedSinceLastSet = true;
	}
}

std::vector<UInt32> ArbitratorPlatformPowerControl::removeRequestAndGetAffectedPortNumbers(UIntN policyIndex)
{
	std::vector<UInt32> affectedPortNumbers;
	m_arbitratedValueChangedSinceLastSet = false;
	auto policyRequests = m_requestedPowerLimits.find(policyIndex);
	if (policyRequests != m_requestedPowerLimits.end())
	{
		affectedPortNumbers = findPortNumbersSetForPolicy(policyRequests->second);
		for (auto portNumber = affectedPortNumbers.begin(); portNumber != affectedPortNumbers.end(); ++portNumber)
		{
			auto currentLimit = getArbitratedValue(*portNumber);
			m_requestedPowerLimits[policyIndex][*portNumber] = Power::createInvalid();
			m_arbitratedPowerLimits[*portNumber] = getLowestRequest(*portNumber, m_requestedPowerLimits);
			if (currentLimit != getArbitratedValue(*portNumber))
			{
				m_arbitratedValueChangedSinceLastSet = true;
			}
		}
	}

	return affectedPortNumbers;
}

Bool ArbitratorPlatformPowerControl::arbitratedValueChanged() const
{
	return m_arbitratedValueChangedSinceLastSet;
}

Power ArbitratorPlatformPowerControl::getArbitratedValue(UInt32 portNumber) const
{
	auto powerLimitForPort = m_arbitratedPowerLimits.find(portNumber);
	if (powerLimitForPort == m_arbitratedPowerLimits.end())
	{
		return Power::createInvalid();
	}

	return powerLimitForPort->second;
}

std::shared_ptr<XmlNode> ArbitratorPlatformPowerControl::getStatusForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("platform_power_control_arbitrator_status");
	auto policyPowerLimitRequests = m_requestedPowerLimits.find(policyIndex);
	if (policyPowerLimitRequests != m_requestedPowerLimits.end())
	{
		auto powerLimits = policyPowerLimitRequests->second;
		for (UInt32 portNumber = 1; portNumber <= MAX_PORT_NUMBER; ++portNumber)
		{
			auto powerLimit = Power::createInvalid();
			auto controlRequest = powerLimits.find(portNumber);
			if (controlRequest != powerLimits.end())
			{
				powerLimit = controlRequest->second;
			}
			requestRoot->addChild(XmlNode::createDataElement(
				"power_limit_for_port_" + std::to_string(portNumber), powerLimit.toString()));
		}
	}
	return requestRoot;
}

Power ArbitratorPlatformPowerControl::getLowestRequest(UInt32 portNumber, std::map<UIntN, std::map<UInt32, Power>> requestedPowerLimits)
{
	Power lowestRequest;
	Bool lowestRequestSet(false);
	for (auto policy = requestedPowerLimits.begin(); policy != requestedPowerLimits.end(); ++policy)
	{
		auto& portNumbers = policy->second;
		auto port = portNumbers.find(portNumber);
		if (port != portNumbers.end())
		{
			if (lowestRequestSet == false)
			{
				lowestRequestSet = true;
				lowestRequest = port->second;
			}
			else if ((lowestRequest.isValid() && port->second.isValid() && port->second < lowestRequest) ||
				(!lowestRequest.isValid() && port->second.isValid()))
			{
				lowestRequest = port->second;
			}
		}
	}

	return lowestRequest;
}

std::vector<UInt32> ArbitratorPlatformPowerControl::findPortNumbersSetForPolicy(
	const std::map<UInt32, Power>& portNumberRequests) const
{
	std::vector<UInt32> portNumbers;
	for (UInt32 portNumber = 1; portNumber <= MAX_PORT_NUMBER; ++portNumber)
	{
		auto controlRequest = portNumberRequests.find(portNumber);
		if (controlRequest != portNumberRequests.end())
		{
			portNumbers.push_back(portNumber);
		}
	}
	return portNumbers;
}
