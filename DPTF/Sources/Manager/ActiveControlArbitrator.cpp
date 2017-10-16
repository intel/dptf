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

#include "ActiveControlArbitrator.h"
#include "Utility.h"
#include <StatusFormat.h>

ActiveControlArbitrator::ActiveControlArbitrator()
	: m_arbitratedFanSpeedPercentage(Percentage::createInvalid())
{
}

ActiveControlArbitrator::~ActiveControlArbitrator(void)
{
}

void ActiveControlArbitrator::commitPolicyRequest(UIntN policyIndex, const Percentage& fanSpeed)
{
	m_requestedfanSpeedPercentage[policyIndex] = fanSpeed;
	Percentage maxRequestedFanSpeedPercentage = getMaxRequestedFanSpeedPercentage(m_requestedfanSpeedPercentage);
	m_arbitratedFanSpeedPercentage = maxRequestedFanSpeedPercentage;
}

Bool ActiveControlArbitrator::hasArbitratedFanSpeedPercentage(void) const
{
	if (m_arbitratedFanSpeedPercentage != Percentage::createInvalid())
	{
		return true;
	}
	return false;
}

Percentage ActiveControlArbitrator::getArbitratedFanSpeedPercentage(void) const
{
	return m_arbitratedFanSpeedPercentage;
}

Percentage ActiveControlArbitrator::arbitrate(UIntN policyIndex, const Percentage& fanSpeed)
{
	auto tempPolicyRequests = m_requestedfanSpeedPercentage;
	tempPolicyRequests[policyIndex] = fanSpeed;
	Percentage maxRequestedFanSpeedPercentage = getMaxRequestedFanSpeedPercentage(tempPolicyRequests);
	return maxRequestedFanSpeedPercentage;
}

void ActiveControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto fanSpeedRequest = m_requestedfanSpeedPercentage.find(policyIndex);
	if (fanSpeedRequest != m_requestedfanSpeedPercentage.end())
	{
		m_requestedfanSpeedPercentage[policyIndex] = Percentage::createInvalid();
		commitPolicyRequest(policyIndex, Percentage::createInvalid());
	}
}

std::shared_ptr<XmlNode> ActiveControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("active_control_arbitrator_status");
	auto fanSpeedRequest = m_requestedfanSpeedPercentage.find(policyIndex);
	auto percentageRequest = Percentage::createInvalid();
	if (fanSpeedRequest != m_requestedfanSpeedPercentage.end())
	{
		percentageRequest = fanSpeedRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("fan_speed_percentage", percentageRequest.toString()));
	return requestRoot;
}

Percentage ActiveControlArbitrator::getMaxRequestedFanSpeedPercentage(std::map<UIntN, Percentage>& requests)
{
	Percentage maxRequestedFanSpeedPercentage = Percentage::createInvalid();

	for (auto policyRequest = requests.begin(); policyRequest != requests.end(); ++policyRequest)
	{
		if ((policyRequest->second.isValid()) && ((maxRequestedFanSpeedPercentage.isValid() == false)
												  || (policyRequest->second > maxRequestedFanSpeedPercentage)))
		{
			maxRequestedFanSpeedPercentage = policyRequest->second;
		}
	}

	return maxRequestedFanSpeedPercentage;
}