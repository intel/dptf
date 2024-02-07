/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "ArbitratorActiveControlCapabilities.h"
#include "StatusFormat.h"

const Percentage DefaultMinimumSpeed = Percentage(0.0);
const Percentage DefaultMaximumSpeed = Percentage(1.0);

ArbitratorActiveControlCapabilities::ArbitratorActiveControlCapabilities()
	: m_minRequests(std::map<UIntN, Percentage>())
	, m_maxRequests(std::map<UIntN, Percentage>())
	, m_lockRequests(std::map<UIntN, Bool>())
{
}

ArbitratorActiveControlCapabilities::~ArbitratorActiveControlCapabilities(void)
{
}

void ArbitratorActiveControlCapabilities::removeCapabilitiesRequest(UIntN policyIndex)
{
	auto minRequest = m_minRequests.find(policyIndex);
	if (minRequest != m_minRequests.end())
	{
		m_minRequests.erase(minRequest);
	}

	auto maxRequest = m_maxRequests.find(policyIndex);
	if (maxRequest != m_maxRequests.end())
	{
		m_maxRequests.erase(maxRequest);
	}
}

ActiveControlDynamicCaps ArbitratorActiveControlCapabilities::getArbitratedCapabilities() const
{
	return calculateArbitratedCapabilities(m_minRequests, m_maxRequests);
}

ActiveControlDynamicCaps ArbitratorActiveControlCapabilities::calculateNewArbitratedCapabilities(
	UIntN policyIndex,
	const Percentage& minFanSpeed,
	const Percentage& maxFanSpeed) const
{
	auto minRequestsCopy = m_minRequests;
	if (minFanSpeed.isValid())
	{
		minRequestsCopy[policyIndex] = minFanSpeed;
	}
	auto maxRequestsCopy = m_maxRequests;
	if (maxFanSpeed.isValid())
	{
		maxRequestsCopy[policyIndex] = maxFanSpeed;
	}
	return calculateArbitratedCapabilities(minRequestsCopy, maxRequestsCopy);
}

std::shared_ptr<XmlNode> ArbitratorActiveControlCapabilities::getStatusForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("active_control_capabilities_arbitrator_status");

	auto minSpeed = Percentage::createInvalid();
	auto minRequest = m_minRequests.find(policyIndex);
	if (minRequest != m_minRequests.end())
	{
		minSpeed = minRequest->second;
	}

	auto maxSpeed = Percentage::createInvalid();
	auto maxRequest = m_maxRequests.find(policyIndex);
	if (maxRequest != m_maxRequests.end())
	{
		maxSpeed = maxRequest->second;
	}

	auto capsRequestRoot = XmlNode::createWrapperElement("active_control_caps_request");
	capsRequestRoot->addChild(XmlNode::createDataElement("min_fan_speed", minSpeed.toStringWithPrecision(0)));
	capsRequestRoot->addChild(XmlNode::createDataElement("max_fan_speed", maxSpeed.toStringWithPrecision(0)));
	requestRoot->addChild(capsRequestRoot);

	auto policyLockRequest = m_lockRequests.find(policyIndex);
	Bool lockRequested = false;
	if (policyLockRequest != m_lockRequests.end())
	{
		lockRequested = policyLockRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("requested_lock", StatusFormat::friendlyValue(lockRequested)));

	return requestRoot;
}

void ArbitratorActiveControlCapabilities::commitDynamicCapsRequest(
	UIntN policyIndex,
	const ActiveControlDynamicCaps& caps)
{
	if (caps.getMinFanSpeed().isValid())
	{
		m_minRequests[policyIndex] = caps.getMinFanSpeed();
	}
	if (caps.getMaxFanSpeed().isValid())
	{
		m_maxRequests[policyIndex] = caps.getMaxFanSpeed();
	}
}

ActiveControlDynamicCaps ArbitratorActiveControlCapabilities::calculateArbitratedCapabilities(
	const std::map<UIntN, Percentage>& minRequests,
	const std::map<UIntN, Percentage>& maxRequests)
{
	auto minSpeed = getHighestMinSpeed(minRequests);
	auto maxSpeed = getLowestMaxSpeed(maxRequests);
	if (maxSpeed.isValid() && minSpeed.isValid() && (maxSpeed < minSpeed))
	{
		maxSpeed = minSpeed;
	}
	return ActiveControlDynamicCaps(minSpeed, maxSpeed);
}

Percentage ArbitratorActiveControlCapabilities::getHighestMinSpeed(const std::map<UIntN, Percentage>& requests)
{
	if (requests.size() > 0)
	{
		Percentage minSpeed = Percentage::createInvalid();
		for (auto request = requests.begin(); request != requests.end(); ++request)
		{
			auto speed = request->second;
			if (speed.isValid() == false)
			{
				speed = DefaultMinimumSpeed;
			}

			if (minSpeed.isValid())
			{
				minSpeed = std::max(speed, minSpeed);
			}
			else
			{
				minSpeed = speed;
			}
		}
		return minSpeed;
	}
	else
	{
		return DefaultMinimumSpeed;
	}
}

Percentage ArbitratorActiveControlCapabilities::getLowestMaxSpeed(const std::map<UIntN, Percentage>& requests)
{
	if (requests.size() > 0)
	{
		Percentage maxSpeed = Percentage::createInvalid();
		for (auto request = requests.begin(); request != requests.end(); ++request)
		{
			auto speed = request->second;
			if (speed.isValid() == false)
			{
				speed = DefaultMaximumSpeed;
			}

			if (maxSpeed.isValid())
			{
				maxSpeed = std::min(speed, maxSpeed);
			}
			else
			{
				maxSpeed = speed;
			}
		}
		return maxSpeed;
	}
	else
	{
		return DefaultMaximumSpeed;
	}
}

void ArbitratorActiveControlCapabilities::commitLockRequest(UIntN policyIndex, const Bool& value)
{
	m_lockRequests[policyIndex] = value;
}

void ArbitratorActiveControlCapabilities::removeLockRequest(UIntN policyIndex)
{
	auto request = m_lockRequests.find(policyIndex);
	if (request != m_lockRequests.end())
	{
		m_lockRequests.erase(request);
	}
}

Bool ArbitratorActiveControlCapabilities::getArbitratedLock() const
{
	return calculateArbitratedLock(m_lockRequests);
}

Bool ArbitratorActiveControlCapabilities::calculateNewArbitratedLock(UIntN policyIndex, const Bool& newValue) const
{
	auto requestsCopy = m_lockRequests;
	requestsCopy[policyIndex] = newValue;
	return calculateArbitratedLock(requestsCopy);
}

Bool ArbitratorActiveControlCapabilities::calculateArbitratedLock(const std::map<UIntN, Bool>& requests)
{
	for (auto request = requests.begin(); request != requests.end(); ++request)
	{
		if (request->second == true)
		{
			return true;
		}
	}
	return false;
}
