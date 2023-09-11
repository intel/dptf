/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
	PolicyBase();
	~PolicyBase() override = default;

	// Required policy events
	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
	virtual void onEnable() = 0;
	virtual void onDisable() = 0;
	virtual void onBindParticipant(UIntN participantIndex) = 0;
	virtual void onUnbindParticipant(UIntN participantIndex) = 0;
	virtual void onBindDomain(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void onUnbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

	// Required policy properties
	Guid getGuid() const override = 0;
	std::string getName() const override = 0;
	std::string getStatusAsXml() const override = 0;
	std::string getDiagnosticsAsXml() const override = 0;
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
	virtual void onDomainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor);
	virtual void onDomainEppSensitivityHintChanged(UIntN participantIndex, UIntN domainIndex, MbtHint::Type mbtHint);
	virtual void onDomainExtendedWorkloadPredictionChanged(UIntN participantIndex, UIntN domainIndex, ExtendedWorkloadPrediction::Type extendedWorkloadPrediction);
	virtual void onDomainFanOperatingModeChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		FanOperatingMode::Type fanOperatingMode);
	virtual void onDomainPcieThrottleRequested(
		UIntN participantIndex,
		UIntN domainIndex,
		OnOffToggle::Type pcieThrottleRequested);
	virtual void onActiveRelationshipTableChanged();
	virtual void onThermalRelationshipTableChanged();
	virtual void onAdaptivePerformanceConditionsTableChanged();
	virtual void onAdaptivePerformanceActionsTableChanged();
	virtual void onDdrfTableChanged();
	virtual void onRfimTableChanged();
	virtual void onIgccBroadcastReceived(IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData);
	virtual void onEnvironmentProfileChanged(const EnvironmentProfile& environmentProfile);
	virtual void onConnectedStandbyEntry();
	virtual void onConnectedStandbyExit();
	virtual void onLowPowerModeEntry();
	virtual void onLowPowerModeExit();
	virtual void onSuspend();
	virtual void onResume();
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
	virtual void onProcessLoaded(const std::string& processName);
	virtual void onSystemModeChanged(SystemMode::Type systemMode);
	virtual void onCoolingModePolicyChanged(CoolingMode::Type coolingMode);
	virtual void onTpgaTableChanged();
	virtual void onPassiveTableChanged();
	virtual void onSensorOrientationChanged(SensorOrientation::Type sensorOrientation);
	virtual void onSensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation);
	virtual void onSensorMotionChanged(OnOffToggle::Type sensorMotion);
	virtual void onOverrideTimeObject(const std::shared_ptr<TimeInterface>& timeObject);
	virtual void onOemVariablesChanged();
	virtual void onSwOemVariablesChanged();
	virtual void onPowerBossConditionsTableChanged();
	virtual void onPowerBossActionsTableChanged();
	virtual void onPowerBossMathTableChanged();
	virtual void onVoltageThresholdMathTableChanged();
	virtual void onEmergencyCallModeTableChanged();
	virtual void onPidAlgorithmTableChanged();
	virtual void onActiveControlPointRelationshipTableChanged();
	virtual void onPowerShareAlgorithmTableChanged();
	virtual void onPowerLimitChanged();
	virtual void onPowerLimitTimeWindowChanged();
	virtual void onPerformanceCapabilitiesChanged(UIntN participantIndex);
	virtual void onWorkloadHintConfigurationChanged();
	virtual void onOperatingSystemGameModeChanged(OnOffToggle::Type gameMode);
	virtual void onPowerShareAlgorithmTable2Changed();
	virtual void onIntelligentThermalManagementTableChanged();
	virtual void onEnergyPerformanceOptimizerTableChanged();
	virtual void onPlatformUserPresenceChanged(SensorUserPresence::Type userPresence);
	virtual void onExternalMonitorStateChanged(Bool externalMonitorState);
	virtual void onUserInteractionChanged(UserInteraction::Type userInteraction);
	virtual void onForegroundRatioChanged(UIntN ratio);
	virtual void onCollaborationModeChanged(OnOffToggle::Type collaborationModeState);
	virtual void onThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff);
	virtual void onThirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP);

	// Implementation of the Policy Interface
	void create(
		Bool enabled,
		const PolicyServicesInterfaceContainer& policyServices,
		UIntN policyIndex,
		const std::string& dynamicPolicyUuid = Constants::EmptyString,
		const std::string& dynamicPolicyName = Constants::EmptyString) final;
	void destroy() final;
	void enable() final;
	void disable() final;
	void bindParticipant(UIntN participantIndex) final;
	void unbindParticipant(UIntN participantIndex) final;
	void bindDomain(UIntN participantIndex, UIntN domainIndex) final;
	void unbindDomain(UIntN participantIndex, UIntN domainIndex) final;
	void domainTemperatureThresholdCrossed(UIntN participantIndex) final;
	void domainPowerControlCapabilityChanged(UIntN participantIndex) final;
	void domainPerformanceControlCapabilityChanged(UIntN participantIndex) final;
	void domainPerformanceControlsChanged(UIntN participantIndex) final;
	void domainCoreControlCapabilityChanged(UIntN participantIndex) final;
	void domainPriorityChanged(UIntN participantIndex) final;
	void domainDisplayControlCapabilityChanged(UIntN participantIndex) final;
	void domainDisplayStatusChanged(UIntN participantIndex) final;
	void domainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) final;
	void domainRfProfileChanged(UIntN participantIndex) final;
	void participantSpecificInfoChanged(UIntN participantIndex) final;
	void domainVirtualSensorCalibrationTableChanged(UIntN participantIndex) final;
	void domainVirtualSensorPollingTableChanged(UIntN participantIndex) final;
	void domainVirtualSensorRecalcChanged(UIntN participantIndex) final;
	void domainBatteryStatusChanged(UIntN participantIndex) final;
	void domainBatteryInformationChanged(UIntN participantIndex) final;
	void domainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) final;
	void domainBatteryNoLoadVoltageChanged(UIntN participantIndex) final;
	void domainMaxBatteryPeakCurrentChanged(UIntN participantIndex) final;
	void domainPlatformPowerSourceChanged(UIntN participantIndex) final;
	void domainAdapterPowerRatingChanged(UIntN participantIndex) final;
	void domainChargerTypeChanged(UIntN participantIndex) final;
	void domainPlatformRestOfPowerChanged(UIntN participantIndex) final;
	void domainMaxBatteryPowerChanged(UIntN participantIndex) final;
	void domainPlatformBatterySteadyStateChanged(UIntN participantIndex) final;
	void domainACNominalVoltageChanged(UIntN participantIndex) final;
	void domainACOperationalCurrentChanged(UIntN participantIndex) final;
	void domainAC1msPercentageOverloadChanged(UIntN participantIndex) final;
	void domainAC2msPercentageOverloadChanged(UIntN participantIndex) final;
	void domainAC10msPercentageOverloadChanged(UIntN participantIndex) final;
	void domainEnergyThresholdCrossed(UIntN participantIndex) final;
	void domainFanCapabilityChanged(UIntN participantIndex) final;
	void domainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification) final;
	void domainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor) final;
	void domainEppSensitivityHintChanged(UIntN participantIndex, UIntN domainIndex, MbtHint::Type mbtHint)
		final;
	void domainExtendedWorkloadPredictionChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		ExtendedWorkloadPrediction::Type extendedWorkloadPrediction)
	final;
	void domainFanOperatingModeChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		FanOperatingMode::Type fanOperatingMode) final;
	void domainPcieThrottleRequested(
		UIntN participantIndex,
		UIntN domainIndex,
		OnOffToggle::Type pcieThrottleRequested) final;
	void activeRelationshipTableChanged() final;
	void thermalRelationshipTableChanged() final;
	void adaptivePerformanceConditionsTableChanged() final;
	void ddrfTableChanged() final;
	void rfimTableChanged() final;
	void adaptivePerformanceActionsTableChanged() final;
	void connectedStandbyEntry() final;
	void igccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) final;
	void environmentProfileChanged(const EnvironmentProfile& environmentProfile) final;
	void connectedStandbyExit() final;
	void lowPowerModeEntry() final;
	void lowPowerModeExit() final;
	void suspend() final;
	void resume() final;
	void foregroundApplicationChanged(const std::string& foregroundApplicationName) final;
	void policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) final;
	void operatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) final;
	void operatingSystemLidStateChanged(OsLidState::Type lidState) final;
	void operatingSystemBatteryPercentageChanged(UIntN batteryPercentage) final;
	void operatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality) final;
	void operatingSystemPlatformTypeChanged(OsPlatformType::Type platformType) final;
	void operatingSystemDockModeChanged(OsDockMode::Type dockMode) final;
	void operatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState) final;
	void operatingSystemMobileNotification(OsMobileNotificationType::Type notificationType, UIntN value)
		final;
	void operatingSystemMixedRealityModeChanged(OnOffToggle::Type mixedRealityMode) final;
	void operatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) final;
	void operatingSystemSessionStateChanged(OsSessionState::Type sessionState) final;
	void operatingSystemScreenStateChanged(OnOffToggle::Type screenState) final;
	void operatingSystemBatteryCountChanged(UIntN batteryCount) final;
	void operatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) final;
	void processLoaded(const std::string& processName) final;
	void systemModeChanged(SystemMode::Type systemMode) final;
	void coolingModePolicyChanged(CoolingMode::Type coolingMode) final;
	void passiveTableChanged() final;
	void tpgaTableChanged() final;
	void sensorOrientationChanged(SensorOrientation::Type sensorOrientation) final;
	void sensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) final;
	void sensorMotionChanged(OnOffToggle::Type sensorMotion) final;
	void oemVariablesChanged() final;
	void swOemVariablesChanged() final;
	void powerBossConditionsTableChanged() final;
	void powerBossActionsTableChanged() final;
	void powerBossMathTableChanged() final;
	void voltageThresholdMathTableChanged() final;
	void emergencyCallModeTableChanged() final;
	void pidAlgorithmTableChanged() final;
	void activeControlPointRelationshipTableChanged() final;
	void powerShareAlgorithmTableChanged() final;
	void intelligentThermalManagementTableChanged() final;
	void energyPerformanceOptimizerTableChanged() final;
	void powerLimitChanged() final;
	void powerLimitTimeWindowChanged() final;
	void performanceCapabilitiesChanged(UIntN participantIndex) final;
	void workloadHintConfigurationChanged() final;
	void operatingSystemGameModeChanged(OnOffToggle::Type osGameMode) final;
	void powerShareAlgorithmTable2Changed() final;
	void platformUserPresenceChanged(SensorUserPresence::Type userPresence) final;
	void externalMonitorStateChanged(Bool externalMonitorState) final;
	void userInteractionChanged(UserInteraction::Type userInteraction) final;
	void foregroundRatioChanged(UIntN ratio) final;
	void collaborationModeChanged(OnOffToggle::Type collaborationModeState) final;
	void thirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) final;
	void thirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type) final;

	// allows overriding the default time object with a different one
	void overrideTimeObject(const std::shared_ptr<TimeInterface>& timeObject);

	// trip point statistics
	std::shared_ptr<XmlNode> getXmlForTripPointStatistics(const std::set<UIntN>& targetIndexes) const;

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
	void sendOscRequest(Bool shouldSendOscRequest, Bool isPolicyEnabled) const;
	void updateOscRequestIfNeeded(
		Bool hasActiveControlCapabilityLastSet = false,
		Bool hasPassiveControlCapabilityLastSet = false,
		Bool hasCriticalShutdownCapabilityLastSet = false) const;

	// checks for errors and throws an exception
	void throwIfPolicyRequirementsNotMet() const;
	void throwIfPolicyIsDisabled() const;
};
