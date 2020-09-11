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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_UF

#include "esif_ccb_lock.h"
#include "esif_uf_upsm.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_primitive.h"

#define ESIF_UPSM_POLLING_INTERVAL 1000 /* ms */
#define ESIF_UPSM_HID_INPUT_TIME 2 /* s */

// Values from ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED
typedef enum UpSensorState_e {
	UP_SENSOR_STATE_NOT_PRESENT = 0,
	UP_SENSOR_STATE_DISENGAGED = 1,
	UP_SENSOR_STATE_ENGAGED = 2,
	UP_SENSOR_STATE_INVALID = 3 // Not a real sensor state but used for init only
} UpSensorState, *UpSensorStatePtr;

#define UP_SENSOR_STATE_MAX UP_SENSOR_STATE_INVALID

// Values from ESIF_EVENT_OS_USER_PRESENCE_CHANGED
typedef enum UpOsState_e {
	UP_OS_STATE_PRESENT = 0,
	UP_OS_STATE_NOT_PRESENT = 1, /* Should not actually happen...OS will report only 0/2*/
	UP_OS_STATE_INACTIVE = 2
} UpOsState, *UpOsStatePtr;

#define UP_OS_STATE_MAX UP_OS_STATE_INACTIVE

typedef enum UpOsState_e UpsmTableOsState;
typedef enum UpOsState_e *UpsmTableOsStatePtr;

//From GET_LAST_HID_INPUT_TIME
typedef enum UpsmTableHidState_e {
	UPSM_TABLE_HID_STATE_NOT_RECENTLY_ACTIVE = 0,
	UPSM_TABLE_HID_STATE_RECENTLY_ACTIVE = 1,
	UPSM_TABLE_HID_STATE_DONT_CARE = 2
} UpsmTableHidState, *UpsmTableHidStatePtr;


//get_last_hit_input_time
typedef enum UpsmTableResolvedState_e {
	UPSM_TABLE_STATE_NOT_PRESENT = 0,
	UPSM_TABLE_STATE_DISENGAGED = 1,
	UPSM_TABLE_STATE_ENGAGED = 2,
	UPSM_TABLE_STATE_FACE_ENGAGED = 3,
	UPSM_TABLE_STATE_INVALID = 99
} UpsmTableResolvedState, *UpsmTableResolvedStatePtr;


typedef struct UpsmTableEntry_s {
	UpsmTableOsState osState;
	UpSensorState sensorState;
	UpsmTableHidState hidState;
	UpsmTableResolvedState resolvedState;
}UpsmTableEntry, *UpsmTableEntryPtr;

#define UPSM_INITIAL_OS_STATE UP_OS_STATE_PRESENT;
#define UPSM_INITIAL_SENSOR_STATE UP_SENSOR_STATE_INVALID;
#define UPSM_INITIAL_RESOLVED_STATE UPSM_TABLE_STATE_ENGAGED;



typedef struct EsifUpsm_s {
	esif_ccb_lock_t enableLock;  // Used to prevent enabling/disabling at same time
	esif_ccb_lock_t smLock;	     // Used for state machine management
	Int32 refCount;				 // UPSM enabled when count >= 1; disabled when count == 0

	UpOsState curOsState;
	UpSensorState curSensorState;
	UpsmTableResolvedState curResolvedState;

	esif_ccb_timer_t hidPollingTimer;
	Bool hidPollingEnabled;		// HID polling allowed
	Bool hidPollingActive;		// HID polling enabled and active

} EsifUpsm, *EsifUpsmPtr;


