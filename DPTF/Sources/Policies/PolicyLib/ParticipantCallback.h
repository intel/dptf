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

class dptf_export ParticipantCallback
{
public:
	ParticipantCallback();
	ParticipantCallback(const TimeSpan& callbackTime, const TimeSpan& timeStamp, UInt64 callbackHandle);
	virtual ~ParticipantCallback() = default;

	ParticipantCallback(const ParticipantCallback& other) = default;
	ParticipantCallback& operator=(const ParticipantCallback& other) = default;
	ParticipantCallback(ParticipantCallback&& other) = default;
	ParticipantCallback& operator=(ParticipantCallback&& other) = default;

	const TimeSpan& getTimeDelta() const;
	const TimeSpan& getTimeStamp() const;
	UInt64 getCallbackHandle() const;

private:
	TimeSpan m_timeDelta;
	TimeSpan m_timeStamp;
	UInt64 m_callbackHandle;
};
