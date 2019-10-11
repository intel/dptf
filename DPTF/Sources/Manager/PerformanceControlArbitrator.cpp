/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "PerformanceControlArbitrator.h"
#include "Utility.h"
#include <StatusFormat.h>

PerformanceControlArbitrator::PerformanceControlArbitrator()
	: m_arbitratedPerformanceControlIndex(Constants::Invalid)
{
}

PerformanceControlArbitrator::~PerformanceControlArbitrator(void)
{
}

void PerformanceControlArbitrator::commitPolicyRequest(UIntN policyIndex, UIntN performanceControlIndex)
{
	m_requestedPerformanceControlIndex[policyIndex] = performanceControlIndex;
	UIntN maxRequestedPerformanceControlIndex =
		getMaxRequestedPerformanceControlIndex(m_requestedPerformanceControlIndex);
	m_arbitratedPerformanceControlIndex = maxRequestedPerformanceControlIndex;
}

Bool PerformanceControlArbitrator::hasArbitratedPerformanceControlIndex(void) const
{
	if (m_arbitratedPerformanceControlIndex != Constants::Invalid)
	{
		return true;
	}
	return false;
}

UIntN PerformanceControlArbitrator::arbitrate(UIntN policyIndex, UIntN performanceControlIndex)
{
	auto tempPolicyRequests = m_requestedPerformanceControlIndex;
	tempPolicyRequests[policyIndex] = performanceControlIndex;
	UIntN maxRequestedPerformanceControlIndex = getMaxRequestedPerformanceControlIndex(tempPolicyRequests);
	return maxRequestedPerformanceControlIndex;
}

UIntN PerformanceControlArbitrator::getArbitratedPerformanceControlIndex(void) const
{
	return m_arbitratedPerformanceControlIndex;
}

void PerformanceControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto policyRequest = m_requestedPerformanceControlIndex.find(policyIndex);
	if (policyRequest != m_requestedPerformanceControlIndex.end())
	{
		m_requestedPerformanceControlIndex[policyIndex] = Constants::Invalid;
		commitPolicyRequest(policyIndex, Constants::Invalid);
	}
}

std::shared_ptr<XmlNode> PerformanceControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("performance_control_arbitrator_status");
	auto performanceIndex = Constants::Invalid;
	auto policyRequest = m_requestedPerformanceControlIndex.find(policyIndex);
	if (policyRequest != m_requestedPerformanceControlIndex.end())
	{
		performanceIndex = policyRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("performance_index", StatusFormat::friendlyValue(performanceIndex)));
	return requestRoot;
}

UIntN PerformanceControlArbitrator::getMaxRequestedPerformanceControlIndex(std::map<UIntN, UIntN>& requests)
{
	UIntN maxRequestedPerformanceControlIndex = Constants::Invalid;
	for (auto request = requests.begin(); request != requests.end(); ++request)
	{
		if ((request->second != Constants::Invalid) && ((maxRequestedPerformanceControlIndex == Constants::Invalid)
														|| (request->second > maxRequestedPerformanceControlIndex)))
		{
			maxRequestedPerformanceControlIndex = request->second;
		}
	}
	return maxRequestedPerformanceControlIndex;
}