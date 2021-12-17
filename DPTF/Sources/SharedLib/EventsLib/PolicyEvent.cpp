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

#include "PolicyEvent.h"
#include "Dptf.h"

//
// FIXME: the code in this file may be called enough that we should change it to an index based lookup.  Need
// to run a profiler and see.
//

#define CASE(eventType)                                                                                                \
	case eventType:                                                                                                    \
		return FrameworkEvent::eventType;

namespace PolicyEvent
{
	// FIXME:  update to use hash table

	FrameworkEvent::Type ToFrameworkEvent(PolicyEvent::Type policyEventType)
	{
		switch (policyEventType)
		{
			CASE(DptfConnectedStandbyEntry)
			CASE(DptfConnectedStandbyExit)
			CASE(DptfSuspend)
			CASE(DptfResume)
			CASE(ParticipantSpecificInfoChanged)
			CASE(DomainCoreControlCapabilityChanged)
			CASE(DomainDisplayControlCapabilityChanged)
			CASE(DomainDisplayStatusChanged)
			CASE(DomainPerformanceControlCapabilityChanged)
			CASE(DomainPerformanceControlsChanged)
			CASE(DomainPowerControlCapabilityChanged)
			CASE(DomainPriorityChanged)
			CASE(DomainRadioConnectionStatusChanged)
			CASE(DomainRfProfileChanged)
			CASE(DomainTemperatureThresholdCrossed)
			CASE(DomainVirtualSensorCalibrationTableChanged)
			CASE(DomainVirtualSensorPollingTableChanged)
			CASE(DomainVirtualSensorRecalcChanged)
			CASE(DomainBatteryStatusChanged)
			CASE(DomainBatteryInformationChanged)
			CASE(DomainBatteryHighFrequencyImpedanceChanged)
			CASE(DomainBatteryNoLoadVoltageChanged)
			CASE(DomainMaxBatteryPeakCurrentChanged)
			CASE(DomainPlatformPowerSourceChanged)
			CASE(DomainAdapterPowerRatingChanged)
			CASE(DomainChargerTypeChanged)
			CASE(DomainPlatformRestOfPowerChanged)
			CASE(DomainMaxBatteryPowerChanged)
			CASE(DomainPlatformBatterySteadyStateChanged)
			CASE(DomainACNominalVoltageChanged)
			CASE(DomainACOperationalCurrentChanged)
			CASE(DomainAC1msPercentageOverloadChanged)
			CASE(DomainAC2msPercentageOverloadChanged)
			CASE(DomainAC10msPercentageOverloadChanged)
			CASE(DomainEnergyThresholdCrossed)
			CASE(DomainFanCapabilityChanged)
			CASE(DomainSocWorkloadClassificationChanged)
			CASE(DomainEppSensitivityHintChanged)
			CASE(PolicyActiveRelationshipTableChanged)
			CASE(PolicyCoolingModePolicyChanged)
			CASE(PolicyForegroundApplicationChanged)
			CASE(PolicyInitiatedCallback)
			CASE(PolicyPassiveTableChanged)
			CASE(PolicySensorOrientationChanged)
			CASE(PolicySensorMotionChanged)
			CASE(PolicySensorSpatialOrientationChanged)
			CASE(PolicyThermalRelationshipTableChanged)
			CASE(PolicyOperatingSystemPowerSourceChanged)
			CASE(PolicyOperatingSystemLidStateChanged)
			CASE(PolicyOperatingSystemBatteryPercentageChanged)
			CASE(PolicyOperatingSystemPlatformTypeChanged)
			CASE(PolicyOperatingSystemDockModeChanged)
			CASE(PolicyOperatingSystemMobileNotification)
			CASE(PolicyOperatingSystemMixedRealityModeChanged)
			CASE(PolicyOperatingSystemUserPresenceChanged)
			CASE(PolicyOperatingSystemSessionStateChanged)
			CASE(PolicyOperatingSystemScreenStateChanged)
			CASE(PolicyOperatingSystemBatteryCountChanged)
			CASE(PolicyOperatingSystemPowerSliderChanged)
			CASE(PolicyOemVariablesChanged)
			CASE(PolicyPowerBossConditionsTableChanged)
			CASE(PolicyPowerBossActionsTableChanged)
			CASE(PolicyPowerBossMathTableChanged)
			CASE(PolicyVoltageThresholdMathTableChanged)
			CASE(DptfPolicyLoadedUnloadedEvent)
			CASE(DptfPolicyActivityLoggingEnabled)
			CASE(DptfPolicyActivityLoggingDisabled)
			CASE(PolicyOperatingSystemPowerSchemePersonalityChanged)
			CASE(PolicyEmergencyCallModeTableChanged)
			CASE(PolicyPidAlgorithmTableChanged)
			CASE(PolicyActiveControlPointRelationshipTableChanged)
			CASE(PolicyPowerShareAlgorithmTableChanged)
			CASE(PowerLimitChanged)
			CASE(PerformanceCapabilitiesChanged)
			CASE(PolicyWorkloadHintConfigurationChanged)
			CASE(PolicyOperatingSystemGameModeChanged)
			CASE(PolicyPowerShareAlgorithmTable2Changed)
			CASE(PolicyPlatformUserPresenceChanged)
			CASE(PolicyExternalMonitorStateChanged)
			CASE(PolicyUserInteractionChanged)
			CASE(PolicyForegroundRatioChanged)
			CASE(PolicySystemModeChanged)
			CASE(PolicyCollaborationChanged)
			CASE(PolicyThirdPartyGraphicsPowerStateChanged)
			CASE(PolicyAppBroadcastListen)
		default :
			throw dptf_exception("PolicyEvent::Type is invalid.");
		}
	}

