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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_UF

#include "esif_ccb_lock.h"
#include "esif_uf_upsm.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_primitive.h"

#define ESIF_UPSM_DEFAULT_STABILITY_WINDOW 5000 /* ms */
#define STABILITY_WINDOW_MAX 15000
#define ESIF_UPSM_CORRELATION_RESET_TIME 120000 /* ms */

#if defined(ESIF_ATTR_OS_WINDOWS)

esif_error_t EsifSetActionDelegateDppeSettingWin(
	EsifDataPtr requestPtr,
	const GUID* guid
);

#define SetUserPresenceDppeSetting(requestPtr, guid) EsifSetActionDelegateDppeSettingWin(requestPtr, guid)

#elif defined(ESIF_ATTR_OS_LINUX)

#define SetUserPresenceDppeSetting(requestPtr, guid) (ESIF_E_NOT_IMPLEMENTED)

#endif

// Values from ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED
typedef enum UpSensorState_e {
	UP_SENSOR_STATE_NOT_PRESENT = 0,
	UP_SENSOR_STATE_DISENGAGED = 1,
	UP_SENSOR_STATE_ENGAGED = 2,
	UP_SENSOR_STATE_INVALID = 99 // Not a real sensor state but used for init only
} UpSensorState, *UpSensorStatePtr;

typedef enum UpState_e {
	UP_STATE_NOT_PRESENT = 0,
	UP_STATE_DISENGAGED = 1,
	UP_STATE_ENGAGED = 2,
	UP_STATE_FACE_ENGAGED = 3,
	UP_STATE_INVALID = 99 // Not a real sensor state but used for init only
} UpState, * UpStatePtr;

typedef enum UpCorrelationState_e {
	UP_CORRELATION_STATE_NEGATIVE = 0,
	UP_CORRELATION_STATE_POSITIVE = 1
} UpCorrelationState, * UpCorrelationStatePtr;

typedef struct EsifUpsm_s {
	esif_ccb_lock_t enableLock;  // Used to prevent enabling/disabling at same time
	esif_ccb_lock_t smLock;	     // Used for state machine management
	Int32 refCount;				 // UPSM enabled when count >= 1; disabled when count == 0

	UpSensorState curSensorState;
	UpSensorState filteredSensorState;
	UpSensorState queuedSensorState;

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
static esif_error_t EsifUpsm_FilterSensorEvent_SmLocked(UpSensorState sensorData);
static esif_error_t EsifUpsm_ReportSensorEvent_SmLocked(UpSensorState sensorData, EsifDataPtr esifDataPtr);
static esif_error_t EsifUpsm_StartEventFiltering_SmLocked(UpSensorState sensorState);
static esif_error_t EsifUpsm_EnableEventFiltering_SmLocked();
static void EsifUpsm_StopEventFiltering_SmLocked();
static void EsifUpsm_DisableEventFiltering();
static esif_error_t EsifUpsm_HandleSensorEvent_SmLocked(UpSensorState sensorData, EsifDataPtr esifDataPtr);
static esif_error_t EsifUpsm_EnableCorrelationResetTimer_SmLocked();
static esif_error_t EsifUpsm_StartCorrelationResetTimer_SmLocked();
static void EsifUpsm_StopCorrelationResetTimer_SmLocked();
static void EsifUpsm_DisableEventFiltering();
static esif_error_t EsifUpsm_EvaluateCorrelation_SmLocked(Bool interacting);
static esif_error_t EsifUpsm_SendEvents();
static esif_error_t EsifUpsm_SendPresenceEvent(UpSensorState state);
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
	EsifData esifData = { ESIF_DATA_UINT32, &data, sizeof(data), sizeof(data) };

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
	esifData.buf_ptr = &data;

	esif_ccb_write_lock(&g_upsm.smLock);

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED:
		ESIF_TRACE_DEBUG("Sensor User Presence = %d\n", data);
		rc = EsifUpsm_HandleSensorEvent_SmLocked(data, &esifData);
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


static esif_error_t EsifUpsm_HandleSensorEvent_SmLocked(UpSensorState sensorData, EsifDataPtr esifDataPtr) 
{
	esif_error_t rc = ESIF_OK;
	if (sensorData != g_upsm.curSensorState) {
		if (g_upsm.curSensorState == UP_SENSOR_STATE_INVALID || sensorData == UP_SENSOR_STATE_INVALID) {
			ESIF_TRACE_DEBUG("Invalid sensor data.");
			rc = EsifUpsm_ReportSensorEvent_SmLocked(sensorData, esifDataPtr);
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to report sensor state %d.", sensorData);
			}
		}
		else if (sensorData == g_upsm.filteredSensorState) {
			g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;
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
			rc = EsifUpsm_ReportSensorEvent_SmLocked(sensorData, esifDataPtr);
			if (ESIF_OK != rc) {
				ESIF_TRACE_DEBUG("Failed to report sensor state %d.", sensorData);
			}
		}
	}
	else {
		ESIF_TRACE_DEBUG("Got same data as current sensor state.");
		if (g_upsm.eventFilteringEnabled) {
			EsifUpsm_StopEventFiltering_SmLocked();
		}
		g_upsm.filteredSensorState = UP_SENSOR_STATE_INVALID;
		g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;
	}
	return rc;
}


