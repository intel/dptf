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

#include "PeakPowerControlArbitrator.h"
#include "Utility.h"
#include <StatusFormat.h>

PeakPowerControlArbitrator::PeakPowerControlArbitrator(void)
	: m_requestedPeakPower()
	, m_arbitratedPeakPower()
{
}

PeakPowerControlArbitrator::~PeakPowerControlArbitrator(void)
{
}

void PeakPowerControlArbitrator::commitPolicyRequest(
	UIntN policyIndex,
	PeakPowerType::Type peakPowerType,
	const Power& peakPower)
{
	updatePolicyRequest(policyIndex, peakPowerType, peakPower, m_requestedPeakPower);
	Power lowestRequest = getLowestRequest(peakPowerType, m_requestedPeakPower);
	setArbitratedRequest(peakPowerType, lowestRequest);
}

Bool PeakPowerControlArbitrator::hasArbitratedPeakPower(PeakPowerType::Type peakPowerType) const
{
	auto controlRequests = m_arbitratedPeakPower.find(peakPowerType);
	if (controlRequests != m_arbitratedPeakPower.end())
	{
		return true;
	}
	return false;
}

Power PeakPowerControlArbitrator::arbitrate(
	UIntN policyIndex,
	PeakPowerType::Type peakPowerType,
	const Power& peakPowerRequest)
{
	auto tempPolicyRequests = m_requestedPeakPower;
	updatePolicyRequest(policyIndex, peakPowerType, peakPowerRequest, tempPolicyRequests);
	Power lowestRequest = getLowestRequest(peakPowerType, tempPolicyRequests);
	return lowestRequest;
}

Power PeakPowerControlArbitrator::getArbitratedPeakPower(PeakPowerType::Type peakPowerType) const
{
	auto peakPower = m_arbitratedPeakPower.find(peakPowerType);
	if (peakPower == m_arbitratedPeakPower.end())
	{
		throw dptf_exception(
			"No peak power has been set for peak power type " + PeakPowerType::ToString(peakPowerType) + ".");
	}
	else
	{
		return peakPower->second;
	}
}

void PeakPowerControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	auto policyRequests = m_requestedPeakPower.find(policyIndex);
	if (policyRequests != m_requestedPeakPower.end())
	{
		auto peakPowerTypes = findPeakPowerTypesSetForPolicy(policyRequests->second);
		m_requestedPeakPower.erase(policyRequests);
		if (m_requestedPeakPower.size() > 0)
		{
			setArbitratedPeakPowerForTypes(peakPowerTypes);
		}
		else
		{
			m_arbitratedPeakPower.clear();
		}
	}
}

std::shared_ptr<XmlNode> PeakPowerControlArbitrator::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("peak_power_control_arbitrator_status");
	std::map<PeakPowerType::Type, Power> peakPowers;
	auto policyRequest = m_requestedPeakPower.find(policyIndex);
	if (policyRequest != m_requestedPeakPower.end())
	{
		peakPowers = policyRequest->second;
	}

	for (auto peakPower = peakPowers.begin(); peakPower != peakPowers.end(); ++peakPower)
	{
		requestRoot->addChild(XmlNode::createDataElement("peak_power_" + PeakPowerType::ToXmlString((PeakPowerType::Type)peakPower->first), peakPower->second.toString()));
	}

	return requestRoot;
}

Power PeakPowerControlArbitrator::getLowestRequest(
	PeakPowerType::Type peakPowerType,
	const std::map<UIntN, std::map<PeakPowerType::Type, Power>>& peakPowerValues)
{
	Power lowestRequest;
	Bool lowestRequestSet(false);
	for (auto policy = peakPowerValues.begin(); policy != peakPowerValues.end(); policy++)
	{
		auto& controls = policy->second;
		auto control = controls.find(peakPowerType);
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
			"There were no peak power requests to pick from when choosing the lowest for \
							 arbitration.");
	}
	else
	{
		return lowestRequest;
	}
}

void PeakPowerControlArbitrator::setArbitratedRequest(PeakPowerType::Type peakPowerType, const Power& lowestRequest)
{
	auto controlRequests = m_arbitratedPeakPower.find(peakPowerType);
	if (controlRequests == m_arbitratedPeakPower.end())
	{
		m_arbitratedPeakPower[peakPowerType] = lowestRequest;
	}
	else
	{
		if (controlRequests->second != lowestRequest)
		{
			m_arbitratedPeakPower[peakPowerType] = lowestRequest;
		}
	}
}

void PeakPowerControlArbitrator::updatePolicyRequest(
	UIntN policyIndex,
	PeakPowerType::Type peakPowerType,
	const Power& peakPowerRequest,
	std::map<UIntN, std::map<PeakPowerType::Type, Power>>& peakPowerValues)
{
	auto policyRequests = peakPowerValues.find(policyIndex);
	if (policyRequests == peakPowerValues.end())
	{
		peakPowerValues[policyIndex] = std::map<PeakPowerType::Type, Power>();
	}
	peakPowerValues[policyIndex][peakPowerType] = peakPowerRequest;
}

std::vector<PeakPowerType::Type> PeakPowerControlArbitrator::findPeakPowerTypesSetForPolicy(
	const std::map<PeakPowerType::Type, Power>& controlRequests)
{
	std::vector<PeakPowerType::Type> peakPowerTypes;
	for (UIntN peakPowerType = 0; peakPowerType < (UIntN)PeakPowerType::MAX; ++peakPowerType)
	{
		auto controlRequest = controlRequests.find((PeakPowerType::Type)peakPowerType);
		if (controlRequest != controlRequests.end())
		{
			peakPowerTypes.push_back((PeakPowerType::Type)peakPowerType);
		}
	}
	return peakPowerTypes;
}

void PeakPowerControlArbitrator::setArbitratedPeakPowerForTypes(const std::vector<PeakPowerType::Type>& peakPowerTypes)
{
	for (auto peakPowerType = peakPowerTypes.begin(); peakPowerType != peakPowerTypes.end(); ++peakPowerType)
	{
		try
		{
			Power lowestRequest = getLowestRequest(*peakPowerType, m_requestedPeakPower);
			setArbitratedRequest(*peakPowerType, lowestRequest);
		}
		catch (...)
		{
		}
	}
}