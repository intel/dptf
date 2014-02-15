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

#include "PolicyBase.h"
#include "DptfTime.h"
using namespace std;

PolicyBase::PolicyBase(void)
    : m_enabled(false)
{
    m_time.reset(new DptfTime());
    m_trackedParticipants.setTimeServiceObject(m_time);
}

PolicyBase::~PolicyBase(void)
{
}

// If a policy chooses not to load itself, it should throw out of its onCreate() function.
void PolicyBase::create(Bool enabled, PolicyServicesInterfaceContainer policyServices, UIntN policyIndex)
{
    m_enabled = enabled;
    m_policyServices = policyServices;
    m_trackedParticipants.setPolicyServices(policyServices);
    throwIfPolicyRequirementsNotMet();
    takeControlOfOsc(m_enabled && autoNotifyPlatformOscOnCreateDestroy());

    try
    {
        onCreate();
    }
    catch (...)
    {
        releaseControlofOsc(m_enabled && autoNotifyPlatformOscOnCreateDestroy());
        m_enabled = false;
        throw;
    }
}

void PolicyBase::destroy(void)
{
    try
    {
        onDestroy();
    }
    catch (...)
    {
        releaseControlofOsc(autoNotifyPlatformOscOnCreateDestroy());
        m_enabled = false;
        throw;
    }

    releaseControlofOsc(autoNotifyPlatformOscOnCreateDestroy());
    m_enabled = false;
}

void PolicyBase::enable(void)
{
    takeControlOfOsc(autoNotifyPlatformOscOnEnableDisable());
    try
    {
        onEnable();
        m_enabled = true;
    }
    catch (...)
    {
        releaseControlofOsc(autoNotifyPlatformOscOnEnableDisable());
        throw;
    }
}

void PolicyBase::disable(void)
{
    try
    {
        onDisable();
    }
    catch (...)
    {
        releaseControlofOsc(autoNotifyPlatformOscOnEnableDisable());
        throw;
    }

    releaseControlofOsc(autoNotifyPlatformOscOnEnableDisable());
    m_enabled = false;
}

void PolicyBase::bindParticipant(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onBindParticipant(participantIndex);
}

void PolicyBase::unbindParticipant(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onUnbindParticipant(participantIndex);
}

void PolicyBase::bindDomain(UIntN participantIndex, UIntN domainIndex)
{
    throwIfPolicyIsDisabled();
    onBindDomain(participantIndex, domainIndex);
}

void PolicyBase::unbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    throwIfPolicyIsDisabled();
    onUnbindDomain(participantIndex, domainIndex);
}

void PolicyBase::domainTemperatureThresholdCrossed(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainTemperatureThresholdCrossed(participantIndex);
}

void PolicyBase::domainPowerControlCapabilityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPowerControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPerformanceControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainPerformanceControlsChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPerformanceControlsChanged(participantIndex);
}

void PolicyBase::domainCoreControlCapabilityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainCoreControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainConfigTdpCapabilityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainConfigTdpCapabilityChanged(participantIndex);
}

void PolicyBase::domainPriorityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPriorityChanged(participantIndex);
}

void PolicyBase::domainDisplayControlCapabilityChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainDisplayControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainDisplayStatusChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainDisplayStatusChanged(participantIndex);
}

void PolicyBase::domainRadioConnectionStatusChanged(UIntN participantIndex,
    RadioConnectionStatus::Type radioConnectionStatus)
{
    throwIfPolicyIsDisabled();
    onDomainRadioConnectionStatusChanged(participantIndex, radioConnectionStatus);
}

void PolicyBase::domainRfProfileChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainRfProfileChanged(participantIndex);
}

void PolicyBase::participantSpecificInfoChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onParticipantSpecificInfoChanged(participantIndex);
}

void PolicyBase::activeRelationshipTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onActiveRelationshipTableChanged();
}

void PolicyBase::thermalRelationshipTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onThermalRelationshipTableChanged();
}

void PolicyBase::connectedStandbyEntry(void)
{
    throwIfPolicyIsDisabled();
    try
    {
        if (autoNotifyPlatformOscOnConnectedStandbyEntryExit())
        {
            takeControlOfOsc(autoNotifyPlatformOscOnConnectedStandbyEntryExit());
        }
        onConnectedStandbyEntry();
    }
    catch (...)
    {
        releaseControlofOsc(autoNotifyPlatformOscOnConnectedStandbyEntryExit());
        throw;
    }
}

void PolicyBase::connectedStandbyExit(void)
{
    throwIfPolicyIsDisabled();
    try
    {
        onConnectedStandbyExit();
        if (autoNotifyPlatformOscOnConnectedStandbyEntryExit())
        {
            releaseControlofOsc(autoNotifyPlatformOscOnConnectedStandbyEntryExit());
        }
    }
    catch (...)
    {
        releaseControlofOsc(autoNotifyPlatformOscOnConnectedStandbyEntryExit());
        throw;
    }
}

void PolicyBase::foregroundApplicationChanged(const std::string& foregroundApplicationName)
{
    throwIfPolicyIsDisabled();
    onForegroundApplicationChanged(foregroundApplicationName);
}

void PolicyBase::policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
    throwIfPolicyIsDisabled();
    onPolicyInitiatedCallback(policyDefinedEventCode, param1, param2);
}

