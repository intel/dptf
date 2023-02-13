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

#include "ArbitratorFanSpeed.h"

ArbitratorFanSpeed::ArbitratorFanSpeed()
	: m_requests(std::map<UIntN, Percentage>())
{
}

ArbitratorFanSpeed::~ArbitratorFanSpeed(void)
{
}

void ArbitratorFanSpeed::commitRequest(UIntN policyIndex, const Percentage& value)
{
	m_requests[policyIndex] = value;
}

void ArbitratorFanSpeed::removeRequest(UIntN policyIndex)
{
	auto request = m_requests.find(policyIndex);
	if (request != m_requests.end())
	{
		m_requests.erase(request);
	}
}

Percentage ArbitratorFanSpeed::getArbitratedValue() const
{
	return getHighestRequest(m_requests);
}

Percentage ArbitratorFanSpeed::calculateNewArbitratedValue(UIntN policyIndex, const Percentage& newValue) const
{
	std::map<UIntN, Percentage> requestsCopy = m_requests;
	requestsCopy[policyIndex] = newValue;
	return getHighestRequest(requestsCopy);
}

std::shared_ptr<XmlNode> ArbitratorFanSpeed::getStatusForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("active_control_arbitrator_status");
	auto fanSpeedRequest = m_requests.find(policyIndex);
	auto percentageRequest = Percentage::createInvalid();
	if (fanSpeedRequest != m_requests.end())
	{
		percentageRequest = fanSpeedRequest->second;
	}
	requestRoot->addChild(
		XmlNode::createDataElement("fan_speed_percentage", percentageRequest.toStringWithPrecision(0)));
	return requestRoot;
}

Percentage ArbitratorFanSpeed::getHighestRequest(const std::map<UIntN, Percentage>& requests)
{
	if (requests.size() == 0)
	{
		return Percentage::createInvalid();
	}
	else
	{
		Percentage highestRequest = Percentage::fromWholeNumber(0);
		for (auto policyRequest = requests.begin(); policyRequest != requests.end(); ++policyRequest)
		{
			highestRequest = std::max(policyRequest->second, highestRequest);
		}
		return highestRequest;
	}
}
