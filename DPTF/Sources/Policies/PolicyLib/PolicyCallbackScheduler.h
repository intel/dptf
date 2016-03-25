/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

    PolicyCallbackScheduler(const PolicyServicesInterfaceContainer& policyServicesContainer,
        std::shared_ptr<TimeInterface> timeInterface);
    virtual ~PolicyCallbackScheduler();

    virtual void suspend(UIntN participantIndex, const TimeSpan& time) override;
    virtual void suspend(UIntN participantIndex, UInt64 fromTime, const TimeSpan& suspendTime) override;
    virtual void cancelCallback(UIntN participantIndex) override;
    virtual Bool hasCallbackWithinTimeRange(UIntN participantIndex, UInt64 beginTime, UInt64 endTime) const override;
    virtual void acknowledgeCallback(UIntN participantIndex) override;
    virtual void setTimeObject(std::shared_ptr<TimeInterface> time) override;
    virtual std::shared_ptr<XmlNode> getStatus() override;
    virtual std::shared_ptr<XmlNode> getStatusForParticipant(UIntN participantIndex) override;

private:

    PolicyServicesInterfaceContainer m_policyServices;
    std::shared_ptr<TimeInterface> m_time;
    std::map<UIntN, ParticipantCallback> m_schedule;

    UInt64 getCurrentTime();
    void scheduleCallback(UIntN participantIndex, UInt64 currentTime, const TimeSpan& pollTime);
};