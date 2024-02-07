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
#include "PolicyServicesInterfaceContainer.h"
#include "CoolingMode.h"
#include "SensorOrientation.h"
#include "OnOffToggle.h"
#include "SensorSpatialOrientation.h"
#include "RadioConnectionStatus.h"
#include "OsPowerSource.h"
#include "OsLidState.h"
#include "OsPlatformType.h"
#include "OsDockMode.h"
#include "OsPowerSchemePersonality.h"
#include "OsUserPresence.h"
#include "OsSessionState.h"
#include "OsPowerSlider.h"
#include "SocWorkloadClassification.h"
#include "SensorUserPresence.h"
#include "UserInteraction.h"
#include "MbtHint.h"
#include "SystemMode.h"
#include "IgccBroadcastData.h"
#include "ExtendedWorkloadPrediction.h"
#include "FanOperatingMode.h"
#include "SocPowerFloor.h"
#include "SystemInBag.h"
#include "OpportunisticBoostMode.h"
#include "ScenarioMode.h"
#include "DttGamingMode.h"

class dptf_export PolicyInterface
{
public:
	PolicyInterface() = default;
	PolicyInterface(const PolicyInterface& other) = default;
	PolicyInterface(PolicyInterface&& other) noexcept = default;
	PolicyInterface& operator=(const PolicyInterface& other) = default;
	PolicyInterface& operator=(PolicyInterface&& other) noexcept = default;
	virtual ~PolicyInterface() = default;

	//
	// This is the main entry point for bringing up a policy.  If a policy chooses not to load itself,
	// it should throw an exception.  In this case the policy will not get called again.
	//
	virtual void create(
		Bool enabled,
		const PolicyServicesInterfaceContainer& policyServicesInterfaceContainer,
		UIntN policyIndex,
		const std::string& dynamicPolicyUuid,
		const std::string& dynamicPolicyName) = 0;

	//
	// In response to this call, the policy must clean up all of its internal data structures.
	//
	virtual void destroy() = 0;

	//
	// This function is called by the DPTF framework whenever it receives a participant
	// ready notification for the first time.  No domain is available when bindParticipant is called.
	//
	virtual void bindParticipant(UIntN participantIndex) = 0;

	//
	// This function is called when the participant interface De-negotiation happens at
	// the DPTF framework level. meaning, the participant is being unloaded etc.
	//
	virtual void unbindParticipant(UIntN participantIndex) = 0;

	//
	// This function is called by the DPTF framework whenever it receives a new domain.
	//
	virtual void bindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

	//
	// This function is called by the DPTF framework when a domain is being removed.
	//
	virtual void unbindDomain(UIntN participantIndex, UIntN domainIndex) = 0;

	//
	// This function is called if the framework wants to enable this specific policy.  This may
	// happen for debug and validation purposes.
	//
	virtual void enable() = 0;

	//
	// This function is called if the framework wants to disable this specific policy.  This may
	// happen for debug and validation purposes.
	//
	// When a policy is disabled, it should clear anything that it 'set', for example, the
	// aux trip points should be cleared and the fan turned off.
	//
	virtual void disable() = 0;

	virtual Guid getGuid() const = 0;
	virtual std::string getName() const = 0;
	virtual std::string getStatusAsXml() const = 0;
	virtual std::string getDiagnosticsAsXml() const = 0;
	virtual std::map<std::string, std::string> getPolicyStateLogData() const = 0;
	virtual std::string getConfigurationForExport() const = 0;

	// DPTF Event handlers
	virtual void igccBroadcastReceived(IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData) = 0;
	virtual void iaoBroadcastReceived(const DptfBuffer& broadcastNotificationData) = 0;
	virtual void environmentProfileChanged(const EnvironmentProfile& environmentProfile) = 0;
	virtual void extendedWorkloadPredictionEventRegistrationChanged(UInt32 consumerCount) = 0;
	virtual void connectedStandbyEntry() = 0;
	virtual void connectedStandbyExit() = 0;
	virtual void lowPowerModeEntry() = 0;
	virtual void lowPowerModeExit() = 0;
	virtual void suspend() = 0;
	virtual void resume() = 0;

