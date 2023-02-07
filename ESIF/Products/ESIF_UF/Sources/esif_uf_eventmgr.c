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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_uf.h"	/* Upper Framework */
#include "esif_uf_appmgr.h"
#include "esif_link_list.h"
#include "esif_event.h"
#include "esif_ccb_atomic.h"
#include "esif_uf_eventmgr.h"
#include "esif_queue.h"
#include "esif_uf_sensors.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define NUM_EVENT_LISTS 64
#define EVENT_MGR_FILTERED_EVENTS_PER_LINE 64
#define EVENT_MGR_ITERATOR_MARKER 'UFEM'

typedef struct EsifEventMgr_s {
	EsifLinkListPtr observerLists[NUM_EVENT_LISTS];
	esif_ccb_lock_t listLock;

	EsifLinkListPtr garbageList;

	EsifQueuePtr eventQueuePtr;

	Bool eventQueueExitFlag;
	Bool eventsDisabled;

	esif_thread_t eventQueueThread;

	UInt64 filteredEvents[(MAX_ESIF_EVENT_ENUM_VALUE / EVENT_MGR_FILTERED_EVENTS_PER_LINE) + 1];
	Bool *cacheableEventListPtr;

	/*
	* List of applications whose events will be unregistered on CS/MS exit
	* Allows sensors to remain registered (for WOA for example) until CS/MS exit
	* Contains EsifEventMgr_AppRemovalEntry items
	*/
	EsifLinkListPtr appUnregisterList;
	esif_ccb_lock_t appUnregisterListLock;
	Bool delayedAppUnregistrationEnabled;
	Bool delayAppUnregistration;
}EsifEventMgr, *EsifEventMgrPtr;

typedef struct EventMgrEntry_s {
	esif_handle_t participantId;		/* UF Participant ID */
	Bool isParticipant0Id;				/* Used to match as multiple Participant 0 IDs are supported */
	UInt16 domainId;					/* Domain ID - '0D'*/
	EsifFpcEvent fpcEvent;				/* Event definition from the DSP */
	EVENT_OBSERVER_CALLBACK callback;	/* Callback routine - Also used to uniquely identify an event observer when unregistering */
	esif_context_t context;				/* This is normally expected to be a pointer to the observing event object.
										 * Expected to act as a context for the callback, an event observer identifier,
										 * and to help uniquely identify an event observer while unregistering.
										 */
	atomic_t refCount;					/* Reference count */
	Bool isInUse;						/* Indicates the event is being processed */
	Bool markedForDelete;				/* Indicates the event is marked for deletion */
} EventMgrEntry, *EventMgrEntryPtr;

typedef struct EsifEventQueueItem_s {
	esif_handle_t participantId;
	UInt16 domainId;
	eEsifEventType eventType;
	EsifData eventData;
	Bool isLfEvent;
	Bool isUnfiltered;
}EsifEventQueueItem, *EsifEventQueueItemPtr;


typedef struct EsifEventMgr_AppRemovalEntry_s {
	EVENT_OBSERVER_CALLBACK eventCallback;
	esif_context_t context; /* App handle */
}EsifEventMgr_AppRemovalEntry;


/*
 * All event received are asynchronous and placed in an event queue to be handled by a worker thread.
 *
 * The manager maintains information on registered event "observers"
 * Interface:
 *   EsifEventMgr_SignalEvent
 *   EsifEventMgr_RegisterEventByType
 *   EsifEventMgr_UnregisterEventByType
 *
 * Event observer information is maintained as an array of linked lists with a list of observers the event types
 * There are 64 event lists (Information is placed in a given list given by the modulo 64 of the event type)
 * Event observers may register based on the event type or GUID.
 * EVENT_MGR_MATCH_ANY may be used as the participant ID during registration to observe events from all participants;
 * or if registration takes place before the participants are present.
 * EVENT_MGR_MATCH_ANY_DOMAIN may be used as the domain ID during registration to observe events for all domains.
 * Locks are released before any calls outside the event manager which may result in obtaining other locks;
 * locks re-acquired upon return.
 * A reference count is kept for each observer; events are only sent to observers with a positive reference count
 * When the reference count reaches 0, the node is garbage collected
 * Before an observer is called, the reference count is incremented so that the node is not removed while in use;
 * the reference count is decrement upon return and the node is garbage collected if the reference count is then 0.
 * A single garbage collection linked list is maintained.  Any garbage nodes are moved to that list for destruction.
 * Any steps required to enable/disable an event, for example DPPE, will be performed during creation/destruction.
 * Simulation Support:
 * The event manager maintains a list of "filtered" event types.  When marked as "filtered", only events from
 * the shell are handled; while the events from other sources are ignored.
 */

static EsifEventMgr g_EsifEventMgr = {0};


/* Private friend functions for the Event Manger use only */
esif_error_t EsifEventCache_UpdateData(
	esif_event_type_t eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifDataPtr eventDataPtr
	);

Bool EsifEventCache_IsEventCacheable(esif_event_type_t eventType);

static eEsifError EsifEventMgr_AddEntry(
	EsifFpcEventPtr fpcEventPtr,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	);

static eEsifError EsifEventMgr_ReleaseEntry(
	EsifFpcEventPtr fpcEventPtr,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	);

static eEsifError EsifEventMgr_ProcessEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	EsifDataPtr eventDataPtr
	);

static eEsifError EsifEventMgr_EnableEvent(EventMgrEntryPtr entryPtr);
static eEsifError EsifEventMgr_DisableEvent(EventMgrEntryPtr entryPtr);
static eEsifError EsifEventMgr_MoveEntryToGarbage(EventMgrEntryPtr entryPtr);
static eEsifError EsifEventMgr_DumpGarbage();
static void EsifEventMgr_QueueDestroyCallback(void *ctxPtr);
static void EsifEventMgr_LLEntryDestroyCallback(void *dataPtr);

static eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent_Local (
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr,
	Bool isLfEvent,
	Bool isFilteredEvent
);

