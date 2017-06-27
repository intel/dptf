/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
void PolicyBase::create(Bool enabled, const PolicyServicesInterfaceContainer& policyServices, UIntN policyIndex)
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
		m_policyServices.messageLogging->writeMessageInfo(
			PolicyMessage(FLF, getName() + ": Policy enable event received."));
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
		m_policyServices.messageLogging->writeMessageInfo(
			PolicyMessage(FLF, getName() + ": Policy disable event received."));
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
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Binding participant.", participantIndex));
	onBindParticipant(participantIndex);
}

void PolicyBase::unbindParticipant(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Unbinding participant.", participantIndex));
	onUnbindParticipant(participantIndex);
}

void PolicyBase::bindDomain(UIntN participantIndex, UIntN domainIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Binding domain for participant.", participantIndex, domainIndex));
	onBindDomain(participantIndex, domainIndex);
}

void PolicyBase::unbindDomain(UIntN participantIndex, UIntN domainIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Unbinding domain for participant.", participantIndex, domainIndex));
	onUnbindDomain(participantIndex, domainIndex);
}

void PolicyBase::domainTemperatureThresholdCrossed(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Temperature threshold crossed for participant.", participantIndex));
	onDomainTemperatureThresholdCrossed(participantIndex);
}

void PolicyBase::domainPowerControlCapabilityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Power Control Capabilities Changed for participant.", participantIndex));
	onDomainPowerControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": Performance Control Capabilities Changed for participant.", participantIndex));
	onDomainPerformanceControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainPerformanceControlsChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Performance control set changed for participant.", participantIndex));
	onDomainPerformanceControlsChanged(participantIndex);
}

void PolicyBase::domainCoreControlCapabilityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Core control capabilities changed for participant.", participantIndex));
	onDomainCoreControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainConfigTdpCapabilityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Config TDP Capabilities Changed for participant.", participantIndex));
	onDomainConfigTdpCapabilityChanged(participantIndex);
}

void PolicyBase::domainPriorityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Domain priority changed for participant.", participantIndex));
	onDomainPriorityChanged(participantIndex);
}

void PolicyBase::domainDisplayControlCapabilityChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Display control capabilities changed for participant.", participantIndex));
	onDomainDisplayControlCapabilityChanged(participantIndex);
}

void PolicyBase::domainDisplayStatusChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Display status changed for participant.", participantIndex));
	onDomainDisplayStatusChanged(participantIndex);
}

void PolicyBase::domainRadioConnectionStatusChanged(
	UIntN participantIndex,
	RadioConnectionStatus::Type radioConnectionStatus)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF,
		getName() + ": Radio Connection Status Changed to " + RadioConnectionStatus::ToString(radioConnectionStatus)
			+ ".",
		participantIndex));
	onDomainRadioConnectionStatusChanged(participantIndex, radioConnectionStatus);
}

void PolicyBase::domainRfProfileChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": RF Profile Changed for participant.", participantIndex));
	onDomainRfProfileChanged(participantIndex);
}

void PolicyBase::participantSpecificInfoChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Specific info changed for participant.", participantIndex));
	onParticipantSpecificInfoChanged(participantIndex);
}

void PolicyBase::domainVirtualSensorCalibrationTableChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": VSCT changed for participant.", participantIndex));
	onDomainVirtualSensorCalibrationTableChanged(participantIndex);
}

void PolicyBase::domainVirtualSensorPollingTableChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": VSPT changed for participant.", participantIndex));
	onDomainVirtualSensorPollingTableChanged(participantIndex);
}

void PolicyBase::domainVirtualSensorRecalcChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Virtual Sensor recalculation requested for participant.", participantIndex));
	onDomainVirtualSensorRecalcChanged(participantIndex);
}

void PolicyBase::domainBatteryStatusChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Battery status changed for participant.", participantIndex));
	onDomainBatteryStatusChanged(participantIndex);
}

void PolicyBase::domainBatteryInformationChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Battery information changed for participant.", participantIndex));
	onDomainBatteryInformationChanged(participantIndex);
}

void PolicyBase::domainPlatformPowerSourceChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Platform power source changed."));
	onDomainPlatformPowerSourceChanged(participantIndex);
}

void PolicyBase::domainAdapterPowerRatingChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Adapter power rating changed."));
	onDomainAdapterPowerRatingChanged(participantIndex);
}

void PolicyBase::domainChargerTypeChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": Charger type changed."));
	onDomainChargerTypeChanged(participantIndex);
}

void PolicyBase::domainPlatformRestOfPowerChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Platform rest of power changed."));
	onDomainPlatformRestOfPowerChanged(participantIndex);
}

void PolicyBase::domainMaxBatteryPowerChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": Max battery power changed."));
	onDomainMaxBatteryPowerChanged(participantIndex);
}

void PolicyBase::domainPlatformBatterySteadyStateChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Platform Battery Steady State changed."));
	onDomainPlatformBatterySteadyStateChanged(participantIndex);
}

void PolicyBase::domainACNominalVoltageChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": AC Nominal Voltage changed."));
	onDomainACNominalVoltageChanged(participantIndex);
}

void PolicyBase::domainACOperationalCurrentChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": AC Operational Current changed."));
	onDomainACOperationalCurrentChanged(participantIndex);
}

void PolicyBase::domainAC1msPercentageOverloadChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": AC 1ms Percentage Overload changed."));
	onDomainAC1msPercentageOverloadChanged(participantIndex);
}

void PolicyBase::domainAC2msPercentageOverloadChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": AC 2ms Percentage Overload changed."));
	onDomainAC2msPercentageOverloadChanged(participantIndex);
}

void PolicyBase::domainAC10msPercentageOverloadChanged(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": AC 10ms Percentage Overload changed."));
	onDomainAC10msPercentageOverloadChanged(participantIndex);
}

void PolicyBase::activeRelationshipTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Active Relationship Table changed."));
	onActiveRelationshipTableChanged();
}

void PolicyBase::domainEnergyThresholdCrossed(UIntN participantIndex)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Energy threshold crossed for participant.", participantIndex));
	onDomainEnergyThresholdCrossed(participantIndex);
}

void PolicyBase::thermalRelationshipTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Thermal Relationship Table changed"));
	onThermalRelationshipTableChanged();
}

void PolicyBase::adaptivePerformanceConditionsTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Adaptive Performance Conditions Table changed."));
	onAdaptivePerformanceConditionsTableChanged();
}

void PolicyBase::adaptivePerformanceParticipantConditionTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Adaptive Performance Participant Condition Table changed."));
	onAdaptivePerformanceParticipantConditionTableChanged();
}

void PolicyBase::adaptivePerformanceActionsTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Adaptive Performance Actions Table changed."));
	onAdaptivePerformanceActionsTableChanged();
}

void PolicyBase::pidAlgorithmTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": PID Algorithm Table changed."));
	onPidAlgorithmTableChanged();
}

void PolicyBase::activeControlPointRelationshipTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Active Control Point Relationship Table changed."));
	onActiveControlPointRelationshipTableChanged();
}

void PolicyBase::powerShareAlgorithmTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Power Share Algorithm Table changed."));
	onPowerShareAlgorithmTableChanged();
}

void PolicyBase::workloadHintConfigurationChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Workload Hint Configuration changed."));
	onWorkloadHintConfigurationChanged();
}

void PolicyBase::connectedStandbyEntry(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Connected standby entry event received."));
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
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Connected standby exit event received."));
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
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Policy suspend event received."));
	onSuspend();
}

void PolicyBase::resume(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Policy resume event received."));
	onResume();
}

void PolicyBase::foregroundApplicationChanged(const std::string& foregroundApplicationName)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Foreground application changed to " + foregroundApplicationName + "."));
	onForegroundApplicationChanged(foregroundApplicationName);
}

void PolicyBase::policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": Policy Initiated Callback."));
	onPolicyInitiatedCallback(policyDefinedEventCode, param1, param2);
}

void PolicyBase::operatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": Config TDP Level Changed to index " + std::to_string(configTdpLevel) + "."));
	onOperatingSystemConfigTdpLevelChanged(configTdpLevel);
}

