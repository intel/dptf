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
#include "SupportedPolicyList.h"
#include "PolicyInterface.h"
#include "EsifLibrary.h"
#include "esif_sdk_logging_data.h"

class DptfManager;

class dptf_export IPolicy
{
public:
	virtual ~IPolicy(void){};
	virtual void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) = 0;
	virtual void destroyPolicy(void) = 0;

	virtual Guid getGuid(void) = 0;

	virtual void bindParticipant(UIntN participantIndex) = 0;
	virtual void unbindParticipant(UIntN participantIndex) = 0;
	virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;

	virtual std::string getName(void) const = 0;
	virtual std::string getPolicyFileName(void) const = 0;
	virtual UIntN getPolicyIndex() const = 0;
	virtual std::string getStatusAsXml(void) const = 0;
	virtual std::string getDiagnosticsAsXml(void) const = 0;

	virtual Bool isDynamicPolicy(void) = 0;
	virtual std::string getDynamicPolicyUuidString(void) const = 0;

	// Event handlers

	virtual void executeIgccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) = 0;
	virtual void executeSwOemVariablesChanged(void) = 0;
	virtual void executeEnvironmentProfileChanged(const EnvironmentProfile& environmentProfile) = 0;
	virtual void executeConnectedStandbyEntry(void) = 0;
	virtual void executeConnectedStandbyExit(void) = 0;
	virtual void executeLowPowerModeEntry(void) = 0;
	virtual void executeLowPowerModeExit(void) = 0;
	virtual void executeSuspend(void) = 0;
	virtual void executeResume(void) = 0;
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
	virtual void executePolicyActiveRelationshipTableChanged(void) = 0;
	virtual void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) = 0;
	virtual void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) = 0;
	virtual void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) = 0;
	virtual void executePolicyPassiveTableChanged(void) = 0;
	virtual void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) = 0;
	virtual void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) = 0;
	virtual void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) = 0;
	virtual void executePolicyThermalRelationshipTableChanged(void) = 0;
	virtual void executePolicyAdaptivePerformanceConditionsTableChanged(void) = 0;
	virtual void executePolicyAdaptivePerformanceActionsTableChanged(void) = 0;
	virtual void executePolicyDdrfTableChanged(void) = 0;
	virtual void executePolicyRfimTableChanged(void) = 0;
	virtual void executePolicyTpgaTableChanged(void) = 0;
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
	virtual void executePolicyProcessLoaded(const std::string& processName) = 0;
	virtual void executePolicySystemModeChanged(SystemMode::Type systemMode) = 0;
	virtual void executePolicyOemVariablesChanged(void) = 0;
	virtual void executePolicyPowerBossConditionsTableChanged(void) = 0;
	virtual void executePolicyPowerBossActionsTableChanged(void) = 0;
	virtual void executePolicyPowerBossMathTableChanged(void) = 0;
	virtual void executePolicyVoltageThresholdMathTableChanged(void) = 0;
	virtual void executePolicyActivityLoggingEnabled(void) = 0;
	virtual void executePolicyActivityLoggingDisabled(void) = 0;
	virtual void executePolicyEmergencyCallModeTableChanged(void) = 0;
	virtual void executePolicyPidAlgorithmTableChanged(void) = 0;
	virtual void executePolicyActiveControlPointRelationshipTableChanged(void) = 0;
	virtual void executePolicyPowerShareAlgorithmTableChanged(void) = 0;
	virtual void executePowerLimitChanged(void) = 0;
	virtual void executePowerLimitTimeWindowChanged(void) = 0;
	virtual void executePerformanceCapabilitiesChanged(UIntN participantIndex) = 0;
	virtual void executePolicyWorkloadHintConfigurationChanged(void) = 0;
	virtual void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osGameMode) = 0;
	virtual void executePolicyPowerShareAlgorithmTable2Changed(void) = 0;
	virtual void executePolicyIntelligentThermalManagementTableChanged(void) = 0;
	virtual void executePolicyEnergyPerformanceOptimizerTableChanged(void) = 0;
	virtual void executePolicyPlatformUserPresenceChanged(SensorUserPresence::Type platformUserPresence) = 0;
	virtual void executePolicyExternalMonitorStateChanged(Bool externalMonitorState) = 0;
	virtual void executePolicyUserInteractionChanged(UserInteraction::Type userInteraction) = 0;
	virtual void executePolicyForegroundRatioChanged(UIntN ratio) = 0;
	virtual void executePolicyCollaborationChanged(OnOffToggle::Type collaboration) = 0;
	virtual void executePolicyThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) = 0;
	virtual void executePolicyThirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP) = 0;
};

class dptf_export Policy : public IPolicy
{
public:
	Policy(DptfManagerInterface* dptfManager);
	~Policy(void) override;

