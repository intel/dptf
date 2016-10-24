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

#include "FrameworkEvent.h"
#include "esif_sdk_event_guid.h"

//
// Macros used in FrameworkEventInfo::initializeEvents
//

#define INIT_EVENT_WITH_GUID(eventType, priority, guid) \
    { \
    UInt8 theGuid[Constants::GuidSize] = guid; \
    initializeEvent(FrameworkEvent::eventType, priority, #eventType, Guid(theGuid)); \
    }

#define INIT_EVENT(eventType, priority) \
    { \
    initializeEvent(FrameworkEvent::eventType, priority, #eventType, Guid()); \
    }

FrameworkEventInfo* FrameworkEventInfo::frameworkEventInfo = NULL;

FrameworkEventInfo* FrameworkEventInfo::instance(void)
{
    if (frameworkEventInfo == NULL)
    {
        frameworkEventInfo = new FrameworkEventInfo();
    }
    return frameworkEventInfo;
}

void FrameworkEventInfo::destroy(void)
{
    DELETE_MEMORY_TC(frameworkEventInfo);
}

const FrameworkEventData& FrameworkEventInfo::operator[](FrameworkEvent::Type frameworkEvent) const
{
    return m_events[frameworkEvent];
}

UIntN FrameworkEventInfo::getPriority(FrameworkEvent::Type frameworkEvent) const
{
    throwIfFrameworkEventIsInvalid(frameworkEvent);
    return m_events[frameworkEvent].priority;
}

std::string FrameworkEventInfo::getName(FrameworkEvent::Type frameworkEvent) const
{
    throwIfFrameworkEventIsInvalid(frameworkEvent);
    return m_events[frameworkEvent].name;
}

Guid FrameworkEventInfo::getGuid(FrameworkEvent::Type frameworkEvent) const
{
    throwIfFrameworkEventIsInvalid(frameworkEvent);
    return m_events[frameworkEvent].guid;
}

FrameworkEvent::Type FrameworkEventInfo::getFrameworkEventType(const Guid& guid) const
{
    //FIXME:  convert to map or hash table to improve performance

    for (UIntN i = 0; i < FrameworkEvent::Max; i++)
    {
        if (m_events[i].guid == guid)
        {
            return FrameworkEvent::Type(i);
        }
    }

    throw dptf_exception("Received guid that doesn't match an event know to DPTF.");
}

FrameworkEventInfo::FrameworkEventInfo(void)
{
    initializeAllEventsToInvalid();
    initializeEvents();
    verifyAllEventsCorrectlyInitialized();
}

FrameworkEventInfo::~FrameworkEventInfo(void)
{
}

void FrameworkEventInfo::initializeAllEventsToInvalid()
{
    const UIntN invalidPriority = m_maxPriority + 1;

    for (int i = 0; i < FrameworkEvent::Max; i++)
    {
        m_events[i].priority = invalidPriority;
    }
}

#define DUMMY_GUID { \
    0x1C, 0x66, 0x20, 0x50, 0xFA, 0x0A, 0x47, 0x99, \
    0xBF, 0x56, 0x3E, 0x09, 0x1A, 0x00, 0x73, 0x38 }

void FrameworkEventInfo::initializeEvents()
{
    // DPTF Events
    INIT_EVENT_WITH_GUID(DptfConnectedStandbyEntry, 0, CONNECTED_STANDBY_ENTRY);
    INIT_EVENT_WITH_GUID(DptfConnectedStandbyExit, 0, CONNECTED_STANDBY_EXIT);
    INIT_EVENT_WITH_GUID(DptfSuspend, 0, SUSPEND);
    INIT_EVENT_WITH_GUID(DptfResume, 0, RESUME);
    INIT_EVENT(DptfGetStatus, 0);
    INIT_EVENT_WITH_GUID(DptfLogVerbosityChanged, 0, LOG_VERBOSITY_CHANGED);
    INIT_EVENT_WITH_GUID(DptfParticipantActivityLoggingEnabled, 0, DPTF_PARTICIPANT_ACTIVITY_LOGGING_ENABLED);
    INIT_EVENT_WITH_GUID(DptfParticipantActivityLoggingDisabled, 0, DPTF_PARTICIPANT_ACTIVITY_LOGGING_DISABLED);
    INIT_EVENT_WITH_GUID(DptfPolicyActivityLoggingEnabled, 0, DPTF_POLICY_ACTIVITY_LOGGING_ENABLED);
    INIT_EVENT_WITH_GUID(DptfPolicyActivityLoggingDisabled, 0, DPTF_POLICY_ACTIVITY_LOGGING_DISABLED);

    // Participant and Domain events
    INIT_EVENT(ParticipantAllocate, 31);
    INIT_EVENT(ParticipantCreate, 31);
    INIT_EVENT(ParticipantDestroy, 31);
    INIT_EVENT_WITH_GUID(ParticipantSpecificInfoChanged, 0, SPEC_INFO_CHANGED);
    INIT_EVENT_WITH_GUID(DptfParticipantControlAction, 0, DPTF_PARTICIPANT_CONTROL_ACTION);
    INIT_EVENT(DomainAllocate, 31);
    INIT_EVENT(DomainCreate, 31);
    INIT_EVENT(DomainDestroy, 31);
    INIT_EVENT_WITH_GUID(DomainConfigTdpCapabilityChanged, 0, CTDP_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainCoreControlCapabilityChanged, 0, CORE_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainDisplayControlCapabilityChanged, 0, DISPLAY_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainDisplayStatusChanged, 0, DISPLAY_STATUS_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPerformanceControlCapabilityChanged, 0, PERF_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPerformanceControlsChanged, 0, PERF_CONTROL_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPowerControlCapabilityChanged, 0, POWER_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPriorityChanged, 0, PRIORITY_CHANGED);
    INIT_EVENT_WITH_GUID(DomainRadioConnectionStatusChanged, 0, RF_CONNECTION_STATUS_CHANGED);
    INIT_EVENT_WITH_GUID(DomainRfProfileChanged, 0, RF_PROFILE_CHANGED);
    INIT_EVENT_WITH_GUID(DomainTemperatureThresholdCrossed, 0, TEMP_THRESHOLD_CROSSED);
    INIT_EVENT_WITH_GUID(DomainVirtualSensorCalibrationTableChanged, 0, VIRTUAL_SENSOR_CALIB_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(DomainVirtualSensorPollingTableChanged, 0, VIRTUAL_SENSOR_POLLING_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(DomainVirtualSensorRecalcChanged, 0, VIRTUAL_SENSOR_RECALC_CHANGED);
    INIT_EVENT_WITH_GUID(DomainBatteryStatusChanged, 0, BATTERY_STATUS_CHANGED);
    INIT_EVENT_WITH_GUID(DomainBatteryInformationChanged, 0, BATTERY_INFORMATION_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPlatformPowerSourceChanged, 0, PLATFORM_POWER_SOURCE_CHANGED);
    INIT_EVENT_WITH_GUID(DomainAdapterPowerRatingChanged, 0, DUMMY_GUID);
    INIT_EVENT_WITH_GUID(DomainChargerTypeChanged, 0, CHARGER_TYPE_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPlatformRestOfPowerChanged, 0, PLATFORM_REST_OF_POWER_CHANGED);
    INIT_EVENT_WITH_GUID(DomainACPeakPowerChanged, 0, DUMMY_GUID);
    INIT_EVENT_WITH_GUID(DomainACPeakTimeWindowChanged, 0, DUMMY_GUID);
    INIT_EVENT_WITH_GUID(DomainMaxBatteryPowerChanged, 0, MAX_BATTERY_POWER_CHANGED);
    INIT_EVENT_WITH_GUID(DomainPlatformBatterySteadyStateChanged, 0, PLATFORM_BATTERY_STEADY_STATE_CHANGED);

    // Policy events
    INIT_EVENT(PolicyCreate, 31);
    INIT_EVENT(PolicyDestroy, 31);
    INIT_EVENT_WITH_GUID(PolicyActiveRelationshipTableChanged, 0, ACTIVE_RELATIONSHIP_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyCoolingModePolicyChanged, 0, SYSTEM_COOLING_POLICY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyForegroundApplicationChanged, 0, FOREGROUND_CHANGED);
    INIT_EVENT(PolicyInitiatedCallback, 0);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemConfigTdpLevelChanged, 0, OS_CTDP_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPassiveTableChanged, 0, PASSIVE_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorOrientationChanged, 0, DISPLAY_ORIENTATION_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorMotionChanged, 0, MOTION_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorSpatialOrientationChanged, 0, DEVICE_ORIENTATION_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyThermalRelationshipTableChanged, 0, THERMAL_RELATIONSHIP_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyAdaptivePerformanceConditionsTableChanged, 0, ADAPTIVE_PERFORMANCE_CONDITIONS_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyAdaptivePerformanceActionsTableChanged, 0, ADAPTIVE_PERFORMANCE_ACTIONS_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyAdaptivePerformanceParticipantConditionTableChanged, 0, ADAPTIVE_PERFORMANCE_PARTICIPANT_CONDITION_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemPowerSourceChanged, 0, OS_POWER_SOURCE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemLidStateChanged, 0, OS_LID_STATE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemBatteryPercentageChanged, 0, OS_BATTERY_PERCENT_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemPlatformTypeChanged, 0, OS_PLATFORM_TYPE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemDockModeChanged, 0, OS_DOCK_MODE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemMobileNotification, 0, OS_MOBILE_NOTIFICATION);
    INIT_EVENT_WITH_GUID(PolicyOemVariablesChanged, 0, OEM_VARS_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPowerBossConditionsTableChanged, 0, POWER_BOSS_CONDITIONS_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPowerBossActionsTableChanged, 0, POWER_BOSS_ACTIONS_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPowerBossMathTableChanged, 0, POWER_BOSS_MATH_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(DptfPolicyLoadedUnloadedEvent, 0, DPTF_POLICY_LOADED_UNLOADED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemPowerSchemePersonalityChanged, 0, OS_POWERSCHEME_PERSONALITY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyEmergencyCallModeTableChanged, 0, EMERGENCY_CALL_MODE_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPidAlgorithmTableChanged, 0, PID_ALGORITHM_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyActiveControlPointRelationshipTableChanged, 0, ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE_CHANGED);

    INIT_EVENT_WITH_GUID(DptfAppLoaded, 0, APP_LOADED);
    INIT_EVENT_WITH_GUID(DptfAppUnloaded, 0, APP_UNLOADED);
    INIT_EVENT_WITH_GUID(DptfAppUnloading, 0, APP_UNLOADING);
}

void FrameworkEventInfo::initializeEvent(FrameworkEvent::Type eventId, UIntN immediateQueuePriority,
                                         const std::string& name, const Guid& guid)
{
    m_events[eventId].priority = immediateQueuePriority;
    m_events[eventId].name = name;
    m_events[eventId].guid = guid;
}

void FrameworkEventInfo::verifyAllEventsCorrectlyInitialized() const
{
    for (int i = 0; i < FrameworkEvent::Max; i++)
    {
        if (m_events[i].priority > m_maxPriority)
        {
            throw dptf_exception("Error while trying to initialize FrameworkEventInfo.");
        }
    }
}

void FrameworkEventInfo::throwIfFrameworkEventIsInvalid(FrameworkEvent::Type frameworkEvent) const
{
    if (frameworkEvent >= FrameworkEvent::Max)
    {
        throw dptf_exception("Received invalid FrameworkEvent::Type.");
    }
}