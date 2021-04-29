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

#include "CoreControlArbitrator.h"
#include "Utility.h"
#include <StatusFormat.h>

CoreControlArbitrator::CoreControlArbitrator()
	: m_arbitratedActiveCoreCount(Constants::Invalid)
	, m_requestedActiveCoreCount(std::map<UIntN, UIntN>())
{
}

CoreControlArbitrator::~CoreControlArbitrator(void)
{
}

void CoreControlArbitrator::commitPolicyRequest(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
	// save the CoreControlStatus at the correct location for this policy
	m_requestedActiveCoreCount[policyIndex] = coreControlStatus.getNumActiveLogicalProcessors();
	UIntN minActiveCoreCount = getMinActiveCoreCount(m_requestedActiveCoreCount);
	m_arbitratedActiveCoreCount = minActiveCoreCount;
}

Bool CoreControlArbitrator::hasArbitratedCoreCount(void) const
{
	if (m_arbitratedActiveCoreCount != Constants::Invalid)
	{
		return true;
	}
	return false;
}

CoreControlStatus CoreControlArbitrator::arbitrate(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
	auto tempPolicyRequests = m_requestedActiveCoreCount;
	tempPolicyRequests[policyIndex] = coreControlStatus.getNumActiveLogicalProcessors();
	UIntN minActiveCoreCount = getMinActiveCoreCount(tempPolicyRequests);
	return CoreControlStatus(minActiveCoreCount);
}

CoreControlStatus CoreControlArbitrator::getArbitratedCoreControlStatus(void) const
{
	return CoreControlStatus(m_arbitratedActiveCoreCount);
}

void CoreControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto policyRequest = m_requestedActiveCoreCount.find(policyIndex);
	if (policyRequest != m_requestedActiveCoreCount.end())
	{
		m_requestedActiveCoreCount[policyIndex] = Constants::Invalid;
		commitPolicyRequest(policyIndex, Constants::Invalid);
	}
}

std::shared_ptr<XmlNode> CoreControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("core_control_arbitrator_status");
	auto activeCoreCount = Constants::Invalid;
	auto policyRequest = m_requestedActiveCoreCount.find(policyIndex);
	if (policyRequest != m_requestedActiveCoreCount.end())
	{
		activeCoreCount = policyRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("active_core_count", StatusFormat::friendlyValue(activeCoreCount)));
	return requestRoot;
}

UIntN CoreControlArbitrator::getMinActiveCoreCount(std::map<UIntN, UIntN>& requests)
{
	UIntN minActiveCoreCount = Constants::Invalid;
	for (auto policyRequest = requests.begin(); policyRequest != requests.end(); ++policyRequest)
	{
		UIntN currentActiveCoreCount = policyRequest->second;

		if ((currentActiveCoreCount != Constants::Invalid)
			&& ((minActiveCoreCount == Constants::Invalid) || (currentActiveCoreCount < minActiveCoreCount)))
		{
			minActiveCoreCount = currentActiveCoreCount;
		}
	}
	return minActiveCoreCount;
}