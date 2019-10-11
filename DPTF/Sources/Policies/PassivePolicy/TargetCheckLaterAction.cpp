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

#include "TargetCheckLaterAction.h"
#include "PolicyLogger.h"

using namespace std;

TargetCheckLaterAction::TargetCheckLaterAction(
	PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<TimeInterface> time,
	std::shared_ptr<ParticipantTrackerInterface> participantTracker,
	std::shared_ptr<ThermalRelationshipTable> trt,
	std::shared_ptr<CallbackScheduler> callbackScheduler,
	TargetMonitor& targetMonitor,
	UIntN target)
	: TargetActionBase(policyServices, time, participantTracker, trt, callbackScheduler, targetMonitor, target)
{
}

TargetCheckLaterAction::~TargetCheckLaterAction()
{
}

void TargetCheckLaterAction::execute()
{
	try
	{
		// make sure target is now being monitored
		getTargetMonitor().startMonitoring(getTarget());

		// schedule a callback as soon as possible
		// TODO: want to pass in participant index
		POLICY_LOG_MESSAGE_DEBUG({
			std::stringstream message;
			message << "Attempting to schedule callback for target participant."
					<< " ParticipantIndex = " << getTarget();
			return message.str();
		});
		auto time = getTime()->getCurrentTime();
		getCallbackScheduler()->ensureCallbackByShortestSamplePeriod(getTarget(), time);
	}
	catch (...)
	{
		POLICY_LOG_MESSAGE_WARNING({
			std::stringstream message;
			message << "Failed to schedule callback for target participant."
					<< " ParticipantIndex = " << getTarget();
			return message.str();
		});
	}
}
