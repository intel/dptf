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

#include "DisplayControlArbitrator.h"
#include "Utility.h"

DisplayControlArbitrator::DisplayControlArbitrator()
	: m_arbitratedDisplayControlIndex(Constants::Invalid)
{
}

DisplayControlArbitrator::~DisplayControlArbitrator(void)
{
}

void DisplayControlArbitrator::commitPolicyRequest(UIntN policyIndex, UIntN displayControlIndex)
{
	m_requestedDisplayControlIndex[policyIndex] = displayControlIndex;
	UIntN maxRequestedDisplayControlIndex = getMaxRequestedDisplayControlIndex(m_requestedDisplayControlIndex);
	m_arbitratedDisplayControlIndex = maxRequestedDisplayControlIndex;
}

Bool DisplayControlArbitrator::hasArbitratedDisplayControlIndex(void) const
{
	if (m_arbitratedDisplayControlIndex != Constants::Invalid)
	{
		return true;
	}
	return false;
}

UIntN DisplayControlArbitrator::arbitrate(UIntN policyIndex, UIntN displayControlIndex)
{
	auto tempPolicyRequests = m_requestedDisplayControlIndex;
	tempPolicyRequests[policyIndex] = displayControlIndex;
	UIntN maxRequestedDisplayControlIndex = getMaxRequestedDisplayControlIndex(tempPolicyRequests);
	return maxRequestedDisplayControlIndex;
}

UIntN DisplayControlArbitrator::getArbitratedDisplayControlIndex(void) const
{
	return m_arbitratedDisplayControlIndex;
}

void DisplayControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto policyRequest = m_requestedDisplayControlIndex.find(policyIndex);
	if (policyRequest != m_requestedDisplayControlIndex.end())
	{
		m_requestedDisplayControlIndex[policyIndex] = Constants::Invalid;
		commitPolicyRequest(policyIndex, Constants::Invalid);
	}
}

UIntN DisplayControlArbitrator::getMaxRequestedDisplayControlIndex(std::map<UIntN, UIntN>& requests)
{
	UIntN maxRequestedDisplayControlIndex = Constants::Invalid;
	for (auto request = requests.begin(); request != requests.end(); ++request)
	{
		if ((request->second != Constants::Invalid) && ((maxRequestedDisplayControlIndex == Constants::Invalid)
														|| (request->second > maxRequestedDisplayControlIndex)))
		{
			maxRequestedDisplayControlIndex = request->second;
		}
	}
	return maxRequestedDisplayControlIndex;
}
