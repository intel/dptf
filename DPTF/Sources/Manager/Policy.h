/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
		std::shared_ptr<SupportedPolicyList> supportedPolicyList) = 0;
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
	virtual std::string getStatusAsXml(void) const = 0;
	virtual std::string getDiagnosticsAsXml(void) const = 0;

	// Event handlers

	virtual void executeConnectedStandbyEntry(void) = 0;
	virtual void executeConnectedStandbyExit(void) = 0;
	virtual void executeSuspend(void) = 0;
	virtual void executeResume(void) = 0;
	virtual void executeDomainConfigTdpCapabilityChanged(UIntN participantIndex) = 0;
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
	virtual void executePolicyActiveRelationshipTableChanged(void) = 0;
	virtual void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) = 0;
	virtual void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) = 0;
	virtual void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) = 0;
	virtual void executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel) = 0;
	virtual void executePolicyPassiveTableChanged(void) = 0;
	virtual void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) = 0;
	virtual void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) = 0;
	virtual void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) = 0;
	virtual void executePolicyThermalRelationshipTableChanged(void) = 0;
	virtual void executePolicyAdaptivePerformanceParticipantConditionTableChanged(void) = 0;
	virtual void executePolicyAdaptivePerformanceConditionsTableChanged(void) = 0;
	virtual void executePolicyAdaptivePerformanceActionsTableChanged(void) = 0;
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
	virtual void executePolicyOperatingSystemScreenStateChanged(OnOffToggle::Type screenState) = 0;
	virtual void executePolicyOperatingSystemBatteryCountChanged(UIntN batteryCount) = 0;
	virtual void executePolicyOperatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) = 0;
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
	virtual void executePolicyWorkloadHintConfigurationChanged(void) = 0;
	virtual void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osGameMode) = 0;
	virtual void executePolicyPowerShareAlgorithmTable2Changed(void) = 0;
};

class dptf_export Policy : public IPolicy
{
public:
	Policy(DptfManagerInterface* dptfManager);
	~Policy(void);

