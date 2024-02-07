/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "SupportedPolicyList.h"
#include "PolicyInterface.h"
#include "EsifLibrary.h"

class DptfManager;

class dptf_export IPolicy
{
public:
	IPolicy() = default;
	IPolicy(const IPolicy& other) = delete;
	IPolicy(IPolicy&& other) noexcept = delete;
	IPolicy& operator=(const IPolicy& other) = delete;
	IPolicy& operator=(IPolicy&& other) noexcept = delete;
	virtual ~IPolicy() = default;

	virtual void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) = 0;
	virtual void destroyPolicy() = 0;

	virtual Guid getGuid() = 0;

	virtual void bindParticipant(UIntN participantIndex) = 0;
	virtual void unbindParticipant(UIntN participantIndex) = 0;
	virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;

	virtual std::string getName() const = 0;
	virtual std::string getPolicyFileName() const = 0;
	virtual UIntN getPolicyIndex() const = 0;
	virtual std::string getStatusAsXml() const = 0;
	virtual std::string getDiagnosticsAsXml() const = 0;
	virtual std::map<std::string, std::string> getPolicyStateLogData() const = 0;
	virtual std::string getConfigurationForExport() const = 0;

	virtual Bool isDynamicPolicy() = 0;
	virtual std::string getDynamicPolicyUuidString() const = 0;

	// Event handlers

	virtual void executePolicyIgccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) = 0;
	virtual void executeIaoBroadcastReceived(
		const DptfBuffer& broadcastNotificationData) = 0;
	virtual void executeSwOemVariablesChanged() = 0;
	virtual void executeEnvironmentProfileChanged(const EnvironmentProfile& environmentProfile) = 0;
	virtual void executeConnectedStandbyEntry() = 0;
	virtual void executeConnectedStandbyExit() = 0;
	virtual void executeLowPowerModeEntry() = 0;
	virtual void executeLowPowerModeExit() = 0;
	virtual void executeSuspend() = 0;
	virtual void executeResume() = 0;
	virtual void executeDomainCoreControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainDisplayControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainDisplayStatusChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPerformanceControlsChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPowerControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPriorityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) = 0;
	virtual void executeDomainRfProfileChanged(UIntN participantIndex) = 0;
	virtual void executeDomainTemperatureThresholdCrossed(UIntN participantIndex) = 0;
	virtual void executeParticipantSpecificInfoChanged(UIntN participantIndex) = 0;
	virtual void executeDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex) = 0;
	virtual void executeDomainVirtualSensorPollingTableChanged(UIntN participantIndex) = 0;
	virtual void executeDomainVirtualSensorRecalcChanged(UIntN participantIndex) = 0;
	virtual void executeDomainBatteryStatusChanged(UIntN participantIndex) = 0;
	virtual void executeDomainBatteryInformationChanged(UIntN participantIndex) = 0;
	virtual void executeDomainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) = 0;
	virtual void executeDomainBatteryNoLoadVoltageChanged(UIntN participantIndex) = 0;
	virtual void executeDomainMaxBatteryPeakCurrentChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPlatformPowerSourceChanged(UIntN participantIndex) = 0;
	virtual void executeDomainAdapterPowerRatingChanged(UIntN participantIndex) = 0;
	virtual void executeDomainChargerTypeChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPlatformRestOfPowerChanged(UIntN participantIndex) = 0;
	virtual void executeDomainMaxBatteryPowerChanged(UIntN participantIndex) = 0;
	virtual void executeDomainPlatformBatterySteadyStateChanged(UIntN participantIndex) = 0;
	virtual void executeDomainACNominalVoltageChanged(UIntN participantIndex) = 0;
	virtual void executeDomainACOperationalCurrentChanged(UIntN participantIndex) = 0;
	virtual void executeDomainAC1msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void executeDomainAC2msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void executeDomainAC10msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void executeDomainEnergyThresholdCrossed(UIntN participantIndex) = 0;
	virtual void executeDomainFanCapabilityChanged(UIntN participantIndex) = 0;
	virtual void executeDomainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification) = 0;
	virtual void executeDomainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor) = 0;
	virtual void executeDomainEppSensitivityHintChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		MbtHint::Type mbtHint) = 0;
	virtual void executeDomainExtendedWorkloadPredictionChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		ExtendedWorkloadPrediction::Type extendedWorkloadPrediction) = 0;
	virtual void executeDomainFanOperatingModeChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		FanOperatingMode::Type fanOperatingMode) = 0;
	virtual void executeDomainPcieThrottleRequested(
		UIntN participantIndex,
		UIntN domainIndex,
		OnOffToggle::Type pcieThrottleRequested) = 0;
	virtual void executePolicyActiveRelationshipTableChanged() = 0;
	virtual void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) = 0;
	virtual void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) = 0;
	virtual void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) = 0;
	virtual void executePolicyPassiveTableChanged() = 0;
	virtual void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) = 0;
	virtual void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) = 0;
	virtual void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) = 0;
	virtual void executePolicyThermalRelationshipTableChanged() = 0;
	virtual void executePolicyAdaptivePerformanceConditionsTableChanged() = 0;
	virtual void executePolicyAdaptivePerformanceActionsTableChanged() = 0;
	virtual void executePolicyDdrfTableChanged() = 0;
	virtual void executePolicyRfimTableChanged() = 0;
	virtual void executePolicyTpgaTableChanged() = 0;
	virtual void executePolicyOpbtTableChanged() = 0;
	virtual void executePolicyOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) = 0;
	virtual void executePolicyOperatingSystemLidStateChanged(OsLidState::Type lidState) = 0;
	virtual void executePolicyOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage) = 0;
	virtual void executePolicyOperatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality) = 0;
	virtual void executePolicyOperatingSystemPlatformTypeChanged(OsPlatformType::Type osPlatformType) = 0;
	virtual void executePolicyOperatingSystemDockModeChanged(OsDockMode::Type osDockMode) = 0;
	virtual void executePolicyOperatingSystemEmergencyCallModeStateChanged(
		OnOffToggle::Type emergencyCallModeState) = 0;
	virtual void executePolicyOperatingSystemMobileNotification(
		OsMobileNotificationType::Type notificationType,
		UIntN value) = 0;
	virtual void executePolicyOperatingSystemMixedRealityModeChanged(OnOffToggle::Type osMixedRealityMode) = 0;
	virtual void executePolicyOperatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) = 0;
	virtual void executePolicyOperatingSystemSessionStateChanged(OsSessionState::Type sessionState) = 0;
	virtual void executePolicyOperatingSystemScreenStateChanged(OnOffToggle::Type screenState) = 0;
	virtual void executePolicyOperatingSystemBatteryCountChanged(UIntN batteryCount) = 0;
	virtual void executePolicyOperatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) = 0;
	virtual void executePolicyProcessLoaded(const std::string& processName, UInt64 processId) = 0;
	virtual void executePolicyProcessUnLoaded(UInt64 processId) = 0;
	virtual void executePolicySystemModeChanged(SystemMode::Type systemMode) = 0;
	virtual void executePolicyOemVariablesChanged() = 0;
	virtual void executePolicyPowerBossConditionsTableChanged() = 0;
	virtual void executePolicyPowerBossActionsTableChanged() = 0;
	virtual void executePolicyPowerBossMathTableChanged() = 0;
	virtual void executePolicyVoltageThresholdMathTableChanged() = 0;
	virtual void executePolicyActivityLoggingEnabled() = 0;
	virtual void executePolicyActivityLoggingDisabled() = 0;
	virtual void executePolicyEmergencyCallModeTableChanged() = 0;
	virtual void executePolicyPidAlgorithmTableChanged() = 0;
	virtual void executePolicyActiveControlPointRelationshipTableChanged() = 0;
	virtual void executePolicyPowerShareAlgorithmTableChanged() = 0;
	virtual void executePowerLimitChanged() = 0;
	virtual void executePowerLimitTimeWindowChanged() = 0;
	virtual void executePerformanceCapabilitiesChanged(UIntN participantIndex) = 0;
	virtual void executePolicyWorkloadHintConfigurationChanged() = 0;
	virtual void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osGameMode) = 0;
	virtual void executePolicyPowerShareAlgorithmTable2Changed() = 0;
	virtual void executePolicyIntelligentThermalManagementTableChanged() = 0;
	virtual void executePolicyIntelligentThermalManagementTable3Changed() = 0;
	virtual void executePolicyEnergyPerformanceOptimizerTableChanged() = 0;
	virtual void executePolicyPlatformUserPresenceChanged(SensorUserPresence::Type platformUserPresence) = 0;
	virtual void executePolicyExternalMonitorStateChanged(Bool externalMonitorState) = 0;
	virtual void executePolicyUserInteractionChanged(UserInteraction::Type userInteraction) = 0;
	virtual void executePolicyForegroundRatioChanged(UIntN ratio) = 0;
	virtual void executePolicyCollaborationChanged(OnOffToggle::Type collaboration) = 0;
	virtual void executePolicyThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) = 0;
	virtual void executePolicyThirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP) = 0;
	virtual void executePolicySystemConfigurationFeatureTableChanged() = 0;
	virtual void executePolicySystemInBagChanged(SystemInBag::Type systemInBag) = 0;
	virtual void executeDptfExtendedWorkloadPredictionEventRegistrationChanged(UInt32 consumerCount) = 0;
	virtual void executePolicyThirdPartyGraphicsReservedTgpChanged(Power reservedTgp) = 0;
	virtual void executePolicyThirdPartyGraphicsOppBoostModeChanged(OpportunisticBoostMode::Type oppBoostMode) = 0;
	virtual void executePolicyScenarioModeChanged(ScenarioMode::Type scenarioMode) = 0;
	virtual void executePolicyDttGamingModeChanged(DttGamingMode::Type dttGamingMode) = 0;
	virtual void executePolicyApplicationOptimizationChanged(Bool isActive) = 0;
};

