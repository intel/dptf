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

#include "FrameworkEvent.h"
#include "esif_sdk_event_guid.h"

//
// Macros used in FrameworkEventInfo::initializeEvents
//

#define INIT_EVENT_WITH_GUID(eventType, priority, guid)                                                                \
	{                                                                                                                  \
		UInt8 theGuid[Constants::GuidSize] = guid;                                                                     \
		initializeEvent(FrameworkEvent::eventType, priority, #eventType, Guid(theGuid));                               \
	}

#define INIT_EVENT(eventType, priority)                                                                                \
	{                                                                                                                  \
		initializeEvent(FrameworkEvent::eventType, priority, #eventType, Guid());                                      \
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
	// FIXME:  convert to map or hash table to improve performance

	for (UIntN i = 0; i < FrameworkEvent::Max; i++)
	{
		if (m_events[i].guid == guid)
		{
			return FrameworkEvent::Type(i);
		}
	}

	throw dptf_exception("GUID is not a framework event known to DPTF.");
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

#define DUMMY_GUID                                                                                                     \
	{                                                                                                                  \
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                 \
	}

void FrameworkEventInfo::initializeEvents()
{
	// DPTF Events
	INIT_EVENT_WITH_GUID(DptfConnectedStandbyEntry, 0, DISPLAY_OFF);
	INIT_EVENT_WITH_GUID(DptfConnectedStandbyExit, 0, DISPLAY_ON);
	INIT_EVENT_WITH_GUID(DptfSuspend, 0, PARTICIPANT_SUSPEND);
	INIT_EVENT_WITH_GUID(DptfResume, 0, PARTICIPANT_RESUME);
	INIT_EVENT(DptfGetStatus, 0);
	INIT_EVENT_WITH_GUID(DptfLogVerbosityChanged, 0, LOG_VERBOSITY_CHANGED);
	INIT_EVENT_WITH_GUID(DptfParticipantActivityLoggingEnabled, 0, DTT_PARTICIPANT_ACTIVITY_LOGGING_ENABLED);
	INIT_EVENT_WITH_GUID(DptfParticipantActivityLoggingDisabled, 0, DTT_PARTICIPANT_ACTIVITY_LOGGING_DISABLED);
	INIT_EVENT_WITH_GUID(DptfPolicyActivityLoggingEnabled, 0, DTT_POLICY_ACTIVITY_LOGGING_ENABLED);
	INIT_EVENT_WITH_GUID(DptfPolicyActivityLoggingDisabled, 0, DTT_POLICY_ACTIVITY_LOGGING_DISABLED);
	INIT_EVENT_WITH_GUID(DptfSupportedPoliciesChanged, 0, DTT_SUPPORTED_POLICIES_CHANGED);

	// Participant and Domain events
	INIT_EVENT(ParticipantAllocate, 31);
	INIT_EVENT(ParticipantCreate, 31);
	INIT_EVENT(ParticipantDestroy, 31);
	INIT_EVENT_WITH_GUID(ParticipantSpecificInfoChanged, 0, PARTICIPANT_SPEC_INFO_CHANGED);
	INIT_EVENT_WITH_GUID(DptfParticipantControlAction, 0, DTT_PARTICIPANT_CONTROL_ACTION);
	INIT_EVENT(DomainAllocate, 31);
	INIT_EVENT(DomainCreate, 31);
	INIT_EVENT(DomainDestroy, 31);
	INIT_EVENT_WITH_GUID(DomainCoreControlCapabilityChanged, 0, CORE_CAPABILITY_CHANGED);
	INIT_EVENT_WITH_GUID(DomainDisplayControlCapabilityChanged, 0, DTT_DISPLAY_CAPABILITY_CHANGED);
	INIT_EVENT_WITH_GUID(DomainDisplayStatusChanged, 0, DTT_DISPLAY_STATUS_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPerformanceControlCapabilityChanged, 0, PERF_CAPABILITY_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPerformanceControlsChanged, 0, PERF_CONTROL_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPowerControlCapabilityChanged, 0, POWER_CAPABILITY_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPriorityChanged, 0, DTT_PRIORITY_CHANGED);
	INIT_EVENT_WITH_GUID(DomainRadioConnectionStatusChanged, 0, RF_CONNECTION_STATUS_CHANGED);
	INIT_EVENT_WITH_GUID(DomainRfProfileChanged, 0, RF_PROFILE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainTemperatureThresholdCrossed, 0, TEMP_THRESHOLD_CROSSED);
	INIT_EVENT_WITH_GUID(DomainVirtualSensorCalibrationTableChanged, 0, DTT_VIRTUAL_SENSOR_CALIB_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainVirtualSensorPollingTableChanged, 0, DTT_VIRTUAL_SENSOR_POLLING_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainVirtualSensorRecalcChanged, 0, DTT_VIRTUAL_SENSOR_RECALC_CHANGED);
	INIT_EVENT_WITH_GUID(DomainBatteryStatusChanged, 0, BATTERY_STATUS_CHANGED);
	INIT_EVENT_WITH_GUID(DomainBatteryInformationChanged, 0, BATTERY_INFORMATION_CHANGED);
	INIT_EVENT_WITH_GUID(DomainBatteryHighFrequencyImpedanceChanged, 0, BATTERY_HIGH_FREQUENCY_IMPEDANCE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainBatteryNoLoadVoltageChanged, 0, BATTERY_NO_LOAD_VOLTAGE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainMaxBatteryPeakCurrentChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainPlatformPowerSourceChanged, 0, PLATFORM_POWER_SOURCE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainAdapterPowerRatingChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainChargerTypeChanged, 0, CHARGER_TYPE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPlatformRestOfPowerChanged, 0, PLATFORM_REST_OF_POWER_CHANGED);
	INIT_EVENT_WITH_GUID(DomainMaxBatteryPowerChanged, 0, MAX_BATTERY_POWER_CHANGED);
	INIT_EVENT_WITH_GUID(DomainPlatformBatterySteadyStateChanged, 0, PLATFORM_BATTERY_STEADY_STATE_CHANGED);
	INIT_EVENT_WITH_GUID(DomainACNominalVoltageChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainACOperationalCurrentChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainAC1msPercentageOverloadChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainAC2msPercentageOverloadChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainAC10msPercentageOverloadChanged, 0, DUMMY_GUID);
	INIT_EVENT_WITH_GUID(DomainEnergyThresholdCrossed, 0, ENERGY_THRESHOLD_CROSSED);
	INIT_EVENT_WITH_GUID(DomainFanCapabilityChanged, 0, FAN_CAPABILITIES_CHANGED);
	INIT_EVENT_WITH_GUID(DomainSocWorkloadClassificationChanged, 0, WORKLOAD_CLASSIFICATION_CHANGED);
	INIT_EVENT_WITH_GUID(DomainEppSensitivityHintChanged, 0, DTT_EPP_SENSITIVITY_HINT_CHANGED);

	// Policy events
	INIT_EVENT(PolicyCreate, 31);
	INIT_EVENT(PolicyDestroy, 31);
	INIT_EVENT_WITH_GUID(PolicyActiveRelationshipTableChanged, 0, DTT_ACTIVE_RELATIONSHIP_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyCoolingModePolicyChanged, 0, DTT_SYSTEM_COOLING_POLICY_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyForegroundApplicationChanged, 0, FOREGROUND_APP_CHANGED);
	INIT_EVENT(PolicyInitiatedCallback, 0);
	INIT_EVENT_WITH_GUID(PolicyPassiveTableChanged, 0, DTT_PASSIVE_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicySensorOrientationChanged, 0, DISPLAY_ORIENTATION_CHANGED);
	INIT_EVENT_WITH_GUID(PolicySensorMotionChanged, 0, MOTION_CHANGED);
	INIT_EVENT_WITH_GUID(PolicySensorSpatialOrientationChanged, 0, DEVICE_ORIENTATION_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyThermalRelationshipTableChanged, 0, DTT_THERMAL_RELATIONSHIP_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemPowerSourceChanged, 0, OS_POWER_SOURCE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemLidStateChanged, 0, OS_LID_STATE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemBatteryPercentageChanged, 0, OS_BATTERY_PERCENT_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemPlatformTypeChanged, 0, OS_PLATFORM_TYPE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemDockModeChanged, 0, OS_DOCK_MODE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemMobileNotification, 0, OS_MOBILE_NOTIFICATION);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemMixedRealityModeChanged, 0, OS_MIXED_REALITY_MODE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemPowerSchemePersonalityChanged, 0, OS_POWERSCHEME_PERSONALITY_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemUserPresenceChanged, 0, OS_USER_PRESENCE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemSessionStateChanged, 0, SESSION_STATE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemScreenStateChanged, 0, OS_SCREEN_STATE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemBatteryCountChanged, 0, BATTERY_COUNT_NOTIFICATION);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemPowerSliderChanged, 0, OS_POWER_SLIDER_VALUE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOemVariablesChanged, 0, OEM_VARS_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPowerBossConditionsTableChanged, 0, DTT_POWER_BOSS_CONDITIONS_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPowerBossActionsTableChanged, 0, DTT_POWER_BOSS_ACTIONS_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPowerBossMathTableChanged, 0, DTT_POWER_BOSS_MATH_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyVoltageThresholdMathTableChanged, 0, DTT_VOLTAGE_THRESHOLD_MATH_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(DptfPolicyLoadedUnloadedEvent, 0, DTT_POLICY_LOADED_UNLOADED);
	INIT_EVENT_WITH_GUID(PolicyEmergencyCallModeTableChanged, 0, EMERGENCY_CALL_MODE_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPidAlgorithmTableChanged, 0, DTT_PID_ALGORITHM_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(
		PolicyActiveControlPointRelationshipTableChanged, 0, DTT_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPowerShareAlgorithmTableChanged, 0, DTT_POWER_SHARING_ALGORITHM_TABLE_CHANGED);
	INIT_EVENT(PowerLimitChanged, 0);
	INIT_EVENT(PerformanceCapabilitiesChanged, 0);
	INIT_EVENT_WITH_GUID(PolicyWorkloadHintConfigurationChanged, 0, DTT_WORKLOAD_HINT_CONFIGURATION_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyOperatingSystemGameModeChanged, 0, OS_GAME_MODE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPowerShareAlgorithmTable2Changed, 0, DTT_POWER_SHARING_ALGORITHM_TABLE_2_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyPlatformUserPresenceChanged, 0, PLATFORM_USER_PRESENCE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyExternalMonitorStateChanged, 0, EXTERNAL_MONITOR_CONNECTION_STATE_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyUserInteractionChanged, 0, OS_USER_INTERACTION_CHANGED);
	INIT_EVENT_WITH_GUID(PolicyForegroundRatioChanged, 0, FOREGROUND_BACKGROUND_RATIO_CHANGED);
	INIT_EVENT(PolicySystemModeChanged, 0);

	INIT_EVENT_WITH_GUID(DptfAppLoaded, 0, APP_LOADED);
	INIT_EVENT_WITH_GUID(DptfAppUnloaded, 0, APP_UNLOADED);
	INIT_EVENT_WITH_GUID(DptfAppUnloading, 0, APP_UNLOADING);
	INIT_EVENT_WITH_GUID(DptfAppAliveRequest, 0, DTT_ALIVE_REQUEST);
	INIT_EVENT(DptfCommand, 0);
}

void FrameworkEventInfo::initializeEvent(
	FrameworkEvent::Type eventId,
	UIntN immediateQueuePriority,
	const std::string& name,
	const Guid& guid)
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

Bool FrameworkEventInfo::usesDummyGuid(FrameworkEvent::Type frameworkEvent)
{
	Bool usesDummyGuid = false;

	if (this->getGuid(frameworkEvent) == Guid(DUMMY_GUID))
	{
		usesDummyGuid = true;
	}

	return usesDummyGuid;
}