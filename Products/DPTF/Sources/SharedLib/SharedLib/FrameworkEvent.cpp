/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "DptfExceptions.h"
#include "autogen.h"

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

void FrameworkEventInfo::initializeEvents()
{
    // DPTF Events
    INIT_EVENT_WITH_GUID(DptfConnectedStandbyEntry, 0, CONNECTED_STANDBY_ENTRY);
    INIT_EVENT_WITH_GUID(DptfConnectedStandbyExit, 0, CONNECTED_STANDBY_EXIT);
    INIT_EVENT(DptfGetStatus, 0);
    INIT_EVENT_WITH_GUID(DptfLogVerbosityChanged, 0, LOG_VERBOSITY_CHANGED);

    // Participant and Domain events
    INIT_EVENT(ParticipantAllocate, 31);
    INIT_EVENT(ParticipantCreate, 31);
    INIT_EVENT(ParticipantDestroy, 31);
    INIT_EVENT_WITH_GUID(ParticipantSpecificInfoChanged, 0, SPEC_INFO_CHANGED);
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

    // Policy events
    INIT_EVENT(PolicyCreate, 31);
    INIT_EVENT(PolicyDestroy, 31);
    INIT_EVENT_WITH_GUID(PolicyActiveRelationshipTableChanged, 0, ACTIVE_RELATIONSHIP_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyCoolingModeAcousticLimitChanged, 0, COOLING_MODE_ACOUSTIC_LIMIT_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyCoolingModePolicyChanged, 0, SYSTEM_COOLING_POLICY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyCoolingModePowerLimitChanged, 0, COOLING_MODE_POWER_LIMIT_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyForegroundApplicationChanged, 0, FOREGROUND_CHANGED);
    INIT_EVENT(PolicyInitiatedCallback, 0);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemConfigTdpLevelChanged, 0, OS_CTDP_CAPABILITY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyOperatingSystemLpmModeChanged, 0, OS_LPM_MODE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPassiveTableChanged, 0, PASSIVE_TABLE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyPlatformLpmModeChanged, 0, LPM_MODE_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorOrientationChanged, 0, SENSOR_ORIENTATION_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorProximityChanged, 0, SENSOR_PROXIMITY_CHANGED);
    INIT_EVENT_WITH_GUID(PolicySensorSpatialOrientationChanged, 0, SENSOR_SPATIAL_ORIENTATION_CHANGED);
    INIT_EVENT_WITH_GUID(PolicyThermalRelationshipTableChanged, 0, THERMAL_RELATIONSHIP_CHANGED);
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