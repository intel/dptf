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
#include "TargetActionBase.h"
#include <tuple>

// implements the algorithm for limiting in the passive policy
class dptf_export TargetLimitAction : public TargetActionBase
{
public:
	TargetLimitAction(
		PolicyServicesInterfaceContainer& policyServices,
		std::shared_ptr<TimeInterface> time,
		std::shared_ptr<ParticipantTrackerInterface> participantTracker,
		std::shared_ptr<ThermalRelationshipTable> trt,
		std::shared_ptr<CallbackScheduler> callbackScheduler,
		TargetMonitor& targetMonitor,
		UIntN target);
	virtual ~TargetLimitAction();

	virtual void execute() override;

private:
	// source filtering
	std::vector<UIntN> chooseSourcesToLimitForTarget(UIntN target);
	std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> getEntriesWithControlsToLimit(
		UIntN target,
		const std::vector<std::shared_ptr<ThermalRelationshipTableEntry>>& sourcesForTarget);

	// domain filtering
	std::vector<UIntN> getDomainsWithControlKnobsToLimit(ParticipantProxyInterface* participant, UIntN target);
	std::vector<UIntN> chooseDomainsToLimitForSource(UIntN target, UIntN source);
	UIntN getDomainWithHighestTemperature(UIntN source, const std::vector<UIntN>& domainsWithControlKnobsToTurn);
	std::vector<std::pair<UIntN, UtilizationStatus>> getDomainsSortedByPriorityThenUtilization(
		UIntN source,
		std::vector<UIntN> domains);
	Bool domainReportsUtilization(UIntN source, UIntN domain);

	// domain limiting
	void requestLimit(UIntN source, UIntN domainIndex, UIntN target);
	void commitLimit(UIntN source, const TimeSpan& time);
};
