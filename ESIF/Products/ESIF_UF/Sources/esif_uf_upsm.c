/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_UF

#include "esif_ccb_lock.h"
#include "esif_uf_upsm.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_primitive.h"

#define ESIF_UPSM_DEFAULT_STABILITY_WINDOW 5000 /* ms */
#define STABILITY_WINDOW_MAX 15000
#define ESIF_UPSM_CORRELATION_RESET_TIME 120000 /* ms */


typedef enum UpCorrelationState_e {
	UP_CORRELATION_STATE_NEGATIVE = 0,
	UP_CORRELATION_STATE_POSITIVE = 1
} UpCorrelationState, * UpCorrelationStatePtr;

typedef struct EsifUpsm_s {
	esif_ccb_lock_t enableLock;  // Used to prevent enabling/disabling at same time
	esif_ccb_lock_t smLock;	     // Used for state machine management
	Int32 refCount;				 // UPSM enabled when count >= 1; disabled when count == 0

	SensorUserPresenceWithEnrollmentState curSensorWithEnrollmentState;
	SensorUserPresenceState curSensorState;
	SensorUserPresenceState filteredSensorState;
	SensorUserPresenceState queuedSensorState;

	Bool negativeEventFilteringEnabled;
	Bool positiveEventFilteringEnabled;
	UInt32 notPresentStabilityWindow;
	UInt32 disengagedStabilityWindow;
	UInt32 presentStabilityWindow;
	esif_ccb_timer_t eventFilteringTimer;
	Bool eventFilteringEnabled;
	Bool eventFilteringTimerActive;

	Bool interacting;
	UpCorrelationState correlationState;
	esif_ccb_timer_t correlationResetTimer;
	Bool correlationResetEnabled;
	Bool correlationResetTimerActive;

} EsifUpsm, *EsifUpsmPtr;


static EsifUpsm g_upsm = { 0 };

static esif_error_t EsifUpsm_RegisterEvents();
static void EsifUpsm_UnregisterEvents();
static esif_error_t EsifUpsm_FilterSensorEvent_SmLocked(SensorUserPresenceState sensorData);
static esif_error_t EsifUpsm_StartEventFiltering_SmLocked(SensorUserPresenceState sensorState);
static esif_error_t EsifUpsm_EnableEventFiltering_SmLocked();
static void EsifUpsm_StopEventFiltering_SmLocked();
static void EsifUpsm_DisableEventFiltering();
static esif_error_t EsifUpsm_HandleSensorEvent_SmLocked(SensorUserPresenceState sensorData);
static esif_error_t EsifUpsm_HandleSensorWithEnrollmentEvent_SmLocked(SensorUserPresenceWithEnrollmentState sensorData);
static esif_error_t EsifUpsm_EnableCorrelationResetTimer_SmLocked();
static esif_error_t EsifUpsm_StartCorrelationResetTimer_SmLocked();
static void EsifUpsm_StopCorrelationResetTimer_SmLocked();
static void EsifUpsm_DisableEventFiltering();
static esif_error_t EsifUpsm_EvaluateCorrelation_SmLocked(Bool interacting);
static esif_error_t EsifUpsm_SendEvents_SmLocked();
static esif_error_t EsifUpsm_SendPresenceEvent(SensorUserPresenceState state);
static esif_error_t EsifUpsm_SendCorrelationEvent(UpCorrelationState correlationState);

