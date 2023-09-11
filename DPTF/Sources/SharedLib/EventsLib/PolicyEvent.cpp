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
			CASE(DptfEnvironmentProfileChanged)
			CASE(DptfConnectedStandbyEntry)
			CASE(DptfConnectedStandbyExit)
			CASE(DptfLowPowerModeEntry)
			CASE(DptfLowPowerModeExit)
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
			CASE(DomainSocPowerFloorChanged)
			CASE(DomainPcieThrottleRequested)
			CASE(DomainEppSensitivityHintChanged)
			CASE(DomainExtendedWorkloadPredictionChanged)
			CASE(DomainFanOperatingModeChanged)
			CASE(DomainHardwareSocWorkloadHintChanged)
			CASE(PolicyCoolingModePolicyChanged)
			CASE(PolicyForegroundApplicationChanged)
			CASE(PolicyInitiatedCallback)
			CASE(PolicySensorOrientationChanged)
			CASE(PolicySensorMotionChanged)
			CASE(PolicySensorSpatialOrientationChanged)
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
			CASE(PolicyProcessLoadNotification)
			CASE(DptfPolicyLoadedUnloadedEvent)
			CASE(DptfPolicyActivityLoggingEnabled)
			CASE(DptfPolicyActivityLoggingDisabled)
			CASE(PolicyOperatingSystemPowerSchemePersonalityChanged)
			CASE(PolicyEmergencyCallModeTableChanged)
			CASE(PowerLimitChanged)
			CASE(PowerLimitTimeWindowChanged)
			CASE(PerformanceCapabilitiesChanged)
			CASE(PolicyWorkloadHintConfigurationChanged)
			CASE(PolicyOperatingSystemGameModeChanged)
			CASE(PolicyPlatformUserPresenceChanged)
			CASE(PolicyExternalMonitorStateChanged)
			CASE(PolicyUserInteractionChanged)
			CASE(PolicyForegroundRatioChanged)
			CASE(PolicySystemModeChanged)
			CASE(PolicyCollaborationChanged)
			CASE(PolicyThirdPartyGraphicsPowerStateChanged)
			CASE(PolicyOemVariablesChanged)
			CASE(PolicyThirdPartyGraphicsTPPLimitChanged)
			CASE(DptfAppBroadcastPrivileged)
			CASE(DptfAppBroadcastUnprivileged)
			CASE(PolicySwOemVariablesChanged)
		default :
			throw dptf_exception("PolicyEvent::Type is invalid.");
		}
	}

	Bool RequiresEsifEventRegistration(PolicyEvent::Type policyEventType)
	{
		return (
			(policyEventType == PolicyCoolingModePolicyChanged)
			|| (policyEventType == PolicyForegroundApplicationChanged)
			|| (policyEventType == PolicySensorOrientationChanged)
			|| (policyEventType == PolicySensorMotionChanged)
			|| (policyEventType == PolicySensorSpatialOrientationChanged)
			|| (policyEventType == PolicyOperatingSystemPowerSourceChanged)
			|| (policyEventType == PolicyOperatingSystemLidStateChanged)
			|| (policyEventType == PolicyOperatingSystemBatteryPercentageChanged)
			|| (policyEventType == PolicyOperatingSystemPlatformTypeChanged)
			|| (policyEventType == PolicyOperatingSystemDockModeChanged)
			|| (policyEventType == PolicyOperatingSystemMobileNotification)
			|| (policyEventType == PolicyOperatingSystemMixedRealityModeChanged)
			|| (policyEventType == PolicyOperatingSystemUserPresenceChanged)
			|| (policyEventType == PolicyOperatingSystemSessionStateChanged)
			|| (policyEventType == PolicyOperatingSystemScreenStateChanged)
			|| (policyEventType == PolicyOperatingSystemBatteryCountChanged)
			|| (policyEventType == PolicyOperatingSystemPowerSliderChanged)
			|| (policyEventType == PolicyOperatingSystemPowerSchemePersonalityChanged)
			|| (policyEventType == PolicyProcessLoadNotification)
			|| (policyEventType == PolicyEmergencyCallModeTableChanged)
			|| (policyEventType == PolicyWorkloadHintConfigurationChanged)
			|| (policyEventType == PolicyOperatingSystemGameModeChanged)
			|| (policyEventType == PolicyPlatformUserPresenceChanged)
			|| (policyEventType == PolicyExternalMonitorStateChanged)
			|| (policyEventType == PolicyUserInteractionChanged)
			|| (policyEventType == PolicyForegroundRatioChanged)
			|| (policyEventType == PolicyCollaborationChanged)
			|| (policyEventType == PolicyThirdPartyGraphicsPowerStateChanged)
			|| (policyEventType == PolicyOemVariablesChanged)
			|| (policyEventType == PolicyThirdPartyGraphicsTPPLimitChanged)
			|| (policyEventType == DptfAppBroadcastPrivileged)
			|| (policyEventType == DptfAppBroadcastUnprivileged));
	}

	std::string toString(PolicyEvent::Type type)
	{
		switch (type)
		{
		case DptfEnvironmentProfileChanged:
			return "DptfEnvironmentProfileChanged";
		case DptfAppBroadcastPrivileged:
			return "DptfAppBroadcastPrivileged";
		case DptfAppBroadcastUnprivileged:
			return "DptfAppBroadcastUnprivileged";
		case DptfConnectedStandbyEntry:
			return "DptfConnectedStandbyEntry";
		case DptfConnectedStandbyExit:
			return "DptfConnectedStandbyExit";
		case DptfLowPowerModeEntry:
			return "DptfLowPowerModeEntry";
		case DptfLowPowerModeExit:
			return "DptfLowPowerModeExit";
		case DptfSuspend:
			return "DptfSuspend";
		case DptfResume:
			return "DptfResume";
		case ParticipantSpecificInfoChanged:
			return "ParticipantSpecificInfoChanged";
		case DomainCoreControlCapabilityChanged:
			return "DomainCoreControlCapabilityChanged";
		case DomainDisplayControlCapabilityChanged:
			return "DomainDisplayControlCapabilityChanged";
		case DomainDisplayStatusChanged:
			return "DomainDisplayStatusChanged";
		case DomainPerformanceControlCapabilityChanged:
			return "DomainPerformanceControlCapabilityChanged";
		case DomainPerformanceControlsChanged:
			return "DomainPerformanceControlsChanged";
		case DomainPowerControlCapabilityChanged:
			return "DomainPowerControlCapabilityChanged";
		case DomainPriorityChanged:
			return "DomainPriorityChanged";
		case DomainRadioConnectionStatusChanged:
			return "DomainRadioConnectionStatusChanged";
		case DomainRfProfileChanged:
			return "DomainRfProfileChanged";
		case DomainTemperatureThresholdCrossed:
			return "DomainTemperatureThresholdCrossed";
		case DomainVirtualSensorCalibrationTableChanged:
			return "DomainVirtualSensorCalibrationTableChanged";
		case DomainVirtualSensorPollingTableChanged:
			return "DomainVirtualSensorPollingTableChanged";
		case DomainVirtualSensorRecalcChanged:
			return "DomainVirtualSensorRecalcChanged";
		case DomainBatteryStatusChanged:
			return "DomainBatteryStatusChanged";
		case DomainBatteryInformationChanged:
			return "DomainBatteryInformationChanged";
		case DomainBatteryHighFrequencyImpedanceChanged:
			return "DomainBatteryHighFrequencyImpedanceChanged";
		case DomainBatteryNoLoadVoltageChanged:
			return "DomainBatteryNoLoadVoltageChanged";
		case DomainMaxBatteryPeakCurrentChanged:
			return "DomainMaxBatteryPeakCurrentChanged";
		case DomainPlatformPowerSourceChanged:
			return "DomainPlatformPowerSourceChanged";
		case DomainAdapterPowerRatingChanged:
			return "DomainAdapterPowerRatingChanged";
		case DomainChargerTypeChanged:
			return "DomainChargerTypeChanged";
		case DomainPlatformRestOfPowerChanged:
			return "DomainPlatformRestOfPowerChanged";
		case DomainMaxBatteryPowerChanged:
			return "DomainMaxBatteryPowerChanged";
		case DomainPlatformBatterySteadyStateChanged:
			return "DomainPlatformBatterySteadyStateChanged";
		case DomainACNominalVoltageChanged:
			return "DomainACNominalVoltageChanged";
		case DomainACOperationalCurrentChanged:
			return "DomainACOperationalCurrentChanged";
		case DomainAC1msPercentageOverloadChanged:
			return "DomainAC1msPercentageOverloadChanged";
		case DomainAC2msPercentageOverloadChanged:
			return "DomainAC2msPercentageOverloadChanged";
		case DomainAC10msPercentageOverloadChanged:
			return "DomainAC10msPercentageOverloadChanged";
		case DomainEnergyThresholdCrossed:
			return "DomainEnergyThresholdCrossed";
		case DomainFanCapabilityChanged:
			return "DomainFanCapabilityChanged";
		case DomainSocWorkloadClassificationChanged:
			return "DomainSocWorkloadClassificationChanged";
		case DomainSocPowerFloorChanged:
			return "DomainSocPowerFloorChanged";
		case DomainPcieThrottleRequested:
			return "DomainPcieThrottleRequested";
		case DomainEppSensitivityHintChanged:
			return "DomainEppSensitivityHintChanged";
		case DomainExtendedWorkloadPredictionChanged:
			return "DomainExtendedWorkloadPredictionChanged";
		case DomainFanOperatingModeChanged:
			return "DomainFanOperatingModeChanged";
		case DomainHardwareSocWorkloadHintChanged:
			return "DomainHardwareSocWorkloadHintChanged";
		case PolicyCoolingModePolicyChanged:
			return "PolicyCoolingModePolicyChanged";
		case PolicyForegroundApplicationChanged:
			return "PolicyForegroundApplicationChanged";
		case PolicyInitiatedCallback:
			return "PolicyInitiatedCallback";
		case PolicyPassiveTableChanged:
			return "PolicyPassiveTableChanged";
		case PolicySensorOrientationChanged:
			return "PolicySensorOrientationChanged";
		case PolicySensorMotionChanged:
			return "PolicySensorMotionChanged";
		case PolicySensorSpatialOrientationChanged:
			return "PolicySensorSpatialOrientationChanged";
		case PolicyThermalRelationshipTableChanged:
			return "PolicyThermalRelationshipTableChanged";
		case PolicyActiveRelationshipTableChanged:
			return "PolicyActiveRelationshipTableChanged";
		case PolicyAdaptivePerformanceConditionsTableChanged:
			return "PolicyAdaptivePerformanceConditionsTableChanged";
		case PolicyAdaptivePerformanceActionsTableChanged:
			return "PolicyAdaptivePerformanceActionsTableChanged";
		case PolicyDdrfTableChanged:
			return "PolicyDdrfTableChanged";
		case PolicyRfimTableChanged:
			return "PolicyRfimTableChanged";
		case PolicyTpgaTableChanged:
			return "PolicyTpgaTableChanged";
		case PolicyOperatingSystemPowerSourceChanged:
			return "PolicyOperatingSystemPowerSourceChanged";
		case PolicyOperatingSystemLidStateChanged:
			return "PolicyOperatingSystemLidStateChanged";
		case PolicyOperatingSystemBatteryPercentageChanged:
			return "PolicyOperatingSystemBatteryPercentageChanged";
		case PolicyOperatingSystemPlatformTypeChanged:
			return "PolicyOperatingSystemPlatformTypeChanged";
		case PolicyOperatingSystemDockModeChanged:
			return "PolicyOperatingSystemDockModeChanged";
		case PolicyOperatingSystemMobileNotification:
			return "PolicyOperatingSystemMobileNotification";
		case PolicyOperatingSystemMixedRealityModeChanged:
			return "PolicyOperatingSystemMixedRealityModeChanged";
		case PolicyOperatingSystemUserPresenceChanged:
			return "PolicyOperatingSystemUserPresenceChanged";
		case PolicyOperatingSystemSessionStateChanged:
			return "PolicyOperatingSystemSessionStateChanged";
		case PolicyOperatingSystemScreenStateChanged:
			return "PolicyOperatingSystemScreenStateChanged";
		case PolicyOperatingSystemBatteryCountChanged:
			return "PolicyOperatingSystemBatteryCountChanged";
		case PolicyOperatingSystemPowerSliderChanged:
			return "PolicyOperatingSystemPowerSliderChanged";
		case PolicyOemVariablesChanged:
			return "PolicyOemVariablesChanged";
		case PolicyProcessLoadNotification:
			return "PolicyProcessLoadNotification";
		case PolicyPowerBossConditionsTableChanged:
			return "PolicyPowerBossConditionsTableChanged";
		case PolicyPowerBossActionsTableChanged:
			return "PolicyPowerBossActionsTableChanged";
		case PolicyPowerBossMathTableChanged:
			return "PolicyPowerBossMathTableChanged";
		case PolicyVoltageThresholdMathTableChanged:
			return "PolicyVoltageThresholdMathTableChanged";
		case DptfPolicyLoadedUnloadedEvent:
			return "DptfPolicyLoadedUnloadedEvent";
		case DptfPolicyActivityLoggingEnabled:
			return "DptfPolicyActivityLoggingEnabled";
		case DptfPolicyActivityLoggingDisabled:
			return "DptfPolicyActivityLoggingDisabled";
		case PolicyOperatingSystemPowerSchemePersonalityChanged:
			return "PolicyOperatingSystemPowerSchemePersonalityChanged";
		case PolicyEmergencyCallModeTableChanged:
			return "PolicyEmergencyCallModeTableChanged";
		case PolicyPidAlgorithmTableChanged:
			return "PolicyPidAlgorithmTableChanged";
		case PolicyIntelligentThermalManagementTableChanged:
			return "PolicyIntelligentThermalManagementTableChanged";
		case PolicyActiveControlPointRelationshipTableChanged:
			return "PolicyActiveControlPointRelationshipTableChanged";
		case PolicyPowerShareAlgorithmTableChanged:
			return "PolicyPowerShareAlgorithmTableChanged";
		case PolicyEnergyPerformanceOptimizerTableChanged:
			return "PolicyEnergyPerformanceOptimizerTableChanged";
		case PowerLimitChanged:
			return "PowerLimitChanged";
		case PowerLimitTimeWindowChanged:
			return "PowerLimitTimeWindowChanged";
		case PerformanceCapabilitiesChanged:
			return "PerformanceCapabilitiesChanged";
		case PolicyWorkloadHintConfigurationChanged:
			return "PolicyWorkloadHintConfigurationChanged";
		case PolicyOperatingSystemGameModeChanged:
			return "PolicyOperatingSystemGameModeChanged";
		case PolicyPowerShareAlgorithmTable2Changed:
			return "PolicyPowerShareAlgorithmTable2Changed";
		case PolicyPlatformUserPresenceChanged:
			return "PolicyPlatformUserPresenceChanged";
		case PolicyExternalMonitorStateChanged:
			return "PolicyExternalMonitorStateChanged";
		case PolicyUserInteractionChanged:
			return "PolicyUserInteractionChanged";
		case PolicyForegroundRatioChanged:
			return "PolicyForegroundRatioChanged";
		case PolicySystemModeChanged:
			return "PolicySystemModeChanged";
		case PolicyCollaborationChanged:
			return "PolicyCollaborationChanged";
		case PolicyThirdPartyGraphicsPowerStateChanged:
			return "PolicyThirdPartyGraphicsPowerStateChanged";
		case PolicySwOemVariablesChanged:
			return "PolicySwOemVariablesChanged";
		case PolicyThirdPartyGraphicsTPPLimitChanged:
			return "PolicyThirdPartyGraphicsTPPLimitChanged";
		case Invalid:
		case Max:
		default:
			throw dptf_exception("Event type is not a Policy Event.");
		}
	}
}
