/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "PolicyServicesInterfaceContainer.h"
#include "ParticipantTrackerInterface.h"
#include "PolicyLogger.h"
#include "StatusFormat.h"

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
	virtual Guid getGuid() const override = 0;
	virtual std::string getName() const override = 0;
	virtual std::string getStatusAsXml(void) const override = 0;
	virtual std::string getDiagnosticsAsXml(void) const override = 0;
	virtual Bool autoNotifyPlatformOscOnCreateDestroy() const = 0;
	virtual Bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const = 0;
	virtual Bool autoNotifyPlatformOscOnEnableDisable() const = 0;

	// Optional policy properties
	virtual Bool hasActiveControlCapability() const;
	virtual Bool hasPassiveControlCapability() const;
	virtual Bool hasCriticalShutdownCapability() const;

	// Optional events
	virtual void onDomainTemperatureThresholdCrossed(UIntN participantIndex);
	virtual void onDomainPowerControlCapabilityChanged(UIntN participantIndex);
	virtual void onDomainPerformanceControlCapabilityChanged(UIntN participantIndex);
	virtual void onDomainPerformanceControlsChanged(UIntN participantIndex);
	virtual void onDomainCoreControlCapabilityChanged(UIntN participantIndex);
	virtual void onDomainPriorityChanged(UIntN participantIndex);
	virtual void onDomainDisplayControlCapabilityChanged(UIntN participantIndex);
	virtual void onDomainDisplayStatusChanged(UIntN participantIndex);
	virtual void onDomainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus);
	virtual void onDomainRfProfileChanged(UIntN participantIndex);
	virtual void onParticipantSpecificInfoChanged(UIntN participantIndex);
	virtual void onDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex);
	virtual void onDomainVirtualSensorPollingTableChanged(UIntN participantIndex);
	virtual void onDomainVirtualSensorRecalcChanged(UIntN participantIndex);
	virtual void onDomainBatteryStatusChanged(UIntN participantIndex);
	virtual void onDomainBatteryInformationChanged(UIntN participantIndex);
	virtual void onDomainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex);
	virtual void onDomainBatteryNoLoadVoltageChanged(UIntN participantIndex);
	virtual void onDomainMaxBatteryPeakCurrentChanged(UIntN participantIndex);
	virtual void onDomainPlatformPowerSourceChanged(UIntN participantIndex);
	virtual void onDomainAdapterPowerRatingChanged(UIntN participantIndex);
	virtual void onDomainChargerTypeChanged(UIntN participantIndex);
	virtual void onDomainPlatformRestOfPowerChanged(UIntN participantIndex);
	virtual void onDomainMaxBatteryPowerChanged(UIntN participantIndex);
	virtual void onDomainPlatformBatterySteadyStateChanged(UIntN participantIndex);
	virtual void onDomainACNominalVoltageChanged(UIntN participantIndex);
	virtual void onDomainACOperationalCurrentChanged(UIntN participantIndex);
	virtual void onDomainAC1msPercentageOverloadChanged(UIntN participantIndex);
	virtual void onDomainAC2msPercentageOverloadChanged(UIntN participantIndex);
	virtual void onDomainAC10msPercentageOverloadChanged(UIntN participantIndex);
	virtual void onDomainEnergyThresholdCrossed(UIntN participantIndex);
	virtual void onDomainFanCapabilityChanged(UIntN participantIndex);
	virtual void onDomainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification);
	virtual void onDomainEppSensitivityHintChanged(UIntN participantIndex, UIntN domainIndex, MbtHint::Type mbtHint);
	virtual void onActiveRelationshipTableChanged(void);
	virtual void onThermalRelationshipTableChanged(void);
	virtual void onAdaptivePerformanceConditionsTableChanged(void);
	virtual void onAdaptivePerformanceActionsTableChanged(void);
	virtual void onDdrfTableChanged(void);
	virtual void onIgccBroadcastReceived(IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData);
	virtual void onConnectedStandbyEntry(void);
	virtual void onConnectedStandbyExit(void);
	virtual void onSuspend(void);
	virtual void onResume(void);
	virtual void onForegroundApplicationChanged(const std::string& foregroundApplicationName);
	virtual void onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2);
	virtual void onOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource);
	virtual void onOperatingSystemLidStateChanged(OsLidState::Type lidState);
	virtual void onOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage);
	virtual void onOperatingSystemPowerSchemePersonalityChanged(OsPowerSchemePersonality::Type powerSchemePersonality);
	virtual void onOperatingSystemPlatformTypeChanged(OsPlatformType::Type platformType);
	virtual void onOperatingSystemDockModeChanged(OsDockMode::Type dockMode);
	virtual void onOperatingSystemEmergencyCallModeChanged(OnOffToggle::Type emergencyCallMode);
	virtual void onOperatingSystemMobileNotification(UIntN mobileNotificationType, UIntN value);
	virtual void onOperatingSystemMixedRealityModeChanged(OnOffToggle::Type mixedRealityMode);
	virtual void onOperatingSystemUserPresenceChanged(OsUserPresence::Type userPresence);
	virtual void onOperatingSystemSessionStateChanged(OsSessionState::Type sessionState);
	virtual void onOperatingSystemScreenStateChanged(OnOffToggle::Type screenState);
	virtual void onOperatingSystemBatteryCountChanged(UIntN batteryCount);
	virtual void onOperatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider);
	virtual void onSystemModeChanged(SystemMode::Type systemMode);
	virtual void onCoolingModePolicyChanged(CoolingMode::Type coolingMode);
	virtual void onTpgaTableChanged(void);
	virtual void onPassiveTableChanged(void);
	virtual void onSensorOrientationChanged(SensorOrientation::Type sensorOrientation);
	virtual void onSensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation);
	virtual void onSensorMotionChanged(OnOffToggle::Type sensorMotion);
	virtual void onOverrideTimeObject(std::shared_ptr<TimeInterface> timeObject);
	virtual void onOemVariablesChanged(void);
	virtual void onPowerBossConditionsTableChanged(void);
	virtual void onPowerBossActionsTableChanged(void);
	virtual void onPowerBossMathTableChanged(void);
	virtual void onVoltageThresholdMathTableChanged(void);
	virtual void onEmergencyCallModeTableChanged(void);
	virtual void onPidAlgorithmTableChanged(void);
	virtual void onActiveControlPointRelationshipTableChanged(void);
	virtual void onPowerShareAlgorithmTableChanged(void);
	virtual void onPowerLimitChanged(void);
	virtual void onPerformanceCapabilitiesChanged(UIntN participantIndex);
	virtual void onWorkloadHintConfigurationChanged(void);
	virtual void onOperatingSystemGameModeChanged(OnOffToggle::Type gameMode);
	virtual void onPowerShareAlgorithmTable2Changed(void);
	virtual void onIntelligentThermalManagementTableChanged(void);
	virtual void onEnergyPerformanceOptimizerTableChanged(void);
	virtual void onPlatformUserPresenceChanged(SensorUserPresence::Type userPresence);
	virtual void onExternalMonitorStateChanged(Bool externalMonitorState);
	virtual void onUserInteractionChanged(UserInteraction::Type userInteraction);
	virtual void onForegroundRatioChanged(UIntN ratio);
	virtual void onCollaborationChanged(OnOffToggle::Type collaborationstate);
	virtual void onThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff);

	// Implementation of the Policy Interface
	virtual void create(
		Bool enabled,
		const PolicyServicesInterfaceContainer& policyServices,
		UIntN policyIndex,
		const std::string& dynamicPolicyUuid = Constants::EmptyString,
		const std::string& dynamicPolicyName = Constants::EmptyString) override final;
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
	virtual void domainPriorityChanged(UIntN participantIndex) override final;
	virtual void domainDisplayControlCapabilityChanged(UIntN participantIndex) override final;
	virtual void domainDisplayStatusChanged(UIntN participantIndex) override final;
	virtual void domainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) override final;
	virtual void domainRfProfileChanged(UIntN participantIndex) override final;
	virtual void participantSpecificInfoChanged(UIntN participantIndex) override final;
	virtual void domainVirtualSensorCalibrationTableChanged(UIntN participantIndex) override final;
	virtual void domainVirtualSensorPollingTableChanged(UIntN participantIndex) override final;
	virtual void domainVirtualSensorRecalcChanged(UIntN participantIndex) override final;
	virtual void domainBatteryStatusChanged(UIntN participantIndex) override final;
	virtual void domainBatteryInformationChanged(UIntN participantIndex) override final;
	virtual void domainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) override final;
	virtual void domainBatteryNoLoadVoltageChanged(UIntN participantIndex) override final;
	virtual void domainMaxBatteryPeakCurrentChanged(UIntN participantIndex) override final;
	virtual void domainPlatformPowerSourceChanged(UIntN participantIndex) override final;
	virtual void domainAdapterPowerRatingChanged(UIntN participantIndex) override final;
	virtual void domainChargerTypeChanged(UIntN participantIndex) override final;
	virtual void domainPlatformRestOfPowerChanged(UIntN participantIndex) override final;
	virtual void domainMaxBatteryPowerChanged(UIntN participantIndex) override final;
	virtual void domainPlatformBatterySteadyStateChanged(UIntN participantIndex) override final;
	virtual void domainACNominalVoltageChanged(UIntN participantIndex) override final;
	virtual void domainACOperationalCurrentChanged(UIntN participantIndex) override final;
	virtual void domainAC1msPercentageOverloadChanged(UIntN participantIndex) override final;
	virtual void domainAC2msPercentageOverloadChanged(UIntN participantIndex) override final;
	virtual void domainAC10msPercentageOverloadChanged(UIntN participantIndex) override final;
	virtual void domainEnergyThresholdCrossed(UIntN participantIndex) override final;
	virtual void domainFanCapabilityChanged(UIntN participantIndex) override final;
	virtual void domainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification) override final;
	virtual void domainEppSensitivityHintChanged(UIntN participantIndex, UIntN domainIndex, MbtHint::Type mbtHint)
		override final;
	virtual void activeRelationshipTableChanged(void) override final;
	virtual void thermalRelationshipTableChanged(void) override final;
	virtual void adaptivePerformanceConditionsTableChanged(void) override final;
	virtual void ddrfTableChanged(void) override final;
	virtual void adaptivePerformanceActionsTableChanged(void) override final;
	virtual void connectedStandbyEntry(void) override final;
	virtual void igccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) override final;
	virtual void connectedStandbyExit(void) override final;
	virtual void suspend(void) override final;
	virtual void resume(void) override final;
	virtual void foregroundApplicationChanged(const std::string& foregroundApplicationName) override final;
	virtual void policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) override final;
	virtual void operatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) override final;
	virtual void operatingSystemLidStateChanged(OsLidState::Type lidState) override final;
	virtual void operatingSystemBatteryPercentageChanged(UIntN batteryPercentage) override final;
	virtual void operatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemaPersonality) override final;
	virtual void operatingSystemPlatformTypeChanged(OsPlatformType::Type platformType) override final;
	virtual void operatingSystemDockModeChanged(OsDockMode::Type dockMode) override final;
	virtual void operatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState) override final;
	virtual void operatingSystemMobileNotification(OsMobileNotificationType::Type notificationType, UIntN value)
		override final;
	virtual void operatingSystemMixedRealityModeChanged(OnOffToggle::Type mixedRealityMode) override final;
	virtual void operatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) override final;
	virtual void operatingSystemSessionStateChanged(OsSessionState::Type sessionState) override final;
	virtual void operatingSystemScreenStateChanged(OnOffToggle::Type screenState) override final;
	virtual void operatingSystemBatteryCountChanged(UIntN batteryCount) override final;
	virtual void operatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) override final;
	virtual void systemModeChanged(SystemMode::Type powerSlider) override final;
	virtual void coolingModePolicyChanged(CoolingMode::Type coolingMode) override final;
	virtual void passiveTableChanged(void) override final;
	virtual void tpgaTableChanged(void) override final;
	virtual void sensorOrientationChanged(SensorOrientation::Type sensorOrientation) override final;
	virtual void sensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) override final;
	virtual void sensorMotionChanged(OnOffToggle::Type sensorMotion) override final;
	virtual void oemVariablesChanged(void) override final;
	virtual void powerBossConditionsTableChanged(void) override final;
	virtual void powerBossActionsTableChanged(void) override final;
	virtual void powerBossMathTableChanged(void) override final;
	virtual void voltageThresholdMathTableChanged(void) override final;
	virtual void emergencyCallModeTableChanged(void) override final;
	virtual void pidAlgorithmTableChanged(void) override final;
	virtual void activeControlPointRelationshipTableChanged(void) override final;
	virtual void powerShareAlgorithmTableChanged(void) override final;
	virtual void intelligentThermalManagementTableChanged(void) override final;
	virtual void energyPerformanceOptimizerTableChanged(void) override final;
	virtual void powerLimitChanged(void) override final;
	virtual void performanceCapabilitiesChanged(UIntN participantIndex) override final;
	virtual void workloadHintConfigurationChanged(void) override final;
	virtual void operatingSystemGameModeChanged(OnOffToggle::Type osGameMode) override final;
	virtual void powerShareAlgorithmTable2Changed(void) override final;
	virtual void platformUserPresenceChanged(SensorUserPresence::Type userPresence) override final;
	virtual void externalMonitorStateChanged(Bool externalMonitorState) override final;
	virtual void userInteractionChanged(UserInteraction::Type userInteraction) override final;
	virtual void foregroundRatioChanged(UIntN ratio) override final;
	virtual void collaborationChanged(OnOffToggle::Type collaborationstate) override final;
	virtual void thirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) override final;

	// allows overriding the default time object with a different one
	void overrideTimeObject(std::shared_ptr<TimeInterface> timeObject);

	// trip point statistics
	std::shared_ptr<XmlNode> getXmlForTripPointStatistics(std::set<UIntN> targetIndexes) const;

protected:
	// policy state access for subclasses
	std::shared_ptr<ParticipantTrackerInterface> getParticipantTracker() const;

	// service access for subclasses
	PolicyServicesInterfaceContainer& getPolicyServices() const;
	std::shared_ptr<TimeInterface>& getTime() const;

	std::string m_dynamicPolicyUuidString;
	std::string m_dynamicPolicyName;

private:
	// policy state
	Bool m_enabled;
	mutable std::shared_ptr<ParticipantTrackerInterface> m_trackedParticipants;

	// policy services
	mutable PolicyServicesInterfaceContainer m_policyServices;
	mutable std::shared_ptr<TimeInterface> m_time;

	// sets/updates OSC on behalf of policy
	void sendOscRequest(Bool shouldSendOscRequest, Bool isPolicyEnabled);
	void updateOscRequestIfNeeded(
		Bool hasActiveControlCapabilityLastSet = false,
		Bool hasPassiveControlCapabilityLastSet = false,
		Bool hasCriticalShutdownCapabilityLastSet = false);

	// checks for errors and throws an exception
	void throwIfPolicyRequirementsNotMet();
	void throwIfPolicyIsDisabled();
};