class dptf_export Policy : public IPolicy
{
public:
	Policy(DptfManagerInterface* dptfManager);
	Policy(const Policy& other) = delete;
	Policy(Policy&& other) noexcept = delete;
	Policy& operator=(const Policy& other) = delete;
	Policy& operator=(Policy&& other) noexcept = delete;
	~Policy() override = default;

	void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) override;
	void destroyPolicy() override;

	Guid getGuid() override;

	void bindParticipant(UIntN participantIndex) override;
	void unbindParticipant(UIntN participantIndex) override;
	void bindDomain(UIntN participantIndex, UIntN domainIndex) override;
	void unbindDomain(UIntN participantIndex, UIntN domainIndex) override;

	void enable() override;
	void disable() override;

	std::string getName() const override;
	std::string getPolicyFileName() const override;
	UIntN getPolicyIndex() const override;
	std::string getStatusAsXml() const override;
	std::string getDiagnosticsAsXml() const override;
	std::map<std::string, std::string> getPolicyStateLogData() const override;

	std::string getConfigurationForExport() const override;
	Bool isDynamicPolicy() override;
	std::string getDynamicPolicyUuidString() const override;

	// Event handlers
	void executePolicyIgccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) override;
	void executeIaoBroadcastReceived(
		const DptfBuffer& broadcastNotificationData) override;
	void executeSwOemVariablesChanged() override;
	void executeEnvironmentProfileChanged(
	const EnvironmentProfile& environmentProfile) override;
	void executeConnectedStandbyEntry() override;
	void executeConnectedStandbyExit() override;
	void executeLowPowerModeEntry() override;
	void executeLowPowerModeExit() override;
	void executeSuspend() override;
	void executeResume() override;
	void executeDomainCoreControlCapabilityChanged(UIntN participantIndex) override;
	void executeDomainDisplayControlCapabilityChanged(UIntN participantIndex) override;
	void executeDomainDisplayStatusChanged(UIntN participantIndex) override;
	void executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex) override;
	void executeDomainPerformanceControlsChanged(UIntN participantIndex) override;
	void executeDomainPowerControlCapabilityChanged(UIntN participantIndex) override;
	void executeDomainPriorityChanged(UIntN participantIndex) override;
	void executeDomainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) override;
	void executeDomainRfProfileChanged(UIntN participantIndex) override;
	void executeDomainTemperatureThresholdCrossed(UIntN participantIndex) override;
	void executeParticipantSpecificInfoChanged(UIntN participantIndex) override;
	void executeDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex) override;
	void executeDomainVirtualSensorPollingTableChanged(UIntN participantIndex) override;
	void executeDomainVirtualSensorRecalcChanged(UIntN participantIndex) override;
	void executeDomainBatteryStatusChanged(UIntN participantIndex) override;
	void executeDomainBatteryInformationChanged(UIntN participantIndex) override;
	void executeDomainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) override;
	void executeDomainBatteryNoLoadVoltageChanged(UIntN participantIndex) override;
	void executeDomainMaxBatteryPeakCurrentChanged(UIntN participantIndex) override;
	void executeDomainPlatformPowerSourceChanged(UIntN participantIndex) override;
	void executeDomainAdapterPowerRatingChanged(UIntN participantIndex) override;
	void executeDomainChargerTypeChanged(UIntN participantIndex) override;
	void executeDomainPlatformRestOfPowerChanged(UIntN participantIndex) override;
	void executeDomainMaxBatteryPowerChanged(UIntN participantIndex) override;
	void executeDomainPlatformBatterySteadyStateChanged(UIntN participantIndex) override;
	void executeDomainACNominalVoltageChanged(UIntN participantIndex) override;
	void executeDomainACOperationalCurrentChanged(UIntN participantIndex) override;
	void executeDomainAC1msPercentageOverloadChanged(UIntN participantIndex) override;
	void executeDomainAC2msPercentageOverloadChanged(UIntN participantIndex) override;
	void executeDomainAC10msPercentageOverloadChanged(UIntN participantIndex) override;
	void executeDomainEnergyThresholdCrossed(UIntN participantIndex) override;
	void executeDomainFanCapabilityChanged(UIntN participantIndex) override;
	void executeDomainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification) override;
	void executeDomainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor) override;
	void executeDomainEppSensitivityHintChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		MbtHint::Type mbtHint) override;
	void executeDomainExtendedWorkloadPredictionChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		ExtendedWorkloadPrediction::Type extendedWorkloadPrediction) override;
	void executeDomainFanOperatingModeChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		FanOperatingMode::Type fanOperatingMode) override;
	void executeDomainPcieThrottleRequested(
		UIntN participantIndex,
		UIntN domainIndex,
		OnOffToggle::Type pcieThrottleRequested) override;
	void executePolicyActiveRelationshipTableChanged() override;
	void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) override;
	void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) override;
	void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) override;
	void executePolicyPassiveTableChanged() override;
	void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) override;
	void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) override;
	void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) override;
	void executePolicyThermalRelationshipTableChanged() override;
	void executePolicyAdaptivePerformanceConditionsTableChanged() override;
	void executePolicyAdaptivePerformanceActionsTableChanged() override;
	void executePolicyDdrfTableChanged() override;
	void executePolicyRfimTableChanged() override;
	void executePolicyTpgaTableChanged() override;
	void executePolicyOpbtTableChanged() override;
	void executePolicyOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) override;
	void executePolicyOperatingSystemLidStateChanged(OsLidState::Type lidState) override;
	void executePolicyOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage) override;
	void executePolicyOperatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality) override;
	void executePolicyOperatingSystemPlatformTypeChanged(OsPlatformType::Type osPlatformType) override;
	void executePolicyOperatingSystemDockModeChanged(OsDockMode::Type osDockMode) override;
	void executePolicyOperatingSystemEmergencyCallModeStateChanged(
		OnOffToggle::Type emergencyCallModeState) override;
	void executePolicyOperatingSystemMobileNotification(
		OsMobileNotificationType::Type notificationType,
		UIntN value) override;
	void executePolicyOperatingSystemMixedRealityModeChanged(OnOffToggle::Type osMixedRealityMode) override;
	void executePolicyOperatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) override;
	void executePolicyOperatingSystemSessionStateChanged(OsSessionState::Type sessionState) override;
	void executePolicyOperatingSystemScreenStateChanged(OnOffToggle::Type screenState) override;
	void executePolicyOperatingSystemBatteryCountChanged(UIntN batteryCount) override;
	void executePolicyOperatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) override;
	void executePolicyProcessLoaded(const std::string& processName, UInt64 processId) override;
	void executePolicyProcessUnLoaded(UInt64 processId) override;
	void executePolicySystemModeChanged(SystemMode::Type systemMode) override;
	void executePolicyOemVariablesChanged() override;
	void executePolicyPowerBossConditionsTableChanged() override;
	void executePolicyPowerBossActionsTableChanged() override;
	void executePolicyPowerBossMathTableChanged() override;
	void executePolicyVoltageThresholdMathTableChanged() override;
	void executePolicyActivityLoggingEnabled() override;
	void executePolicyActivityLoggingDisabled() override;
	void executePolicyEmergencyCallModeTableChanged() override;
	void executePolicyPidAlgorithmTableChanged() override;
	void executePolicyActiveControlPointRelationshipTableChanged() override;
	void executePolicyPowerShareAlgorithmTableChanged() override;
	void executePolicyIntelligentThermalManagementTableChanged() override;
	void executePolicyIntelligentThermalManagementTable3Changed() override;
	void executePolicyEnergyPerformanceOptimizerTableChanged() override;
	void executePowerLimitChanged() override;
	void executePowerLimitTimeWindowChanged() override;
	void executePerformanceCapabilitiesChanged(UIntN participantIndex) override;
	void executePolicyWorkloadHintConfigurationChanged() override;
	void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osGameMode) override;
	void executePolicyPowerShareAlgorithmTable2Changed() override;
	void executePolicyPlatformUserPresenceChanged(SensorUserPresence::Type platformUserPresence) override;
	void executePolicyExternalMonitorStateChanged(Bool externalMonitorState) override;
	void executePolicyUserInteractionChanged(UserInteraction::Type userInteraction) override;
	void executePolicyForegroundRatioChanged(UIntN ratio) override;
	void executePolicyCollaborationChanged(OnOffToggle::Type collaboration) override;
	void executePolicyThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) override;
	void executePolicyThirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP) override;
	void executePolicySystemConfigurationFeatureTableChanged() override;
	void executePolicySystemInBagChanged(SystemInBag::Type systemInBag) override;
	void executeDptfExtendedWorkloadPredictionEventRegistrationChanged(UInt32 consumerCount) override;
	void executePolicyThirdPartyGraphicsReservedTgpChanged(Power reservedTgp) override;
	void executePolicyThirdPartyGraphicsOppBoostModeChanged(OpportunisticBoostMode::Type oppBoostMode) override;
	void executePolicyScenarioModeChanged(ScenarioMode::Type scenarioMode) override;
	void executePolicyDttGamingModeChanged(DttGamingMode::Type dttGamingMode) override;
	void executePolicyApplicationOptimizationChanged(Bool isActive) override;

