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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_uf.h"	/* Upper Framework */
#include "esif_uf_appmgr.h"
#include "esif_link_list.h"
#include "esif_event.h"
#include "esif_ccb_atomic.h"
#include "esif_uf_eventmgr.h"
#include "esif_queue.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 * All event received are asynchronous and placed in an event queue to be handled by a worker thread.
 *
 * The manager maintains information on registered event "observers"
 * Interface:
 *   EsifEventMgr_SignalEvent
 *   EsifEventMgr_RegisterEventByType
 *   EsifEventMgr_UnregisterEventByType
 *   EsifEventMgr_RegisterEventByGuid
 *   EsifEventMgr_UnregisterEventByGuid
 *
 * Event observer information is maintained as an array of linked lists with a list of observers the event types
 * There are 64 event lists (Information is placed in a given list given by the modulo 64 of the event type)
 * Event observers may register based on the event type or GUID.
 * EVENT_MGR_MATCH_ANY may be used as the participant ID during registration to observe events from all participants;
 * or if registration takes place before the participants are present.
 * Locks are released before any calls outside the event manager which may result in obtaining other locks;
 * locks re-acquired upon return.
 * A reference count is kept for each observer; events are only sent to observers with a positive reference count
 * When the reference count reaches 0, the node is garbage collected
 * Before an observer is called, the reference count is incremented so that the node is not removed while in use;
 * the reference count is decrement upon return and the node is garbage collected if the reference count is then 0.
 * A single garbage collection linked list is maintained.  Any garbage nodes are moved to that list for destruction.
 * Any steps required to enable/disable an event, for example DPPE, will be performed during creation/destruction.
 */
static EsifEventMgr g_EsifEventMgr = {0};



