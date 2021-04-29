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
#include "TimeSpan.h"
#include "PsysPowerLimitType.h"
#include <XmlNode.h>

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

class dptf_export SystemPowerControlArbitrator
{
public:
	SystemPowerControlArbitrator();
	~SystemPowerControlArbitrator();

	void commitPolicyRequest(UIntN policyIndex, PsysPowerLimitType::Type controlType, const Power& powerLimit);
	void commitPolicyRequest(UIntN policyIndex, PsysPowerLimitType::Type controlType, const TimeSpan& timeWindow);
	void commitPolicyRequest(UIntN policyIndex, PsysPowerLimitType::Type controlType, const Percentage& dutyCycle);

	Bool hasArbitratedSystemPowerLimit(PsysPowerLimitType::Type controlType) const;
	Bool hasArbitratedTimeWindow(PsysPowerLimitType::Type controlType) const;
	Bool hasArbitratedDutyCycle(PsysPowerLimitType::Type controlType) const;

	Power getArbitratedSystemPowerLimit(PsysPowerLimitType::Type controlType) const;
	TimeSpan getArbitratedTimeWindow(PsysPowerLimitType::Type controlType) const;
	Percentage getArbitratedDutyCycle(PsysPowerLimitType::Type controlType) const;

	Power arbitrate(UIntN policyIndex, PsysPowerLimitType::Type controlType, const Power& powerLimit);
	TimeSpan arbitrate(UIntN policyIndex, PsysPowerLimitType::Type controlType, const TimeSpan& timeWindow);
	Percentage arbitrate(UIntN policyIndex, PsysPowerLimitType::Type controlType, const Percentage& dutyCycle);

	void removeRequestsForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, std::map<PsysPowerLimitType::Type, Power>> m_requestedSystemPowerLimits;
	std::map<PsysPowerLimitType::Type, Power> m_arbitratedSystemPowerLimit;
	std::map<UIntN, std::map<PsysPowerLimitType::Type, TimeSpan>> m_requestedTimeWindows;
	std::map<PsysPowerLimitType::Type, TimeSpan> m_arbitratedTimeWindow;
	std::map<UIntN, std::map<PsysPowerLimitType::Type, Percentage>> m_requestedDutyCycles;
	std::map<PsysPowerLimitType::Type, Percentage> m_arbitratedDutyCycle;

	Power getLowestRequest(
		PsysPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PsysPowerLimitType::Type, Power>>& powerLimits);
	TimeSpan getLowestRequest(
		PsysPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PsysPowerLimitType::Type, TimeSpan>>& timeWindows);
	Percentage getLowestRequest(
		PsysPowerLimitType::Type controlType,
		const std::map<UIntN, std::map<PsysPowerLimitType::Type, Percentage>>& dutyCycles);
	void setArbitratedRequest(PsysPowerLimitType::Type controlType, const Power& lowestRequest);
	void setArbitratedRequest(PsysPowerLimitType::Type controlType, const TimeSpan& lowestRequest);
	void setArbitratedRequest(PsysPowerLimitType::Type controlType, const Percentage& lowestRequest);
	void updatePolicyRequest(
		UIntN policyIndex,
		PsysPowerLimitType::Type controlType,
		const Power& powerLimit,
		std::map<UIntN, std::map<PsysPowerLimitType::Type, Power>>& powerLimits);
	void updatePolicyRequest(
		UIntN policyIndex,
		PsysPowerLimitType::Type controlType,
		const TimeSpan& timeWindow,
		std::map<UIntN, std::map<PsysPowerLimitType::Type, TimeSpan>>& timeWindows);
	void updatePolicyRequest(
		UIntN policyIndex,
		PsysPowerLimitType::Type controlType,
		const Percentage& dutyCycle,
		std::map<UIntN, std::map<PsysPowerLimitType::Type, Percentage>>& dutyCycles);

	void removeSystemPowerLimitRequest(UIntN policyIndex);
	void removeTimeWindowRequest(UIntN policyIndex);
	void removeDutyCycleRequest(UIntN policyIndex);

	std::vector<PsysPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PsysPowerLimitType::Type, Power>& controlRequests);
	std::vector<PsysPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PsysPowerLimitType::Type, TimeSpan>& controlRequests);
	std::vector<PsysPowerLimitType::Type> findControlTypesSetForPolicy(
		const std::map<PsysPowerLimitType::Type, Percentage>& controlRequests);

	void setArbitratedSystemPowerLimitForControlTypes(const std::vector<PsysPowerLimitType::Type>& controlTypes);
	void setArbitratedTimeWindowsForControlTypes(const std::vector<PsysPowerLimitType::Type>& controlTypes);
	void setArbitratedDutyCyclesForControlTypes(const std::vector<PsysPowerLimitType::Type>& controlTypes);
};