static esif_error_t ESIF_CALLCONV EsifUpsm_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	esif_error_t rc = ESIF_OK;
	UInt32 data = 0;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(domainId);

	if ((NULL == fpcEventPtr) || (NULL == eventDataPtr) || (NULL == eventDataPtr->buf_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventDataPtr->data_len < sizeof(data)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	data = *(UInt32*)eventDataPtr->buf_ptr;

	esif_ccb_write_lock(&g_upsm.smLock);

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED:
		ESIF_TRACE_DEBUG("Sensor User Presence = %d\n", data);
		rc = EsifUpsm_HandleSensorEvent_SmLocked(data);
		break;
	case ESIF_EVENT_SENSOR_USER_PRESENCE_WITH_ENROLLMENT_CHANGED:
		ESIF_TRACE_DEBUG("Sensor User Presence with Enrollment = %d\n", data);
		rc = EsifUpsm_HandleSensorWithEnrollmentEvent_SmLocked(data);
		break;
	case ESIF_EVENT_OS_USER_INTERACTION_CHANGED:
		ESIF_TRACE_DEBUG("User interaction = %d\n", data);
		Bool interacting = ESIF_TRUE;
		if (data == 0) {
			interacting = ESIF_FALSE;
		}
		rc = EsifUpsm_EvaluateCorrelation_SmLocked(interacting);
		break;
	default:
		break;
	}

	esif_ccb_write_unlock(&g_upsm.smLock);
exit:
	return rc;
}


static esif_error_t EsifUpsm_HandleSensorEvent_SmLocked(SensorUserPresenceState sensorData)
{
	esif_error_t rc = ESIF_OK;
	if (sensorData != g_upsm.curSensorState) {
		if (g_upsm.curSensorState == SENSOR_USER_PRESENCE_STATE_INVALID || sensorData == SENSOR_USER_PRESENCE_STATE_INVALID) {
			ESIF_TRACE_DEBUG("Invalid sensor data.");

			g_upsm.curSensorState = sensorData;
			EsifUpsm_StopEventFiltering_SmLocked();
			rc = EsifUpsm_SendEvents_SmLocked();
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to report sensor state %d.", sensorData);
			}
		}
		else if (sensorData == g_upsm.filteredSensorState) {
			g_upsm.queuedSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
		}
		else if (g_upsm.negativeEventFilteringEnabled && sensorData < g_upsm.curSensorState) {
			ESIF_TRACE_DEBUG("Negative filtering active. Current state %d, incoming data %d.", g_upsm.curSensorState, sensorData);
			rc = EsifUpsm_FilterSensorEvent_SmLocked(sensorData);
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to filter sensor state %d.", sensorData);
			}
		}
		else if (g_upsm.positiveEventFilteringEnabled && sensorData > g_upsm.curSensorState) {
			ESIF_TRACE_DEBUG("Positive filtering active. Current state %d, incoming data %d.", g_upsm.curSensorState, sensorData);
			rc = EsifUpsm_FilterSensorEvent_SmLocked(sensorData);
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to filter sensor state %d.", sensorData);
			}
		}
		else {
			ESIF_TRACE_DEBUG("Event filtering disabled.");
			g_upsm.curSensorState = sensorData;
			EsifUpsm_StopEventFiltering_SmLocked();
			rc = EsifUpsm_SendEvents_SmLocked();
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to report sensor state %d.", sensorData);
			}
		}
	}
	else {
		ESIF_TRACE_DEBUG("Got same data as current sensor state.");
		EsifUpsm_StopEventFiltering_SmLocked();
	}
	return rc;
}


static esif_error_t EsifUpsm_HandleSensorWithEnrollmentEvent_SmLocked(
	SensorUserPresenceWithEnrollmentState sensorData
	)
{
	esif_error_t rc = ESIF_OK;
	//
	// All we do is save state for the enrollment event.  When platform events are
	// sent is all based on the non-enrollment event.
	//
	g_upsm.curSensorWithEnrollmentState = sensorData;
	return rc;
}

