/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_ccb.h"
#include "esif_uf_event_cache.h"
#include "esif_ccb_lock.h"
#include "esif_uf_eventmgr.h"
#include "esif_pm.h"



///////////////////////////////////////////////////////////////////////////////
// Type Declarations
///////////////////////////////////////////////////////////////////////////////

typedef struct EsifEventCacheMgr_s {
	esif_ccb_lock_t dataLock;
	Bool isStarted;
} EsifEventCacheMgr;

typedef struct EsifEventCacheEntry_s {
	esif_event_type_t eventType;
	Bool mustRegister;
	EsifData data;
} EsifEventCacheEntry;


///////////////////////////////////////////////////////////////////////////////
// Global Definitions
///////////////////////////////////////////////////////////////////////////////

static EsifEventCacheEntry g_CachedEvents[] = {
	{ESIF_EVENT_OS_POWER_SOURCE_CHANGED,			ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED,			ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_DOCK_MODE_CHANGED,				ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_GAME_MODE_CHANGED,				ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_LID_STATE_CHANGED,				ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_POWER_SLIDER_VALUE_CHANGED,		ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_SCREEN_STATE_CHANGED,			ESIF_TRUE, {0}},
	{ESIF_EVENT_DTT_SYSTEM_COOLING_POLICY_CHANGED,	ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED,			ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_POWERSCHEME_PERSONALITY_CHANGED,	ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_MIXED_REALITY_MODE_CHANGED,		ESIF_TRUE, {0}},
	{ESIF_EVENT_FOREGROUND_BACKGROUND_RATIO_CHANGED,ESIF_TRUE, {0}},
	{ESIF_EVENT_COLLABORATION_CHANGED,              ESIF_TRUE, {0}},
	{ESIF_EVENT_OS_USER_PRESENCE_CHANGED,           ESIF_FALSE, {0}},
	{ESIF_EVENT_DEVICE_ORIENTATION_CHANGED,         ESIF_FALSE, {0}},
	{ESIF_EVENT_MOTION_CHANGED,						ESIF_FALSE, {0}},
	{ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED,        ESIF_FALSE, {0}},
	{ESIF_EVENT_OS_USER_INTERACTION_CHANGED,        ESIF_FALSE, {0}},
	{ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED,     ESIF_FALSE, {0}},
};

#define ESIF_EVENT_CACHE_NUM_ENTRIES (sizeof(g_CachedEvents) / sizeof(*g_CachedEvents))

static EsifEventCacheMgr g_EventCacheMgr = { 0 };

/* Private friend functions for the Event Manger use only */
esif_error_t EsifEventCache_UpdateData(
	esif_event_type_t eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifDataPtr eventDataPtr
	);

Bool EsifEventCache_IsEventCacheable(esif_event_type_t eventType);

///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////

esif_error_t EsifEventCache_GetValue(
	esif_event_type_t eventType,
	EsifData* dataPtr
)
{
	esif_error_t rc = ESIF_E_NOT_SUPPORTED;
	EsifEventCacheEntry* curEntryPtr = g_CachedEvents;
	size_t index = 0;

	if (!g_EventCacheMgr.isStarted) {
		goto exit;
	}

	rc = ESIF_E_PARAMETER_IS_NULL;

	if (dataPtr) {

		rc = ESIF_E_NOT_FOUND;

		esif_ccb_write_lock(&g_EventCacheMgr.dataLock);

		for (index = 0; index < ESIF_EVENT_CACHE_NUM_ENTRIES; index++, curEntryPtr++) {
			if (curEntryPtr->eventType == eventType) {
				if (curEntryPtr->data.buf_ptr != NULL) {
					if (dataPtr->buf_len >= curEntryPtr->data.data_len) {
						esif_ccb_memcpy(dataPtr->buf_ptr, curEntryPtr->data.buf_ptr, curEntryPtr->data.data_len);
						rc = ESIF_OK;
					}
					else {
						rc = ESIF_E_NEED_LARGER_BUFFER;
					}
					dataPtr->data_len = curEntryPtr->data.data_len;
				}
				break;
			}
		}
		esif_ccb_write_unlock(&g_EventCacheMgr.dataLock);
	}
	exit:
	return rc;
}


