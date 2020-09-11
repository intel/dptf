/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include <XmlNode.h>
#include "ActiveControlDynamicCaps.h"

class dptf_export ArbitratorActiveControlCapabilities
{
public:
	ArbitratorActiveControlCapabilities();
	virtual ~ArbitratorActiveControlCapabilities();

	void commitDynamicCapsRequest(UIntN policyIndex, const ActiveControlDynamicCaps& caps);
	void removeCapabilitiesRequest(UIntN policyIndex);
	ActiveControlDynamicCaps calculateNewArbitratedCapabilities(
		UIntN policyIndex,
		const Percentage& minFanSpeed,
		const Percentage& maxFanSpeed) const;
	ActiveControlDynamicCaps getArbitratedCapabilities() const;

	void commitLockRequest(UIntN policyIndex, const Bool& value);
	void removeLockRequest(UIntN policyIndex);
	Bool calculateNewArbitratedLock(UIntN policyIndex, const Bool& newValue) const;
	Bool getArbitratedLock() const;

	std::shared_ptr<XmlNode> getStatusForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, Percentage> m_minRequests;
	std::map<UIntN, Percentage> m_maxRequests;
	static ActiveControlDynamicCaps calculateArbitratedCapabilities(
		const std::map<UIntN, Percentage>& minRequests,
		const std::map<UIntN, Percentage>& maxRequests);
	static Percentage getHighestMinSpeed(const std::map<UIntN, Percentage>& requests);
	static Percentage getLowestMaxSpeed(const std::map<UIntN, Percentage>& requests);

	std::map<UIntN, Bool> m_lockRequests;
	static Bool calculateArbitratedLock(const std::map<UIntN, Bool>& requests);
};