//
// Called based on ESIF_EVENT_OS_USER_INTERACTION_CHANGED
// (When user HID activity occurs or when no HID activity for 5s. Also called
// every time we send other events with whatever current state is...)
//
static esif_error_t EsifUpsm_EvaluateCorrelation_SmLocked(Bool interacting)
{
	esif_error_t rc = ESIF_OK;
	UpCorrelationState correlationState = UP_CORRELATION_STATE_POSITIVE;

	if (g_upsm.curSensorState == SENSOR_USER_PRESENCE_STATE_ENGAGED) {
		if (g_upsm.correlationResetTimerActive) {
			EsifUpsm_StopCorrelationResetTimer_SmLocked();
		}
	}
	else if (interacting) {
		correlationState = UP_CORRELATION_STATE_NEGATIVE;
		if (g_upsm.correlationResetTimerActive) {
			EsifUpsm_StopCorrelationResetTimer_SmLocked();
		}

		//if correlation event was triggered by interaction event only, also send engaged event
		if (!g_upsm.interacting && g_upsm.curSensorState < SENSOR_USER_PRESENCE_STATE_ENGAGED) {
			g_upsm.interacting = ESIF_TRUE;
			rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
			if (rc != ESIF_OK) {
				ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
			}
		}
	}
	else {
		correlationState = g_upsm.correlationState; // Maintain current correlation state if not engaged and not interacting

		//not interacting and not sensor engaged -> correlation is positive unless previously negative
		if (g_upsm.correlationState == UP_CORRELATION_STATE_NEGATIVE && g_upsm.curSensorState == SENSOR_USER_PRESENCE_STATE_NOT_PRESENT) {
			//now that we are not present without interaction, start 2min timer to reset correlation
			if (!g_upsm.correlationResetTimerActive) {
				rc = EsifUpsm_EnableCorrelationResetTimer_SmLocked();
				if (rc == ESIF_OK) {
					rc = EsifUpsm_StartCorrelationResetTimer_SmLocked();
					if (rc != ESIF_OK) {
						ESIF_TRACE_DEBUG("Failed to start correlation reset timer.");
					}
				}
				else {
					ESIF_TRACE_DEBUG("Failed to enable correlation reset timer.");
				}
			}
		}
		else if (g_upsm.curSensorState == SENSOR_USER_PRESENCE_STATE_DISENGAGED) {
			if (g_upsm.correlationResetTimerActive) {
				EsifUpsm_StopCorrelationResetTimer_SmLocked();
			}
		}

		if (g_upsm.interacting) {
			g_upsm.interacting = ESIF_FALSE;
			//signal non engaged event that was previously overridden by interacting state
			rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
			if (rc != ESIF_OK) {
				ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
			}
		}
	}
	// Save current state
	g_upsm.interacting = interacting;

	if (correlationState == UP_CORRELATION_STATE_POSITIVE) {
		ESIF_TRACE_DEBUG("Evaluated correlation state as positive.");
	}
	else {
		ESIF_TRACE_DEBUG("Evaluated correlation state as negative.");
	}

	if (correlationState != g_upsm.correlationState) {
		//signal change in correlation state
		g_upsm.correlationState = correlationState;
		rc = EsifUpsm_SendCorrelationEvent(g_upsm.correlationState);
	}

	return rc;
}


static esif_error_t EsifUpsm_FilterSensorEvent_SmLocked(SensorUserPresenceState sensorData)
{
	esif_error_t rc = ESIF_OK;

	if (!g_upsm.eventFilteringTimerActive) {
		g_upsm.filteredSensorState = sensorData;
		rc = EsifUpsm_EnableEventFiltering_SmLocked();
		if (ESIF_OK != rc) {
			ESIF_TRACE_DEBUG("Failed to enable event filtering.");
		}

		if (g_upsm.eventFilteringEnabled) {
			rc = EsifUpsm_StartEventFiltering_SmLocked(g_upsm.filteredSensorState);
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to start event filtering.");
			}
		}
	}
	else {
		if (g_upsm.filteredSensorState > sensorData) {
			ESIF_TRACE_DEBUG("Incoming state is lower priority than pending state %d.", g_upsm.filteredSensorState);
			g_upsm.queuedSensorState = sensorData;
		}
		else {
			ESIF_TRACE_DEBUG("Incoming state is higher priority than pending state %d.", g_upsm.filteredSensorState);
			g_upsm.filteredSensorState = sensorData;
			g_upsm.queuedSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
		}
	}

	return rc;
}


