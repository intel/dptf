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

#pragma once

#include "Dptf.h"
#include "SupportedPolicyList.h"
#include "PolicyInterface.h"
#include "EsifLibrary.h"
#include "esif_sdk_logging_data.h"

class DptfManager;

class dptf_export Policy final
{
public:
	Policy(DptfManagerInterface* dptfManager);
	~Policy(void);

	void createPolicy(
		const std::string& policyFileName,
		UIntN newPolicyIndex,
		std::shared_ptr<SupportedPolicyList> supportedPolicyList);
	void destroyPolicy(void);

	Guid getGuid(void);

	void bindParticipant(UIntN participantIndex);
	void unbindParticipant(UIntN participantIndex);
	void bindDomain(UIntN participantIndex, UIntN domainIndex);
	void unbindDomain(UIntN participantIndex, UIntN domainIndex);

	void enable(void);
	void disable(void);

	std::string getName(void) const;
	std::string getPolicyFileName(void) const;
	std::string getStatusAsXml(void) const;
	std::string getDiagnosticsAsXml(void) const;

	// Event handlers

	void executeConnectedStandbyEntry(void);
	void executeConnectedStandbyExit(void);
	void executeSuspend(void);
	void executeResume(void);
	void executeDomainConfigTdpCapabilityChanged(UIntN participantIndex);
	void executeDomainCoreControlCapabilityChanged(UIntN participantIndex);
	void executeDomainDisplayControlCapabilityChanged(UIntN participantIndex);
	void executeDomainDisplayStatusChanged(UIntN participantIndex);
	void executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex);
	void executeDomainPerformanceControlsChanged(UIntN participantIndex);
	void executeDomainPowerControlCapabilityChanged(UIntN participantIndex);
	void executeDomainPriorityChanged(UIntN participantIndex);
	void executeDomainRadioConnectionStatusChanged(
		UIntN participantIndex,
		RadioConnectionStatus::Type radioConnectionStatus);
	void executeDomainRfProfileChanged(UIntN participantIndex);
	void executeDomainTemperatureThresholdCrossed(UIntN participantIndex);
	void executeParticipantSpecificInfoChanged(UIntN participantIndex);
	void executeDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex);
	void executeDomainVirtualSensorPollingTableChanged(UIntN participantIndex);
	void executeDomainVirtualSensorRecalcChanged(UIntN participantIndex);
	void executeDomainBatteryStatusChanged(UIntN participantIndex);
	void executeDomainBatteryInformationChanged(UIntN participantIndex);
	void executeDomainPlatformPowerSourceChanged(UIntN participantIndex);
	void executeDomainAdapterPowerRatingChanged(UIntN participantIndex);
	void executeDomainChargerTypeChanged(UIntN participantIndex);
	void executeDomainPlatformRestOfPowerChanged(UIntN participantIndex);
	void executeDomainMaxBatteryPowerChanged(UIntN participantIndex);
	void executeDomainPlatformBatterySteadyStateChanged(UIntN participantIndex);
	void executeDomainACNominalVoltageChanged(UIntN participantIndex);
	void executeDomainACOperationalCurrentChanged(UIntN participantIndex);
	void executeDomainAC1msPercentageOverloadChanged(UIntN participantIndex);
	void executeDomainAC2msPercentageOverloadChanged(UIntN participantIndex);
	void executeDomainAC10msPercentageOverloadChanged(UIntN participantIndex);
	void executeDomainEnergyThresholdCrossed(UIntN participantIndex);
	void executePolicyActiveRelationshipTableChanged(void);
	void executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode);
	void executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName);
	void executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2);
	void executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel);
	void executePolicyPassiveTableChanged(void);
	void executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation);
	void executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion);
	void executePolicySensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation);
	void executePolicyThermalRelationshipTableChanged(void);
	void executePolicyAdaptivePerformanceParticipantConditionTableChanged(void);
	void executePolicyAdaptivePerformanceConditionsTableChanged(void);
	void executePolicyAdaptivePerformanceActionsTableChanged(void);
	void executePolicyOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource);
	void executePolicyOperatingSystemLidStateChanged(OsLidState::Type lidState);
	void executePolicyOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage);
	void executePolicyOperatingSystemPowerSchemePersonalityChanged(
		OsPowerSchemePersonality::Type powerSchemePersonality);
	void executePolicyOperatingSystemPlatformTypeChanged(OsPlatformType::Type osPlatformType);
	void executePolicyOperatingSystemDockModeChanged(OsDockMode::Type osDockMode);
	void executePolicyOperatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState);
	void executePolicyOperatingSystemMobileNotification(OsMobileNotificationType::Type notificationType, UIntN value);
	void executePolicyOemVariablesChanged(void);
	void executePolicyPowerBossConditionsTableChanged(void);
	void executePolicyPowerBossActionsTableChanged(void);
	void executePolicyPowerBossMathTableChanged(void);
	void executePolicyActivityLoggingEnabled(void);
	void executePolicyActivityLoggingDisabled(void);
	void executePolicyEmergencyCallModeTableChanged(void);
	void executePolicyPidAlgorithmTableChanged(void);
	void executePolicyActiveControlPointRelationshipTableChanged(void);
	void executePolicyPowerShareAlgorithmTableChanged(void);
	void executePowerLimitChanged(void);
	void executePolicyWorkloadHintConfigurationChanged(void);

private:
	// hide the copy constructor and assignment operator.
	Policy(const Policy& rhs);
	Policy& operator=(const Policy& rhs);

	// Pointer to the DPTF manager class which has access to most of the manager components.
	DptfManagerInterface* m_dptfManager;

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