void PolicyBase::operatingSystemLpmModeChanged(UIntN lpmMode)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemLpmModeChanged(lpmMode);
}

void PolicyBase::platformLpmModeChanged()
{
    throwIfPolicyIsDisabled();
    onPlatformLpmModeChanged();
}

void PolicyBase::operatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemConfigTdpLevelChanged(configTdpLevel);
}

void PolicyBase::coolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit)
{
    throwIfPolicyIsDisabled();
    onCoolingModePowerLimitChanged(powerLimit);
}

void PolicyBase::coolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit)
{
    throwIfPolicyIsDisabled();
    onCoolingModeAcousticLimitChanged(acousticLimit);
}

void PolicyBase::coolingModePolicyChanged(CoolingMode::Type coolingMode)
{
    throwIfPolicyIsDisabled();
    onCoolingModePolicyChanged(coolingMode);
}

void PolicyBase::passiveTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onPassiveTableChanged();
}

void PolicyBase::sensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
    throwIfPolicyIsDisabled();
    onSensorOrientationChanged(sensorOrientation);
}

void PolicyBase::sensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
    throwIfPolicyIsDisabled();
    onSensorSpatialOrientationChanged(sensorSpatialOrientation);
}

void PolicyBase::sensorProximityChanged(SensorProximity::Type sensorProximity)
{
    throwIfPolicyIsDisabled();
    onSensorProximityChanged(sensorProximity);
}

void PolicyBase::overrideTimeObject(std::shared_ptr<TimeInterface> timeObject)
{
    UInt32 enabled;
    try
    {
        enabled = getPolicyServices().platformConfigurationData->readConfigurationUInt32("EnablePolicyDebugInterface");
    }
    catch (...)
    {
        // assume enable not set
        enabled = 0;
    }

    if (enabled > 0)
    {
        m_time = timeObject;
        m_trackedParticipants.setTimeServiceObject(timeObject);
        onOverrideTimeObject(timeObject);
    }
}

void PolicyBase::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPerformanceControlsChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainConfigTdpCapabilityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPriorityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainDisplayControlCapabilityChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainDisplayStatusChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainRadioConnectionStatusChanged(UIntN participantIndex,
    RadioConnectionStatus::Type radioConnectionStatus)
{
    throw not_implemented();
}

void PolicyBase::onDomainRfProfileChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onActiveRelationshipTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onThermalRelationshipTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onConnectedStandbyEntry(void)
{
    throw not_implemented();
}

void PolicyBase::onConnectedStandbyExit(void)
{
    throw not_implemented();
}

void PolicyBase::onForegroundApplicationChanged(const std::string& foregroundApplicationName)
{
    throw not_implemented();
}

void PolicyBase::onPolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemLpmModeChanged(UIntN lpmMode)
{
    throw not_implemented();
}

void PolicyBase::onPlatformLpmModeChanged()
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
    throw not_implemented();
}

void PolicyBase::onCoolingModePowerLimitChanged(CoolingModePowerLimit::Type powerLimit)
{
    throw not_implemented();
}

void PolicyBase::onCoolingModeAcousticLimitChanged(CoolingModeAcousticLimit::Type acousticLimit)
{
    throw not_implemented();
}

void PolicyBase::onCoolingModePolicyChanged(CoolingMode::Type coolingMode)
{
    throw not_implemented();
}

void PolicyBase::onPassiveTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onSensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
    throw not_implemented();
}

void PolicyBase::onSensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
    throw not_implemented();
}

void PolicyBase::onSensorProximityChanged(SensorProximity::Type sensorProximity)
{
    throw not_implemented();
}

void PolicyBase::onOverrideTimeObject(std::shared_ptr<TimeInterface> timeObject)
{
    // optional to implement
}

PolicyServicesInterfaceContainer& PolicyBase::getPolicyServices() const
{
    return m_policyServices;
}

ParticipantTracker& PolicyBase::getParticipantTracker() const
{
    return m_trackedParticipants;
}

std::shared_ptr<TimeInterface>& PolicyBase::getTime() const
{
    return m_time;
}

void PolicyBase::throwIfPolicyRequirementsNotMet()
{
    if ((m_policyServices.platformNotification == nullptr) ||
        (m_policyServices.platformConfigurationData == nullptr))
    {
        throw dptf_exception("Policy Services does not have an implementation \
                              for platformConfigurationData or platformNotification \
                              interfaces.");
    }
}

void PolicyBase::throwIfPolicyIsDisabled()
{
    if (m_enabled == false)
    {
        throw dptf_exception("The policy has been disabled.");
    }
}

void PolicyBase::takeControlOfOsc(bool shouldTakeControl)
{
    if (shouldTakeControl)
    {
        try
        {
            m_policyServices.platformNotification->notifyPlatformPolicyTakeControl();
        }
        catch (...)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, "Failed to acquire OSC."));
        }
    }
}

void PolicyBase::releaseControlofOsc(bool shouldReleaseControl)
{
    if (shouldReleaseControl)
    {
        try
        {
            m_policyServices.platformNotification->notifyPlatformPolicyReleaseControl();
        }
        catch (...)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, "Failed to release OSC."));
        }
    }
}