void PolicyBase::operatingSystemPowerSourceChanged(OsPowerSource::Type powerSource)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": OS Power Source changed to " + OsPowerSource::toString(powerSource) + "."));
	onOperatingSystemPowerSourceChanged(powerSource);
}

void PolicyBase::operatingSystemLidStateChanged(OsLidState::Type lidState)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": OS Lid state changed to " + OsLidState::toString(lidState) + "."));
	onOperatingSystemLidStateChanged(lidState);
}

void PolicyBase::operatingSystemBatteryPercentageChanged(UIntN batteryPercentage)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": OS battery percentage changed to " + std::to_string(batteryPercentage) + "."));
	onOperatingSystemBatteryPercentageChanged(batteryPercentage);
}

void PolicyBase::operatingSystemPowerSchemePersonalityChanged(OsPowerSchemePersonality::Type powerSchemePersosnality)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF,
		getName() + ": OS Power Scheme Personality changed to "
			+ OsPowerSchemePersonality::toString(powerSchemePersosnality)
			+ "."));
	onOperatingSystemPowerSchemePersonalityChanged(powerSchemePersosnality);
}

void PolicyBase::operatingSystemPlatformTypeChanged(OsPlatformType::Type platformType)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": OS Platform Type changed to " + OsPlatformType::toString(platformType) + "."));
	onOperatingSystemPlatformTypeChanged(platformType);
}

void PolicyBase::operatingSystemDockModeChanged(OsDockMode::Type dockMode)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": OS Dock Mode changed to " + OsDockMode::toString(dockMode) + "."));
	onOperatingSystemDockModeChanged(dockMode);
}

void PolicyBase::operatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF,
		getName() + ": OS Emergency Call Mode State changed to " + OnOffToggle::toString(emergencyCallModeState)
			+ "."));
	onOperatingSystemEmergencyCallModeChanged(emergencyCallModeState);
}

void PolicyBase::operatingSystemMobileNotification(OsMobileNotificationType::Type notificationType, UIntN value)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF,
		getName() + ": OS Mobile Notification for " + OsMobileNotificationType::ToString(notificationType)
			+ " changed to "
			+ std::to_string(value)
			+ "."));
	onOperatingSystemMobileNotification(notificationType, value);
}

void PolicyBase::coolingModePolicyChanged(CoolingMode::Type coolingMode)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Cooling mode changed to " + CoolingMode::toString(coolingMode) + "."));
	onCoolingModePolicyChanged(coolingMode);
}

void PolicyBase::passiveTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": Passive Table changed."));
	onPassiveTableChanged();
}

void PolicyBase::sensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": Sensor orientation changed to " + SensorOrientation::toString(sensorOrientation) + "."));
	onSensorOrientationChanged(sensorOrientation);
}

void PolicyBase::sensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF,
		getName() + ": Sensor spatial orientation changed to "
			+ SensorSpatialOrientation::toString(sensorSpatialOrientation)
			+ "."));
	onSensorSpatialOrientationChanged(sensorSpatialOrientation);
}

void PolicyBase::sensorMotionChanged(OnOffToggle::Type sensorMotion)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(
		FLF, getName() + ": Sensor motion state changed to " + OnOffToggle::toString(sensorMotion) + "."));
	onSensorMotionChanged(sensorMotion);
}

void PolicyBase::oemVariablesChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": OEM variable(s) changed."));
	onOemVariablesChanged();
}

void PolicyBase::powerBossConditionsTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Power Boss Conditions Table changed."));
	onPowerBossConditionsTableChanged();
}

void PolicyBase::powerBossActionsTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Power Boss Actions Table changed."));
	onPowerBossActionsTableChanged();
}

void PolicyBase::powerBossMathTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Power Boss Math Table changed."));
	onPowerBossMathTableChanged();
}

void PolicyBase::emergencyCallModeTableChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(
		PolicyMessage(FLF, getName() + ": Emergency Call Mode Table changed."));
	onEmergencyCallModeTableChanged();
}

void PolicyBase::overrideTimeObject(std::shared_ptr<TimeInterface> timeObject)
{
	m_time = timeObject;
	m_trackedParticipants->setTimeServiceObject(m_time);
	onOverrideTimeObject(timeObject);
}