static esif_error_t EsifUpsm_ReportSensorEvent_SmLocked()
{
	esif_error_t rc = ESIF_OK;


	return rc;
}


static esif_error_t EsifUpsm_SendEvents_SmLocked()
{
	esif_error_t rc = ESIF_OK;
	
	rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
		goto exit;
	}

	if (g_upsm.correlationResetTimerActive && g_upsm.curSensorState != SENSOR_USER_PRESENCE_STATE_NOT_PRESENT) {
		EsifUpsm_StopCorrelationResetTimer_SmLocked();
	}

	rc = EsifUpsm_EvaluateCorrelation_SmLocked(g_upsm.interacting);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Failed to evaluate correlation!");
	}

exit:
	return rc;
}


static esif_error_t EsifUpsm_StartEventFiltering_SmLocked(SensorUserPresenceState sensorState)
{
	esif_error_t rc = ESIF_OK;
	ESIF_TRACE_DEBUG("Starting timer for sensor state %d.", sensorState);
	if (sensorState == SENSOR_USER_PRESENCE_STATE_ENGAGED) {
		rc = esif_ccb_timer_set_msec(&g_upsm.eventFilteringTimer, g_upsm.presentStabilityWindow);
	}
	else if (sensorState == SENSOR_USER_PRESENCE_STATE_DISENGAGED) {
		rc = esif_ccb_timer_set_msec(&g_upsm.eventFilteringTimer, g_upsm.disengagedStabilityWindow);
	}
	else {
		rc = esif_ccb_timer_set_msec(&g_upsm.eventFilteringTimer, g_upsm.notPresentStabilityWindow);
	}
	
	if (ESIF_OK == rc) {
		g_upsm.eventFilteringTimerActive = ESIF_TRUE;
	}
	else {
		g_upsm.eventFilteringTimerActive = ESIF_FALSE;
		if (ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS == rc) {
			ESIF_TRACE_DEBUG("Event filtering active but stability window was 0ms for sensor state %d.", sensorState);

			g_upsm.curSensorState = sensorState;
			EsifUpsm_StopEventFiltering_SmLocked();
			rc = EsifUpsm_SendEvents_SmLocked();
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to report sensor state %d.", sensorState);
			}
		}
		else {
			ESIF_TRACE_DEBUG("Failed to set timer for sensor state %d.", sensorState);
		}
	}

	return rc;
}


static void EsifUpsm_StabilityWindowCallback(void* context_ptr) 
{
	esif_error_t rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context_ptr); 

	esif_ccb_write_lock(&g_upsm.smLock);
	ESIF_TRACE_DEBUG("Timer expired for sensor state %d.", g_upsm.filteredSensorState);
	g_upsm.curSensorState = g_upsm.filteredSensorState;
	
	rc = EsifUpsm_SendEvents_SmLocked();

	if (g_upsm.queuedSensorState != SENSOR_USER_PRESENCE_STATE_INVALID) {
		g_upsm.filteredSensorState = g_upsm.queuedSensorState;
		g_upsm.queuedSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
		EsifUpsm_StartEventFiltering_SmLocked(g_upsm.filteredSensorState);
	}
	else {
		g_upsm.filteredSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
		g_upsm.eventFilteringTimerActive = ESIF_FALSE;
	}

	esif_ccb_write_unlock(&g_upsm.smLock);
}


static esif_error_t EsifUpsm_RegisterEvents()
{
	esif_error_t rc = ESIF_OK;

	rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	//
	// Enroll for ESIF_EVENT_SENSOR_USER_PRESENCE_WITH_ENROLLMENT_CHANGED first so
	// the gratuitous event is received first as ESIF_EVENT_OS_USER_INTERACTION_CHANGED
	// will control when events are sent
	//
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_WITH_ENROLLMENT_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);		
	}
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_OS_USER_INTERACTION_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	}
	
	return rc;
}


static void EsifUpsm_UnregisterEvents()
{
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_OS_USER_INTERACTION_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_WITH_ENROLLMENT_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
}