static esif_error_t EsifUpsm_EvaluateCorrelation_SmLocked(Bool interacting)
{
	esif_error_t rc = ESIF_OK;
	UpCorrelationState correlationState = UP_CORRELATION_STATE_POSITIVE;

	if (g_upsm.curSensorState == UP_SENSOR_STATE_ENGAGED) {
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
		if (!g_upsm.interacting && g_upsm.curSensorState < UP_SENSOR_STATE_ENGAGED) {
			g_upsm.interacting = interacting;
			rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
			if (rc != ESIF_OK) {
				ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
			}
		}
	}
	else {
		//not interacting and not sensor engaged -> correlation is positive unless previously negative
		correlationState = g_upsm.correlationState;
		if (g_upsm.correlationState == UP_CORRELATION_STATE_NEGATIVE && g_upsm.curSensorState == UP_SENSOR_STATE_NOT_PRESENT) {
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
		else if (g_upsm.curSensorState == UP_SENSOR_STATE_DISENGAGED) {
			if (g_upsm.correlationResetTimerActive) {
				EsifUpsm_StopCorrelationResetTimer_SmLocked();
			}
		}

		if (g_upsm.interacting) {
			g_upsm.interacting = interacting;
			//signal non engaged event that was previously overridden by interacting state
			rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
			if (rc != ESIF_OK) {
				ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
			}
		}
	}
	
	if (g_upsm.interacting != interacting) {
		g_upsm.interacting = interacting;
	}

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


static esif_error_t EsifUpsm_FilterSensorEvent_SmLocked(UpSensorState sensorData) 
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
			g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;
		}
	}

	return rc;
}


static esif_error_t EsifUpsm_ReportSensorEvent_SmLocked(UpSensorState sensorData, EsifDataPtr esifDataPtr)
{
	esif_error_t rc = ESIF_OK;

	g_upsm.curSensorState = sensorData;
	g_upsm.filteredSensorState = UP_SENSOR_STATE_INVALID;
	g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;
	if (g_upsm.eventFilteringEnabled) {
		EsifUpsm_StopEventFiltering_SmLocked();
	}
	rc = SetUserPresenceDppeSetting(esifDataPtr, &GUID_DTT_SENSOR_PRESENCE_STATUS);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("Failed to set sensor presence DPPE value.");
	}

	rc = EsifUpsm_SendEvents();
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Failed to send presence events.");
	}

	return rc;
}


static esif_error_t EsifUpsm_SendEvents() 
{
	esif_error_t rc = ESIF_OK;
	
	rc = EsifUpsm_SendPresenceEvent(g_upsm.curSensorState);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
		goto exit;
	}

	if (g_upsm.correlationResetTimerActive && g_upsm.curSensorState != UP_SENSOR_STATE_NOT_PRESENT) {
		EsifUpsm_StopCorrelationResetTimer_SmLocked();
	}

	rc = EsifUpsm_EvaluateCorrelation_SmLocked(g_upsm.interacting);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Failed to evaluate correlation!");
	}

exit:
	return rc;
}


