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

#include "PolicyBase.h"
#include "DptfTime.h"
#include "ParticipantTracker.h"
using namespace std;

PolicyBase::PolicyBase(void)
    : m_enabled(false)
{
    m_time.reset(new DptfTime());
    m_trackedParticipants.reset(new ParticipantTracker());
    m_trackedParticipants->setTimeServiceObject(m_time);
}

PolicyBase::~PolicyBase(void)
{
}

// If a policy chooses not to load itself, it should throw out of its onCreate() function.
void PolicyBase::create(Bool enabled, PolicyServicesInterfaceContainer policyServices, UIntN policyIndex)
{
    m_enabled = enabled;
    m_policyServices = policyServices;
    m_trackedParticipants->setPolicyServices(policyServices);
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

void PolicyBase::domainVirtualSensorCalibrationTableChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainVirtualSensorCalibrationTableChanged(participantIndex);
}

void PolicyBase::domainVirtualSensorPollingTableChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainVirtualSensorPollingTableChanged(participantIndex);
}

void PolicyBase::domainVirtualSensorRecalcChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainVirtualSensorRecalcChanged(participantIndex);
}

void PolicyBase::domainBatteryStatusChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainBatteryStatusChanged(participantIndex);
}

void PolicyBase::domainAdapterPowerChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainAdapterPowerChanged(participantIndex);
}

void PolicyBase::domainPlatformPowerConsumptionChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPlatformPowerConsumptionChanged(participantIndex);
}

void PolicyBase::domainPlatformPowerSourceChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPlatformPowerSourceChanged(participantIndex);
}

void PolicyBase::domainAdapterPowerRatingChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainAdapterPowerRatingChanged(participantIndex);
}

void PolicyBase::domainChargerTypeChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainChargerTypeChanged(participantIndex);
}

void PolicyBase::domainPlatformRestOfPowerChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainPlatformRestOfPowerChanged(participantIndex);
}

void PolicyBase::domainACPeakPowerChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainACPeakPowerChanged(participantIndex);
}

void PolicyBase::domainACPeakTimeWindowChanged(UIntN participantIndex)
{
    throwIfPolicyIsDisabled();
    onDomainACPeakTimeWindowChanged(participantIndex);
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

void PolicyBase::adaptivePerformanceConditionsTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onAdaptivePerformanceConditionsTableChanged();
}

void PolicyBase::adaptivePerformanceActionsTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onAdaptivePerformanceActionsTableChanged();
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

void PolicyBase::suspend(void)
{
    throwIfPolicyIsDisabled();
    onSuspend();
}

void PolicyBase::resume(void)
{
    throwIfPolicyIsDisabled();
    onResume();
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

void PolicyBase::operatingSystemHdcStatusChanged(OsHdcStatus::Type status)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemHdcStatusChanged(status);
}

void PolicyBase::operatingSystemPowerSourceChanged(OsPowerSource::Type powerSource)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemPowerSourceChanged(powerSource);
}

void PolicyBase::operatingSystemLidStateChanged(OsLidState::Type lidState)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemLidStateChanged(lidState);
}

void PolicyBase::operatingSystemBatteryPercentageChanged(UIntN batteryPercentage)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemBatteryPercentageChanged(batteryPercentage);
}

void PolicyBase::operatingSystemPlatformTypeChanged(OsPlatformType::Type platformType)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemPlatformTypeChanged(platformType);
}

void PolicyBase::operatingSystemDockModeChanged(OsDockMode::Type dockMode)
{
    throwIfPolicyIsDisabled();
    onOperatingSystemDockModeChanged(dockMode);
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

void PolicyBase::sensorMotionChanged(SensorMotion::Type sensorMotion)
{
    throwIfPolicyIsDisabled();
    onSensorMotionChanged(sensorMotion);
}

void PolicyBase::oemVariablesChanged(void)
{
    throwIfPolicyIsDisabled();
    onOemVariablesChanged();
}

void PolicyBase::powerDeviceRelationshipTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onPowerDeviceRelationshipTableChanged();
}

void PolicyBase::powerBossConditionsTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onPowerBossConditionsTableChanged();
}

void PolicyBase::powerBossActionsTableChanged(void)
{
    throwIfPolicyIsDisabled();
    onPowerBossActionsTableChanged();
}

void PolicyBase::overrideTimeObject(std::shared_ptr<TimeInterface> timeObject)
{
    m_time = timeObject;
    m_trackedParticipants->setTimeServiceObject(m_time);
    onOverrideTimeObject(timeObject);
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

void PolicyBase::onDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainVirtualSensorPollingTableChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainVirtualSensorRecalcChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainBatteryStatusChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainAdapterPowerChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPlatformPowerConsumptionChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPlatformPowerSourceChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainAdapterPowerRatingChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainChargerTypeChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainPlatformRestOfPowerChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainACPeakPowerChanged(UIntN participantIndex)
{
    throw not_implemented();
}

void PolicyBase::onDomainACPeakTimeWindowChanged(UIntN participantIndex)
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

void PolicyBase::onAdaptivePerformanceConditionsTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onAdaptivePerformanceActionsTableChanged(void)
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

void PolicyBase::onSuspend(void)
{
    throw not_implemented();
}

void PolicyBase::onResume(void)
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

void PolicyBase::onOperatingSystemHdcStatusChanged(OsHdcStatus::Type status)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemLidStateChanged(OsLidState::Type lidState)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemPlatformTypeChanged(OsPlatformType::Type platformType)
{
    throw not_implemented();
}

void PolicyBase::onOperatingSystemDockModeChanged(OsDockMode::Type dockMode)
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

void PolicyBase::onSensorMotionChanged(SensorMotion::Type sensorMotion)
{
    throw not_implemented();
}

void PolicyBase::onOemVariablesChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onPowerDeviceRelationshipTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onPowerBossConditionsTableChanged(void)
{
    throw not_implemented();
}

void PolicyBase::onPowerBossActionsTableChanged(void)
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

std::shared_ptr<ParticipantTrackerInterface> PolicyBase::getParticipantTracker() const
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
        catch (std::exception ex)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, "Failed to acquire OSC: " +
                string(ex.what())));
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
        catch (std::exception ex)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, "Failed to release OSC: " +
                string(ex.what())));
        }
        catch (...)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, "Failed to release OSC."));
        }
    }
}