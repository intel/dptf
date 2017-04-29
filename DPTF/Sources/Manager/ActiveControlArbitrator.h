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

#pragma once

#include "Dptf.h"

//
// Arbitration Rule:
//
// 1) if fine grain fan control is supported we only arbitrate based on percentage.  otherwise we only arbitrate
//    based on control index.
// 2) for percentage the highest fan speed wins
// 3) for control index the lowest index (which is highest fan speed) wins
//

class dptf_export ActiveControlArbitrator
{
public:
	ActiveControlArbitrator();
	~ActiveControlArbitrator(void);

	void commitPolicyRequest(UIntN policyIndex, const Percentage& fanSpeed);
	void commitPolicyRequest(UIntN policyIndex, UIntN activeControlIndex);

	Bool hasArbitratedFanSpeedPercentage(void) const;
	Bool hasArbitratedActiveControlIndex(void) const;

	Percentage getArbitratedFanSpeedPercentage(void) const;
	UIntN getArbitratedActiveControlIndex(void) const;

	Percentage arbitrate(UIntN policyIndex, const Percentage& fanSpeed);
	UIntN arbitrate(UIntN policyIndex, UIntN activeControlIndex);

	void clearPolicyCachedData(UIntN policyIndex);

private:
	Percentage m_arbitratedFanSpeedPercentage;
	std::map<UIntN, Percentage> m_requestedfanSpeedPercentage;
	Percentage getMaxRequestedFanSpeedPercentage(std::map<UIntN, Percentage>& requests);

	UIntN m_arbitratedActiveControlIndex;
	std::map<UIntN, UIntN> m_requestedActiveControlIndex;
	UIntN getMinRequestedActiveControlIndex(std::map<UIntN, UIntN>& requests);
};
