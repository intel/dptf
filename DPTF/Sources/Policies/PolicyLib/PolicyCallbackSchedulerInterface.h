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
#include "TimeInterface.h"
#include "XmlNode.h"
#include "EventCode.h"

class dptf_export PolicyCallbackSchedulerInterface
{
public:
	virtual ~PolicyCallbackSchedulerInterface(){};

	virtual void suspend(UIntN participantIndex, const TimeSpan& time) = 0;
	virtual void suspend(UIntN participantIndex, const TimeSpan& fromTime, const TimeSpan& suspendTime) = 0;
	virtual void suspend(EventCode::Type participantRole, UIntN participantIndex, const TimeSpan& time) = 0;
	virtual void setTimerForObject(void* object, const TimeSpan& time) = 0;
	virtual void cancelCallback(UIntN participantIndex) = 0;
	virtual void cancelCallback(EventCode::Type participantRole, UIntN participantIndex) = 0;
	virtual void cancelAllCallbackRequests() = 0;
	virtual void cancelTimerForObject(void* object) = 0;
	virtual Bool hasCallbackWithinTimeRange(UIntN participantIndex, const TimeSpan& beginTime, const TimeSpan& endTime)
		const = 0;
	virtual void acknowledgeCallback(UIntN participantIndex) = 0;
	virtual void acknowledgeCallback(EventCode::Type participantRole, UIntN participantIndex) = 0;
	virtual void acknowledgeCallback(void* object) = 0;
	virtual void setTimeObject(std::shared_ptr<TimeInterface> time) = 0;
	virtual std::shared_ptr<XmlNode> getStatus() = 0;
	virtual std::shared_ptr<XmlNode> getStatusForParticipant(UIntN participantIndex) = 0;
	virtual std::shared_ptr<XmlNode> getStatusForParticipant(
		EventCode::Type participantRole,
		UIntN participantIndex) = 0;
	virtual std::shared_ptr<XmlNode> getTimerStatusForObject(void* object) = 0;
};