static EsifUpsm g_upsm = { 0 };
static UpsmTableEntry g_upsmTable[] = {
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_ENGAGED,     UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_FACE_ENGAGED},
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_DISENGAGED,  UPSM_TABLE_HID_STATE_RECENTLY_ACTIVE,     UPSM_TABLE_STATE_ENGAGED},
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_DISENGAGED,  UPSM_TABLE_HID_STATE_NOT_RECENTLY_ACTIVE, UPSM_TABLE_STATE_DISENGAGED},
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_NOT_PRESENT, UPSM_TABLE_HID_STATE_RECENTLY_ACTIVE,     UPSM_TABLE_STATE_ENGAGED},
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_NOT_PRESENT, UPSM_TABLE_HID_STATE_NOT_RECENTLY_ACTIVE, UPSM_TABLE_STATE_NOT_PRESENT},
	{UP_OS_STATE_INACTIVE, UP_SENSOR_STATE_ENGAGED,     UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_FACE_ENGAGED},
	{UP_OS_STATE_INACTIVE, UP_SENSOR_STATE_DISENGAGED,  UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_DISENGAGED},
	{UP_OS_STATE_INACTIVE, UP_SENSOR_STATE_NOT_PRESENT, UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_NOT_PRESENT},
	{UP_OS_STATE_PRESENT,  UP_SENSOR_STATE_INVALID,     UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_INVALID},
	{UP_OS_STATE_INACTIVE, UP_SENSOR_STATE_INVALID,     UPSM_TABLE_HID_STATE_DONT_CARE,           UPSM_TABLE_STATE_INVALID},
};


static esif_error_t EsifUpsm_RegisterEvents();
static void EsifUpsm_UnregisterEvents();
static esif_error_t EsifUpsm_GetHidInputTime(UInt32 *timePtr);
static esif_error_t EsifUpsm_StartHidPolling_SmLocked();
static void EsifUpsm_DisableHidPolling();
static void EsifUpsm_StopHidPolling_SmLocked();
static esif_error_t EsifUpsm_SendEvent(UpsmTableResolvedState state);


static esif_error_t EsifUpsm_ResolveState_SmLocked()
{
	esif_error_t rc = ESIF_OK;
	UpsmTableEntryPtr curEntryPtr = NULL;
	UInt32 i = 0;
	UInt32 hidTime = 0;
	UpsmTableHidState hidState = UPSM_TABLE_HID_STATE_DONT_CARE;
	Bool isHidStateValid = ESIF_FALSE;

	curEntryPtr = g_upsmTable;

	for (i = 0; i < (sizeof(g_upsmTable)/sizeof(*g_upsmTable)); i++, curEntryPtr++) {
		if ((g_upsm.curOsState == curEntryPtr->osState) && (g_upsm.curSensorState == curEntryPtr->sensorState)) {
			if ((curEntryPtr->hidState != UPSM_TABLE_HID_STATE_DONT_CARE) && (!isHidStateValid)) {
				rc = EsifUpsm_GetHidInputTime(&hidTime);
				if (ESIF_OK == rc) {
					hidState = UPSM_TABLE_HID_STATE_NOT_RECENTLY_ACTIVE;
					if (hidTime <= ESIF_UPSM_HID_INPUT_TIME) {
						hidState = UPSM_TABLE_HID_STATE_RECENTLY_ACTIVE;
					}
					isHidStateValid = ESIF_TRUE;
				}
			}
			if (hidState == curEntryPtr->hidState) {
				if (curEntryPtr->resolvedState != g_upsm.curResolvedState) {
					g_upsm.curResolvedState = curEntryPtr->resolvedState;
					EsifUpsm_SendEvent(curEntryPtr->resolvedState);
				}

				if (curEntryPtr->hidState != UPSM_TABLE_HID_STATE_DONT_CARE) {
					EsifUpsm_StartHidPolling_SmLocked();
				}
				else {
					EsifUpsm_StopHidPolling_SmLocked();
				}
				break;
			}
		}
	}
	if (isHidStateValid) {
		ESIF_TRACE_DEBUG("User presence state: OS-%d, Sensor-%d, HID-%ds, Resolved state-%d\n", g_upsm.curOsState, g_upsm.curSensorState, hidTime, g_upsm.curResolvedState);
	}
	else {
		ESIF_TRACE_DEBUG("User presence state: OS-%d, Sensor-%d, HID-N/A, Resolved state-%d\n", g_upsm.curOsState, g_upsm.curSensorState, g_upsm.curResolvedState);
	}
		return rc;
}


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
		if (data != (UInt32)g_upsm.curSensorState) {
			g_upsm.curSensorState = data;
			rc = EsifUpsm_ResolveState_SmLocked();
		}
		break;
	case ESIF_EVENT_OS_USER_PRESENCE_CHANGED:
		if (data != (UInt32)g_upsm.curOsState) {
			g_upsm.curOsState = data;
			rc = EsifUpsm_ResolveState_SmLocked();
		}
		break;
	default:
		break;
	}

	esif_ccb_write_unlock(&g_upsm.smLock);
