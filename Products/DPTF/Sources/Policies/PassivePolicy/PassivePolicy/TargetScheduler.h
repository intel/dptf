/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "PolicyServicesInterfaceContainer.h"
#include "TargetCallback.h"
#include <memory>

// responsible for scheduling callbacks for targets
class dptf_export TargetScheduler
{
public:

    TargetScheduler(const PolicyServicesInterfaceContainer& policyServices, std::shared_ptr<TimeInterface> time);
    ~TargetScheduler();

    // making and removing callbacks
    void scheduleCallback(UIntN target, UInt64 time, UInt64 timeDelta);
    void cancelCallback(UIntN target);
    void removeCallback(UIntN target);

    // callback queries
    Bool hasCallbackScheduledAtOrBeforeTime(UIntN target, UInt64 time) const;
    Bool hasCallbackWithinTimeRange(UIntN target, UInt64 beginTime, UInt64 endTime) const;
    
    // update services
    void setTimeObject(std::shared_ptr<TimeInterface> time);

    // status
    XmlNode* getXml() const;
    
private:

    // services
    std::shared_ptr<TimeInterface> m_time;
    PolicyServicesInterfaceContainer m_policyServices;

    // scheduled callbacks
    std::map<UIntN, TargetCallback> m_schedule;
};