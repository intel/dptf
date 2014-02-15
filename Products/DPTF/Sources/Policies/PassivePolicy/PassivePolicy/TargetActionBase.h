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
#include "PolicyServicesInterfaceContainer.h"
#include <memory>
#include "TimeInterface.h"
#include "ParticipantTracker.h"
#include "CallbackScheduler.h"
#include "TargetMonitor.h"

// represents the base class for all passive policy actions
class dptf_export TargetActionBase
{
public:

    TargetActionBase(
        PolicyServicesInterfaceContainer& policyServices, 
        std::shared_ptr<TimeInterface> time,
        ParticipantTracker& participantTracker, 
        ThermalRelationshipTable& trt,
        std::shared_ptr<CallbackScheduler> callbackScheduler,
        TargetMonitor& targetMonitor,
        UIntN target);
    virtual ~TargetActionBase();

    virtual void execute() = 0;

protected:

    // policy state access
    PolicyServicesInterfaceContainer getPolicyServices() const;
    std::shared_ptr<TimeInterface> getTime() const;
    ParticipantTracker& getParticipantTracker() const;
    ThermalRelationshipTable& getTrt() const;
    std::shared_ptr<CallbackScheduler> getCallbackScheduler() const;
    TargetMonitor& getTargetMonitor() const;
    UIntN getTarget() const;

    // messaging
    std::string constructMessageForSources(
        std::string actionName, UIntN target, const std::vector<UIntN>& sources);
    std::string constructMessageForSourceDomains(
        std::string actionName, UIntN target, UIntN source, const std::vector<UIntN>& domains);

    // domain selection
    std::vector<UIntN> getPackageDomains(UIntN source, const std::vector<UIntN>& domainsWithControlKnobsToTurn);
    std::vector<UIntN> filterDomainList(
        const std::vector<UIntN>& domainList, const std::vector<UIntN>& filterList) const;
    std::vector<UIntN> getDomainsThatDoNotReportTemperature(UIntN source, std::vector<UIntN> domains);

    // comparisons
    static Bool compareTrtTableEntriesOnInfluence(
        const ThermalRelationshipTableEntry& left, 
        const ThermalRelationshipTableEntry& right);
    static Bool compareDomainsOnPriorityAndUtilization(
        const std::tuple<UIntN, DomainPriority, UtilizationStatus>& left, 
        const std::tuple<UIntN, DomainPriority, UtilizationStatus>& right);

private:

    TargetActionBase(TargetActionBase& lhs);
    TargetActionBase& operator=(TargetActionBase& lhs);

    // action state
    std::shared_ptr<TimeInterface> m_time;
    PolicyServicesInterfaceContainer& m_policyServices;
    ParticipantTracker& m_participantTracker;
    ThermalRelationshipTable& m_trt;
    std::shared_ptr<CallbackScheduler> m_callbackScheduler;
    TargetMonitor& m_targetMonitor;
    UIntN m_target;
};