exit:
	return rc;
}


static void EsifUpsm_HidPollingCallback(void* context_ptr)
{
	UNREFERENCED_PARAMETER(context_ptr);

	esif_ccb_write_lock(&g_upsm.smLock);
	EsifUpsm_ResolveState_SmLocked();

	if (g_upsm.hidPollingEnabled && g_upsm.hidPollingActive) {
		esif_ccb_timer_set_msec(&g_upsm.hidPollingTimer, ESIF_UPSM_POLLING_INTERVAL);
	}
	esif_ccb_write_unlock(&g_upsm.smLock);
}


static esif_error_t EsifUpsm_StartHidPolling_SmLocked()
{
	esif_error_t rc = ESIF_OK;

	if (g_upsm.hidPollingEnabled && !g_upsm.hidPollingActive) {
		rc = esif_ccb_timer_init(&g_upsm.hidPollingTimer, EsifUpsm_HidPollingCallback, NULL);
		if (ESIF_OK == rc) {
			rc = esif_ccb_timer_set_msec(&g_upsm.hidPollingTimer, ESIF_UPSM_POLLING_INTERVAL);
			if (ESIF_OK == rc) {
				g_upsm.hidPollingActive = ESIF_TRUE;
			}
			else {
				esif_ccb_timer_kill_w_wait(&g_upsm.hidPollingTimer);
			}
		}
	}

	return rc;
}


static void EsifUpsm_StopHidPolling_SmLocked()
{
	esif_ccb_timer_kill(&g_upsm.hidPollingTimer);
	g_upsm.hidPollingActive = ESIF_FALSE;
}


static void EsifUpsm_DisableHidPolling()
{
	esif_ccb_write_lock(&g_upsm.smLock);
	g_upsm.hidPollingEnabled = ESIF_FALSE;
	g_upsm.hidPollingActive = ESIF_FALSE;
	esif_ccb_write_unlock(&g_upsm.smLock);
	esif_ccb_timer_kill_w_wait(&g_upsm.hidPollingTimer);
}


static esif_error_t EsifUpsm_GetHidInputTime(UInt32 *timePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	UInt32 time = 0;
	EsifData esifData = { ESIF_DATA_UINT32, &time, sizeof(time), 0 };

	if (timePtr != NULL) {
		rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_LAST_HID_INPUT_TIME, "D0", 255, NULL, &esifData);
		if (ESIF_OK == rc) {
			*timePtr = time;
		}
	}
	return rc;
}


static esif_error_t EsifUpsm_RegisterEvents()
{
	esif_error_t rc = ESIF_OK;

	rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_OS_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	}
	return rc;
}


static void EsifUpsm_UnregisterEvents()
{
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_OS_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_SENSOR_USER_PRESENCE_CHANGED, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifUpsm_EventCallback, 0);
}


static esif_error_t EsifUpsm_SendEvent(UpsmTableResolvedState state)
{
	esif_error_t rc = ESIF_OK;
	EsifData esifData = { ESIF_DATA_UINT32, &state, sizeof(state), sizeof(state) };

	rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED, &esifData);

	return rc;
}


esif_error_t EsifUpsm_Init(void)
{
	g_upsm.curOsState = UPSM_INITIAL_OS_STATE;
	g_upsm.curSensorState = UP_SENSOR_STATE_INVALID;
	g_upsm.curResolvedState = UPSM_INITIAL_RESOLVED_STATE;

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
	EsifUpsm_DisableHidPolling();
	return;
}


esif_error_t EsifUpsm_Enable(void)
{
	esif_error_t rc = ESIF_OK;

	esif_ccb_write_lock(&g_upsm.enableLock);

	g_upsm.refCount++;

	if (1 == g_upsm.refCount) {
		rc = EsifUpsm_RegisterEvents();

		if (ESIF_OK == rc) {
			g_upsm.hidPollingEnabled = ESIF_TRUE;
		} else {
			EsifUpsm_Stop();
			g_upsm.refCount--;
		}
	}

	if (g_upsm.refCount >= 1) {
		EsifUpsm_SendEvent(g_upsm.curResolvedState);
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



