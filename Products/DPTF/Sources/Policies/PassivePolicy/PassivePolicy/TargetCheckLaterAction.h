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
#include "TargetActionBase.h"
#include <tuple>

// implements the algorithm for limiting in the passive policy
class dptf_export TargetCheckLaterAction : public TargetActionBase
{
public:

    TargetCheckLaterAction(
        PolicyServicesInterfaceContainer& policyServices, 
        std::shared_ptr<TimeInterface> time,
        ParticipantTracker& participantTracker, 
        ThermalRelationshipTable& trt,
        std::shared_ptr<CallbackScheduler> callbackScheduler,
        TargetMonitor& targetMonitor,
        UtilizationStatus utilizationBiasThreshold,
        UIntN target);
    virtual ~TargetCheckLaterAction();

    virtual void execute() override;

};