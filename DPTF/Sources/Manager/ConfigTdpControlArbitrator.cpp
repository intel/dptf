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

#include "ConfigTdpControlArbitrator.h"
#include "Utility.h"
#include <StatusFormat.h>

ConfigTdpControlArbitrator::ConfigTdpControlArbitrator()
	: m_arbitratedConfigTdpControlIndex(Constants::Invalid)
{
}

ConfigTdpControlArbitrator::~ConfigTdpControlArbitrator(void)
{
}

void ConfigTdpControlArbitrator::commitPolicyRequest(UIntN policyIndex, UIntN configTdpControlIndex)
{
	m_requestedConfigTdpControlIndex[policyIndex] = configTdpControlIndex;
	UIntN maxRequestedConfigTdpControlIndex = getMaxRequestedIndex(m_requestedConfigTdpControlIndex);
	m_arbitratedConfigTdpControlIndex = maxRequestedConfigTdpControlIndex;
}

Bool ConfigTdpControlArbitrator::hasArbitratedConfigTdpControlIndex(void) const
{
	if (m_arbitratedConfigTdpControlIndex != Constants::Invalid)
	{
		return true;
	}
	return false;
}

UIntN ConfigTdpControlArbitrator::arbitrate(UIntN policyIndex, UIntN configTdpControlIndex)
{
	auto tempPolicyRequests = m_requestedConfigTdpControlIndex;
	tempPolicyRequests[policyIndex] = configTdpControlIndex;
	UIntN maxRequestedConfigTdpControlIndex = getMaxRequestedIndex(tempPolicyRequests);
	return maxRequestedConfigTdpControlIndex;
}

UIntN ConfigTdpControlArbitrator::getArbitratedConfigTdpControlIndex(void) const
{
	return m_arbitratedConfigTdpControlIndex;
}

void ConfigTdpControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto policyRequest = m_requestedConfigTdpControlIndex.find(policyIndex);
	if (policyRequest != m_requestedConfigTdpControlIndex.end())
	{
		m_requestedConfigTdpControlIndex[policyIndex] = Constants::Invalid;
		commitPolicyRequest(policyIndex, Constants::Invalid);
	}
}

std::shared_ptr<XmlNode> ConfigTdpControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("ctdp_control_arbitrator_status");
	auto cTdpIndex = Constants::Invalid;
	auto policyRequest = m_requestedConfigTdpControlIndex.find(policyIndex);
	if (policyRequest != m_requestedConfigTdpControlIndex.end())
	{
		cTdpIndex = policyRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("ctdp_index", StatusFormat::friendlyValue(cTdpIndex)));
	return requestRoot;
}

UIntN ConfigTdpControlArbitrator::getMaxRequestedIndex(std::map<UIntN, UIntN>& requests)
{
	// find the lowest config tdp level requested (this is the max index)
	UIntN maxRequestedConfigTdpControlIndex = Constants::Invalid;
	for (auto policyRequest = requests.begin(); policyRequest != requests.end(); ++policyRequest)
	{
		if ((policyRequest->second != Constants::Invalid)
			&& ((maxRequestedConfigTdpControlIndex == Constants::Invalid)
				|| (policyRequest->second > maxRequestedConfigTdpControlIndex)))
		{
			maxRequestedConfigTdpControlIndex = policyRequest->second;
		}
	}
	return maxRequestedConfigTdpControlIndex;
}