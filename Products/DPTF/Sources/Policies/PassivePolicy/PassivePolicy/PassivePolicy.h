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
#include "PolicyBase.h"
#include "ParticipantTracker.h"
#include "ThermalRelationshipTable.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "CallbackScheduler.h"
#include "DptfTime.h"
#include "TargetMonitor.h"
#include "TargetActionBase.h"

class dptf_export PassivePolicy final : public PolicyBase
{
public:

    PassivePolicy(void);
    virtual ~PassivePolicy(void);

    virtual void onCreate(void) override;
    virtual void onDestroy(void) override;
    virtual void onEnable(void) override;
    virtual void onDisable(void) override;
    virtual void onConnectedStandbyEntry(void) override;
    virtual void onConnectedStandbyExit(void) override;

    virtual bool autoNotifyPlatformOscOnCreateDestroy() const override;
    virtual bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const override;
    virtual bool autoNotifyPlatformOscOnEnableDisable() const override;

    virtual Guid getGuid(void) const override;
    virtual std::string getName(void) const override;
    virtual std::string getStatusAsXml(void) const override;

    virtual void onBindParticipant(UIntN participantIndex) override;
    virtual void onUnbindParticipant(UIntN participantIndex) override;
    virtual void onBindDomain(UIntN participantIndex, UIntN domainIndex) override;
    virtual void onUnbindDomain(UIntN participantIndex, UIntN domainIndex) override;
    virtual void onDomainTemperatureThresholdCrossed(UIntN participantIndex) override;
    virtual void onParticipantSpecificInfoChanged(UIntN participantIndex) override;
    virtual void onDomainPowerControlCapabilityChanged(UIntN participantIndex) override;
    virtual void onDomainPerformanceControlCapabilityChanged(UIntN participantIndex) override;
    virtual void onDomainPerformanceControlsChanged(UIntN participantIndex) override;
    virtual void onDomainCoreControlCapabilityChanged(UIntN participantIndex) override;
    virtual void onDomainPriorityChanged(UIntN participantIndex) override;
    virtual void onDomainDisplayControlCapabilityChanged(UIntN participantIndex) override;
    virtual void onDomainDisplayStatusChanged(UIntN participantIndex) override;
    virtual void onThermalRelationshipTableChanged(void) override;
    virtual void onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2) override;
    virtual void onOverrideTimeObject(std::shared_ptr<TimeInterface> timeObject) override;

private:
    
    // policy state
    mutable ThermalRelationshipTable m_trt;
    std::shared_ptr<CallbackScheduler> m_callbackScheduler;
    TargetMonitor m_targetMonitor;
    UtilizationStatus m_utilizationBiasThreshold;

    // thermal action decisions
    TargetActionBase* determineAction(UIntN target);
    void takeThermalActionForAllTargetsForSource(UIntN source);
    void takeThermalActionForTarget(UIntN target);
    
    // TRT actions
    void associateParticipantInTrt(ParticipantProxy& trackedParticipants);
    void reloadTrtAndCheckAllTargets();

    // temperature notification actions
    void notifyPlatformOfDeviceTemperature(ParticipantProxy& participant, UIntN currentTemperature);
    void setParticipantTemperatureThresholdNotification(
        ParticipantProxy& participant,
        Temperature currentTemperature);

    // testing participant qualifications
    Bool participantIsSourceOrTargetDevice(UIntN participantIndex) const;
    Bool participantIsSourceDevice(UIntN participantIndex) const;
    Bool participantIsTargetDevice(UIntN participantIndex) const;

    // status
    XmlNode* getXmlForPassiveTripPoints() const;
};