	virtual void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList) override;
	virtual void destroyPolicy(void) override;

	virtual Guid getGuid(void) override;

	virtual void bindParticipant(UIntN participantIndex) override;
	virtual void unbindParticipant(UIntN participantIndex) override;
	virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) override;
	virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) override;

	virtual void enable(void) override;
	virtual void disable(void) override;

	virtual std::string getName(void) const override;
	virtual std::string getPolicyFileName(void) const override;
	virtual std::string getStatusAsXml(void) const override;
	virtual std::string getDiagnosticsAsXml(void) const override;

	// Event handlers

	virtual void executeConnectedStandbyEntry(void) override;
	virtual void executeConnectedStandbyExit(void) override;
	virtual void executeSuspend(void) override;
	virtual void executeResume(void) override;
	virtual void executeDomainConfigTdpCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainCoreControlCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainDisplayControlCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainDisplayStatusChanged(UIntN participantIndex) override;
	virtual void executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainPerformanceControlsChanged(UIntN participantIndex) override;
	virtual void executeDomainPowerControlCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainPriorityChanged(UIntN participantIndex) override;
	virtual void executeDomainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) override;
	virtual void executeDomainRfProfileChanged(UIntN participantIndex) override;
	virtual void executeDomainTemperatureThresholdCrossed(UIntN participantIndex) override;
	virtual void executeParticipantSpecificInfoChanged(UIntN participantIndex) override;
	virtual void executeDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex) override;
	virtual void executeDomainVirtualSensorPollingTableChanged(UIntN participantIndex) override;
	virtual void executeDomainVirtualSensorRecalcChanged(UIntN participantIndex) override;
	virtual void executeDomainBatteryStatusChanged(UIntN participantIndex) override;
	virtual void executeDomainBatteryInformationChanged(UIntN participantIndex) override;
	virtual void executeDomainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) override;
	virtual void executeDomainBatteryNoLoadVoltageChanged(UIntN participantIndex) override;
	virtual void executeDomainMaxBatteryPeakCurrentChanged(UIntN participantIndex) override;
	virtual void executeDomainPlatformPowerSourceChanged(UIntN participantIndex) override;
	virtual void executeDomainAdapterPowerRatingChanged(UIntN participantIndex) override;
	virtual void executeDomainChargerTypeChanged(UIntN participantIndex) override;
	virtual void executeDomainPlatformRestOfPowerChanged(UIntN participantIndex) override;
	virtual void executeDomainMaxBatteryPowerChanged(UIntN participantIndex) override;
	virtual void executeDomainPlatformBatterySteadyStateChanged(UIntN participantIndex) override;
	virtual void executeDomainACNominalVoltageChanged(UIntN participantIndex) override;
	virtual void executeDomainACOperationalCurrentChanged(UIntN participantIndex) override;
	virtual void executeDomainAC1msPercentageOverloadChanged(UIntN participantIndex) override;
	virtual void executeDomainAC2msPercentageOverloadChanged(UIntN participantIndex) override;
	virtual void executeDomainAC10msPercentageOverloadChanged(UIntN participantIndex) override;
	virtual void executeDomainEnergyThresholdCrossed(UIntN participantIndex) override;
	virtual void executeDomainFanCapabilityChanged(UIntN participantIndex) override;
	virtual void executeDomainSocWorkloadClassificationChanged(
		UIntN participantIndex, 
		UIntN domainIndex, 
		SocWorkloadClassification::Type socWorkloadClassification) override;
	virtual void executePolicyActiveRelationshipTableChanged(void) override;
	virtual void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode) override;
	virtual void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName) override;
	virtual void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) override;
	virtual void executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel) override;
	virtual void executePolicyPassiveTableChanged(void) override;
	virtual void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation) override;
	virtual void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion) override;
	virtual void executePolicySensorSpatialOrientationChanged(
		SensorSpatialOrientation::Type sensorSpatialOrientation) override;
	virtual void executePolicyThermalRelationshipTableChanged(void) override;
	virtual void executePolicyAdaptivePerformanceParticipantConditionTableChanged(void) override;
	virtual void executePolicyAdaptivePerformanceConditionsTableChanged(void) override;
	virtual void executePolicyAdaptivePerformanceActionsTableChanged(void) override;
	virtual void executePolicyOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) override;
	virtual void executePolicyOperatingSystemLidStateChanged(OsLidState::Type lidState) override;
	virtual void executePolicyOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage) override;
	virtual void executePolicyOperatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality) override;
	virtual void executePolicyOperatingSystemPlatformTypeChanged(OsPlatformType::Type osPlatformType) override;
	virtual void executePolicyOperatingSystemDockModeChanged(OsDockMode::Type osDockMode) override;
	virtual void executePolicyOperatingSystemEmergencyCallModeStateChanged(
		OnOffToggle::Type emergencyCallModeState) override;
	virtual void executePolicyOperatingSystemMobileNotification(
		OsMobileNotificationType::Type notificationType,
		UIntN value) override;
	virtual void executePolicyOperatingSystemMixedRealityModeChanged(OnOffToggle::Type osMixedRealityMode) override;
	virtual void executePolicyOperatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) override;
	virtual void executePolicyOperatingSystemScreenStateChanged(OnOffToggle::Type screenState) override;
	virtual void executePolicyOperatingSystemBatteryCountChanged(UIntN batteryCount) override;
	virtual void executePolicyOperatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) override;
	virtual void executePolicyOemVariablesChanged(void) override;
	virtual void executePolicyPowerBossConditionsTableChanged(void) override;
	virtual void executePolicyPowerBossActionsTableChanged(void) override;
	virtual void executePolicyPowerBossMathTableChanged(void) override;
	virtual void executePolicyVoltageThresholdMathTableChanged(void) override;
	virtual void executePolicyActivityLoggingEnabled(void) override;
	virtual void executePolicyActivityLoggingDisabled(void) override;
	virtual void executePolicyEmergencyCallModeTableChanged(void) override;
	virtual void executePolicyPidAlgorithmTableChanged(void) override;
	virtual void executePolicyActiveControlPointRelationshipTableChanged(void) override;
	virtual void executePolicyPowerShareAlgorithmTableChanged(void) override;
	virtual void executePowerLimitChanged(void) override;
	virtual void executePolicyWorkloadHintConfigurationChanged(void) override;
	virtual void executePolicyOperatingSystemGameModeChanged(OnOffToggle::Type osMixedRealityMode) override;
	virtual void executePolicyPowerShareAlgorithmTable2Changed(void) override;

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
