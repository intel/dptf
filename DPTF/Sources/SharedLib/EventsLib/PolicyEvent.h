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

#pragma once

#include "Dptf.h"
#include "FrameworkEvent.h"

// This is the list of events that policies can subscribe to.

namespace PolicyEvent
{
	enum Type
	{
		Invalid,
		DptfConnectedStandbyEntry,
		DptfConnectedStandbyExit,
		DptfSuspend,
		DptfResume,
		ParticipantSpecificInfoChanged,
		DomainCoreControlCapabilityChanged,
		DomainDisplayControlCapabilityChanged,
		DomainDisplayStatusChanged,
		DomainPerformanceControlCapabilityChanged,
		DomainPerformanceControlsChanged,
		DomainPowerControlCapabilityChanged,
		DomainPriorityChanged,
		DomainRadioConnectionStatusChanged,
		DomainRfProfileChanged,
		DomainTemperatureThresholdCrossed,
		DomainVirtualSensorCalibrationTableChanged,
		DomainVirtualSensorPollingTableChanged,
		DomainVirtualSensorRecalcChanged,
		DomainBatteryStatusChanged,
		DomainBatteryInformationChanged,
		DomainBatteryHighFrequencyImpedanceChanged,
		DomainBatteryNoLoadVoltageChanged,
		DomainMaxBatteryPeakCurrentChanged,
		DomainPlatformPowerSourceChanged,
		DomainAdapterPowerRatingChanged,
		DomainChargerTypeChanged,
		DomainPlatformRestOfPowerChanged,
		DomainMaxBatteryPowerChanged,
		DomainPlatformBatterySteadyStateChanged,
		DomainACNominalVoltageChanged,
		DomainACOperationalCurrentChanged,
		DomainAC1msPercentageOverloadChanged,
		DomainAC2msPercentageOverloadChanged,
		DomainAC10msPercentageOverloadChanged,
		DomainEnergyThresholdCrossed,
		DomainFanCapabilityChanged,
		DomainSocWorkloadClassificationChanged,
		DomainEppSensitivityHintChanged,
		PolicyActiveRelationshipTableChanged,
		PolicyCoolingModePolicyChanged,
		PolicyForegroundApplicationChanged,
		PolicyInitiatedCallback,
		PolicyPassiveTableChanged,
		PolicySensorOrientationChanged,
		PolicySensorMotionChanged,
		PolicySensorSpatialOrientationChanged,
		PolicyThermalRelationshipTableChanged,
		PolicyAdaptiveUserPresenceTableChanged,
		PolicyAdaptivePerformanceConditionsTableChanged,
		PolicyAdaptivePerformanceParticipantConditionTableChanged,
		PolicyAdaptivePerformanceActionsTableChanged,
		PolicyOperatingSystemPowerSourceChanged,
		PolicyOperatingSystemLidStateChanged,
		PolicyOperatingSystemBatteryPercentageChanged,
		PolicyOperatingSystemPlatformTypeChanged,
		PolicyOperatingSystemDockModeChanged,
		PolicyOperatingSystemMobileNotification,
		PolicyOperatingSystemMixedRealityModeChanged,
		PolicyOperatingSystemUserPresenceChanged,
		PolicyOperatingSystemSessionStateChanged,
		PolicyOperatingSystemScreenStateChanged,
		PolicyOperatingSystemBatteryCountChanged,
		PolicyOperatingSystemPowerSliderChanged,
		PolicyOemVariablesChanged,
		PolicyPowerBossConditionsTableChanged,
		PolicyPowerBossActionsTableChanged,
		PolicyPowerBossMathTableChanged,
		PolicyVoltageThresholdMathTableChanged,
		DptfPolicyActivityLoggingEnabled,
		DptfPolicyActivityLoggingDisabled,
		DptfPolicyLoadedUnloadedEvent,
		PolicyOperatingSystemPowerSchemePersonalityChanged,
		PolicyEmergencyCallModeTableChanged,
		PolicyPidAlgorithmTableChanged,
		PolicyActiveControlPointRelationshipTableChanged,
		PolicyPowerShareAlgorithmTableChanged,
		PowerLimitChanged,
		PerformanceCapabilitiesChanged,
		PolicyWorkloadHintConfigurationChanged,
		PolicyOperatingSystemGameModeChanged,
		PolicyPowerShareAlgorithmTable2Changed,
		PolicySensorUserPresenceChanged,
		PolicyPlatformUserPresenceChanged,
		PolicyWakeOnApproachFeatureStateChanged,
		PolicyWakeOnApproachWithExternalMonitorFeatureStateChanged,
		PolicyWakeOnApproachLowBatteryFeatureStateChanged,
		PolicyWakeOnApproachBatteryRemainingPercentageChanged,
		PolicyWalkAwayLockFeatureStateChanged,
		PolicyWalkAwayLockWithExternalMonitorFeatureStateChanged,
		PolicyWalkAwayLockDimScreenFeatureStateChanged,
		PolicyWalkAwayLockDisplayOffAfterLockFeatureStateChanged,
		PolicyWalkAwayLockHonorPowerRequestsForDisplayFeatureStateChanged,
		PolicyWalkAwayLockHonorUserInCallFeatureStateChanged,
		PolicyUserInCallStateChanged,
		PolicyWalkAwayLockScreenLockWaitTimeChanged,
		PolicyWalkAwayLockPreDimWaitTimeChanged,
		PolicyWalkAwayLockUserPresentWaitTimeChanged,
		PolicyWalkAwayLockDimIntervalChanged,
		PolicyAdaptiveDimmingFeatureStateChanged,
		PolicyAdaptiveDimmingWithExternalMonitorFeatureStateChanged,
		PolicyAdaptiveDimmingWithPresentationModeFeatureStateChanged,
		PolicyAdaptiveDimmingPreDimWaitTimeChanged,
		PolicyMispredictionFaceDetectionFeatureStateChanged,
		PolicyMispredictionTimeWindowChanged,
		PolicyMisprediction1DimWaitTimeChanged,
		PolicyMisprediction2DimWaitTimeChanged,
		PolicyMisprediction3DimWaitTimeChanged,
		PolicyMisprediction4DimWaitTimeChanged,
		PolicyNoLockOnPresenceFeatureStateChanged,
		PolicyNoLockOnPresenceExternalMonitorFeatureStateChanged,
		PolicyNoLockOnPresenceOnBatteryFeatureStateChanged,
		PolicyNoLockOnPresenceBatteryRemainingPercentageChanged,
		PolicyNoLockOnPresenceResetWaitTimeChanged,
		PolicyFailsafeTimeoutChanged,
		PolicyContextServiceStatusChanged,
		PolicyExternalMonitorStateChanged,
		PolicyUserNotPresentDimTargetChanged,
		PolicyUserDisengagedDimmingIntervalChanged,
		PolicyUserDisengagedDimTargetChanged,
		PolicyUserDisengagedDimWaitTimeChanged,
		PolicySensorModeChanged,
		PolicyBiometricPresenceSensorInstanceChanged,
		PolicyUserInteractionChanged,
		PolicyUserPresenceCorrelationChanged,
		PolicyForegroundRatioChanged,
		Max
	};

	FrameworkEvent::Type ToFrameworkEvent(PolicyEvent::Type policyEventType);
	Bool RequiresEsifEventRegistration(PolicyEvent::Type policyEventType);
	std::string toString(PolicyEvent::Type type);
}