static esif_error_t EsifUpsm_SendPresenceEvent(SensorUserPresenceState state)
{
	esif_error_t rc = ESIF_OK;
	PlatformUserPresenceState resolvedState = PLATFORM_USER_PRESENCE_STATE_INVALID;
	PlatformUserPresenceWithEnrollmentState resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_INVALID;
	EsifData esifData = { ESIF_DATA_UINT32, &resolvedState, sizeof(resolvedState), sizeof(resolvedState) };
	EsifData esifEnrollmentData = { ESIF_DATA_UINT32, &resolvedEnrollmentState, sizeof(resolvedEnrollmentState), sizeof(resolvedEnrollmentState) };

	if (state == SENSOR_USER_PRESENCE_STATE_ENGAGED) {
		resolvedState = PLATFORM_USER_PRESENCE_STATE_FACE_ENGAGED;

		resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_FACE_ENGAGED;
		if (SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED_WITH_FACEID == g_upsm.curSensorWithEnrollmentState) {
			resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED_WITH_FACEID;
		}
	}
	else if (state != SENSOR_USER_PRESENCE_STATE_INVALID && g_upsm.interacting) {
		ESIF_TRACE_DEBUG("User interaction detected. Overriding %d presence state to engaged.", state);
		resolvedState = PLATFORM_USER_PRESENCE_STATE_ENGAGED;

		resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED;
		if (SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED_WITH_FACEID == g_upsm.curSensorWithEnrollmentState) {
			resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_ENGAGED_WITH_FACEID;
		}
	}
	else {
		resolvedState = (PlatformUserPresenceState)state;

		if (SENSOR_USER_PRESENCE_STATE_NOT_PRESENT == state) {
			resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_NOT_PRESENT;
		}
		else if (SENSOR_USER_PRESENCE_STATE_DISENGAGED == state) {
			resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED;
			if (SENSOR_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED_WITH_FACEID == g_upsm.curSensorWithEnrollmentState) {
				resolvedEnrollmentState = PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_STATE_DISENGAGED_WITH_FACEID;
			}
		}
	}

	rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED, &esifData);
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PLATFORM_USER_PRESENCE_WITH_ENROLLMENT_CHANGED, &esifEnrollmentData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_DEBUG("Couldn't send platform user presence with enrollment event!");
		}
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
	}
	return rc;
}


static esif_error_t EsifUpsm_EnableEventFiltering_SmLocked() 
{
	esif_error_t rc = ESIF_OK;
	if (!g_upsm.eventFilteringEnabled) {
		rc = esif_ccb_timer_init(&g_upsm.eventFilteringTimer, EsifUpsm_StabilityWindowCallback, NULL);
		if (ESIF_OK == rc) {
			g_upsm.eventFilteringEnabled = ESIF_TRUE;
			ESIF_TRACE_DEBUG("Event Filtering enabled.");
		}
	}
	
	return rc;
}


static void EsifUpsm_StopEventFiltering_SmLocked() 
{
	g_upsm.filteredSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
	g_upsm.queuedSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;

	if (g_upsm.eventFilteringEnabled) {
		esif_ccb_timer_kill(&g_upsm.eventFilteringTimer);
		g_upsm.eventFilteringTimerActive = ESIF_FALSE;
		g_upsm.eventFilteringEnabled = ESIF_FALSE;
	}
}


static void EsifUpsm_DisableEventFiltering()
{
	esif_ccb_write_lock(&g_upsm.smLock);
	g_upsm.eventFilteringEnabled = ESIF_FALSE;
	g_upsm.eventFilteringTimerActive = ESIF_FALSE;
	esif_ccb_write_unlock(&g_upsm.smLock);
	esif_ccb_timer_kill_w_wait(&g_upsm.eventFilteringTimer);
}

static UInt32 EsifUpsm_SnapTimeWindowBounds(UInt32 stabilityWindow)
{
	UInt32 timeWindow = 0;

	if (stabilityWindow > STABILITY_WINDOW_MAX) {
		timeWindow = STABILITY_WINDOW_MAX;
	}
	else {
		timeWindow = stabilityWindow;
	}

	return timeWindow;
}