static eEsifError EsifEventMgr_AddEntry(
	EsifFpcEventPtr fpcEventPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

static eEsifError EsifEventMgr_ReleaseEntry(
	EsifFpcEventPtr fpcEventPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

static eEsifError EsifEventMgr_ProcessEvent(
	UInt8 participantId,
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

eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent(
	UInt8 participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifEventQueueItemPtr queueEventPtr = NULL;
	EsifDataPtr queueDataPtr = NULL;

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
	EsifEventQueueItemPtr queueEventPtr = NULL;

	UNREFERENCED_PARAMETER(ctxPtr);

	while(!g_EsifEventMgr.eventQueueExitFlag) {
		queueEventPtr = esif_queue_pull(g_EsifEventMgr.eventQueuePtr);

		if (NULL == queueEventPtr) {
			continue;
		}

		ESIF_TRACE_INFO("Dequeuing %s event for Part. %u Dom. 0x%04X\n",
			esif_event_type_str(queueEventPtr->eventType),
			queueEventPtr->participantId,
			queueEventPtr->domainId);

		EsifEventMgr_ProcessEvent(queueEventPtr->participantId,
			queueEventPtr->domainId,
			queueEventPtr->eventType,
			&queueEventPtr->eventData);

		esif_ccb_free(queueEventPtr->eventData.buf_ptr);
		esif_ccb_free(queueEventPtr);
	}
	return 0;
}


static eEsifError EsifEventMgr_ProcessEvent(
	UInt8 participantId,
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
			"  ParticipantID: %u\n"
			"  Domain:        %s(%04X)\n"
			"  EventType:     %s(%d)\n",
			participantId,
			esif_primitive_domain_str(domainId, domain_str, sizeof(domain_str)),
			domainId,
			esif_event_type_str(eventType), eventType);
	} else {
		ESIF_TRACE_DEBUG("APPLICATION_EVENT\n"
						 "  ParticipantID: %u\n"
						 "  Domain:        %s(%04X)\n"
						 "  EventType:     %s(%d)\n"
						 "  EventDataType: %s(%d)\n"
						 "  EventData:     %p\n"
						 "    buf_ptr      %p\n"
						 "    buf_len      %d\n"
						 "    data_len     %d\n",
						 participantId,
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

	esif_ccb_write_lock(&g_EsifEventMgr.listLock);

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
			((entryPtr->participantId == participantId) || (entryPtr->participantId == EVENT_MGR_MATCH_ANY)) &&
			((entryPtr->domainId == domainId) || (entryPtr->domainId == EVENT_MGR_MATCH_ANY) || (domainId == EVENT_MGR_DOMAIN_NA)) &&
			(entryPtr->refCount > 0)) {

			/*
			 * Increment the reference count so that the node is not removed while in use,
			 * then release the lock so that we avoid a deadlock condition
			 */
			atomic_inc(&entryPtr->refCount);
			esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

			entryPtr->callback(entryPtr->contextPtr,
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

			refCount = atomic_dec(&entryPtr->refCount);
			if (refCount <= 0) {
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
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
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
		if (esif_event_map_type2guid(&phonyFpcEvent.event_guid, &eventType) == ESIF_FALSE) {
			rc = ESIF_E_EVENT_NOT_FOUND;
			goto exit;
		}
		phonyFpcEvent.esif_event = eventType;
		fpcEventPtr = &phonyFpcEvent;
	}

	ESIF_TRACE_DEBUG(
		"Registering event\n"
		"  Alias: %s\n"
		"  GUID: %s\n"
		"  Type: %s(%d)\n"
		"  Data: %s(%d)\n"
		"  Key: %s\n"
		"  Group: %s\n",
		fpcEventPtr->event_name,
		esif_guid_print((esif_guid_t *)fpcEventPtr->event_guid, guidStr),
		esif_event_type_str(eventType), eventType,
		esif_data_type_str(fpcEventPtr->esif_group_data_type), fpcEventPtr->esif_group_data_type,
		esif_guid_print((esif_guid_t *)fpcEventPtr->event_key, guidStr),
		esif_event_group_enum_str(fpcEventPtr->esif_group));

	rc = EsifEventMgr_AddEntry(
		fpcEventPtr,
		participantId,
		domainId,
		eventCallback,
		contextPtr);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByType(
	eEsifEventType eventType,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
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
		contextPtr);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_RegisterEventByGuid(
	esif_guid_t *guidPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifFpcEventPtr fpcEventPtr = NULL;
	EsifUpPtr upPtr = NULL;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	if((NULL == guidPtr) || (NULL == eventCallback)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Find Our Participant */
	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	/* Find the event associated with the participant */
	fpcEventPtr = EsifUp_GetFpcEventByGuid(upPtr, guidPtr);
	if (NULL == fpcEventPtr) {
		rc = ESIF_E_EVENT_NOT_FOUND;
		goto exit;
	}

	ESIF_TRACE_DEBUG(
		"Registering event\n"
		"  Alias: %s\n"
		"  GUID: %s\n"
		"  Type: %s(%d)\n"
		"  Data: %s(%d)\n"
		"  Key: %s\n"
		"  Group: %s\n",
		fpcEventPtr->event_name,
		esif_guid_print((esif_guid_t *)fpcEventPtr->event_guid, guidStr),
		esif_event_type_str(fpcEventPtr->esif_event), fpcEventPtr->esif_event,
		esif_data_type_str(fpcEventPtr->esif_group_data_type), fpcEventPtr->esif_group_data_type,
		esif_guid_print((esif_guid_t *)fpcEventPtr->event_key, guidStr),
		esif_event_group_enum_str(fpcEventPtr->esif_group));

	rc = EsifEventMgr_AddEntry(
		fpcEventPtr,
		participantId,
		domainId,
		eventCallback,
		contextPtr);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByGuid(
	esif_guid_t *guidPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifFpcEventPtr fpcEventPtr = NULL;
	EsifUpPtr upPtr = NULL;

	if((NULL == guidPtr) || (NULL == eventCallback)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Find Our Participant */
	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	/* Find the event associated with the participant */
	fpcEventPtr = EsifUp_GetFpcEventByGuid(upPtr, guidPtr);
	if (NULL == fpcEventPtr) {
		rc = ESIF_E_EVENT_NOT_FOUND;
		goto exit;
	}

	rc = EsifEventMgr_ReleaseEntry(
		fpcEventPtr,
		participantId,
		domainId,
		eventCallback,
		contextPtr);

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


static eEsifError EsifEventMgr_AddEntry(
	EsifFpcEventPtr fpcEventPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListPtr listPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EventMgrEntryPtr curEntryPtr = NULL;
	EventMgrEntryPtr newEntryPtr = NULL;
	atomic_t refCount = 1;

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
			(curEntryPtr->participantId == participantId) &&
			(curEntryPtr->domainId == domainId) &&
			(curEntryPtr->contextPtr == contextPtr) &&
			(curEntryPtr->callback == eventCallback)){
			break;
		}
		nodePtr = nodePtr->next_ptr;
	}
	/* If we found an existing entry, update the reference count */
	if (nodePtr != NULL) {
		atomic_inc(&curEntryPtr->refCount);
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
	newEntryPtr->contextPtr = contextPtr;
	newEntryPtr->domainId = domainId;
	newEntryPtr->participantId = participantId;
	newEntryPtr->refCount = refCount;
	esif_ccb_memcpy(&newEntryPtr->fpcEvent, fpcEventPtr, sizeof(newEntryPtr->fpcEvent));

	nodePtr = esif_link_list_create_node(newEntryPtr);
	if (NULL == nodePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	esif_link_list_add_node_at_back(listPtr, nodePtr);
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	rc = EsifEventMgr_EnableEvent(newEntryPtr);
	if (ESIF_OK != rc)
	{
		esif_ccb_write_lock(&g_EsifEventMgr.listLock);
		esif_link_list_node_remove(listPtr, nodePtr);
		esif_ccb_write_unlock(&g_EsifEventMgr.listLock);
		goto exit;
	}

exit:
	ESIF_TRACE_DEBUG("  RefCount: " ATOMIC_FMT "\n", refCount);

	if (ESIF_OK != rc) {
		esif_ccb_free(newEntryPtr);
	}

	return rc;
}


static eEsifError EsifEventMgr_ReleaseEntry(
	EsifFpcEventPtr fpcEventPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
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
			(curEntryPtr->contextPtr == contextPtr) &&
			(curEntryPtr->callback == eventCallback)){
			break;
		}
		nodePtr = nodePtr->next_ptr;
	}

	if (nodePtr != NULL) {
		refCount = atomic_dec(&curEntryPtr->refCount);
		if (refCount <= 0) {
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

	if (entryPtr->participantId != 0) {
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

	case ESIF_EVENT_GROUP_DPTF:
	case ESIF_EVENT_GROUP_ACPI:
	case ESIF_EVENT_GROUP_CODE:
	default:
		break;
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

	if (entryPtr->participantId != 0) {
		goto exit;
	}

	switch(entryPtr->fpcEvent.esif_group)
	{
	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("  Disabling Power Event: %s\n", esif_guid_print((esif_guid_t *)&entryPtr->fpcEvent.event_key, guidStr));
		rc = unregister_power_notification((esif_guid_t *)&entryPtr->fpcEvent.event_key);
		break;

	case ESIF_EVENT_GROUP_SENSOR:
		ESIF_TRACE_DEBUG("Disabling CEM sensor events\n");
		rc = esif_unregister_sensors(entryPtr->fpcEvent.esif_event);
		break;

	case ESIF_EVENT_GROUP_DPTF:
	case ESIF_EVENT_GROUP_ACPI:
	case ESIF_EVENT_GROUP_CODE:
	case ESIF_EVENT_GROUP_SYSTEM_METRICS:
	default:
		break;
	}
exit:
	return rc;
}


Bool EsifEventMgr_IsEventRegistered(
	eEsifEventType eventType,
	void *key,
	UInt8 participantId,
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
			(entryPtr->contextPtr == key) && 
			((entryPtr->participantId == participantId) || (entryPtr->participantId == EVENT_MGR_MATCH_ANY)) &&
			((entryPtr->domainId == domainId) || (entryPtr->domainId == EVENT_MGR_MATCH_ANY) || (domainId == EVENT_MGR_DOMAIN_NA)) &&
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


eEsifError EsifEventMgr_Init(void)
{
	eEsifError rc = ESIF_OK;
	UInt8 i;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_EsifEventMgr.listLock);

	for (i = 0; i < NUM_EVENT_LISTS; i++) {
		g_EsifEventMgr.observerLists[i] = esif_link_list_create();
		if (NULL == g_EsifEventMgr.observerLists[i]) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	}

	g_EsifEventMgr.eventQueuePtr = esif_queue_create(ESIF_UF_EVENT_QUEUE_SIZE, ESIF_UF_EVENT_QUEUE_NAME, ESIF_UF_EVENT_QUEUE_TIMEOUT);
	g_EsifEventMgr.garbageList = esif_link_list_create();

	if ((NULL == g_EsifEventMgr.eventQueuePtr) ||
		(NULL == g_EsifEventMgr.garbageList)) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = esif_ccb_thread_create(&g_EsifEventMgr.eventQueueThread, EsifEventMgr_EventQueueThread, NULL);
	if (rc != ESIF_OK) {
		goto exit;
	}
exit:
	if (rc != ESIF_OK) {
		EsifEventMgr_Exit();
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifEventMgr_Exit(void)
{
	UInt8 i;
	EsifLinkListPtr listPtr = NULL;

	ESIF_TRACE_ENTRY_INFO();

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


	/* Destroy the garbage list */
	esif_ccb_write_lock(&g_EsifEventMgr.listLock);
	esif_link_list_free_data_and_destroy(g_EsifEventMgr.garbageList, EsifEventMgr_LLEntryDestroyCallback);
	g_EsifEventMgr.garbageList = NULL;
	esif_ccb_write_unlock(&g_EsifEventMgr.listLock);

	esif_ccb_lock_uninit(&g_EsifEventMgr.listLock);

	ESIF_TRACE_EXIT_INFO();
}

void EsifEventMgr_Disable(void)
{
	ESIF_TRACE_ENTRY_INFO();

	/* Release and destroy the event thread */
	g_EsifEventMgr.eventQueueExitFlag = ESIF_TRUE;
	esif_queue_signal_event(g_EsifEventMgr.eventQueuePtr);
	esif_ccb_thread_join(&g_EsifEventMgr.eventQueueThread);

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


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