private:
	// Pointer to the DPTF manager class which has access to most of the manager components.
	DptfManagerInterface* m_dptfManager;
	EsifServicesInterface* getEsifServices() const;

	// The instance returned when we called CreatePolicyInstance on the .dll/.so
	PolicyInterface* m_theRealPolicy;
	Bool m_theRealPolicyCreated;

	// The index assigned by the policy manager
	UIntN m_policyIndex;

	// The guid retrieved when the file was loaded
	Guid m_guid;

	// Dynamic policy
	Bool m_isDynamicPolicy;
	std::string m_dynamicPolicyUuidString;

	Bool m_isPolicyLoggingEnabled;
	Bool isPolicyLoggingEnabled() const;
	void enablePolicyLogging();
	void disablePolicyLogging();
	void sendPolicyLogDataIfLoggingEnabled(Bool loaded) const;

	// The name of the policy.
	std::string m_policyName;

	// full path to the file that contains this policy
	std::string m_policyFileName;

	// functionality to load/unload .dll/.so and retrieve function pointers
	EsifLibrary* m_esifLibrary;

	// function pointers exposed in the .dll/.so
	GetAppVersionFuncPtr m_getAppVersion;
	CreatePolicyInstanceFuncPtr m_createPolicyInstanceFuncPtr;
	DestroyPolicyInstanceFuncPtr m_destroyPolicyInstanceFuncPtr;

	// Policy services should be used as an interface instead of an interface container.
	// Because of this design decision the code is split between the shared lib and
	// the manager which is why the following functions are here.  This should be improved
	// in the future.
	PolicyServicesInterfaceContainer m_policyServices;
	void createPolicyServices();
	void destroyPolicyServices();

	// track the events that will be forwarded to the policy
	std::bitset<PolicyEvent::Max> m_registeredEvents;

	// Event Registration.  This is private as the policy manager must handle registering and unregistering
	// the event with ESIF.  This is necessary since multiple policies may register for an event such as
	// PolicyEvent::PolicyForegroundApplicationChanged.  We can't have two policies register for this event and
	// one go away and unregister for both of them.
	friend class PolicyManager;
	void registerEvent(PolicyEvent::Type policyEvent);
	void unregisterEvent(PolicyEvent::Type policyEvent);
	Bool isEventRegistered(PolicyEvent::Type policyEvent) const;
};