static esif_error_t EsifUpsm_GetEventFilteringSettings_SmLocked()
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	UInt32 result = 0;
	EsifData esifData = { ESIF_DATA_UINT32, &result, sizeof(result), 0 };

	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_NEGATIVE_EVENT_FILTERING_STATE, "D0", 255, NULL, &esifData);
	if (ESIF_OK == rc) {
		g_upsm.negativeEventFilteringEnabled = (Bool)result;
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't get negative event filtering state; defaulting to disabled.");
	}
	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_POSITIVE_EVENT_FILTERING_STATE, "D0", 255, NULL, &esifData);
	if (ESIF_OK == rc) {
		g_upsm.positiveEventFilteringEnabled = (Bool)result;
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't get positive event filtering state; defaulting to disabled.");
	}

	EsifData esifTime = { ESIF_DATA_TIME, &result, sizeof(result), 0 };
	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_NOT_PRESENT_STABILITY_WINDOW, "D0", 255, NULL, &esifTime);
	if (ESIF_OK == rc) {
		g_upsm.notPresentStabilityWindow = EsifUpsm_SnapTimeWindowBounds(result);
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't get NotPresentStabilityWindow; defaulting to 5s.");
	}
	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_DISENGAGED_STABILITY_WINDOW, "D0", 255, NULL, &esifTime);
	if (ESIF_OK == rc) {
		g_upsm.disengagedStabilityWindow = EsifUpsm_SnapTimeWindowBounds(result);
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't get DisengagedStabilityWindow; defaulting to 5s.");
	}
	rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_PRESENT_STABILITY_WINDOW, "D0", 255, NULL, &esifTime);
	if (ESIF_OK == rc) {
		g_upsm.presentStabilityWindow = EsifUpsm_SnapTimeWindowBounds(result);
	}
	else {
		ESIF_TRACE_DEBUG("Couldn't get PresentStabilityWindow; defaulting to 5s.");
	}

	return rc;
}


static esif_error_t EsifUpsm_SendCorrelationEvent(UpCorrelationState correlationState) 
{
	esif_error_t rc = ESIF_OK;
	EsifData esifDataCorrelation = { ESIF_DATA_UINT32, &correlationState, sizeof(correlationState), sizeof(correlationState) };
	 
	rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_USER_PRESENCE_CORRELATION_STATUS_CHANGED, &esifDataCorrelation);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't send correlation event!");
	}

	return rc;
}


static void EsifUpsm_CorrelationResetCallback(void* context_ptr) 
{
	UNREFERENCED_PARAMETER(context_ptr);

	esif_ccb_write_lock(&g_upsm.smLock);
	ESIF_TRACE_DEBUG("Timer expired for correlation reset. Resetting correlation.");
	g_upsm.correlationResetTimerActive = ESIF_FALSE;
	g_upsm.correlationState = UP_CORRELATION_STATE_POSITIVE;
	EsifUpsm_SendCorrelationEvent(g_upsm.correlationState);
	EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
	esif_ccb_write_unlock(&g_upsm.smLock);
}


static esif_error_t EsifUpsm_EnableCorrelationResetTimer_SmLocked()
{
	esif_error_t rc = ESIF_OK;
	if (!g_upsm.correlationResetEnabled) {
		rc = esif_ccb_timer_init(&g_upsm.correlationResetTimer, EsifUpsm_CorrelationResetCallback, NULL);
		if (ESIF_OK == rc) {
			g_upsm.correlationResetEnabled = ESIF_TRUE;
			ESIF_TRACE_DEBUG("Correlation reset timer enabled.");
		}
		else {
			g_upsm.correlationResetEnabled = ESIF_FALSE;
			ESIF_TRACE_DEBUG("Failed to enable correlation reset timer.");
		}
	}

	return rc;
}


