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
#include "PlatformPowerLimitType.h"

//
// Arbitration Rule:
//
// for each (PsysPL1, PsysPL2, PsysPl3)
//  lowest power level wins
//  lowest time window wins
//  lowest duty cycle wins
//
// *It is allowed to mix and match between policies.  The lowest PsysPL1 power level can be from one policy and the
//  lowest time window can be from another policy
//

class dptf_export PlatformPowerControlArbitrator
{
public:
	PlatformPowerControlArbitrator();
	~PlatformPowerControlArbitrator();

	void commitPolicyRequest(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const Power& powerLimit);
	void commitPolicyRequest(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const TimeSpan& timeWindow);
	void commitPolicyRequest(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const Percentage& dutyCycle);

	Bool hasArbitratedPlatformPowerLimit(PlatformPowerLimitType::Type controlType) const;
	Bool hasArbitratedTimeWindow(PlatformPowerLimitType::Type controlType) const;
	Bool hasArbitratedDutyCycle(PlatformPowerLimitType::Type controlType) const;

	Power getArbitratedPlatformPowerLimit(PlatformPowerLimitType::Type controlType) const;
	TimeSpan getArbitratedTimeWindow(PlatformPowerLimitType::Type controlType) const;
	Percentage getArbitratedDutyCycle(PlatformPowerLimitType::Type controlType) const;

	Power arbitrate(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const Power& powerLimit);
	TimeSpan arbitrate(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const TimeSpan& timeWindow);
	Percentage arbitrate(UIntN policyIndex, PlatformPowerLimitType::Type controlType, const Percentage& dutyCycle);

	void removeRequestsForPolicy(UIntN policyIndex);

private:
	std::map<UIntN, std::map<PlatformPowerLimitType::Type, Power>> m_requestedPlatformPowerLimits;
	std::map<PlatformPowerLimitType::Type, Power> m_arbitratedPlatformPowerLimit;
	std::map<UIntN, std::map<PlatformPowerLimitType::Type, TimeSpan>> m_requestedTimeWindows;
	std::map<PlatformPowerLimitType::Type, TimeSpan> m_arbitratedTimeWindow;
	std::map<UIntN, std::map<PlatformPowerLimitType::Type, Percentage>> m_requestedDutyCycles;
	std::map<PlatformPowerLimitType::Type, Percentage> m_arbitratedDutyCycle;

	Power getLowestRequest(
		PlatformPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PlatformPowerLimitType::Type, Power>>& powerLimits);
	TimeSpan getLowestRequest(
		PlatformPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PlatformPowerLimitType::Type, TimeSpan>>& timeWindows);
	Percentage getLowestRequest(
		PlatformPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PlatformPowerLimitType::Type, Percentage>>& dutyCycles);
	void setArbitratedRequest(PlatformPowerLimitType::Type controlType, const Power& lowestRequest);
	void setArbitratedRequest(PlatformPowerLimitType::Type controlType, const TimeSpan& lowestRequest);
	void setArbitratedRequest(PlatformPowerLimitType::Type controlType, const Percentage& lowestRequest);
	void updatePolicyRequest(
		UIntN policyIndex,
		PlatformPowerLimitType::Type controlType,
		const Power& powerLimit,
		std::map<UIntN, std::map<PlatformPowerLimitType::Type, Power>>& powerLimits);
	void updatePolicyRequest(
		UIntN policyIndex,
		PlatformPowerLimitType::Type controlType,
		const TimeSpan& timeWindow,
		std::map<UIntN, std::map<PlatformPowerLimitType::Type, TimeSpan>>& timeWindows);
	void updatePolicyRequest(
		UIntN policyIndex,
		PlatformPowerLimitType::Type controlType,
		const Percentage& dutyCycle,
		std::map<UIntN, std::map<PlatformPowerLimitType::Type, Percentage>>& dutyCycles);

	void removePlatformPowerLimitRequest(UIntN policyIndex);
	void removeTimeWindowRequest(UIntN policyIndex);
	void removeDutyCycleRequest(UIntN policyIndex);

	std::vector<PlatformPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PlatformPowerLimitType::Type, Power>& controlRequests);
	std::vector<PlatformPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PlatformPowerLimitType::Type, TimeSpan>& controlRequests);
	std::vector<PlatformPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PlatformPowerLimitType::Type, Percentage>& controlRequests);

	void setArbitratedPlatformPowerLimitForControlTypes(const std::vector<PlatformPowerLimitType::Type>& controlTypes);
	void setArbitratedTimeWindowsForControlTypes(const std::vector<PlatformPowerLimitType::Type>& controlTypes);
	void setArbitratedDutyCyclesForControlTypes(const std::vector<PlatformPowerLimitType::Type>& controlTypes);
};