void PolicyBase::powerLimitChanged(void)
{
	throwIfPolicyIsDisabled();
	m_policyServices.messageLogging->writeMessageInfo(PolicyMessage(FLF, getName() + ": Power Limit Changed."));
	onPowerLimitChanged();
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

void PolicyBase::onDomainRadioConnectionStatusChanged(
	UIntN participantIndex,
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

void PolicyBase::onDomainBatteryInformationChanged(UIntN participantIndex)
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

void PolicyBase::onDomainMaxBatteryPowerChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainPlatformBatterySteadyStateChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainACNominalVoltageChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainACOperationalCurrentChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainAC1msPercentageOverloadChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainAC2msPercentageOverloadChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainAC10msPercentageOverloadChanged(UIntN participantIndex)
{
	throw not_implemented();
}

void PolicyBase::onDomainEnergyThresholdCrossed(UIntN participantIndex)
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

void PolicyBase::onAdaptivePerformanceParticipantConditionTableChanged(void)
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

void PolicyBase::onOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
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

void PolicyBase::onOperatingSystemPowerSchemePersonalityChanged(OsPowerSchemePersonality::Type powerSchemePersonality)
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

void PolicyBase::onOperatingSystemEmergencyCallModeChanged(OnOffToggle::Type emergencyCallMode)
{
	throw not_implemented();
}

void PolicyBase::onOperatingSystemMobileNotification(UIntN mobileNotificationType, UIntN value)
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

void PolicyBase::onSensorMotionChanged(OnOffToggle::Type sensorMotion)
{
	throw not_implemented();
}

void PolicyBase::onOemVariablesChanged(void)
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

void PolicyBase::onPowerBossMathTableChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onEmergencyCallModeTableChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onPidAlgorithmTableChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onActiveControlPointRelationshipTableChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onPowerShareAlgorithmTableChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onWorkloadHintConfigurationChanged(void)
{
	throw not_implemented();
}

void PolicyBase::onPowerLimitChanged(void)
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
	if ((m_policyServices.platformNotification == nullptr) || (m_policyServices.platformConfigurationData == nullptr))
	{
		throw dptf_exception(
			"Policy Services does not have an implementation \
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

void PolicyBase::takeControlOfOsc(Bool shouldTakeControl)
{
	if (shouldTakeControl)
	{
		try
		{
			m_policyServices.platformNotification->notifyPlatformPolicyTakeControl();
		}
		catch (std::exception& ex)
		{
			m_policyServices.messageLogging->writeMessageError(
				PolicyMessage(FLF, getName() + ": Failed to acquire OSC: " + string(ex.what())));
		}
		catch (...)
		{
			m_policyServices.messageLogging->writeMessageError(
				PolicyMessage(FLF, getName() + ": Failed to acquire OSC."));
		}
	}
}

void PolicyBase::releaseControlofOsc(Bool shouldReleaseControl)
{
	if (shouldReleaseControl)
	{
		try
		{
			m_policyServices.platformNotification->notifyPlatformPolicyReleaseControl();
		}
		catch (std::exception& ex)
		{
			m_policyServices.messageLogging->writeMessageError(
				PolicyMessage(FLF, getName() + ": Failed to release OSC: " + string(ex.what())));
		}
		catch (...)
		{
			m_policyServices.messageLogging->writeMessageError(
				PolicyMessage(FLF, getName() + ": Failed to release OSC."));
		}
	}
}

std::shared_ptr<XmlNode> PolicyBase::getXmlForTripPointStatistics(std::set<UIntN> targetIndexes) const
{
	auto status = XmlNode::createWrapperElement("trip_point_statistics");

	for (auto targetIndex = targetIndexes.begin(); targetIndex != targetIndexes.end(); ++targetIndex)
	{
		if (*targetIndex != Constants::Invalid)
		{
			auto participant = getParticipantTracker()->getParticipant(*targetIndex);
			status->addChild(participant->getXmlForTripPointStatistics());
		}
	}

	return status;
}
