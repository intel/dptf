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
#include "ThermalRelationshipTable.h"
#include "TimeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include "SourceAvailability.h"
#include "PolicyCallbackScheduler.h"
#include "TargetMonitor.h"
#include "TargetSourceRelationship.h"

// responsible for determining when to schedule callbacks for targets based on sampling intervals in the TRT
class dptf_export CallbackScheduler
{
public:
	CallbackScheduler(
		const PolicyServicesInterfaceContainer& policyServices,
		std::shared_ptr<ThermalRelationshipTable> trt,
		std::shared_ptr<TimeInterface> time);
	~CallbackScheduler();

	Bool isFreeForRequests(UIntN target, UIntN source, const TimeSpan& time) const;
	void markBusyForRequests(UIntN target, UIntN source, const TimeSpan& time);
	void ensureCallbackByNextSamplePeriod(UIntN target, UIntN source, const TimeSpan& time);
	Bool isFreeForCommits(UIntN source, const TimeSpan& time) const;
	void ensureCallbackByShortestSamplePeriod(UIntN target, const TimeSpan& time);
	void acknowledgeCallback(UIntN target);

	// participant availability
	void removeParticipantFromSchedule(UIntN participant);
	void markSourceAsBusy(UIntN source, const TargetMonitor& targetMonitor, const TimeSpan& time);

	// updates service objects
	void setTrt(std::shared_ptr<ThermalRelationshipTable> trt);
	void setTimeObject(std::shared_ptr<TimeInterface> time);

	// status
	std::shared_ptr<XmlNode> getXml() const;

private:
	SourceAvailability m_sourceAvailability;
	std::shared_ptr<ThermalRelationshipTable> m_trt;
	MessageLoggingInterface* m_logger;
	std::shared_ptr<PolicyCallbackSchedulerInterface> m_targetScheduler;
	TimeSpan m_minSampleTime;
	std::map<TargetSourceRelationship, TimeSpan> m_requestSchedule;
};
