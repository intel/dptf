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
#include "PolicyInterface.h"
#include "DptfExceptions.h"
#include "PolicyServicesInterfaceContainer.h"
#include "ParticipantTracker.h"

class dptf_export PolicyBase : public PolicyInterface
{
public:

    PolicyBase(void);
    virtual ~PolicyBase(void);

    // Required policy events
    virtual void onCreate(void) = 0;
    virtual void onDestroy(void) = 0;
    virtual void onEnable(void) = 0;
    virtual void onDisable(void) = 0;
    virtual void onBindParticipant(UIntN participantIndex) = 0;
    virtual void onUnbindParticipant(UIntN participantIndex) = 0;
    virtual void onBindDomain(UIntN participantIndex, UIntN domainIndex) = 0;
    virtual void onUnbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

    // Required policy properties
    virtual Guid getGuid() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getStatusAsXml(void) const = 0;
    virtual Bool autoNotifyPlatformOscOnCreateDestroy() const = 0;
    virtual Bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const = 0;
    virtual Bool autoNotifyPlatformOscOnEnableDisable() const = 0;

    // Optional events
    virtual void onDomainTemperatureThresholdCrossed(UIntN participantIndex);
    virtual void onDomainPowerControlCapabilityChanged(UIntN participantIndex);
    virtual void onDomainPerformanceControlCapabilityChanged(UIntN participantIndex);
    virtual void onDomainPerformanceControlsChanged(UIntN participantIndex);
    virtual void onDomainCoreControlCapabilityChanged(UIntN participantIndex);
    virtual void onDomainConfigTdpCapabilityChanged(UIntN participantIndex);
    virtual void onDomainPriorityChanged(UIntN participantIndex);
    virtual void onDomainDisplayControlCapabilityChanged(UIntN participantIndex);
    virtual void onDomainDisplayStatusChanged(UIntN participantIndex);
    virtual void onDomainRadioConnectionStatusChanged(UIntN participantIndex,
        RadioConnectionStatus::Type radioConnectionStatus);
    virtual void onDomainRfProfileChanged(UIntN participantIndex);
    virtual void onParticipantSpecificInfoChanged(UIntN participantIndex);
    virtual void onActiveRelationshipTableChanged(void);
    virtual void onThermalRelationshipTableChanged(void);
    virtual void onConnectedStandbyEntry(void);
    virtual void onConnectedStandbyExit(void);
    virtual void onForegroundApplicationChanged(const std::string& foregroundApplicationName);
    virtual void onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2);
    virtual void onOperatingSystemLpmModeChanged(UIntN lpmMode);
    virtual void onPlatformLpmModeChanged(void);
    virtual void onOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel);
    virtual void onCoolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit);
    virtual void onCoolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit);
    virtual void onCoolingModePolicyChanged(CoolingMode::Type coolingMode);
    virtual void onPassiveTableChanged(void);
    virtual void onSensorOrientationChanged(SensorOrientation::Type sensorOrientation);
    virtual void onSensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation);
    virtual void onSensorProximityChanged(SensorProximity::Type sensorProximity);
    virtual void onOverrideTimeObject(std::shared_ptr<TimeInterface> timeObject);

    // Implementation of the Policy Interface
    virtual void create(Bool enabled, PolicyServicesInterfaceContainer policyServices, UIntN policyIndex) override final;
    virtual void destroy(void) override final;
    virtual void enable(void) override final;
    virtual void disable(void) override final;
    virtual void bindParticipant(UIntN participantIndex) override final;
    virtual void unbindParticipant(UIntN participantIndex) override final;
    virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void domainTemperatureThresholdCrossed(UIntN participantIndex) override final;
    virtual void domainPowerControlCapabilityChanged(UIntN participantIndex) override final;
    virtual void domainPerformanceControlCapabilityChanged(UIntN participantIndex) override final;
    virtual void domainPerformanceControlsChanged(UIntN participantIndex) override final;
    virtual void domainCoreControlCapabilityChanged(UIntN participantIndex) override final;
    virtual void domainConfigTdpCapabilityChanged(UIntN participantIndex) override final;
    virtual void domainPriorityChanged(UIntN participantIndex) override final;
    virtual void domainDisplayControlCapabilityChanged(UIntN participantIndex) override final;
    virtual void domainDisplayStatusChanged(UIntN participantIndex) override final;
    virtual void domainRadioConnectionStatusChanged(UIntN participantIndex,
        RadioConnectionStatus::Type radioConnectionStatus) override final;
    virtual void domainRfProfileChanged(UIntN participantIndex) override final;
    virtual void participantSpecificInfoChanged(UIntN participantIndex) override final;
    virtual void activeRelationshipTableChanged(void) override final;
    virtual void thermalRelationshipTableChanged(void) override final;
    virtual void connectedStandbyEntry(void) override final;
    virtual void connectedStandbyExit(void) override final;
    virtual void foregroundApplicationChanged(const std::string& foregroundApplicationName) override final;
    virtual void policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) override final;
    virtual void operatingSystemLpmModeChanged(UIntN lpmMode) override final;
    virtual void platformLpmModeChanged(void) override final;
    virtual void operatingSystemConfigTdpLevelChanged(UIntN configTdpLevel) override final;
    virtual void coolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit) override final;
    virtual void coolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit) override final;
    virtual void coolingModePolicyChanged(CoolingMode::Type coolingMode) override final;
    virtual void passiveTableChanged(void) override final;
    virtual void sensorOrientationChanged(SensorOrientation::Type sensorOrientation) override final;
    virtual void sensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation) override final;
    virtual void sensorProximityChanged(SensorProximity::Type sensorProximity) override final;

    // allows overriding the default time object with a different one
    void overrideTimeObject(std::shared_ptr<TimeInterface> timeObject);

protected:

    // policy state access for subclasses
    ParticipantTracker& getParticipantTracker() const;

    // service access for subclasses
    PolicyServicesInterfaceContainer& getPolicyServices() const;
    std::shared_ptr<TimeInterface>& getTime() const;

private:

    // policy state
    Bool m_enabled;
    mutable ParticipantTracker m_trackedParticipants;

    // policy services
    mutable PolicyServicesInterfaceContainer m_policyServices;
    mutable std::shared_ptr<TimeInterface> m_time;

    // acquires/releases OSC on behalf of policy
    void takeControlOfOsc(Bool shouldTakeControl);
    void releaseControlofOsc(Bool shouldReleaseControl);

    // checks for errors and throws an exception
    void throwIfPolicyRequirementsNotMet();
    void throwIfPolicyIsDisabled();
};