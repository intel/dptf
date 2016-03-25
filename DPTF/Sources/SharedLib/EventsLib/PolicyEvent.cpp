/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#define CASE(eventType) \
    case eventType: return FrameworkEvent::eventType;

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
            CASE(DomainConfigTdpCapabilityChanged)
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
            CASE(DomainPlatformPowerSourceChanged)
            CASE(DomainAdapterPowerRatingChanged)
            CASE(DomainChargerTypeChanged)
            CASE(DomainPlatformRestOfPowerChanged)
            CASE(DomainACPeakPowerChanged)
            CASE(DomainACPeakTimeWindowChanged)
            CASE(DomainMaxBatteryPowerChanged)
            CASE(DomainPlatformBatterySteadyStateChanged)
            CASE(PolicyActiveRelationshipTableChanged)
            CASE(PolicyCoolingModeAcousticLimitChanged)
            CASE(PolicyCoolingModePolicyChanged)
            CASE(PolicyCoolingModePowerLimitChanged)
            CASE(PolicyForegroundApplicationChanged)
            CASE(PolicyInitiatedCallback)
            CASE(PolicyOperatingSystemConfigTdpLevelChanged)
            CASE(PolicyOperatingSystemLpmModeChanged)
            CASE(PolicyOperatingSystemHdcStatusChanged)
            CASE(PolicyPassiveTableChanged)
            CASE(PolicyPlatformLpmModeChanged)
            CASE(PolicySensorOrientationChanged)
            CASE(PolicySensorMotionChanged)
            CASE(PolicySensorSpatialOrientationChanged)
            CASE(PolicyThermalRelationshipTableChanged)
            CASE(PolicyAdaptivePerformanceConditionsTableChanged)
            CASE(PolicyAdaptivePerformanceActionsTableChanged)
            CASE(PolicyOperatingSystemPowerSourceChanged)
            CASE(PolicyOperatingSystemLidStateChanged)
            CASE(PolicyOperatingSystemBatteryPercentageChanged)
            CASE(PolicyOperatingSystemPlatformTypeChanged)
            CASE(PolicyOperatingSystemDockModeChanged)
            CASE(PolicyOperatingSystemMobileNotification)
            CASE(PolicyOemVariablesChanged)
            CASE(PolicyPowerDeviceRelationshipTableChanged)
            CASE(PolicyPowerBossConditionsTableChanged)
            CASE(PolicyPowerBossActionsTableChanged)
            CASE(DptfPolicyLoadedUnloadedEvent)
            CASE(DptfPolicyActivityLoggingEnabled)
            CASE(DptfPolicyActivityLoggingDisabled)
            CASE(PolicyOperatingSystemPowerSchemePersonalityChanged)
            default:
                throw dptf_exception("PolicyEvent::Type is invalid.");
        }
    }

    Bool RequiresEsifEventRegistration(PolicyEvent::Type policyEventType)
    {
        return ((policyEventType == PolicyEvent::PolicyActiveRelationshipTableChanged) ||
            (policyEventType == PolicyEvent::PolicyCoolingModeAcousticLimitChanged) ||
            (policyEventType == PolicyEvent::PolicyCoolingModePolicyChanged) ||
            (policyEventType == PolicyEvent::PolicyCoolingModePowerLimitChanged) ||
            (policyEventType == PolicyEvent::PolicyForegroundApplicationChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemConfigTdpLevelChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemLpmModeChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemHdcStatusChanged) ||
            (policyEventType == PolicyEvent::PolicyPassiveTableChanged) ||
            (policyEventType == PolicyEvent::PolicyPlatformLpmModeChanged) ||
            (policyEventType == PolicyEvent::PolicySensorOrientationChanged) ||
            (policyEventType == PolicyEvent::PolicySensorMotionChanged) ||
            (policyEventType == PolicyEvent::PolicySensorSpatialOrientationChanged) ||
            (policyEventType == PolicyEvent::PolicyThermalRelationshipTableChanged) ||
            (policyEventType == PolicyEvent::PolicyAdaptivePerformanceConditionsTableChanged) ||
            (policyEventType == PolicyEvent::PolicyAdaptivePerformanceActionsTableChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemPowerSourceChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemLidStateChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemBatteryPercentageChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemPlatformTypeChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemDockModeChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemMobileNotification) ||
            (policyEventType == PolicyEvent::PolicyOemVariablesChanged) ||
            (policyEventType == PolicyEvent::PolicyPowerDeviceRelationshipTableChanged) ||
            (policyEventType == PolicyEvent::PolicyPowerBossConditionsTableChanged) ||
            (policyEventType == PolicyEvent::PolicyPowerBossActionsTableChanged) ||
            (policyEventType == PolicyEvent::PolicyOperatingSystemPowerSchemePersonalityChanged));
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
        case PolicyEvent::DomainConfigTdpCapabilityChanged:
            return "DomainConfigTdpCapabilityChanged";
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
        case PolicyEvent::DomainPlatformPowerSourceChanged:
            return "DomainPlatformPowerSourceChanged";
        case PolicyEvent::DomainAdapterPowerRatingChanged:
            return "DomainAdapterPowerRatingChanged";
        case PolicyEvent::DomainChargerTypeChanged:
            return "DomainChargerTypeChanged";
        case PolicyEvent::DomainPlatformRestOfPowerChanged:
            return "DomainPlatformRestOfPowerChanged";
        case PolicyEvent::DomainACPeakPowerChanged:
            return "DomainACPeakPowerChanged";
        case PolicyEvent::DomainACPeakTimeWindowChanged:
            return "DomainACPeakTimeWindowChanged";
        case PolicyEvent::DomainMaxBatteryPowerChanged:
            return "DomainMaxBatteryPowerChanged";
        case PolicyEvent::DomainPlatformBatterySteadyStateChanged:
            return "DomainPlatformBatterySteadyStateChanged";
        case PolicyEvent::PolicyActiveRelationshipTableChanged:
            return "PolicyActiveRelationshipTableChanged";
        case PolicyEvent::PolicyCoolingModeAcousticLimitChanged:
            return "PolicyCoolingModeAcousticLimitChanged";
        case PolicyEvent::PolicyCoolingModePolicyChanged:
            return "PolicyCoolingModePolicyChanged";
        case PolicyEvent::PolicyCoolingModePowerLimitChanged:
            return "PolicyCoolingModePowerLimitChanged";
        case PolicyEvent::PolicyForegroundApplicationChanged:
            return "PolicyForegroundApplicationChanged";
        case PolicyEvent::PolicyInitiatedCallback:
            return "PolicyInitiatedCallback";
        case PolicyEvent::PolicyOperatingSystemConfigTdpLevelChanged:
            return "PolicyOperatingSystemConfigTdpLevelChanged";
        case PolicyEvent::PolicyOperatingSystemLpmModeChanged:
            return "PolicyOperatingSystemLpmModeChanged";
        case PolicyEvent::PolicyOperatingSystemHdcStatusChanged:
            return "PolicyOperatingSystemHdcStatusChanged";
        case PolicyEvent::PolicyPassiveTableChanged:
            return "PolicyPassiveTableChanged";
        case PolicyEvent::PolicyPlatformLpmModeChanged:
            return "PolicyPlatformLpmModeChanged";
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
        case PolicyEvent::PolicyOemVariablesChanged:
            return "PolicyOemVariablesChanged";
        case PolicyEvent::PolicyPowerDeviceRelationshipTableChanged:
            return "PolicyPowerDeviceRelationshipTableChanged";
        case PolicyEvent::PolicyPowerBossConditionsTableChanged:
            return "PolicyPowerBossConditionsTableChanged";
        case PolicyEvent::PolicyPowerBossActionsTableChanged:
            return "PolicyPowerBossActionsTableChanged";
        case PolicyEvent::DptfPolicyLoadedUnloadedEvent:
            return "DptfPolicyLoadedUnloadedEvent";
        case PolicyEvent::DptfPolicyActivityLoggingEnabled:
            return "DptfPolicyActivityLoggingEnabled";
        case PolicyEvent::DptfPolicyActivityLoggingDisabled:
            return "DptfPolicyActivityLoggingDisabled";
        case PolicyEvent::PolicyOperatingSystemPowerSchemePersonalityChanged:
            return "PolicyOperatingSystemPowerSchemePersonalityChanged";
        case PolicyEvent::Invalid:
        case PolicyEvent::Max:
        default:
            throw dptf_exception("PolicyEvent::Type is invalid.");
        }
    }

}