static esif_error_t UpdateData_Locked(
	EsifEventCacheEntry* entryPtr,
	EsifData* dataPtr
	)
{
	esif_error_t rc = ESIF_OK;

	ESIF_ASSERT(entryPtr);
	ESIF_ASSERT(dataPtr);

	if ((NULL == entryPtr->data.buf_ptr) || (entryPtr->data.buf_len < dataPtr->data_len)) {

		if (0 == dataPtr->data_len) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		esif_ccb_free(entryPtr->data.buf_ptr);
		entryPtr->data.buf_ptr = esif_ccb_malloc(dataPtr->data_len);
		if (NULL == entryPtr->data.buf_ptr) {
			ESIF_TRACE_ERROR("Allocation failure\n");
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		entryPtr->data.buf_len = dataPtr->data_len;
		entryPtr->data.data_len = dataPtr->data_len;
	}

	esif_ccb_memcpy(entryPtr->data.buf_ptr, dataPtr->buf_ptr, dataPtr->data_len);
exit:
	return rc;
}


static esif_error_t ESIF_CALLCONV EsifEventCache_EventCallback(
	esif_context_t context,
	esif_handle_t upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
)
{
	esif_error_t rc = ESIF_OK;
	EsifEventCacheEntry* curEntryPtr = g_CachedEvents;
	size_t index = 0;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(upInstance);
	UNREFERENCED_PARAMETER(domainId);

	if ((NULL == fpcEventPtr) || (NULL == eventDataPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&g_EventCacheMgr.dataLock);

	for (index = 0; index < ESIF_EVENT_CACHE_NUM_ENTRIES; index++, curEntryPtr++) {
		if (curEntryPtr->eventType == fpcEventPtr->esif_event) {
			UpdateData_Locked(curEntryPtr, eventDataPtr);
			break;
		}
	}

	esif_ccb_write_unlock(&g_EventCacheMgr.dataLock);
exit:
	return rc;
}


/* Private friend function for the Event Manger use only */
esif_error_t EsifEventCache_UpdateData(
	esif_event_type_t eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifDataPtr eventDataPtr
	)
{
	esif_error_t rc = ESIF_OK;
	EsifFpcEvent fpcEvent = { 0 };

	fpcEvent.esif_event = eventType;

	if (EsifUpPm_IsPrimaryParticipantId(participantId)) {
		rc = EsifEventCache_EventCallback(0, participantId, domainId, &fpcEvent, eventDataPtr);
	}

	return rc;
}


Bool EsifEventCache_IsEventCacheable(
	esif_event_type_t eventType
	)
{
	Bool isCacheable = ESIF_FALSE;
	EsifEventCacheEntry *curEntryPtr = g_CachedEvents;
	size_t index = 0;

	for (index = 0; index < ESIF_EVENT_CACHE_NUM_ENTRIES; index++, curEntryPtr++) {
		if (eventType == curEntryPtr->eventType) {
			isCacheable = ESIF_TRUE;
			break;
		}
	}
	return isCacheable;
}



esif_error_t EsifEventCache_Init(void)
{
	esif_ccb_lock_init(&g_EventCacheMgr.dataLock);
	return ESIF_OK;
}


void EsifEventCache_Exit(void)
{
	esif_ccb_lock_uninit(&g_EventCacheMgr.dataLock);
}


esif_error_t EsifEventCache_Start(void)
{
	esif_error_t rc = ESIF_OK;

#if defined (ESIF_FEAT_OPT_EVENT_CACHE_ENABLED)
	EsifEventCacheEntry* curEntryPtr = g_CachedEvents;
	size_t index = 0;

	g_EventCacheMgr.isStarted = ESIF_TRUE;

	for (index = 0; index < ESIF_EVENT_CACHE_NUM_ENTRIES; index++, curEntryPtr++) {
		if (curEntryPtr->mustRegister) {
			EsifEventMgr_RegisterEventByType(curEntryPtr->eventType, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventCache_EventCallback, 0);
		}
	}
#endif
	return rc;
}


void EsifEventCache_Stop(void)
{
	EsifEventCacheEntry* curEntryPtr = g_CachedEvents;
	size_t index = 0;

	esif_ccb_write_lock(&g_EventCacheMgr.dataLock);

	if (g_EventCacheMgr.isStarted) {
		for (index = 0; index < ESIF_EVENT_CACHE_NUM_ENTRIES; index++, curEntryPtr++) {

			if (curEntryPtr->mustRegister) {
				EsifEventMgr_UnregisterEventByType(curEntryPtr->eventType, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventCache_EventCallback, 0);
			}

			esif_ccb_free(curEntryPtr->data.buf_ptr);
			curEntryPtr->data.buf_ptr = NULL;
		}
	}

	esif_ccb_write_unlock(&g_EventCacheMgr.dataLock);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