static esif_error_t EsifUpsm_StartCorrelationResetTimer_SmLocked()
{
	esif_error_t rc = esif_ccb_timer_set_msec(&g_upsm.correlationResetTimer, ESIF_UPSM_CORRELATION_RESET_TIME);

	if (ESIF_OK == rc) {
		g_upsm.correlationResetTimerActive = ESIF_TRUE;
	}
	return rc;
}


static void EsifUpsm_StopCorrelationResetTimer_SmLocked() 
{
	esif_ccb_timer_kill(&g_upsm.correlationResetTimer);
	g_upsm.correlationResetTimerActive = ESIF_FALSE;
	g_upsm.correlationResetEnabled = ESIF_FALSE;
}


static void EsifUpsm_DisableCorrelationResetTimer()
{
	esif_ccb_write_lock(&g_upsm.smLock);
	g_upsm.correlationResetEnabled = ESIF_FALSE;
	g_upsm.correlationResetTimerActive = ESIF_FALSE;
	esif_ccb_write_unlock(&g_upsm.smLock);
	esif_ccb_timer_kill_w_wait(&g_upsm.correlationResetTimer);
}


esif_error_t EsifUpsm_Init(void)
{
	g_upsm.curSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
	g_upsm.filteredSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;
	g_upsm.queuedSensorState = SENSOR_USER_PRESENCE_STATE_INVALID;

	g_upsm.negativeEventFilteringEnabled = ESIF_FALSE;
	g_upsm.positiveEventFilteringEnabled = ESIF_FALSE;
	g_upsm.presentStabilityWindow = ESIF_UPSM_DEFAULT_STABILITY_WINDOW;
	g_upsm.disengagedStabilityWindow = ESIF_UPSM_DEFAULT_STABILITY_WINDOW;
	g_upsm.notPresentStabilityWindow = ESIF_UPSM_DEFAULT_STABILITY_WINDOW;

	g_upsm.interacting = ESIF_FALSE;
	g_upsm.correlationState = UP_CORRELATION_STATE_POSITIVE;

	esif_ccb_lock_init(&g_upsm.smLock);
	esif_ccb_lock_init(&g_upsm.enableLock);
	
	return ESIF_OK;
}


void EsifUpsm_Exit(void)
{
	esif_ccb_lock_uninit(&g_upsm.smLock);
	esif_ccb_lock_uninit(&g_upsm.enableLock);
}


esif_error_t EsifUpsm_Start(void)
{
	return ESIF_OK;
}


void EsifUpsm_Stop(void)
{
	EsifUpsm_UnregisterEvents();
	EsifUpsm_DisableEventFiltering();
	EsifUpsm_DisableCorrelationResetTimer();
	return;
}


esif_error_t EsifUpsm_Enable(void)
{
	esif_error_t rc = ESIF_OK;

	esif_ccb_write_lock(&g_upsm.enableLock);

	g_upsm.refCount++;

	if (1 == g_upsm.refCount) {
		EsifUpsm_GetEventFilteringSettings_SmLocked();
		if (g_upsm.negativeEventFilteringEnabled || g_upsm.positiveEventFilteringEnabled) {
			rc = EsifUpsm_EnableEventFiltering_SmLocked();
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to enable event filtering.");
			}
		}

		rc = EsifUpsm_RegisterEvents();

		if (ESIF_OK != rc) {
			EsifUpsm_Stop();
			g_upsm.refCount--;
		}
	}

	if (g_upsm.refCount >= 1) {
		EsifUpsm_SendEvents_SmLocked();
	}
	esif_ccb_write_unlock(&g_upsm.enableLock);

	return rc;
}


void EsifUpsm_Disable(void)
{
	esif_ccb_write_lock(&g_upsm.enableLock);

	g_upsm.refCount--;

	if (g_upsm.refCount <= 0) {
		g_upsm.refCount = 0;
		EsifUpsm_Stop();
	}

	esif_ccb_write_unlock(&g_upsm.enableLock);
	return;
}



