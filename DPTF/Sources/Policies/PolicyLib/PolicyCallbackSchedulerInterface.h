/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

class dptf_export PolicyCallbackSchedulerInterface
{
public:

    virtual ~PolicyCallbackSchedulerInterface()  {};
    virtual void suspend(UIntN participantIndex, const TimeSpan& time) = 0;
    virtual void suspend(UIntN participantIndex, UInt64 fromTime, const TimeSpan& suspendTime) = 0;
    virtual void cancelCallback(UIntN participantIndex) = 0;
    virtual Bool hasCallbackWithinTimeRange(UIntN participantIndex, UInt64 beginTime, UInt64 endTime) const = 0;
    virtual void acknowledgeCallback(UIntN participantIndex) = 0;
    virtual void setTimeObject(std::shared_ptr<TimeInterface> time) = 0;
    virtual XmlNode* getStatus() = 0;
    virtual XmlNode* getStatusForParticipant(UIntN participantIndex) = 0;
};