static esif_error_t EsifUpsm_StartEventFiltering_SmLocked(UpSensorState sensorState)
{
	esif_error_t rc = ESIF_OK;
	ESIF_TRACE_DEBUG("Starting timer for sensor state %d.", sensorState);
	if (sensorState == UP_SENSOR_STATE_ENGAGED) {
		rc = esif_ccb_timer_set_msec(&g_upsm.eventFilteringTimer, g_upsm.presentStabilityWindow);
	}
	else if (sensorState == UP_SENSOR_STATE_DISENGAGED) {
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
			EsifData esifData = { ESIF_DATA_UINT32, &sensorState, sizeof(sensorState), sizeof(sensorState) };
			esifData.buf_ptr = &sensorState;
			rc = EsifUpsm_ReportSensorEvent_SmLocked(sensorState, &esifData);
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
	EsifData esifData = { ESIF_DATA_UINT32, &g_upsm.curSensorState, sizeof(g_upsm.curSensorState), sizeof(g_upsm.curSensorState) };
	esif_error_t rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context_ptr); 

	esif_ccb_write_lock(&g_upsm.smLock);
	ESIF_TRACE_DEBUG("Timer expired for sensor state %d.", g_upsm.filteredSensorState);
	g_upsm.curSensorState = g_upsm.filteredSensorState;
	
	esifData.buf_ptr = &g_upsm.curSensorState;

	rc = SetUserPresenceDppeSetting(&esifData, &GUID_DTT_SENSOR_PRESENCE_STATUS);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't set sensor presence DPPE value!");
	}

	rc = EsifUpsm_SendEvents();

	if (g_upsm.queuedSensorState != UP_SENSOR_STATE_INVALID) {
		g_upsm.filteredSensorState = g_upsm.queuedSensorState;
		g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;
		EsifUpsm_StartEventFiltering_SmLocked(g_upsm.filteredSensorState);
	}
	else {
		g_upsm.filteredSensorState = UP_SENSOR_STATE_INVALID;
		g_upsm.eventFilteringTimerActive = ESIF_FALSE;
	}

	esif_ccb_write_unlock(&g_upsm.smLock);
}


static esif_error_t EsifUpsm_RegisterEvents()
{
	esif_error_t rc = ESIF_OK;

	rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_OS_USER_INTERACTION_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	}
	
	return rc;
}


static void EsifUpsm_UnregisterEvents()
{
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_OS_USER_INTERACTION_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
}


static esif_error_t EsifUpsm_SendPresenceEvent(UpSensorState state)
{
	esif_error_t rc = ESIF_OK;
	UpState resolvedState = UP_STATE_INVALID;
	EsifData esifData = { ESIF_DATA_UINT32, &resolvedState, sizeof(resolvedState), sizeof(resolvedState) };

	if (state == UP_SENSOR_STATE_ENGAGED) {
		resolvedState = UP_STATE_FACE_ENGAGED;
	}
	else if (state != UP_SENSOR_STATE_INVALID && g_upsm.interacting) {
		ESIF_TRACE_DEBUG("User interaction detected. Overriding %d presence state to engaged.", state);
		resolvedState = UP_STATE_ENGAGED;
	}
	else {
		resolvedState = (UpState)state;
	}

	rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED, &esifData);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't send platform user presence event!");
		goto exit;
	}

	rc = SetUserPresenceDppeSetting(&esifData, &GUID_DTT_PLATFORM_PRESENCE_STATUS);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Couldn't update platform user presence status!");
	}

exit:
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
	esif_ccb_timer_kill(&g_upsm.eventFilteringTimer);
	g_upsm.eventFilteringTimerActive = ESIF_FALSE;
	g_upsm.eventFilteringEnabled = ESIF_FALSE;
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
	g_upsm.curSensorState = UP_SENSOR_STATE_INVALID;
	g_upsm.filteredSensorState = UP_SENSOR_STATE_INVALID;
	g_upsm.queuedSensorState = UP_SENSOR_STATE_INVALID;

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
		}

		rc = EsifUpsm_RegisterEvents();

		if (ESIF_OK != rc) {
			EsifUpsm_Stop();
			g_upsm.refCount--;
		}
	}

	if (g_upsm.refCount >= 1) {
		EsifUpsm_SendEvents();
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



