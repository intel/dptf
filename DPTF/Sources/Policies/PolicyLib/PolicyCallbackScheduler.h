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

#include "PolicyCallbackSchedulerInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include "TimeInterface.h"
#include "Dptf.h"
#include "ParticipantCallback.h"

class dptf_export PolicyCallbackScheduler : public PolicyCallbackSchedulerInterface
{
public:
	PolicyCallbackScheduler(
		const PolicyServicesInterfaceContainer& policyServicesContainer,
		std::shared_ptr<TimeInterface> timeInterface);
	virtual ~PolicyCallbackScheduler();

	virtual void suspend(UIntN participantIndex, const TimeSpan& time) override;
	virtual void suspend(UIntN participantIndex, const TimeSpan& fromTime, const TimeSpan& suspendTime) override;
	virtual void suspend(EventCode::Type participantRole, UIntN participantIndex, const TimeSpan& time) override;
	virtual void setTimerForObject(void* object, const TimeSpan& time) override;
	virtual void cancelCallback(UIntN participantIndex) override;
	virtual void cancelCallback(EventCode::Type participantRole, UIntN participantIndex) override;
	virtual void cancelTimerForObject(void* object) override;
	virtual Bool hasCallbackWithinTimeRange(UIntN participantIndex, const TimeSpan& beginTime, const TimeSpan& endTime)
		const override;
	virtual void acknowledgeCallback(UIntN participantIndex) override;
	virtual void acknowledgeCallback(EventCode::Type participantRole, UIntN participantIndex) override;
	virtual void acknowledgeCallback(void* object) override;
	virtual void setTimeObject(std::shared_ptr<TimeInterface> time) override;
	virtual std::shared_ptr<XmlNode> getStatus() override;
	virtual std::shared_ptr<XmlNode> getStatusForParticipant(UIntN participantIndex) override;
	virtual std::shared_ptr<XmlNode> getStatusForParticipant(EventCode::Type participantRole, UIntN participantIndex)
		override;
	virtual std::shared_ptr<XmlNode> getTimerStatusForObject(void* object) override;

private:
	PolicyServicesInterfaceContainer m_policyServices;
	std::shared_ptr<TimeInterface> m_time;
	std::map<std::pair<EventCode::Type, UIntN>, ParticipantCallback> m_schedule;
	std::map<UInt64, ParticipantCallback> m_timers;

	TimeSpan getCurrentTime() const;
	void scheduleCallback(UIntN participantIndex, const TimeSpan& currentTime, const TimeSpan& pollTime);
	void scheduleCallback(
		EventCode::Type participantRole,
		UIntN participantIndex,
		const TimeSpan& currentTime,
		const TimeSpan& pollTime);
	void scheduleTimerCallback(void* object, const TimeSpan& currentTime, const TimeSpan& pollTime);
};