	void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) override;
	void destroyPolicy(void) override;

	Guid getGuid(void) override;

	void bindParticipant(UIntN participantIndex) override;
	void unbindParticipant(UIntN participantIndex) override;
	void bindDomain(UIntN participantIndex, UIntN domainIndex) override;
	void unbindDomain(UIntN participantIndex, UIntN domainIndex) override;

	void enable(void) override;
	void disable(void) override;

	std::string getName(void) const override;
	std::string getPolicyFileName(void) const override;
	UIntN getPolicyIndex() const override;
	std::string getStatusAsXml(void) const override;
	std::string getDiagnosticsAsXml(void) const override;

	Bool isDynamicPolicy(void) override;
	std::string getDynamicPolicyUuidString(void) const override;

	// Event handlers
	void executeIgccBroadcastReceived(
		IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) override;
	void executeSwOemVariablesChanged(void) override;
	void executeEnvironmentProfileChanged(
	const EnvironmentProfile& environmentProfile) override;
	void executeConnectedStandbyEntry(void) override;
	void executeConnectedStandbyExit(void) override;
	void executeLowPowerModeEntry(void) override;
	void executeLowPowerModeExit(void) override;
	void executeSuspend(void) override;
	void executeResume(void) override;
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
	virtual void executeDomainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor) override;
	virtual void executeDomainEppSensitivityHintChanged(
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
	void executePolicyActiveRelationshipTableChanged(void) override;
	void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) override;
	void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) override;
	void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) override;
	void executePolicyPassiveTableChanged(void) override;
	void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) override;
	void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) override;
	void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) override;
	void executePolicyThermalRelationshipTableChanged(void) override;
	void executePolicyAdaptivePerformanceConditionsTableChanged(void) override;
	void executePolicyAdaptivePerformanceActionsTableChanged() override;
	void executePolicyDdrfTableChanged(void) override;
	void executePolicyRfimTableChanged(void) override;
	void executePolicyTpgaTableChanged(void) override;
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
	void executePolicyProcessLoaded(const std::string& processName) override;
	void executePolicySystemModeChanged(SystemMode::Type systemMode) override;
	void executePolicyOemVariablesChanged(void) override;
	void executePolicyPowerBossConditionsTableChanged(void) override;
	void executePolicyPowerBossActionsTableChanged(void) override;
	void executePolicyPowerBossMathTableChanged(void) override;
	void executePolicyVoltageThresholdMathTableChanged(void) override;
	void executePolicyActivityLoggingEnabled(void) override;
	void executePolicyActivityLoggingDisabled(void) override;
	void executePolicyEmergencyCallModeTableChanged(void) override;
	void executePolicyPidAlgorithmTableChanged(void) override;
	void executePolicyActiveControlPointRelationshipTableChanged(void) override;
	void executePolicyPowerShareAlgorithmTableChanged(void) override;
	void executePolicyIntelligentThermalManagementTableChanged(void) override;
	void executePolicyEnergyPerformanceOptimizerTableChanged(void) override;
	void executePowerLimitChanged(void) override;
	void executePowerLimitTimeWindowChanged(void) override;
	void executePerformanceCapabilitiesChanged(UIntN participantIndex) override;
	void executePolicyWorkloadHintConfigurationChanged(void) override;
	void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osMixedRealityMode) override;
	void executePolicyPowerShareAlgorithmTable2Changed(void) override;
	void executePolicyPlatformUserPresenceChanged(SensorUserPresence::Type platformUserPresence) override;
	void executePolicyExternalMonitorStateChanged(Bool externalMonitorState) override;
	void executePolicyUserInteractionChanged(UserInteraction::Type userInteraction) override;
	void executePolicyForegroundRatioChanged(UIntN ratio) override;
	void executePolicyCollaborationChanged(OnOffToggle::Type collaboration) override;
	void executePolicyThirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) override;
	void executePolicyThirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP) override;

private:
	// hide the copy constructor and assignment operator.
	Policy(const Policy& rhs);
	Policy& operator=(const Policy& rhs);

	// Pointer to the DPTF manager class which has access to most of the manager components.
	DptfManagerInterface* m_dptfManager;
	EsifServicesInterface* getEsifServices();

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
	Bool isPolicyLoggingEnabled();
	void enablePolicyLogging(void);
	void disablePolicyLogging(void);
	void sendPolicyLogDataIfLoggingEnabled(Bool loaded);

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
	void createPolicyServices(void);
	void destroyPolicyServices(void);

	// track the events that will be forwarded to the policy
	std::bitset<PolicyEvent::Max> m_registeredEvents;

	// Event Registration.  This is private as the policy manager must handle registering and unregistering
	// the event with ESIF.  This is necessary since multiple policies may register for an event such as
	// PolicyEvent::PolicyForegroundApplicationChanged.  We can't have two policies register for this event and
	// one go away and unregister for both of them.
	friend class PolicyManager;
	void registerEvent(PolicyEvent::Type policyEvent);
	void unregisterEvent(PolicyEvent::Type policyEvent);
	Bool isEventRegistered(PolicyEvent::Type policyEvent);
};