static Bool EsifEventMgr_IsEventFiltered(eEsifEventType eventType);
static Bool EsifEventMgr_IsEventCacheable(eEsifEventType eventType);

/*
* Friend function:  Target is determined only once removed from queue and all
* others before it have been processed
*/
eEsifError ESIF_CALLCONV EsifEventMgr_SignalLfEvent(
	esif_handle_t targetId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventData
);


eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr
)
{
	return EsifEventMgr_SignalEvent_Local(participantId, domainId, eventType, eventDataPtr, ESIF_FALSE, ESIF_TRUE);
}


/*
* Friend function:  Target is determined only once removed from queue and all
* others before it have been processed
*/
eEsifError ESIF_CALLCONV EsifEventMgr_SignalLfEvent(
	esif_handle_t targetId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr
)
{
	return EsifEventMgr_SignalEvent_Local(targetId, domainId, eventType, eventDataPtr, ESIF_TRUE, ESIF_TRUE);
}


eEsifError ESIF_CALLCONV EsifEventMgr_SignalUnfilteredEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr
	)
{
	return EsifEventMgr_SignalEvent_Local(participantId, domainId, eventType, eventDataPtr, ESIF_FALSE, ESIF_FALSE);
}


eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent_Local(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr,
	Bool isLfEvent,
	Bool isFilteredEvent
	)
{
	eEsifError rc = ESIF_OK;
	EsifEventQueueItemPtr queueEventPtr = NULL;
	void *queueDataPtr = NULL;

	/* Exit if filtered event */
	if (isFilteredEvent && EsifEventMgr_IsEventFiltered(eventType)) {
		rc = ESIF_E_EVENT_FILTERED;
		goto exit;
	}

	if (NULL == g_EsifEventMgr.eventQueuePtr) { /* Should never happen */
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	queueEventPtr = esif_ccb_malloc(sizeof(*queueEventPtr));
	if (NULL == queueEventPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if ((eventDataPtr != NULL) &&
		(eventDataPtr->buf_ptr != NULL) &&
		(eventDataPtr->buf_len > 0) &&
		(eventDataPtr->data_len > 0) &&
		(eventDataPtr->buf_len >= eventDataPtr->data_len)) {

		queueDataPtr = esif_ccb_malloc(eventDataPtr->data_len);
		if (NULL == queueDataPtr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		esif_ccb_memcpy(queueDataPtr, eventDataPtr->buf_ptr, eventDataPtr->data_len);

		queueEventPtr->eventData.type = eventDataPtr->type;
		queueEventPtr->eventData.buf_ptr = queueDataPtr;
		queueEventPtr->eventData.buf_len = eventDataPtr->data_len;
		queueEventPtr->eventData.data_len = eventDataPtr->data_len;
	}

	queueEventPtr->participantId = participantId;
	queueEventPtr->domainId = domainId;
	queueEventPtr->eventType = eventType;
	queueEventPtr->isLfEvent = isLfEvent;

	ESIF_TRACE_INFO("Queuing %s event for Part. %u Dom. 0x%04X\n",
		esif_event_type_str(eventType),
		participantId,
		domainId);

	rc = esif_queue_enqueue(g_EsifEventMgr.eventQueuePtr, queueEventPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

exit:
	if (rc != ESIF_OK) {
		esif_ccb_free(queueEventPtr);
		esif_ccb_free(queueDataPtr);

	}
	return rc;
}


static void *ESIF_CALLCONV EsifEventMgr_EventQueueThread(void *ctxPtr)
{
	esif_error_t rc = ESIF_OK;
	EsifEventQueueItemPtr queueEventPtr = NULL;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;

	UNREFERENCED_PARAMETER(ctxPtr);

	while(!g_EsifEventMgr.eventQueueExitFlag) {
		rc = ESIF_OK;
		queueEventPtr = esif_queue_pull(g_EsifEventMgr.eventQueuePtr);

		if (NULL == queueEventPtr) {
			continue;
		}

		ESIF_TRACE_INFO("Dequeuing %s event for Part. %u Dom. 0x%04X\n",
			esif_event_type_str(queueEventPtr->eventType),
			queueEventPtr->participantId,
			queueEventPtr->domainId);

		participantId = queueEventPtr->participantId;
		if (queueEventPtr->isLfEvent) {
			rc = EsifUpPm_MapLpidToParticipantInstance((u8)participantId, &participantId);

			/* For creation, the target is the primary participant always */
			if ((rc != ESIF_OK) && (ESIF_EVENT_PARTICIPANT_CREATE == queueEventPtr->eventType)) {
				participantId = ESIF_HANDLE_PRIMARY_PARTICIPANT;
				rc = ESIF_OK;
			}
		}

		if (ESIF_OK == rc) {
			EsifEventMgr_ProcessEvent(participantId,
				queueEventPtr->domainId,
				queueEventPtr->eventType,
				&queueEventPtr->eventData);
		}
		esif_ccb_free(queueEventPtr->eventData.buf_ptr);
		esif_ccb_free(queueEventPtr);
	}
	return 0;
}


static eEsifError EsifEventMgr_ProcessEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifLinkListNodePtr nextNodePtr = NULL;
	EventMgrEntryPtr entryPtr = NULL;
	char domain_str[8] = "";
	atomic_t refCount = 0;
	UInt8 shouldDumpGarbage = ESIF_FALSE;

	UNREFERENCED_PARAMETER(domain_str);

	if (NULL == eventDataPtr) {
		ESIF_TRACE_DEBUG("APPLICATION_EVENT_NO_DATA:\n"
			"  ParticipantID: " ESIF_HANDLE_FMT "\n"
			"  Domain:        %s(%04X)\n"
			"  EventType:     %s(%d)\n",
			esif_ccb_handle2llu(participantId),
			esif_primitive_domain_str(domainId, domain_str, sizeof(domain_str)),
			domainId,
			esif_event_type_str(eventType), eventType);
	} else {
		ESIF_TRACE_DEBUG("APPLICATION_EVENT\n"
						 "  ParticipantID: " ESIF_HANDLE_FMT "\n"
						 "  Domain:        %s(%04X)\n"
						 "  EventType:     %s(%d)\n"
						 "  EventDataType: %s(%d)\n"
						 "  EventData:     %p\n"
						 "    buf_ptr      %p\n"
						 "    buf_len      %d\n"
						 "    data_len     %d\n",
						 esif_ccb_handle2llu(participantId),
						 esif_primitive_domain_str(domainId, domain_str, 8),
						 domainId,
						 esif_event_type_str(eventType), eventType,
						 esif_data_type_str(eventDataPtr->type), eventDataPtr->type,
						 eventDataPtr,
						 eventDataPtr->buf_ptr, eventDataPtr->buf_len, eventDataPtr->data_len
						 );

		if ((ESIF_DATA_STRING == eventDataPtr->type) && (NULL != eventDataPtr->buf_ptr)) {
			ESIF_TRACE_DEBUG(
				"  Data Length:   %d\n"
				"  Data:          %s\n",
				eventDataPtr->data_len,
				(EsifString)eventDataPtr->buf_ptr);
		}
	}

	/*
	* First, allow the event cache update its data before sending to the
	* listeners, so that the data is updated already when other callbacks
	* are called.
	*/
	if (EsifEventMgr_IsEventCacheable(eventType)) {
		EsifEventCache_UpdateData(
			eventType,
			participantId,
			domainId,
			eventDataPtr
		);
	}
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

	/*
	* Next, send the event to all registered listeners.
	*/
	listPtr = g_EsifEventMgr.observerLists[(unsigned)eventType % NUM_EVENT_LISTS];
	if(NULL == listPtr) {
		rc = ESIF_E_UNSPECIFIED;
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		goto exit;
	}

	nodePtr = listPtr->head_ptr;
	while (NULL != nodePtr) {
		nextNodePtr = nodePtr->next_ptr;

		entryPtr = (EventMgrEntryPtr)nodePtr->data_ptr;
		ESIF_ASSERT(entryPtr != NULL);

		if ((eventType == entryPtr->fpcEvent.esif_event) &&
			((entryPtr->participantId == participantId) || (entryPtr->participantId == EVENT_MGR_MATCH_ANY) || (entryPtr->isParticipant0Id && EsifUpPm_IsPrimaryParticipantId(participantId))) &&
			((entryPtr->domainId == domainId) || (entryPtr->domainId == EVENT_MGR_MATCH_ANY_DOMAIN) || (domainId == EVENT_MGR_DOMAIN_NA)) &&
			(entryPtr->refCount > 0) &&
			(!entryPtr->markedForDelete)) {

			/*
			 * Increment the reference count so that the node is not removed while in use,
			 * then release the lock so that we avoid a deadlock condition
			 */
			atomic_inc(&entryPtr->refCount);
			entryPtr->isInUse = ESIF_TRUE;
			esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

			entryPtr->callback(entryPtr->context,
				participantId,
				domainId,
				&entryPtr->fpcEvent,
				eventDataPtr);

			/* 
			 * Get the lock back and decrement the reference count.  Remove the node now
			 * if the reference count is 0
			 */
			esif_ccb_write_lock(&g_EsifEventMgr.listLock);

			nextNodePtr = nodePtr->next_ptr;

			entryPtr->isInUse = ESIF_FALSE;
			refCount = atomic_dec(&entryPtr->refCount);
			if ((refCount <= 0) || (entryPtr->markedForDelete)) {
				EsifEventMgr_MoveEntryToGarbage(entryPtr);
				esif_link_list_node_remove(listPtr, nodePtr);
				shouldDumpGarbage = ESIF_TRUE;
			}
		}
		nodePtr = nextNodePtr;
	}

	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	if (shouldDumpGarbage) {
		EsifEventMgr_DumpGarbage();
	}

exit:
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_RegisterEventByType(
	eEsifEventType eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifFpcEventPtr fpcEventPtr = NULL;
	EsifUpPtr upPtr = NULL;
	EsifFpcEvent phonyFpcEvent = {0};
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	if(NULL == eventCallback) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (participantId != EVENT_MGR_MATCH_ANY) {
		/* Find Our Participant */
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
		if (NULL == upPtr) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}

		/* Find the event associated with the participant */
		fpcEventPtr = EsifUp_GetFpcEventByType(upPtr, eventType);
		if (NULL == fpcEventPtr) {
			rc = ESIF_E_EVENT_NOT_FOUND;
			goto exit;
		}

	} else {
		phonyFpcEvent.esif_event = eventType;
		fpcEventPtr = &phonyFpcEvent;
	}

	ESIF_TRACE_DEBUG(
		"Registering event\n"
		"  Alias: %s\n"
		"  Type: %s(%d)\n"
		"  Data: %s(%d)\n"
		"  Key: %s\n"
		"  Group: %s\n",
		fpcEventPtr->event_name,
		esif_event_type_str(eventType), eventType,
		esif_data_type_str(fpcEventPtr->esif_group_data_type), fpcEventPtr->esif_group_data_type,
		esif_guid_print((esif_guid_t *)fpcEventPtr->event_key, guidStr),
		esif_event_group_enum_str(fpcEventPtr->esif_group));

	rc = EsifEventMgr_AddEntry(
		fpcEventPtr,
		participantId,
		domainId,
		eventCallback,
		context);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByType(
	eEsifEventType eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifFpcEventPtr fpcEventPtr = NULL;
	EsifUpPtr upPtr = NULL;
	EsifFpcEvent phonyFpcEvent = {0};

	if(NULL == eventCallback) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (participantId != EVENT_MGR_MATCH_ANY) {
		/* Find Our Participant */
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
		if (NULL == upPtr) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}

		/* Find the event associated with the participant */
		fpcEventPtr = EsifUp_GetFpcEventByType(upPtr, eventType);
		if (NULL == fpcEventPtr) {
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}
	} else {
		phonyFpcEvent.esif_event = eventType;
		fpcEventPtr = &phonyFpcEvent;
	}

	rc = EsifEventMgr_ReleaseEntry(
		fpcEventPtr,
		participantId,
		domainId,
		eventCallback,
		context);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


static eEsifError ESIF_CALLCONV EsifEventMgr_DelayAppUnregistration(
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifEventMgr_AppRemovalEntry *entryPtr = NULL;

	ESIF_TRACE_DEBUG("Delaying unregister all events for app " ESIF_HANDLE_FMT "\n", context);

	if ((NULL == eventCallback) || (ESIF_HANDLE_DEFAULT == context)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == g_EsifEventMgr.appUnregisterList) {
		rc = ESIF_E_NOT_INITIALIZED;
		goto exit;
	}

	entryPtr = (EsifEventMgr_AppRemovalEntry *)esif_ccb_malloc(sizeof(*entryPtr));
	if (NULL == entryPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	entryPtr->eventCallback = eventCallback;
	entryPtr->context = context;
	esif_ccb_write_lock(&g_EsifEventMgr.appUnregisterListLock);
	esif_link_list_add_at_back(g_EsifEventMgr.appUnregisterList, entryPtr);
	esif_ccb_write_unlock(&g_EsifEventMgr.appUnregisterListLock);
exit:
	ESIF_TRACE_DEBUG("Exit code = %d\n", rc);
	return rc;
}


static eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterApp(
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr curNodePtr = NULL;
	EsifLinkListNodePtr nextNodePtr = NULL;
	EventMgrEntryPtr curEntryPtr = NULL;
	UInt8 i = 0;

	ESIF_TRACE_DEBUG("Unregistering all events for app " ESIF_HANDLE_FMT "\n", context);

	if ((NULL == eventCallback) || (ESIF_HANDLE_DEFAULT == context)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

	for (i = 0; i < NUM_EVENT_LISTS; i++) {

		listPtr = g_EsifEventMgr.observerLists[i];
		if (NULL == listPtr) {
			continue;
		}

		/* Find the matching entry */
		curNodePtr = listPtr->head_ptr;
		while (curNodePtr != NULL) {
			nextNodePtr = curNodePtr->next_ptr; // Get next ptr now as the current node may be removed below

			curEntryPtr = (EventMgrEntryPtr)curNodePtr->data_ptr;
			if ((curEntryPtr->callback == eventCallback) &&
				(curEntryPtr->context == context)) {

				curEntryPtr->markedForDelete = ESIF_TRUE;
				if (!curEntryPtr->isInUse) {
					EsifEventMgr_MoveEntryToGarbage(curEntryPtr);
					esif_link_list_node_remove(listPtr, curNodePtr);
				}
			}
			curNodePtr = nextNodePtr;
		}
	}

	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
	EsifEventMgr_DumpGarbage();
exit:
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterAllForApp(
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("Unregistering all events for app " ESIF_HANDLE_FMT "\n", context);

	if ((NULL == eventCallback) || (ESIF_HANDLE_DEFAULT == context)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* If in low power state; register for removal after exit of low power state */
	if (g_EsifEventMgr.delayedAppUnregistrationEnabled && g_EsifEventMgr.delayAppUnregistration) {
		EsifEventMgr_DelayAppUnregistration(eventCallback, context);
	}
	else {
		EsifEventMgr_UnregisterApp(eventCallback, context);
	}
exit:
	ESIF_TRACE_DEBUG("Exit code = %d\n", rc);
	return rc;
}


static eEsifError EsifEventMgr_AddEntry(
	EsifFpcEventPtr fpcEventPtr,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr curEntryPtr = NULL;
	EventMgrEntryPtr newEntryPtr = NULL;
	atomic_t refCount = 1;

	ESIF_ASSERT(fpcEventPtr != NULL);
	ESIF_ASSERT(eventCallback != NULL);

	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

	listPtr = g_EsifEventMgr.observerLists[fpcEventPtr->esif_event % NUM_EVENT_LISTS];
	if(NULL == listPtr) {
		rc = ESIF_E_UNSPECIFIED;
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		goto exit;
	}

	/* 
	 * First verify we don't already have the same entry.
	 * If we do, just increment the reference count.
	 */
	nodePtr = listPtr->head_ptr;
	while (nodePtr != NULL) {
		curEntryPtr = (EventMgrEntryPtr) nodePtr->data_ptr;
		if ((curEntryPtr->fpcEvent.esif_event == fpcEventPtr->esif_event) &&
			((curEntryPtr->participantId == participantId) || (curEntryPtr->isParticipant0Id && EsifUpPm_IsPrimaryParticipantId(participantId))) &&
			(curEntryPtr->domainId == domainId) &&
			(curEntryPtr->context == context) &&
			(curEntryPtr->callback == eventCallback)){
			break;
		}
		nodePtr = nodePtr->next_ptr;
	}
	/* If we found an existing entry, update the reference count */
	if (nodePtr != NULL) {
		if (!curEntryPtr->markedForDelete) {
			atomic_inc(&curEntryPtr->refCount);
		}
		else {
			rc = ESIF_E_NO_CREATE;
		}
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		goto exit;
	}
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	/*
	 * If an matching observer entry was not present; create a new observer entry,
	 * enable the events, and then place it into the list
	 */
	newEntryPtr = esif_ccb_malloc(sizeof(*newEntryPtr));
	if (NULL == newEntryPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	newEntryPtr->callback = eventCallback;
	newEntryPtr->context = context;
	newEntryPtr->domainId = domainId;
	newEntryPtr->participantId = participantId;
	newEntryPtr->refCount = refCount;
	esif_ccb_memcpy(&newEntryPtr->fpcEvent, fpcEventPtr, sizeof(newEntryPtr->fpcEvent));
	newEntryPtr->isParticipant0Id = EsifUpPm_IsPrimaryParticipantId(participantId);

	nodePtr = esif_link_list_create_node(newEntryPtr);
	if (NULL == nodePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	esif_link_list_add_node_at_back(listPtr, nodePtr);
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	rc = EsifEventMgr_EnableEvent(newEntryPtr);

exit:
	ESIF_TRACE_DEBUG("  RefCount: " ATOMIC_FMT "\n", refCount);

	if (ESIF_OK != rc) {
		esif_ccb_write_lock(&g_EsifEventMgr.listLock);
		esif_link_list_node_remove(listPtr, nodePtr);
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		esif_ccb_free(newEntryPtr);
	}

	return rc;
}


static eEsifError EsifEventMgr_ReleaseEntry(
	EsifFpcEventPtr fpcEventPtr,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr curEntryPtr = NULL;
	atomic_t refCount = -1;

	ESIF_ASSERT(eventCallback != NULL);
	ESIF_ASSERT(fpcEventPtr != NULL);

	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

	listPtr = g_EsifEventMgr.observerLists[fpcEventPtr->esif_event % NUM_EVENT_LISTS];
	if(NULL == listPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	/* Find the matching entry */
	nodePtr = listPtr->head_ptr;
	while (nodePtr != NULL) {
		curEntryPtr = (EventMgrEntryPtr) nodePtr->data_ptr;
		if ((curEntryPtr->fpcEvent.esif_event == fpcEventPtr->esif_event) &&
			(curEntryPtr->participantId == participantId) &&
			(curEntryPtr->domainId == domainId) &&
			(curEntryPtr->context == context) &&
			(curEntryPtr->callback == eventCallback)){
			break;
		}
		nodePtr = nodePtr->next_ptr;
	}

	if (nodePtr != NULL) {
		refCount = atomic_dec(&curEntryPtr->refCount);
		if ((refCount <= 0) || (curEntryPtr->markedForDelete)) {
			EsifEventMgr_MoveEntryToGarbage(curEntryPtr);
			esif_link_list_node_remove(listPtr, nodePtr);
		}
		goto exit;
	}
exit:
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
	EsifEventMgr_DumpGarbage();
	return rc;
}


static eEsifError EsifEventMgr_EnableEvent(
	EventMgrEntryPtr entryPtr
	)
{
	eEsifError rc = ESIF_OK;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	ESIF_ASSERT(entryPtr != NULL);

	if (!EsifUpPm_IsPrimaryParticipantId(entryPtr->participantId) &&
		(entryPtr->participantId != EVENT_MGR_MATCH_ANY)) {
		goto exit;
	}

	switch(entryPtr->fpcEvent.esif_group)
	{
	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("  Enabling Power Event: %s\n", esif_guid_print((esif_guid_t *)&entryPtr->fpcEvent.event_key, guidStr));
		rc = register_for_power_notification((esif_guid_t *)&entryPtr->fpcEvent.event_key);
		break;

	case ESIF_EVENT_GROUP_SYSTEM_METRICS:
		ESIF_TRACE_DEBUG("  Enabling System Metrics Event: %s\n", esif_guid_print((esif_guid_t *)&entryPtr->fpcEvent.event_key, guidStr));
		rc = register_for_system_metrics_notification((esif_guid_t *)&entryPtr->fpcEvent.event_key);
		break;

	case ESIF_EVENT_GROUP_SENSOR:
		ESIF_TRACE_DEBUG("Enabling CEM sensor events\n");
		rc = esif_register_sensors(entryPtr->fpcEvent.esif_event);
		break;

	case ESIF_EVENT_GROUP_CODE:
		rc = esif_enable_code_event(entryPtr->fpcEvent.esif_event);
		break;

	case ESIF_EVENT_GROUP_DPTF:
	case ESIF_EVENT_GROUP_ACPI:
	default:
		break;
	}

	// If the event is registered successfully, send the initial event with the current state of the event so that the client can get the baseline
	if (ESIF_OK == rc)
	{
		EsifEventMgr_SendInitialEvent(entryPtr->participantId, entryPtr->domainId, entryPtr->fpcEvent.esif_event);
	}
exit:
	return rc;
}


static eEsifError EsifEventMgr_DisableEvent(
	EventMgrEntryPtr entryPtr
	)
{
	eEsifError rc = ESIF_OK;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	ESIF_ASSERT(entryPtr != NULL);

	if (!EsifUpPm_IsPrimaryParticipantId(entryPtr->participantId) &&
		(entryPtr->participantId != EVENT_MGR_MATCH_ANY)) {
		goto exit;
	}

	switch(entryPtr->fpcEvent.esif_group)
	{
	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("  Disabling Power Event: %s\n", esif_guid_print((esif_guid_t *)&entryPtr->fpcEvent.event_key, guidStr));
		rc = unregister_power_notification((esif_guid_t *)&entryPtr->fpcEvent.event_key);
		break;

	case ESIF_EVENT_GROUP_SYSTEM_METRICS:
		rc = unregister_system_metrics_notification((esif_guid_t *)&entryPtr->fpcEvent.event_key);
		break;

	case ESIF_EVENT_GROUP_SENSOR:
		ESIF_TRACE_DEBUG("Disabling CEM sensor events\n");
		rc = esif_unregister_sensors(entryPtr->fpcEvent.esif_event);
		break;

	case ESIF_EVENT_GROUP_CODE:
		rc = esif_disable_code_event(entryPtr->fpcEvent.esif_event);
		break;

	case ESIF_EVENT_GROUP_DPTF:
	case ESIF_EVENT_GROUP_ACPI:
	default:
		break;
	}
exit:
	return rc;
}


Bool EsifEventMgr_IsEventRegistered(
	eEsifEventType eventType,
	esif_context_t key,
	esif_handle_t participantId,
	UInt16 domainId
	)
{
	Bool bRet = ESIF_FALSE;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr entryPtr = NULL;

	esif_ccb_read_lock(&g_EsifEventMgr.listLock);
	listPtr = g_EsifEventMgr.observerLists[eventType % NUM_EVENT_LISTS];
	if (NULL == listPtr) {
		goto exit;
	}
	nodePtr = listPtr->head_ptr;
	while(NULL != nodePtr) {
		entryPtr = (EventMgrEntryPtr)nodePtr->data_ptr;
		ESIF_ASSERT(entryPtr != NULL);

		if((eventType == entryPtr->fpcEvent.esif_event) &&
			(entryPtr->context == key) && 
			((entryPtr->participantId == participantId) || (entryPtr->participantId == EVENT_MGR_MATCH_ANY) || (entryPtr->isParticipant0Id && EsifUpPm_IsPrimaryParticipantId(participantId))) &&
			((entryPtr->domainId == domainId) || (entryPtr->domainId == EVENT_MGR_MATCH_ANY_DOMAIN) || (domainId == EVENT_MGR_DOMAIN_NA)) &&
			(entryPtr->refCount > 0)) {

			bRet = ESIF_TRUE;
			break;
		}

		nodePtr = nodePtr->next_ptr;
	}
exit:
	esif_ccb_read_unlock(&g_EsifEventMgr.listLock);
	return bRet;
}


/* Used with EsifEventMgr_GetNextEvent to iterate through the events present
* in the Event Manager
* Note(s):
* 1) This iteration is based on the event types and the node number for the
* linked list for that event type.  This is due to the fact that items may
* be removed, so using event entry pointers for iteration is not possible
* without reference counting, which would require added complexity which
* is not required for the targeted usage (displaying current registered events.)
* 2) There is no guarantee that all events present at the start of iteration
* will be part of the iteration if events are removed during the iteration
* 3) There is no guarantee that events added after the start of iteration
* will be part of the iteration
*/
esif_error_t EsifEventMgr_InitIterator(
	UfEventIteratorPtr iterPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	
	if (iterPtr) {
		esif_ccb_memset(iterPtr, 0, sizeof(*iterPtr));
		iterPtr->marker = EVENT_MGR_ITERATOR_MARKER;
		rc = ESIF_OK;
	}

	return rc;
}

/* Use EsifEventMgr_InitIterator to initialize an iterator prior to use */
esif_error_t EsifEventMgr_GetNextEvent(
	UfEventIteratorPtr iterPtr,
	EventMgr_IteratorDataPtr dataPtr
	)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	eEsifEventType eventType = 0;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr entryPtr = NULL;
	size_t i = 0;

	if (iterPtr && dataPtr) {

		if (iterPtr->marker != EVENT_MGR_ITERATOR_MARKER) {
			rc = ESIF_E_NOT_INITIALIZED;
			goto exit;
		}

		eventType = iterPtr->eventType;

		esif_ccb_write_lock(&g_EsifEventMgr.listLock);

		while (eventType <= MAX_ESIF_EVENT_ENUM_VALUE) {

			listPtr = g_EsifEventMgr.observerLists[(unsigned)eventType % NUM_EVENT_LISTS];
			if (NULL == listPtr) {
				rc = ESIF_E_UNSPECIFIED;
				goto lockExit;
			}

			/* Get next node based on the item number in the linked list using the index from the iterator */
			i = 0;
			nodePtr = listPtr->head_ptr;
			while (nodePtr && (i < iterPtr->index)) {
				nodePtr = nodePtr->next_ptr;
				i++;
			}

			/* Return the data if node is found and update iterator */
			if (nodePtr && (i == iterPtr->index)) {

				while (nodePtr != NULL) {

					entryPtr = (EventMgrEntryPtr)nodePtr->data_ptr;
					if (!entryPtr) {
						rc = ESIF_E_UNSPECIFIED;
						goto lockExit;
					}

					if (eventType == entryPtr->fpcEvent.esif_event) {
						dataPtr->eventType = eventType;
						dataPtr->participantId = entryPtr->participantId;
						dataPtr->domainId = entryPtr->domainId;
						dataPtr->callback = entryPtr->callback;
						dataPtr->context = entryPtr->context;

						iterPtr->eventType = eventType;
						iterPtr->index = ++i;

						rc = ESIF_OK;
						goto lockExit;
					}
					nodePtr = nodePtr->next_ptr; 
					i++;
				}
			}

			/* If iteration done for current event type; move to next event type */
			iterPtr->index = 0;
			eventType++;
		}
		rc = ESIF_E_ITERATION_DONE;

lockExit:
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
	}
exit:
	return rc;
}


esif_error_t EsifEventMgr_UnregisterRemovedApps()
{
	esif_error_t rc = ESIF_OK;
	EsifLinkListNodePtr curNodePtr = NULL;
	EsifEventMgr_AppRemovalEntry *curEntryPtr = NULL;

	if (NULL == g_EsifEventMgr.appUnregisterList) {
		rc = ESIF_E_NOT_INITIALIZED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Unregistering removed apps\n");

	esif_ccb_write_lock(&g_EsifEventMgr.appUnregisterListLock);

	curNodePtr = g_EsifEventMgr.appUnregisterList->head_ptr;
	while (curNodePtr) {
		curEntryPtr = (EsifEventMgr_AppRemovalEntry *)curNodePtr->data_ptr;
		esif_link_list_node_remove(g_EsifEventMgr.appUnregisterList, curNodePtr);

		if (curEntryPtr) {
			ESIF_TRACE_DEBUG("Unregistering all events for app " ESIF_HANDLE_FMT "\n", curEntryPtr->context);

			esif_ccb_write_unlock(&g_EsifEventMgr.appUnregisterListLock);
			EsifEventMgr_UnregisterApp(curEntryPtr->eventCallback, curEntryPtr->context);
			esif_ccb_write_lock(&g_EsifEventMgr.appUnregisterListLock);

			esif_ccb_free(curEntryPtr);
		}

		curNodePtr = g_EsifEventMgr.appUnregisterList->head_ptr;
	}
	esif_ccb_write_unlock(&g_EsifEventMgr.appUnregisterListLock);
exit:
	ESIF_TRACE_DEBUG("Exit code = %d\n", rc);
	return rc;
}

static esif_error_t ESIF_CALLCONV EsifEventMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
)
{
	esif_error_t rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(domainId);
	UNREFERENCED_PARAMETER(eventDataPtr);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Only handle at the app level
	if (!EsifUpPm_IsPrimaryParticipantId(participantId)) {
		goto exit;
	}

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_DISPLAY_ON:
	case ESIF_EVENT_WINDOWS_LOW_POWER_MODE_EXIT:
		g_EsifEventMgr.delayAppUnregistration = ESIF_FALSE;
		EsifEventMgr_UnregisterRemovedApps();
		break;
	case ESIF_EVENT_DISPLAY_OFF:
	case ESIF_EVENT_WINDOWS_LOW_POWER_MODE_ENTRY:
		g_EsifEventMgr.delayAppUnregistration = ESIF_TRUE;
		break;
	default:
		rc = ESIF_E_NOT_SUPPORTED;
		break;
	}
exit:
	return rc;
}


static esif_error_t EsifEventMgr_RegisterEvents(void)
{
	esif_error_t rc = ESIF_OK;

	rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_DISPLAY_OFF, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_DISPLAY_ON, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	}
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_WINDOWS_LOW_POWER_MODE_ENTRY, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	}
	if (ESIF_OK == rc) {
		rc = EsifEventMgr_RegisterEventByType(ESIF_EVENT_WINDOWS_LOW_POWER_MODE_EXIT, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	}
	if (rc != ESIF_OK) {
		g_EsifEventMgr.delayedAppUnregistrationEnabled = ESIF_FALSE;
	}

	return rc;
}


static void EsifEventMgr_UnregisterEvents(void)
{
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DISPLAY_OFF, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DISPLAY_ON, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_WINDOWS_LOW_POWER_MODE_ENTRY, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_WINDOWS_LOW_POWER_MODE_EXIT, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, EsifEventMgr_EventCallback, 0);
	
	return;
}


eEsifError EsifEventMgr_Init(void)
{
	eEsifError rc = ESIF_OK;
	UInt8 i;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_EsifEventMgr.listLock);
	esif_ccb_lock_init(&g_EsifEventMgr.appUnregisterListLock);

	for (i = 0; i < NUM_EVENT_LISTS; i++) {
		g_EsifEventMgr.observerLists[i] = esif_link_list_create();
		if (NULL == g_EsifEventMgr.observerLists[i]) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	}

	g_EsifEventMgr.eventQueuePtr = esif_queue_create(ESIF_UF_EVENT_QUEUE_SIZE, ESIF_UF_EVENT_QUEUE_NAME, ESIF_UF_EVENT_QUEUE_TIMEOUT);
	g_EsifEventMgr.garbageList = esif_link_list_create();
	g_EsifEventMgr.appUnregisterList = esif_link_list_create();

	if ((NULL == g_EsifEventMgr.eventQueuePtr) ||
		(NULL == g_EsifEventMgr.garbageList) ||
		(NULL == g_EsifEventMgr.appUnregisterList)) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	g_EsifEventMgr.delayedAppUnregistrationEnabled = ESIF_TRUE;
	g_EsifEventMgr.delayAppUnregistration = ESIF_FALSE;

	rc = esif_ccb_thread_create(&g_EsifEventMgr.eventQueueThread, EsifEventMgr_EventQueueThread, NULL);
	if (rc != ESIF_OK) {
		goto exit;
	}

	g_EsifEventMgr.cacheableEventListPtr = (Bool *)esif_ccb_malloc(MAX_ESIF_EVENT_ENUM_VALUE * sizeof(*g_EsifEventMgr.cacheableEventListPtr));
	if (NULL == g_EsifEventMgr.cacheableEventListPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	for (i = 0; i < MAX_ESIF_EVENT_ENUM_VALUE; i++) {
		g_EsifEventMgr.cacheableEventListPtr[i] = EsifEventCache_IsEventCacheable(i);
	}
exit:
	if (rc != ESIF_OK) {
		EsifEventMgr_Exit();
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


esif_error_t EsifEventMgr_Start(void)
{
	return 	EsifEventMgr_RegisterEvents();  /* Register for events after participants are available */
}


void EsifEventMgr_Exit(void)
{
	UInt8 i;
	EsifLinkListPtr listPtr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	if (!g_EsifEventMgr.eventsDisabled) {
		EsifEventMgr_Disable();
	}

	/* Stop accepting event manager events and then unregister any delayed apps */
	EsifEventMgr_UnregisterEvents();
	g_EsifEventMgr.delayAppUnregistration = ESIF_FALSE;
	EsifEventMgr_UnregisterRemovedApps();

	/* Remove all listeners */
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

	for (i = 0; i < NUM_EVENT_LISTS; i++) {
		listPtr = g_EsifEventMgr.observerLists[i];
		esif_link_list_free_data_and_destroy(listPtr, EsifEventMgr_LLEntryDestroyCallback);
		g_EsifEventMgr.observerLists[i] = NULL;
	}

	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	/* Destroy the event thread */

	/* Event thread should already be destroyed in the disable func. Destroy the queue */
	esif_queue_destroy(g_EsifEventMgr.eventQueuePtr, EsifEventMgr_QueueDestroyCallback);
	g_EsifEventMgr.eventQueuePtr = NULL;

	/* Release the cacheable event list */
	esif_ccb_free(g_EsifEventMgr.cacheableEventListPtr);
	g_EsifEventMgr.cacheableEventListPtr = NULL;

	/*Destroy the delayed app unregistration list */
	esif_ccb_write_lock(&g_EsifEventMgr.appUnregisterListLock);
	esif_link_list_free_data_and_destroy(g_EsifEventMgr.appUnregisterList, NULL);
	g_EsifEventMgr.appUnregisterList = NULL;
	esif_ccb_write_unlock(&g_EsifEventMgr.appUnregisterListLock);

	/* Destroy the garbage list */
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	esif_link_list_free_data_and_destroy(g_EsifEventMgr.garbageList, EsifEventMgr_LLEntryDestroyCallback);
	g_EsifEventMgr.garbageList = NULL;
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	esif_ccb_lock_uninit(&g_EsifEventMgr.listLock);
	esif_ccb_lock_uninit(&g_EsifEventMgr.appUnregisterListLock);
	

	ESIF_TRACE_EXIT_INFO();
}

void EsifEventMgr_Disable(void)
{
	ESIF_TRACE_ENTRY_INFO();

	/* Release and destroy the event thread */
	g_EsifEventMgr.eventQueueExitFlag = ESIF_TRUE;
	esif_queue_signal_event(g_EsifEventMgr.eventQueuePtr);
	esif_ccb_thread_join(&g_EsifEventMgr.eventQueueThread);
	g_EsifEventMgr.eventsDisabled = ESIF_TRUE;

	ESIF_TRACE_EXIT_INFO();
}

/* Write lock should be held when called */
static eEsifError EsifEventMgr_MoveEntryToGarbage(EventMgrEntryPtr entryPtr)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = g_EsifEventMgr.garbageList;

	ESIF_ASSERT(NULL != entryPtr);

	if (NULL == listPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	rc = esif_link_list_add_at_back(listPtr, (void *)entryPtr);
exit:
	return rc;
}


static eEsifError EsifEventMgr_DumpGarbage()
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = g_EsifEventMgr.garbageList;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr entryPtr = NULL;

	if (NULL == listPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	nodePtr = listPtr->head_ptr;
	while(nodePtr) {
		entryPtr = nodePtr->data_ptr;

		/* remove the node first so that it isn't considered active while we disable events */
		esif_link_list_node_remove(listPtr, nodePtr); 
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		EsifEventMgr_DisableEvent(entryPtr);
		esif_ccb_free(entryPtr);
		esif_ccb_write_lock(&g_EsifEventMgr.listLock);

		nodePtr = listPtr->head_ptr;
	}
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

exit:
	return rc;
}


static void EsifEventMgr_QueueDestroyCallback(void *ctxPtr)
{
	EsifEventQueueItemPtr queueEventPtr = (EsifEventQueueItemPtr)ctxPtr;

	if(queueEventPtr != NULL) {
		esif_ccb_free(queueEventPtr->eventData.buf_ptr);
		esif_ccb_free(queueEventPtr);
	}
}


static void EsifEventMgr_LLEntryDestroyCallback(
	void *dataPtr
	)
{
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
	EsifEventMgr_DisableEvent((EventMgrEntryPtr)dataPtr);
	esif_ccb_free(dataPtr);
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
}


eEsifError HandlePackagedEvent(
	EsifEventParamsPtr eventParamsPtr,
	size_t dataLen
	)
{
	eEsifError rc = ESIF_OK;
	struct esif_data esifDataPacket = { ESIF_DATA_VOID };
	UInt8 *dataPtr = NULL;

	if (NULL == eventParamsPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (dataLen < sizeof(*eventParamsPtr)) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	if (dataLen < (sizeof(*eventParamsPtr) + eventParamsPtr->dataLen)) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	dataPtr = (UInt8 *)(eventParamsPtr + 1);

	esifDataPacket.buf_ptr = dataPtr;
	esifDataPacket.type = eventParamsPtr->dataType;
	esifDataPacket.buf_len = eventParamsPtr->dataLen;
	esifDataPacket.data_len = eventParamsPtr->dataLen;

	EsifEventMgr_SignalEvent(eventParamsPtr->participantId, eventParamsPtr->domainId, eventParamsPtr->eventType, &esifDataPacket);
exit:
	return rc;
}


esif_error_t EsifEventMgr_FilterEventType(eEsifEventType eventType)
{
	esif_error_t rc = ESIF_E_EVENT_NOT_FOUND;
	size_t line = 0;
	UInt8 bit = 0;

	if ((eventType >= 0) && (eventType <= MAX_ESIF_EVENT_ENUM_VALUE)) {

		line = eventType / EVENT_MGR_FILTERED_EVENTS_PER_LINE;
		bit = eventType % EVENT_MGR_FILTERED_EVENTS_PER_LINE;

		esif_ccb_write_lock(&g_EsifEventMgr.listLock);
		g_EsifEventMgr.filteredEvents[line] |= (UInt64)1 << bit;
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

		rc = ESIF_OK;
	}
	return rc;
}


esif_error_t EsifEventMgr_UnfilterEventType(eEsifEventType eventType)
{
	esif_error_t rc = ESIF_E_EVENT_NOT_FOUND;
	size_t line = 0;
	UInt8 bit = 0;

	if ((eventType >= 0) && (eventType <= MAX_ESIF_EVENT_ENUM_VALUE)) {
		line = eventType / EVENT_MGR_FILTERED_EVENTS_PER_LINE;
		bit = eventType % EVENT_MGR_FILTERED_EVENTS_PER_LINE;

		esif_ccb_write_lock(&g_EsifEventMgr.listLock);
		g_EsifEventMgr.filteredEvents[line] &= ~((UInt64)1 << bit);
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

		rc = ESIF_OK;
	}
	return rc;
}


esif_error_t EsifEventMgr_UnfilterAllEventTypes()
{
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	esif_ccb_memset(g_EsifEventMgr.filteredEvents, 0, sizeof(g_EsifEventMgr.filteredEvents));
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	return ESIF_OK;
}


static Bool EsifEventMgr_IsEventFiltered(eEsifEventType eventType)
{
	Bool bRet = ESIF_FALSE;
	size_t line = 0;
	UInt8 bit = 0;

	//
	// Do not filter events beyond what we know as this may allow for
	// OEM-defined event types in the future.
	//
	if ((eventType >= 0) && (eventType <= MAX_ESIF_EVENT_ENUM_VALUE)) {
		line = eventType / EVENT_MGR_FILTERED_EVENTS_PER_LINE;
		bit = eventType % EVENT_MGR_FILTERED_EVENTS_PER_LINE;

		bRet = g_EsifEventMgr.filteredEvents[line] & ((UInt64)1 << bit) ? ESIF_TRUE : ESIF_FALSE;
	}

	return bRet;
}


static Bool EsifEventMgr_IsEventCacheable(
	eEsifEventType eventType
	)
{
	Bool isCacheable = ESIF_FALSE;

	ESIF_ASSERT(g_EsifEventMgr.cacheableEventListPtr);

	if (eventType < MAX_ESIF_EVENT_ENUM_VALUE) {
		isCacheable = g_EsifEventMgr.cacheableEventListPtr[eventType];
	}

	return isCacheable;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
