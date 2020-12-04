/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
			CASE(PolicyAdaptivePerformanceConditionsTableChanged)
			CASE(PolicyAdaptivePerformanceParticipantConditionTableChanged)
			CASE(PolicyAdaptivePerformanceActionsTableChanged)
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
			CASE(PolicySensorUserPresenceChanged)
			CASE(PolicyAdaptiveUserPresenceTableChanged)
			CASE(PolicyPlatformUserPresenceChanged)
			CASE(PolicyWakeOnApproachFeatureStateChanged)
			CASE(PolicyWakeOnApproachWithExternalMonitorFeatureStateChanged)
			CASE(PolicyWakeOnApproachLowBatteryFeatureStateChanged)
			CASE(PolicyWakeOnApproachBatteryRemainingPercentageChanged)
			CASE(PolicyWalkAwayLockFeatureStateChanged)
			CASE(PolicyWalkAwayLockWithExternalMonitorFeatureStateChanged)
			CASE(PolicyWalkAwayLockDimScreenFeatureStateChanged)
			CASE(PolicyWalkAwayLockDisplayOffAfterLockFeatureStateChanged)
			CASE(PolicyWalkAwayLockHonorPowerRequestsForDisplayFeatureStateChanged)
			CASE(PolicyWalkAwayLockHonorUserInCallFeatureStateChanged)
			CASE(PolicyUserInCallStateChanged)
			CASE(PolicyWalkAwayLockScreenLockWaitTimeChanged)
			CASE(PolicyWalkAwayLockPreDimWaitTimeChanged)
			CASE(PolicyWalkAwayLockUserPresentWaitTimeChanged)
			CASE(PolicyWalkAwayLockDimIntervalChanged)
			CASE(PolicyAdaptiveDimmingFeatureStateChanged)
			CASE(PolicyAdaptiveDimmingWithExternalMonitorFeatureStateChanged)
			CASE(PolicyAdaptiveDimmingWithPresentationModeFeatureStateChanged)
			CASE(PolicyAdaptiveDimmingPreDimWaitTimeChanged)
			CASE(PolicyMispredictionFaceDetectionFeatureStateChanged)
			CASE(PolicyMispredictionTimeWindowChanged)
			CASE(PolicyMisprediction1DimWaitTimeChanged)
			CASE(PolicyMisprediction2DimWaitTimeChanged)
			CASE(PolicyMisprediction3DimWaitTimeChanged)
			CASE(PolicyMisprediction4DimWaitTimeChanged)
			CASE(PolicyNoLockOnPresenceFeatureStateChanged)
			CASE(PolicyNoLockOnPresenceExternalMonitorFeatureStateChanged)
			CASE(PolicyNoLockOnPresenceOnBatteryFeatureStateChanged)
			CASE(PolicyNoLockOnPresenceBatteryRemainingPercentageChanged)
			CASE(PolicyNoLockOnPresenceResetWaitTimeChanged)
			CASE(PolicyFailsafeTimeoutChanged)
			CASE(PolicyContextServiceStatusChanged)
			CASE(PolicyExternalMonitorStateChanged)
			CASE(PolicyUserNotPresentDimTargetChanged)
			CASE(PolicyUserDisengagedDimmingIntervalChanged)
			CASE(PolicyUserDisengagedDimTargetChanged)
			CASE(PolicyUserDisengagedDimWaitTimeChanged)
			CASE(PolicySensorModeChanged)
			CASE(PolicyBiometricPresenceSensorInstanceChanged)
			CASE(PolicyUserInteractionChanged)
			CASE(PolicyUserPresenceCorrelationChanged)
			CASE(PolicyForegroundRatioChanged)
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
			|| (policyEventType == PolicyEvent::PolicyAdaptivePerformanceConditionsTableChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptivePerformanceParticipantConditionTableChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptivePerformanceActionsTableChanged)
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
			|| (policyEventType == PolicyEvent::PolicySensorUserPresenceChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptiveUserPresenceTableChanged)
			|| (policyEventType == PolicyEvent::PolicyOperatingSystemGameModeChanged)
			|| (policyEventType == PolicyEvent::PolicyPowerShareAlgorithmTable2Changed)
			|| (policyEventType == PolicyEvent::PolicyPlatformUserPresenceChanged)
			|| (policyEventType == PolicyEvent::PolicyWakeOnApproachFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWakeOnApproachWithExternalMonitorFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWakeOnApproachLowBatteryFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWakeOnApproachBatteryRemainingPercentageChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockWithExternalMonitorFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockDimScreenFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockDisplayOffAfterLockFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockHonorPowerRequestsForDisplayFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockHonorUserInCallFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyUserInCallStateChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockScreenLockWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockPreDimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockUserPresentWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyWalkAwayLockDimIntervalChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptiveDimmingFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptiveDimmingWithExternalMonitorFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptiveDimmingWithPresentationModeFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyAdaptiveDimmingPreDimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyMispredictionFaceDetectionFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyMispredictionTimeWindowChanged)
			|| (policyEventType == PolicyEvent::PolicyMisprediction1DimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyMisprediction2DimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyMisprediction3DimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyMisprediction4DimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyNoLockOnPresenceFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyNoLockOnPresenceExternalMonitorFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyNoLockOnPresenceOnBatteryFeatureStateChanged)
			|| (policyEventType == PolicyEvent::PolicyNoLockOnPresenceBatteryRemainingPercentageChanged)
			|| (policyEventType == PolicyEvent::PolicyNoLockOnPresenceResetWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicyFailsafeTimeoutChanged)
			|| (policyEventType == PolicyEvent::PolicyContextServiceStatusChanged)
			|| (policyEventType == PolicyEvent::PolicyExternalMonitorStateChanged)
			|| (policyEventType == PolicyEvent::PolicyUserNotPresentDimTargetChanged)
			|| (policyEventType == PolicyEvent::PolicyUserDisengagedDimmingIntervalChanged)
			|| (policyEventType == PolicyEvent::PolicyUserDisengagedDimTargetChanged)
			|| (policyEventType == PolicyEvent::PolicyUserDisengagedDimWaitTimeChanged)
			|| (policyEventType == PolicyEvent::PolicySensorModeChanged)
			|| (policyEventType == PolicyEvent::PolicyBiometricPresenceSensorInstanceChanged)
			|| (policyEventType == PolicyEvent::PolicyUserInteractionChanged)
			|| (policyEventType == PolicyEvent::PolicyUserPresenceCorrelationChanged)
			|| (policyEventType == PolicyEvent::PolicyForegroundRatioChanged));
	}

	std::string toString(Type type)
	{
		switch (type)
		{
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
		case PolicyEvent::PolicyAdaptivePerformanceParticipantConditionTableChanged:
			return "PolicyAdaptivePerformanceParticipantConditionTableChanged";
		case PolicyEvent::PolicyAdaptivePerformanceActionsTableChanged:
			return "PolicyAdaptivePerformanceActionsTableChanged";
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
		case PolicyEvent::PolicyActiveControlPointRelationshipTableChanged:
			return "PolicyActiveControlPointRelationshipTableChanged";
		case PolicyEvent::PolicyPowerShareAlgorithmTableChanged:
			return "PolicyPowerShareAlgorithmTableChanged";
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
		case PolicyEvent::PolicySensorUserPresenceChanged:
			return "PolicySensorUserPresenceChanged";
		case PolicyEvent::PolicyPlatformUserPresenceChanged:
			return "PolicyPlatformUserPresenceChanged";
		case PolicyEvent::PolicyAdaptiveUserPresenceTableChanged:
			return "PolicyAdaptiveUserPresenceTableChanged";
		case PolicyEvent::PolicyWakeOnApproachFeatureStateChanged:
			return "PolicyWakeOnApproachFeatureStateChanged";
		case PolicyEvent::PolicyWakeOnApproachWithExternalMonitorFeatureStateChanged:
			return "PolicyWakeOnApproachWithExternalMonitorFeatureStateChanged";
		case PolicyEvent::PolicyWakeOnApproachLowBatteryFeatureStateChanged:
			return "PolicyWakeOnApproachLowBatteryFeatureStateChanged";
		case PolicyEvent::PolicyWakeOnApproachBatteryRemainingPercentageChanged:
			return "PolicyWakeOnApproachBatteryRemainingPercentageChanged";
		case PolicyEvent::PolicyWalkAwayLockFeatureStateChanged:
			return "PolicyWalkAwayLockFeatureStateChanged";
		case PolicyEvent::PolicyWalkAwayLockWithExternalMonitorFeatureStateChanged:
			return "PolicyWalkAwayLockWithExternalMonitorFeatureStateChanged";
		case PolicyEvent::PolicyWalkAwayLockDimScreenFeatureStateChanged:
			return "PolicyWalkAwayLockDimScreenFeatureStateChanged";
		case PolicyEvent::PolicyWalkAwayLockDisplayOffAfterLockFeatureStateChanged:
			return "PolicyWalkAwayLockDisplayOffAfterLockFeatureStateChanged";
		case PolicyEvent::PolicyWalkAwayLockHonorPowerRequestsForDisplayFeatureStateChanged:
			return "PolicyWalkAwayLockHonorPowerRequestsForDisplayFeatureStateChanged";
		case PolicyEvent::PolicyWalkAwayLockHonorUserInCallFeatureStateChanged:
			return "PolicyWalkAwayLockHonorUserInCallFeatureStateChanged";
		case PolicyEvent::PolicyUserInCallStateChanged:
			return "PolicyUserInCallStateChanged";
		case PolicyEvent::PolicyWalkAwayLockScreenLockWaitTimeChanged:
			return "PolicyWalkAwayLockScreenLockWaitTimeChanged";
		case PolicyEvent::PolicyWalkAwayLockPreDimWaitTimeChanged:
			return "PolicyWalkAwayLockPreDimWaitTimeChanged";
		case PolicyEvent::PolicyWalkAwayLockUserPresentWaitTimeChanged:
			return "PolicyWalkAwayLockUserPresentWaitTimeChanged";
		case PolicyEvent::PolicyWalkAwayLockDimIntervalChanged:
			return "PolicyWalkAwayLockDimIntervalChanged";
		case PolicyEvent::PolicyAdaptiveDimmingFeatureStateChanged:
			return "PolicyAdaptiveDimmingFeatureStateChanged";
		case PolicyEvent::PolicyAdaptiveDimmingWithExternalMonitorFeatureStateChanged:
			return "PolicyAdaptiveDimmingWithExternalMonitorFeatureStateChanged";
		case PolicyEvent::PolicyAdaptiveDimmingWithPresentationModeFeatureStateChanged:
			return "PolicyAdaptiveDimmingWithPresentationModeFeatureStateChanged";
		case PolicyEvent::PolicyAdaptiveDimmingPreDimWaitTimeChanged:
			return "PolicyAdaptiveDimmingPreDimWaitTimeChanged";
		case PolicyEvent::PolicyMispredictionFaceDetectionFeatureStateChanged:
			return "PolicyMispredictionFaceDetectionFeatureStateChanged";
		case PolicyEvent::PolicyMispredictionTimeWindowChanged:
			return "PolicyMispredictionTimeWindowChanged";
		case PolicyEvent::PolicyMisprediction1DimWaitTimeChanged:
			return "PolicyMisprediction1DimWaitTimeChanged";
		case PolicyEvent::PolicyMisprediction2DimWaitTimeChanged:
			return "PolicyMisprediction2DimWaitTimeChanged";
		case PolicyEvent::PolicyMisprediction3DimWaitTimeChanged:
			return "PolicyMisprediction3DimWaitTimeChanged";
		case PolicyEvent::PolicyMisprediction4DimWaitTimeChanged:
			return "PolicyMisprediction4DimWaitTimeChanged";
		case PolicyEvent::PolicyNoLockOnPresenceFeatureStateChanged:
			return "PolicyNoLockOnPresenceFeatureStateChanged";
		case PolicyEvent::PolicyNoLockOnPresenceExternalMonitorFeatureStateChanged:
			return "PolicyNoLockOnPresenceExternalMonitorFeatureStateChanged";
		case PolicyEvent::PolicyNoLockOnPresenceOnBatteryFeatureStateChanged:
			return "PolicyNoLockOnPresenceOnBatteryFeatureStateChanged";
		case PolicyEvent::PolicyNoLockOnPresenceBatteryRemainingPercentageChanged:
			return "PolicyNoLockOnPresenceBatteryRemainingPercentageChanged";
		case PolicyEvent::PolicyNoLockOnPresenceResetWaitTimeChanged:
			return "PolicyNoLockOnPresenceResetWaitTimeChanged";
		case PolicyEvent::PolicyFailsafeTimeoutChanged:
			return "PolicyFailsafeTimeoutChanged";
		case PolicyEvent::PolicyContextServiceStatusChanged:
			return "PolicyContextServiceStatusChanged";
		case PolicyEvent::PolicyExternalMonitorStateChanged:
			return "PolicyExternalMonitorStateChanged";
		case PolicyEvent::PolicyUserNotPresentDimTargetChanged:
			return "PolicyUserNotPresentDimTargetChanged";
		case PolicyEvent::PolicyUserDisengagedDimmingIntervalChanged:
			return "PolicyUserDisengagedDimmingIntervalChanged";
		case PolicyEvent::PolicyUserDisengagedDimTargetChanged:
			return "PolicyUserDisengagedDimTargetChanged";
		case PolicyEvent::PolicyUserDisengagedDimWaitTimeChanged:
			return "PolicyUserDisengagedDimWaitTimeChanged";
		case PolicyEvent::PolicySensorModeChanged:
			return "PolicySensorModeChanged";
		case PolicyEvent::PolicyBiometricPresenceSensorInstanceChanged:
			return "PolicyBiometricPresenceSensorInstanceChanged";
		case PolicyEvent::PolicyUserInteractionChanged:
			return "PolicyUserInteractionChanged";
		case PolicyEvent::PolicyUserPresenceCorrelationChanged:
			return "PolicyUserPresenceCorrelationChanged";
		case PolicyEvent::PolicyForegroundRatioChanged:
			return "PolicyForegroundRatioChanged";
		case PolicyEvent::Invalid:
		case PolicyEvent::Max:
		default:
			throw dptf_exception("Event type is not a Policy Event.");
		}
	}
}
