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

#pragma once

#include "Dptf.h"
#include "PeakPowerType.h"
#include <XmlNode.h>

//
// Arbitration Rule:
//
// Lowest peak power value wins
//

class dptf_export PeakPowerControlArbitrator
{
public:
	PeakPowerControlArbitrator(void);
	~PeakPowerControlArbitrator(void);

	void commitPolicyRequest(UIntN policyIndex, PeakPowerType::Type peakPowerType, const Power& peakPower);
	Bool hasArbitratedPeakPower(PeakPowerType::Type peakPowerType) const;
	Power arbitrate(UIntN policyIndex, PeakPowerType::Type peakPowerType, const Power& coreControlStatus);

	Power getArbitratedPeakPower(PeakPowerType::Type peakPowerType) const;
	void clearPolicyCachedData(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, std::map<PeakPowerType::Type, Power>> m_requestedPeakPower;
	std::map<PeakPowerType::Type, Power> m_arbitratedPeakPower;

	Power getLowestRequest(
		PeakPowerType::Type peakPowerType,
		const std::map<UIntN, std::map<PeakPowerType::Type, Power>>& peakPowerValues);
	void setArbitratedRequest(PeakPowerType::Type peakPowerType, const Power& lowestRequest);
	void updatePolicyRequest(
		UIntN policyIndex,
		PeakPowerType::Type peakPowerType,
		const Power& peakPowerRequest,
		std::map<UIntN, std::map<PeakPowerType::Type, Power>>& peakPowerValues);
	std::vector<PeakPowerType::Type> findPeakPowerTypesSetForPolicy(
		const std::map<PeakPowerType::Type, Power>& controlRequests);
	void setArbitratedPeakPowerForTypes(const std::vector<PeakPowerType::Type>& peakPowerTypes);
};
