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

#pragma once

#include "Dptf.h"
#include "PowerControlDynamicCapsSet.h"
#include <XmlNode.h>

class dptf_export PowerControlCapabilitiesArbitrator
{
public:
	PowerControlCapabilitiesArbitrator();
	~PowerControlCapabilitiesArbitrator();

	void commitPolicyRequest(UIntN policyIndex, const PowerControlDynamicCapsSet& capSet);
	Bool hasArbitratedPowerControlCapabilities() const;
	PowerControlDynamicCapsSet arbitrate(
		UIntN policyIndex,
		const PowerControlDynamicCapsSet& requestedCapSet,
		const PowerControlDynamicCapsSet& currentCapSet);

	Bool arbitrateLockRequests(UIntN policyIndex, Bool lock);
	PowerControlDynamicCapsSet getArbitratedPowerControlCapabilities(const PowerControlDynamicCapsSet& currentCapSet);
	Bool getArbitratedLock() const;
	void removeRequestsForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, std::map<PowerControlType::Type, Power>> m_requestedMaxPowerLimit;
	std::map<UIntN, std::map<PowerControlType::Type, Power>> m_requestedMinPowerLimit;
	std::map<UIntN, std::map<PowerControlType::Type, Power>> m_requestedPowerLimitStep;
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>> m_requestedMaxTimeWindow;
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>> m_requestedMinTimeWindow;
	std::map<UIntN, Bool> m_requestedLocks;

	void updatePolicyRequest(const PowerControlDynamicCapsSet& capSet, UIntN policyIndex);
	void updatePolicyLockRequest(Bool lock, UIntN policyIndex);
	Power getLowestMaxPowerLimit(
		PowerControlType::Type controlType,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const;
	Power getHighestMinPowerLimit(
		PowerControlType::Type controlType,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const;
	Power getHighestPowerLimitStep(
		PowerControlType::Type controlType,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& requests) const;
	TimeSpan getLowestMaxTimeWindow(
		PowerControlType::Type controlType,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& requests) const;
	TimeSpan getHighestMinTimeWindow(
		PowerControlType::Type controlType,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& requests) const;
	std::set<PowerControlType::Type> getControlTypes(
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& maxPowerRequests,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& minPowerRequests,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerStepRequests,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& maxTimeRequests,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& minTimeRequests) const;

	PowerControlDynamicCapsSet createNewArbitratedCapabilitites(
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& maxPowerRequests,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& minPowerRequests,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerStepRequests,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& maxTimeRequests,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& minTimeRequests,
		const PowerControlDynamicCapsSet& currentCapSet);
};