	// Participant/Domain Event Handlers
	virtual void domainCoreControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void domainDisplayControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void domainDisplayStatusChanged(UIntN participantIndex) = 0;
	virtual void domainPerformanceControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void domainPerformanceControlsChanged(UIntN participantIndex) = 0;
	virtual void domainPowerControlCapabilityChanged(UIntN participantIndex) = 0;
	virtual void domainPriorityChanged(UIntN participantIndex) = 0;
	virtual void domainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus) = 0;
	virtual void domainRfProfileChanged(UIntN participantIndex) = 0;
	virtual void domainTemperatureThresholdCrossed(UIntN participantIndex) = 0;
	virtual void participantSpecificInfoChanged(UIntN participantIndex) = 0;
	virtual void domainVirtualSensorCalibrationTableChanged(UIntN participantIndex) = 0;
	virtual void domainVirtualSensorPollingTableChanged(UIntN participantIndex) = 0;
	virtual void domainVirtualSensorRecalcChanged(UIntN participantIndex) = 0;
	virtual void domainBatteryStatusChanged(UIntN participantIndex) = 0;
	virtual void domainBatteryInformationChanged(UIntN participantIndex) = 0;
	virtual void domainBatteryHighFrequencyImpedanceChanged(UIntN participantIndex) = 0;
	virtual void domainBatteryNoLoadVoltageChanged(UIntN participantIndex) = 0;
	virtual void domainMaxBatteryPeakCurrentChanged(UIntN participantIndex) = 0;
	virtual void domainPlatformPowerSourceChanged(UIntN participantIndex) = 0;
	virtual void domainAdapterPowerRatingChanged(UIntN participantIndex) = 0;
	virtual void domainChargerTypeChanged(UIntN participantIndex) = 0;
	virtual void domainPlatformRestOfPowerChanged(UIntN participantIndex) = 0;
	virtual void domainMaxBatteryPowerChanged(UIntN participantIndex) = 0;
	virtual void domainPlatformBatterySteadyStateChanged(UIntN participantIndex) = 0;
	virtual void domainACNominalVoltageChanged(UIntN participantIndex) = 0;
	virtual void domainACOperationalCurrentChanged(UIntN participantIndex) = 0;
	virtual void domainAC1msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void domainAC2msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void domainAC10msPercentageOverloadChanged(UIntN participantIndex) = 0;
	virtual void domainEnergyThresholdCrossed(UIntN participantIndex) = 0;
	virtual void domainFanCapabilityChanged(UIntN participantIndex) = 0;
	virtual void domainSocWorkloadClassificationChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocWorkloadClassification::Type socWorkloadClassification) = 0;
	virtual void domainSocPowerFloorChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		SocPowerFloor::Type socPowerFloor) = 0;
	virtual void domainEppSensitivityHintChanged(UIntN participantIndex, UIntN domainIndex, MbtHint::Type mbtHint) = 0;
	virtual void domainExtendedWorkloadPredictionChanged(UIntN participantIndex, UIntN domainIndex, ExtendedWorkloadPrediction::Type extendedWorkloadPrediction) = 0;
	virtual void domainFanOperatingModeChanged(
		UIntN participantIndex,
		UIntN domainIndex,
		FanOperatingMode::Type fanOperatingMode) = 0;
	virtual void domainPcieThrottleRequested(
		UIntN participantIndex,
		UIntN domainIndex,
		OnOffToggle::Type pcieThrottleRequested) = 0;

	// Policy Event Handlers
	virtual void activeRelationshipTableChanged() = 0;
	virtual void coolingModePolicyChanged(CoolingMode::Type coolingMode) = 0;
	virtual void foregroundApplicationChanged(const std::string& foregroundApplicationName) = 0;
	virtual void policyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2) = 0;
	virtual void operatingSystemPowerSourceChanged(OsPowerSource::Type powerSource) = 0;
	virtual void operatingSystemLidStateChanged(OsLidState::Type lidState) = 0;
	virtual void operatingSystemBatteryPercentageChanged(UIntN batteryPercentage) = 0;
	virtual void operatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality) = 0;
	virtual void operatingSystemPlatformTypeChanged(OsPlatformType::Type osPlatformType) = 0;
	virtual void operatingSystemDockModeChanged(OsDockMode::Type osDockMode) = 0;
	virtual void operatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState) = 0;
	virtual void operatingSystemMobileNotification(OsMobileNotificationType::Type notificationType, UIntN value) = 0;
	virtual void operatingSystemMixedRealityModeChanged(OnOffToggle::Type osMixedRealityMode) = 0;
	virtual void operatingSystemUserPresenceChanged(OsUserPresence::Type userPresence) = 0;
	virtual void operatingSystemSessionStateChanged(OsSessionState::Type sessionState) = 0;
	virtual void operatingSystemScreenStateChanged(OnOffToggle::Type screenState) = 0;
	virtual void operatingSystemBatteryCountChanged(UIntN batteryCount) = 0;
	virtual void operatingSystemPowerSliderChanged(OsPowerSlider::Type powerSlider) = 0;
	virtual void processLoaded(const std::string& processName, UInt64 processId) = 0;
	virtual void processUnLoaded(UInt64 processId) = 0;
	virtual void systemModeChanged(SystemMode::Type systemMode) = 0;
	virtual void passiveTableChanged() = 0;
	virtual void sensorOrientationChanged(SensorOrientation::Type sensorOrientation) = 0;
	virtual void sensorMotionChanged(OnOffToggle::Type sensorMotion) = 0;
	virtual void sensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation) = 0;
	virtual void thermalRelationshipTableChanged() = 0;
	virtual void adaptivePerformanceConditionsTableChanged() = 0;
	virtual void adaptivePerformanceActionsTableChanged() = 0;
	virtual void ddrfTableChanged() = 0;
	virtual void rfimTableChanged() = 0;
	virtual void tpgaTableChanged() = 0;
	virtual void opbtTableChanged() = 0;
	virtual void oemVariablesChanged() = 0;
	virtual void swOemVariablesChanged() = 0;
	virtual void powerBossConditionsTableChanged() = 0;
	virtual void powerBossActionsTableChanged() = 0;
	virtual void powerBossMathTableChanged() = 0;
	virtual void voltageThresholdMathTableChanged() = 0;
	virtual void emergencyCallModeTableChanged() = 0;
	virtual void pidAlgorithmTableChanged() = 0;
	virtual void activeControlPointRelationshipTableChanged() = 0;
	virtual void powerShareAlgorithmTableChanged() = 0;
	virtual void intelligentThermalManagementTableChanged() = 0;
	virtual void intelligentThermalManagementTable3Changed() = 0;
	virtual void energyPerformanceOptimizerTableChanged() = 0;
	virtual void powerLimitChanged() = 0;
	virtual void powerLimitTimeWindowChanged() = 0;
	virtual void performanceCapabilitiesChanged(UIntN participantIndex) = 0;
	virtual void workloadHintConfigurationChanged() = 0;
	virtual void operatingSystemGameModeChanged(OnOffToggle::Type osGameMode) = 0;
	virtual void powerShareAlgorithmTable2Changed() = 0;
	virtual void platformUserPresenceChanged(SensorUserPresence::Type userPresence) = 0;
	virtual void externalMonitorStateChanged(Bool externalMonitorStateChanged) = 0;
	virtual void userInteractionChanged(UserInteraction::Type userInteraction) = 0;
	virtual void foregroundRatioChanged(UIntN ratio) = 0;
	virtual void collaborationModeChanged(OnOffToggle::Type collaborationModeState) = 0;
	virtual void thirdPartyGraphicsPowerStateChanged(UInt32 tpgPowerStateOff) = 0;
	virtual void thirdPartyGraphicsTPPLimitChanged(OsPowerSource::Type powerSourceForTPP) = 0;
	virtual void systemConfigurationFeatureTableChanged() = 0;
	virtual void systemInBagChanged(SystemInBag::Type systemInBag) = 0;
	virtual void thirdPartyGraphicsReservedTgpChanged(Power reservedTgp) = 0;
	virtual void thirdPartyGraphicsOppBoostModeChanged(OpportunisticBoostMode::Type oppBoostMode) = 0;
	virtual void scenarioModeChanged(ScenarioMode::Type scenarioMode) = 0;
	virtual void dttGamingModeChanged(DttGamingMode::Type dttGamingMode) = 0;
	virtual void applicationOptimizationChanged(Bool isActive) = 0;
};

//
// The following functions must be exposed externally by the .dll/.so
//
extern "C"
{
	typedef UInt64 (*GetAppVersionFuncPtr)();
	dptf_public_export UInt64 GetAppVersion();

	typedef PolicyInterface* (*CreatePolicyInstanceFuncPtr)();
	dptf_public_export PolicyInterface* CreatePolicyInstance();

	typedef void (*DestroyPolicyInstanceFuncPtr)(PolicyInterface* policyInterface);
	dptf_public_export void DestroyPolicyInstance(PolicyInterface* policyInterface);
}
