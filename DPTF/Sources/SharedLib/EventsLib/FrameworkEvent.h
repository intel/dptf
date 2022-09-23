/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

namespace FrameworkEvent
{
	enum Type
	{
		// DPTF Events		
		DptfConnectedStandbyEntry,
		DptfConnectedStandbyExit,
		DptfSuspend,
		DptfResume,
		DptfGetStatus,
		DptfLogVerbosityChanged,
		DptfParticipantActivityLoggingEnabled,
		DptfParticipantActivityLoggingDisabled,
		DptfPolicyActivityLoggingEnabled,
		DptfPolicyActivityLoggingDisabled,
		DptfSupportedPoliciesChanged,

		// Participant and Domain events
		ParticipantAllocate,
		ParticipantCreate,
		ParticipantDestroy,
		ParticipantSpecificInfoChanged, // Participant Specific Information Changed.  Param1 holds the Identifier.
		DptfParticipantControlAction,
		DomainAllocate,
		DomainCreate,
		DomainDestroy,
		DomainCoreControlCapabilityChanged,
		DomainDisplayControlCapabilityChanged, // Display control upper/lower limits changed.
		DomainDisplayStatusChanged, // Current Display brightness status has changed due to a user or other override
		DomainPerformanceControlCapabilityChanged, // Performance Control Upper/Lower Limits Changed
		DomainPerformanceControlsChanged, // Not used today but planned for future participants
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

		// Policy events
		PolicyCreate,
		PolicyDestroy,
		PolicyCoolingModePolicyChanged, // Active cooling mode vs. Passive cooling mode
		PolicyForegroundApplicationChanged,
		PolicyInitiatedCallback, // The policy created the event so it will get called back on a work item thread
		PolicySensorOrientationChanged,
		PolicySensorMotionChanged,
		PolicySensorSpatialOrientationChanged,
		PolicyOperatingSystemPowerSourceChanged,
		PolicyOperatingSystemLidStateChanged,
		PolicyOperatingSystemBatteryPercentageChanged,
		PolicyOperatingSystemPlatformTypeChanged,
		PolicyOperatingSystemDockModeChanged,
		PolicyOperatingSystemMobileNotification,
		PolicyOperatingSystemMixedRealityModeChanged,
		PolicyOperatingSystemPowerSchemePersonalityChanged,
		PolicyOperatingSystemUserPresenceChanged,
		PolicyOperatingSystemSessionStateChanged,
		PolicyOperatingSystemScreenStateChanged,
		PolicyOperatingSystemBatteryCountChanged,
		PolicyOperatingSystemPowerSliderChanged,
		DptfPolicyLoadedUnloadedEvent,
		PolicyEmergencyCallModeTableChanged,
		PowerLimitChanged,
		PowerLimitTimeWindowChanged,
		PerformanceCapabilitiesChanged,
		PolicyWorkloadHintConfigurationChanged,
		PolicyOperatingSystemGameModeChanged,
		PolicyPlatformUserPresenceChanged,
		PolicyExternalMonitorStateChanged,
		PolicyUserInteractionChanged,
		PolicyForegroundRatioChanged,
		PolicySystemModeChanged,
		PolicyTableObjectChanged,
		PolicyCollaborationChanged,
		PolicyThirdPartyGraphicsPowerStateChanged,
		PolicyAppBroadcastUnprivileged,
		PolicyAppBroadcastPrivileged,

		// App Events
		DptfAppLoaded,
		DptfAppUnloaded,
		DptfAppUnloading,
		DptfAppAliveRequest,
		DptfCommand,

		Max
	};
}

struct FrameworkEventData
{
	UIntN priority; // Priority for immediate work item
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
	Bool usesDummyGuid(FrameworkEvent::Type frameworkEvent);

private:
	FrameworkEventInfo(void);
	FrameworkEventInfo(const FrameworkEventInfo& rhs);
	~FrameworkEventInfo(void);
	FrameworkEventInfo& operator=(const FrameworkEventInfo& rhs);
	static FrameworkEventInfo* frameworkEventInfo;

	static const UIntN m_maxPriority = 32;

	FrameworkEventData m_events[FrameworkEvent::Max];

	// FIXME:  use map or hash table for converting guid to FrameworkEvent::Type
	// std::map<Guid, FrameworkEvent::Type> m_guidMap;

	void initializeAllEventsToInvalid();
	void initializeEvents();
	void initializeEvent(
		FrameworkEvent::Type eventId,
		UIntN immediateQueuePriority,
		const std::string& name,
		const Guid& guid);
	void verifyAllEventsCorrectlyInitialized() const;

	void throwIfFrameworkEventIsInvalid(FrameworkEvent::Type frameworkEvent) const;
};
