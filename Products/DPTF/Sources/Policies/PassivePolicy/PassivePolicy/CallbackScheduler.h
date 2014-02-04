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
#include "ThermalRelationshipTable.h"
#include "TimeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include <memory>
#include <map>
#include <set>
#include "SourceAvailability.h"
#include "TargetScheduler.h"

// responsible for determining when to schedule callbacks for targets based on sampling intervals in the TRT
class dptf_export CallbackScheduler
{
public:

    CallbackScheduler(
        const PolicyServicesInterfaceContainer& policyServices,
        const ThermalRelationshipTable& trt,
        std::shared_ptr<TimeInterface> time);
    ~CallbackScheduler();

    // scheduling callbacks
    void scheduleCallbackAsSoonAsPossible(UIntN target, UIntN source);
    void scheduleCallbackAfterShortestSamplePeriod(UIntN target);
    void scheduleCallbackAfterNextSamplingPeriod(UIntN target, UIntN source);
    void acknowledgeCallback(UIntN target);

    // participant availability
    void removeParticipantFromSchedule(UIntN participant);
    Bool isSourceBusyNow(UIntN sourceIndex);

    // updates service objects
    void setTrt(const ThermalRelationshipTable& trt);
    void setTimeObject(std::shared_ptr<TimeInterface> time);

    // status
    XmlNode* getXml() const;

private:

    // participant availability
    SourceAvailability m_sourceAvailability;
    TargetScheduler m_targetScheduler;

    // services
    ThermalRelationshipTable m_trt;
    std::shared_ptr<TimeInterface> m_time;
    PolicyServicesInterfaceContainer m_policyServices;
};