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
#include "PerformanceControlDynamicCaps.h"

class dptf_export PerformanceControlCapabilitiesArbitrator
{
public:
	PerformanceControlCapabilitiesArbitrator();
	~PerformanceControlCapabilitiesArbitrator();

	void commitPolicyRequest(UIntN policyIndex, const PerformanceControlDynamicCaps& caps);
	Bool hasArbitratedPerformanceControlCapabilities() const;
	PerformanceControlDynamicCaps arbitrate(UIntN policyIndex, const PerformanceControlDynamicCaps& caps);

	Bool arbitrateLockRequests(UIntN policyIndex, Bool lock);
	PerformanceControlDynamicCaps getArbitratedPerformanceControlCapabilities();
	Bool getArbitratedLock() const;
	void removeRequestsForPolicy(UIntN policyIndex);

private:
	std::map<UIntN, UIntN> m_requestedUpperPState;
	std::map<UIntN, UIntN> m_requestedLowerPState;
	std::map<UIntN, Bool> m_requestedLocks;

	void updatePolicyRequest(
		const PerformanceControlDynamicCaps& caps,
		UIntN policyIndex,
		std::map<UIntN, UIntN>& upperRequests,
		std::map<UIntN, UIntN>& lowerRequests);
	void updatePolicyLockRequest(Bool lock, UIntN policyIndex);
	UIntN getBiggestUpperPStateIndex(std::map<UIntN, UIntN>& upperRequests) const;
	UIntN getSmallestLowerPStateIndex(std::map<UIntN, UIntN>& lowerRequests) const;
};
