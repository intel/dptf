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

#include "Dptf.h"
#include "TargetActionBase.h"
#include <tuple>

// implements the unlimiting algorithm for passive policy
class dptf_export TargetUnlimitAction : public TargetActionBase
{
public:

    TargetUnlimitAction(
        PolicyServicesInterfaceContainer& policyServices, 
        std::shared_ptr<TimeInterface> time,
        std::shared_ptr<ParticipantTrackerInterface> participantTracker,
        std::shared_ptr<ThermalRelationshipTable> trt,
        std::shared_ptr<CallbackScheduler> callbackScheduler,
        TargetMonitor& targetMonitor,
        UIntN target);
    virtual ~TargetUnlimitAction();

    virtual void execute() override;
    
private:
    
    // source filtering
    std::vector<UIntN> chooseSourcesToUnlimitForTarget(UIntN target);
    std::vector< std::shared_ptr<ThermalRelationshipTableEntry>> getEntriesWithControlsToUnlimit(
        UIntN target, const std::vector< std::shared_ptr<ThermalRelationshipTableEntry>>& sourcesForTarget);

    // domain filtering
    std::vector<UIntN> getDomainsWithControlKnobsToUnlimit(ParticipantProxyInterface* participant, UIntN target);
    std::vector<UIntN> chooseDomainsToUnlimitForSource(UIntN target, UIntN source);
    UIntN getDomainWithLowestTemperature(UIntN source, std::vector<UIntN> domainsWithControlKnobsToTurn);
    std::vector<UIntN> getDomainsWithLowestPriority(UIntN source, std::vector<UIntN> domains);

    // domain unlimiting
    void requestUnlimit(UIntN source, UIntN domain, UIntN target);
    void commitUnlimit(UIntN source, const TimeSpan& time);
    void removeAllRequestsForTarget(UIntN target);
};