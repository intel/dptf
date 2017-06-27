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
#include "TimeSpan.h"
#include "PowerControlType.h"
#include <XmlNode.h>

//
// Arbitration Rule:
//
// for each (PL1, PL2, Pl3)
//  lowest power level wins
//  lowest time window wins
//  lowest duty cycle wins
//
// *It is allowed to mix and match between policies.  The lowest PL1 power level can be from one policy and the
//  lowest time window can be from another policy
//

class dptf_export PowerControlArbitrator
{
public:
	PowerControlArbitrator();
	~PowerControlArbitrator();

	void commitPolicyRequest(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void commitPolicyRequest(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void commitPolicyRequest(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle);

	Bool hasArbitratedPowerLimit(PowerControlType::Type controlType) const;
	Bool hasArbitratedTimeWindow(PowerControlType::Type controlType) const;
	Bool hasArbitratedDutyCycle(PowerControlType::Type controlType) const;

	Power getArbitratedPowerLimit(PowerControlType::Type controlType) const;
	TimeSpan getArbitratedTimeWindow(PowerControlType::Type controlType) const;
	Percentage getArbitratedDutyCycle(PowerControlType::Type controlType) const;

	Power arbitrate(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	TimeSpan arbitrate(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	Percentage arbitrate(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle);

	void removeRequestsForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, std::map<PowerControlType::Type, Power>> m_requestedPowerLimits;
	std::map<PowerControlType::Type, Power> m_arbitratedPowerLimit;
	std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>> m_requestedTimeWindows;
	std::map<PowerControlType::Type, TimeSpan> m_arbitratedTimeWindow;
	std::map<UIntN, std::map<PowerControlType::Type, Percentage>> m_requestedDutyCycles;
	std::map<PowerControlType::Type, Percentage> m_arbitratedDutyCycle;

	Power getLowestRequest(
		PowerControlType::Type controlType,
		const std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerLimits);
	TimeSpan getLowestRequest(
		PowerControlType::Type controlType,
		const std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& timeWindows);
	Percentage getLowestRequest(
		PowerControlType::Type controlType,
		const std::map<UIntN, std::map<PowerControlType::Type, Percentage>>& dutyCycles);
	void setArbitratedRequest(PowerControlType::Type controlType, const Power& lowestRequest);
	void setArbitratedRequest(PowerControlType::Type controlType, const TimeSpan& lowestRequest);
	void setArbitratedRequest(PowerControlType::Type controlType, const Percentage& lowestRequest);
	void updatePolicyRequest(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit,
		std::map<UIntN, std::map<PowerControlType::Type, Power>>& powerLimits);
	void updatePolicyRequest(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow,
		std::map<UIntN, std::map<PowerControlType::Type, TimeSpan>>& timeWindows);
	void updatePolicyRequest(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle,
		std::map<UIntN, std::map<PowerControlType::Type, Percentage>>& dutyCycles);

	void removePowerLimitRequest(UIntN policyIndex);
	void removeTimeWindowRequest(UIntN policyIndex);
	void removeDutyCycleRequest(UIntN policyIndex);

	std::vector<PowerControlType::Type> findControlTypesSetForPolicy(
		const std::map<PowerControlType::Type, Power>& controlRequests) const;
	std::vector<PowerControlType::Type> findControlTypesSetForPolicy(
		const std::map<PowerControlType::Type, TimeSpan>& controlRequests) const;
	std::vector<PowerControlType::Type> findControlTypesSetForPolicy(
		const std::map<PowerControlType::Type, Percentage>& controlRequests) const;

	void setArbitratedPowerLimitForControlTypes(const std::vector<PowerControlType::Type>& controlTypes);
	void setArbitratedTimeWindowsForControlTypes(const std::vector<PowerControlType::Type>& controlTypes);
	void setArbitratedDutyCyclesForControlTypes(const std::vector<PowerControlType::Type>& controlTypes);
};