	Bool RequiresEsifEventRegistration(PolicyEvent::Type policyEventType)
	{
		return (
			(policyEventType == PolicyEvent::PolicyActiveRelationshipTableChanged)
			|| (policyEventType == PolicyEvent::PolicyCoolingModePolicyChanged)
			|| (policyEventType == PolicyEvent::PolicyForegroundApplicationChanged)
			|| (policyEventType == PolicyEvent::PolicyPassiveTableChanged)
			|| (policyEventType == PolicyEvent::PolicySensorOrientationChanged)
			|| (policyEventType == PolicyEvent::PolicySensorMotionChanged)
			|| (policyEventType == PolicyEvent::PolicySensorSpatialOrientationChanged)
			|| (policyEventType == PolicyEvent::PolicyThermalRelationshipTableChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemPowerSourceChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemLidStateChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemBatteryPercentageChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemPlatformTypeChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemDockModeChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemMobileNotification)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemMixedRealityModeChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemUserPresenceChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemSessionStateChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemScreenStateChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemBatteryCountChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemPowerSliderChanged)
			|| (policyEventType == PolicyEvent::PolicyOemVariablesChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerBossConditionsTableChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerBossActionsTableChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerBossMathTableChanged)
			|| (policyEventType == PolicyEvent::PolicyVoltageThresholdMathTableChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemPowerSchemePersonalityChanged)
			|| (policyEventType == PolicyEvent::PolicyEmergencyCallModeTableChanged)
			|| (policyEventType == PolicyEvent::PolicyPidAlgorithmTableChanged)
			|| (policyEventType == PolicyEvent::PolicyActiveControlPointRelationshipTableChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerShareAlgorithmTableChanged)
			|| (policyEventType == PolicyEvent::PolicyWorkloadHintConfigurationChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemGameModeChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerShareAlgorithmTable2Changed)
			|| (policyEventType == PolicyEvent::PolicyPlatformUserPresenceChanged)
			|| (policyEventType == PolicyEvent::PolicyExternalMonitorStateChanged)
			|| (policyEventType == PolicyEvent::PolicyUserInteractionChanged)
			|| (policyEventType == PolicyEvent::PolicyForegroundRatioChanged)
			|| (policyEventType == PolicyEvent::PolicyCollaborationChanged)
			|| (policyEventType == PolicyEvent::PolicyThirdPartyGraphicsPowerStateChanged)
			|| (policyEventType == PolicyEvent::PolicyAppBroadcastListen));
	}

	std::string toString(Type type)
	{
		switch (type)
		{
		case PolicyEvent::PolicyAppBroadcastListen:
			return "PolicyAppBroadcastListen";
		case PolicyEvent::DptfConnectedStandbyEntry:
			return "DptfConnectedStandbyEntry";
		case PolicyEvent::DptfConnectedStandbyExit:
			return "DptfConnectedStandbyExit";
		case PolicyEvent::DptfSuspend:
			return "DptfSuspend";
		case PolicyEvent::DptfResume:
			return "DptfResume";
		case PolicyEvent::ParticipantSpecificInfoChanged:
			return "ParticipantSpecificInfoChanged";
		case PolicyEvent::DomainCoreControlCapabilityChanged:
			return "DomainCoreControlCapabilityChanged";
		case PolicyEvent::DomainDisplayControlCapabilityChanged:
			return "DomainDisplayControlCapabilityChanged";
		case PolicyEvent::DomainDisplayStatusChanged:
			return "DomainDisplayStatusChanged";
		case PolicyEvent::DomainPerformanceControlCapabilityChanged:
			return "DomainPerformanceControlCapabilityChanged";
		case PolicyEvent::DomainPerformanceControlsChanged:
			return "DomainPerformanceControlsChanged";
		case PolicyEvent::DomainPowerControlCapabilityChanged:
			return "DomainPowerControlCapabilityChanged";
		case PolicyEvent::DomainPriorityChanged:
			return "DomainPriorityChanged";
		case PolicyEvent::DomainRadioConnectionStatusChanged:
			return "DomainRadioConnectionStatusChanged";
		case PolicyEvent::DomainRfProfileChanged:
			return "DomainRfProfileChanged";
		case PolicyEvent::DomainTemperatureThresholdCrossed:
			return "DomainTemperatureThresholdCrossed";
		case PolicyEvent::DomainVirtualSensorCalibrationTableChanged:
			return "DomainVirtualSensorCalibrationTableChanged";
		case PolicyEvent::DomainVirtualSensorPollingTableChanged:
			return "DomainVirtualSensorPollingTableChanged";
		case PolicyEvent::DomainVirtualSensorRecalcChanged:
			return "DomainVirtualSensorRecalcChanged";
		case PolicyEvent::DomainBatteryStatusChanged:
			return "DomainBatteryStatusChanged";
		case PolicyEvent::DomainBatteryInformationChanged:
			return "DomainBatteryInformationChanged";
		case PolicyEvent::DomainBatteryHighFrequencyImpedanceChanged:
			return "DomainBatteryHighFrequencyImpedanceChanged";
		case PolicyEvent::DomainBatteryNoLoadVoltageChanged:
			return "DomainBatteryNoLoadVoltageChanged";
		case PolicyEvent::DomainMaxBatteryPeakCurrentChanged:
			return "DomainMaxBatteryPeakCurrentChanged";
		case PolicyEvent::DomainPlatformPowerSourceChanged:
			return "DomainPlatformPowerSourceChanged";
		case PolicyEvent::DomainAdapterPowerRatingChanged:
			return "DomainAdapterPowerRatingChanged";
		case PolicyEvent::DomainChargerTypeChanged:
			return "DomainChargerTypeChanged";
		case PolicyEvent::DomainPlatformRestOfPowerChanged:
			return "DomainPlatformRestOfPowerChanged";
		case PolicyEvent::DomainMaxBatteryPowerChanged:
			return "DomainMaxBatteryPowerChanged";
		case PolicyEvent::DomainPlatformBatterySteadyStateChanged:
			return "DomainPlatformBatterySteadyStateChanged";
		case PolicyEvent::DomainACNominalVoltageChanged:
			return "DomainACNominalVoltageChanged";
		case PolicyEvent::DomainACOperationalCurrentChanged:
			return "DomainACOperationalCurrentChanged";
		case PolicyEvent::DomainAC1msPercentageOverloadChanged:
			return "DomainAC1msPercentageOverloadChanged";
		case PolicyEvent::DomainAC2msPercentageOverloadChanged:
			return "DomainAC2msPercentageOverloadChanged";
		case PolicyEvent::DomainAC10msPercentageOverloadChanged:
			return "DomainAC10msPercentageOverloadChanged";
		case PolicyEvent::DomainEnergyThresholdCrossed:
			return "DomainEnergyThresholdCrossed";
		case PolicyEvent::DomainFanCapabilityChanged:
			return "DomainFanCapabilityChanged";
		case PolicyEvent::DomainSocWorkloadClassificationChanged:
			return "DomainSocWorkloadClassificationChanged";
		case PolicyEvent::DomainEppSensitivityHintChanged:
			return "DomainEppSensitivityHintChanged";
		case PolicyEvent::PolicyActiveRelationshipTableChanged:
			return "PolicyActiveRelationshipTableChanged";
		case PolicyEvent::PolicyCoolingModePolicyChanged:
			return "PolicyCoolingModePolicyChanged";
		case PolicyEvent::PolicyForegroundApplicationChanged:
			return "PolicyForegroundApplicationChanged";
		case PolicyEvent::PolicyInitiatedCallback:
			return "PolicyInitiatedCallback";
		case PolicyEvent::PolicyPassiveTableChanged:
			return "PolicyPassiveTableChanged";
		case PolicyEvent::PolicySensorOrientationChanged:
			return "PolicySensorOrientationChanged";
		case PolicyEvent::PolicySensorMotionChanged:
			return "PolicySensorMotionChanged";
		case PolicyEvent::PolicySensorSpatialOrientationChanged:
			return "PolicySensorSpatialOrientationChanged";
		case PolicyEvent::PolicyThermalRelationshipTableChanged:
			return "PolicyThermalRelationshipTableChanged";
		case PolicyEvent::PolicyAdaptivePerformanceConditionsTableChanged:
			return "PolicyAdaptivePerformanceConditionsTableChanged";
		case PolicyEvent::PolicyAdaptivePerformanceActionsTableChanged:
			return "PolicyAdaptivePerformanceActionsTableChanged";
		case PolicyEvent::PolicyDdrfTableChanged:
			return "PolicyDdrfTableChanged";
		case PolicyEvent::PolicyTpgaTableChanged:
			return "PolicyTpgaTableChanged";
		case PolicyEvent::PolicyOperatingSystemPowerSourceChanged:
			return "PolicyOperatingSystemPowerSourceChanged";
		case PolicyEvent::PolicyOperatingSystemLidStateChanged:
			return "PolicyOperatingSystemLidStateChanged";
		case PolicyEvent::PolicyOperatingSystemBatteryPercentageChanged:
			return "PolicyOperatingSystemBatteryPercentageChanged";
		case PolicyEvent::PolicyOperatingSystemPlatformTypeChanged:
			return "PolicyOperatingSystemPlatformTypeChanged";
		case PolicyEvent::PolicyOperatingSystemDockModeChanged:
			return "PolicyOperatingSystemDockModeChanged";
		case PolicyEvent::PolicyOperatingSystemMobileNotification:
			return "PolicyOperatingSystemMobileNotification";
		case PolicyEvent::PolicyOperatingSystemMixedRealityModeChanged:
			return "PolicyOperatingSystemMixedRealityModeChanged";
		case PolicyEvent::PolicyOperatingSystemUserPresenceChanged:
			return "PolicyOperatingSystemUserPresenceChanged";
		case PolicyEvent::PolicyOperatingSystemSessionStateChanged:
			return "PolicyOperatingSystemSessionStateChanged";
		case PolicyEvent::PolicyOperatingSystemScreenStateChanged:
			return "PolicyOperatingSystemScreenStateChanged";
		case PolicyEvent::PolicyOperatingSystemBatteryCountChanged:
			return "PolicyOperatingSystemBatteryCountChanged";
		case PolicyEvent::PolicyOperatingSystemPowerSliderChanged:
			return "PolicyOperatingSystemPowerSliderChanged";
		case PolicyEvent::PolicyOemVariablesChanged:
			return "PolicyOemVariablesChanged";
		case PolicyEvent::PolicyPowerBossConditionsTableChanged:
			return "PolicyPowerBossConditionsTableChanged";
		case PolicyEvent::PolicyPowerBossActionsTableChanged:
			return "PolicyPowerBossActionsTableChanged";
		case PolicyEvent::PolicyPowerBossMathTableChanged:
			return "PolicyPowerBossMathTableChanged";
		case PolicyEvent::PolicyVoltageThresholdMathTableChanged:
			return "PolicyVoltageThresholdMathTableChanged";
		case PolicyEvent::DptfPolicyLoadedUnloadedEvent:
			return "DptfPolicyLoadedUnloadedEvent";
		case PolicyEvent::DptfPolicyActivityLoggingEnabled:
			return "DptfPolicyActivityLoggingEnabled";
		case PolicyEvent::DptfPolicyActivityLoggingDisabled:
			return "DptfPolicyActivityLoggingDisabled";
		case PolicyEvent::PolicyOperatingSystemPowerSchemePersonalityChanged:
			return "PolicyOperatingSystemPowerSchemePersonalityChanged";
		case PolicyEvent::PolicyEmergencyCallModeTableChanged:
			return "PolicyEmergencyCallModeTableChanged";
		case PolicyEvent::PolicyPidAlgorithmTableChanged:
			return "PolicyPidAlgorithmTableChanged";
		case PolicyEvent::PolicyIntelligentThermalManagementTableChanged:
			return "PolicyIntelligentThermalManagementTableChanged";
		case PolicyEvent::PolicyActiveControlPointRelationshipTableChanged:
			return "PolicyActiveControlPointRelationshipTableChanged";
		case PolicyEvent::PolicyPowerShareAlgorithmTableChanged:
			return "PolicyPowerShareAlgorithmTableChanged";
		case PolicyEvent::PolicyEnergyPerformanceOptimizerTableChanged:
			return "PolicyEnergyPerformanceOptimizerTableChanged";
		case PolicyEvent::PowerLimitChanged:
			return "PowerLimitChanged";
		case PolicyEvent::PerformanceCapabilitiesChanged:
			return "PerformanceCapabilitiesChanged";
		case PolicyEvent::PolicyWorkloadHintConfigurationChanged:
			return "PolicyWorkloadHintConfigurationChanged";
		case PolicyEvent::PolicyOperatingSystemGameModeChanged:
			return "PolicyOperatingSystemGameModeChanged";
		case PolicyEvent::PolicyPowerShareAlgorithmTable2Changed:
			return "PolicyPowerShareAlgorithmTable2Changed";
		case PolicyEvent::PolicyPlatformUserPresenceChanged:
			return "PolicyPlatformUserPresenceChanged";
		case PolicyEvent::PolicyExternalMonitorStateChanged:
			return "PolicyExternalMonitorStateChanged";
		case PolicyEvent::PolicyUserInteractionChanged:
			return "PolicyUserInteractionChanged";
		case PolicyEvent::PolicyForegroundRatioChanged:
			return "PolicyForegroundRatioChanged";
		case PolicyEvent::PolicySystemModeChanged:
			return "PolicySystemModeChanged";
		case PolicyEvent::PolicyCollaborationChanged:
			return "PolicyCollaborationChanged";
		case PolicyEvent::PolicyThirdPartyGraphicsPowerStateChanged:
			return "PolicyThirdPartyGraphicsPowerStateChanged";
		case PolicyEvent::Invalid:
		case PolicyEvent::Max:
		default:
			throw dptf_exception("Event type is not a Policy Event.");
		}
	}
}
