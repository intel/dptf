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

#pragma once

#include "Dptf.h"
#include "DisplayControlDynamicCaps.h"
#include <XmlNode.h>

class dptf_export DisplayControlCapabilitiesArbitrator
{
public:
	DisplayControlCapabilitiesArbitrator();
	~DisplayControlCapabilitiesArbitrator();

	void commitPolicyRequest(UIntN policyIndex, const DisplayControlDynamicCaps& caps);
	Bool hasArbitratedDisplayCapabilities() const;
	DisplayControlDynamicCaps arbitrate(UIntN policyIndex, const DisplayControlDynamicCaps& caps);

	Bool arbitrateLockRequests(UIntN policyIndex, Bool lock);
	DisplayControlDynamicCaps getArbitratedDisplayControlCapabilities();
	Bool getArbitratedLock() const;
	void removeRequestsForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, UIntN> m_requestedMaxDisplayIndex;
	std::map<UIntN, UIntN> m_requestedMinDisplayIndex;
	std::map<UIntN, Bool> m_requestedLocks;

	void updatePolicyRequest(
		const DisplayControlDynamicCaps& caps,
		UIntN policyIndex,
		std::map<UIntN, UIntN>& minRequests,
		std::map<UIntN, UIntN>& maxRequests);
	void updatePolicyLockRequest(Bool lock, UIntN policyIndex);
	UIntN getLowestMaxDisplayIndex(std::map<UIntN, UIntN>& maxRequests) const;
	UIntN getHighestMinDisplayIndex(std::map<UIntN, UIntN>& minRequests) const;
};
