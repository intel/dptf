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

#include "ArbitratorOscRequest.h"
#include "StatusFormat.h"
using namespace StatusFormat;

ArbitratorOscRequest::ArbitratorOscRequest()
	: m_requests()
	, m_arbitratedValueChangedSinceLastSet(false)
	, m_arbitratedValue(POLICY_DISABLED)
{
}

ArbitratorOscRequest::~ArbitratorOscRequest(void)
{
}

void ArbitratorOscRequest::updateRequest(std::string policyName, const UInt32 value)
{
	m_requests[policyName] = value;
	auto oldArbitratedValue = m_arbitratedValue;
	m_arbitratedValue = POLICY_DISABLED;
	for (auto policyRequest = m_requests.begin(); policyRequest != m_requests.end(); ++policyRequest)
	{
		m_arbitratedValue = m_arbitratedValue | policyRequest->second;
	}

	m_arbitratedValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedValue);
}

Bool ArbitratorOscRequest::arbitratedOscValueChanged() const
{
	return m_arbitratedValueChangedSinceLastSet;
}

UInt32 ArbitratorOscRequest::getArbitratedOscValue() const
{
	return m_arbitratedValue;
}

std::shared_ptr<XmlNode> ArbitratorOscRequest::getOscArbitratorStatus() const
{
	auto root = XmlNode::createWrapperElement("osc_arbitrator_status");
	auto requests = XmlNode::createWrapperElement("requests");
	for (auto policyRequest = m_requests.begin(); policyRequest != m_requests.end(); ++policyRequest)
	{
		auto request = XmlNode::createWrapperElement("request");
		request->addChild(XmlNode::createDataElement("policy_name", policyRequest->first));
		request->addChild(
			XmlNode::createDataElement("requested_value", std::bitset<4>(policyRequest->second).to_string()));
		request->addChild(XmlNode::createDataElement("description", toString(policyRequest->second)));
		requests->addChild(request);
	}
	root->addChild(requests);

	auto arbitratedValue = XmlNode::createWrapperElement("arbitrated");
	arbitratedValue->addChild(
		XmlNode::createDataElement("arbitrated_value", std::bitset<4>(m_arbitratedValue).to_string()));
	if (m_arbitratedValue != POLICY_DISABLED)
	{
		arbitratedValue->addChild(
			XmlNode::createDataElement("description", "Application " + toString(m_arbitratedValue)));
	}
	else
	{
		arbitratedValue->addChild(
			XmlNode::createDataElement("description", "Application Enabled without any enabled Policies"));
	}
	root->addChild(arbitratedValue);

	return root;
}

std::string ArbitratorOscRequest::toString(UInt32 value) const
{
	switch (value)
	{
	case POLICY_DISABLED:
		return "Disabled";
	case POLICY_ENABLED:
		return "Enabled";
	case POLICY_ENABLED | ACTIVE_CONTROL_SUPPORTED:
		return "Enabled with Active Control";
	case POLICY_ENABLED | PASSIVE_CONTROL_SUPPORTED:
		return "Enabled with Passive Control";
	case POLICY_ENABLED | CRITICAL_SHUTDOWN_SUPPORTED:
		return "Enabled with Critical Control";
	case POLICY_ENABLED | ACTIVE_CONTROL_SUPPORTED | PASSIVE_CONTROL_SUPPORTED:
		return "Enabled with Active and Passive Controls";
	case POLICY_ENABLED | ACTIVE_CONTROL_SUPPORTED | CRITICAL_SHUTDOWN_SUPPORTED:
		return "Enabled with Active and Critical Controls";
	case POLICY_ENABLED | PASSIVE_CONTROL_SUPPORTED | CRITICAL_SHUTDOWN_SUPPORTED:
		return "Enabled with Passive and Critical Controls";
	case POLICY_ENABLED | ACTIVE_CONTROL_SUPPORTED | PASSIVE_CONTROL_SUPPORTED | CRITICAL_SHUTDOWN_SUPPORTED:
		return "Enabled with Active, Passive and Critical Controls";
	default:
		return Constants::InvalidString;
	}
}
