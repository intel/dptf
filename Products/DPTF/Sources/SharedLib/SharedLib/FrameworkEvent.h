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

#pragma once

//
// This is the complete list of events that we can receive from ESIF + the internal events that ESIF doesn't know about.
// This list of events map to individual work items in the manager.  The manager will forward the
// event to the participant or policy as necessary.  The matching event list is in ParticipantEvent.h and
// PolicyEvent.h.  Separate enums are used so participants can only subscribe to participant events and
// policies can only subscribe to policy events.
//

#include "Dptf.h"
#include "BasicTypes.h"
#include "DptfExport.h"
#include <string>

namespace FrameworkEvent
{
    enum Type
    {
        // DPTF Events
        DptfConnectedStandbyEntry,
        DptfConnectedStandbyExit,
        DptfGetStatus,
        DptfLogVerbosityChanged,

        // Participant and Domain events
        ParticipantAllocate,
        ParticipantCreate,
        ParticipantDestroy,
        ParticipantSpecificInfoChanged,                             // Participant Specific Information Changed.  Param1 holds the Identifier.
        DomainAllocate,
        DomainCreate,
        DomainDestroy,
        DomainConfigTdpCapabilityChanged,
        DomainCoreControlCapabilityChanged,
        DomainDisplayControlCapabilityChanged,                      // Display control upper/lower limits changed.
        DomainDisplayStatusChanged,                                 // Current Display brightness status has changed due to a user or other override
        DomainPerformanceControlCapabilityChanged,                  // Performance Control Upper/Lower Limits Changed
        DomainPerformanceControlsChanged,                           // Not used today but planned for future participants
        DomainPowerControlCapabilityChanged,
        DomainPriorityChanged,
        DomainRadioConnectionStatusChanged,
        DomainRfProfileChanged,
        DomainTemperatureThresholdCrossed,

        // Policy events
        PolicyCreate,
        PolicyDestroy,
        PolicyActiveRelationshipTableChanged,
        PolicyCoolingModeAcousticLimitChanged,
        PolicyCoolingModePolicyChanged,                             // Active cooling mode vs. Passive cooling mode
        PolicyCoolingModePowerLimitChanged,
        PolicyForegroundApplicationChanged,
        PolicyInitiatedCallback,                                    // The policy created the event so it will get called back on a work item thread
        PolicyOperatingSystemConfigTdpLevelChanged,
        PolicyOperatingSystemLpmModeChanged,
        PolicyPassiveTableChanged,
        PolicyPlatformLpmModeChanged,
        PolicySensorOrientationChanged,
        PolicySensorProximityChanged,
        PolicySensorSpatialOrientationChanged,
        PolicyThermalRelationshipTableChanged,

        Max
    };
}

struct FrameworkEventData
{
    UIntN priority;                                                 // Priority for immediate work item
    std::string name;
    Guid guid;
};

// Singleton

class FrameworkEventInfo final
{
public:

    static FrameworkEventInfo* instance(void);
    static void destroy(void);

    const FrameworkEventData& operator[](FrameworkEvent::Type frameworkEvent) const;

    UIntN getPriority(FrameworkEvent::Type frameworkEvent) const;
    std::string getName(FrameworkEvent::Type frameworkEvent) const;
    Guid getGuid(FrameworkEvent::Type frameworkEvent) const;
    FrameworkEvent::Type getFrameworkEventType(const Guid& guid) const;

private:

    FrameworkEventInfo(void);
    FrameworkEventInfo(const FrameworkEventInfo& rhs);
    ~FrameworkEventInfo(void);
    FrameworkEventInfo& operator=(const FrameworkEventInfo& rhs);
    static FrameworkEventInfo* frameworkEventInfo;

    static const UIntN m_maxPriority = 32;

    FrameworkEventData m_events[FrameworkEvent::Max];

    //FIXME:  use map or hash table for converting guid to FrameworkEvent::Type
    //std::map<Guid, FrameworkEvent::Type> m_guidMap;

    void initializeAllEventsToInvalid();
    void initializeEvents();
    void initializeEvent(FrameworkEvent::Type eventId, UIntN immediateQueuePriority,
        const std::string& name, const Guid& guid);
    void verifyAllEventsCorrectlyInitialized() const;

    void throwIfFrameworkEventIsInvalid(FrameworkEvent::Type frameworkEvent) const;
};