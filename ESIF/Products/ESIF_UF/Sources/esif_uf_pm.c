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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PARTICIPANT

/* ESIF */
#include "esif_uf.h"		/* Upper Framework */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_dsp.h"		/* Device Support Package */
#include "esif_uf_eventmgr.h"
#include "esif_participant.h"


#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* This Module */
EsifUppMgr g_uppMgr = {0};

/*
 * econo-poll...different from domain polling
 * because it is condensed to one thread
 */
static atomic_t g_ufpollQuit = ATOMIC_INIT(1);
static int g_ufpollPeriod = ESIF_UFPOLL_PERIOD_DEFAULT;
static esif_thread_t g_ufpollThread;
static void EsifUfPollExit(esif_thread_t *ufpollThread);

/*
 * ===========================================================================
 * The followins functions are participant "friend" functions
 * ===========================================================================
 */
eEsifError EsifUp_CreateParticipant(
	const eEsifParticipantOrigin origin,
	UInt8 upInstance,
	const void *metadataPtr,
	EsifUpPtr *upPtr
	);

void EsifUp_DestroyParticipant(
	EsifUpPtr self
	);

eEsifError EsifUp_DspReadyInit(
	EsifUpPtr self
	);

eEsifError EsifUp_ReInitializeParticipant(
	EsifUpPtr self,
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	);

eEsifError EsifUp_SuspendParticipant(
	EsifUpPtr self
	);

eEsifError EsifUp_ResumeParticipant(
	EsifUpPtr self
	); 

void EsifUp_RegisterParticipantForPolling(
	EsifUpPtr self
	);

void EsifUp_UnRegisterParticipantForPolling(
	EsifUpPtr self
	);

void EsifUp_PollParticipant(
	EsifUpPtr self
	);


/*
 * ===========================================================================
 * PRIVATE
 * ===========================================================================
 */
static EsifUpManagerEntryPtr EsifUpPm_GetParticipantEntryFromMetadata(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	);

static eEsifError EsifUpPm_DestroyParticipants(void);

static eEsifError ESIF_CALLCONV EsifUpPm_EventCallback(
	void *contextPtr,
	UInt8 upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

static void *ESIF_CALLCONV EsifUfPollWorkerThread(void *ptr)
{
	UInt8 i = 0;
	
	UNREFERENCED_PARAMETER(ptr);

	atomic_set(&g_ufpollQuit, 0);

	CMD_OUT("Starting Upper Framework Polling... \n");

	/* check temperature */
	while (!atomic_read(&g_ufpollQuit)) {
		
		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);
			
			if (NULL == upPtr) {
				continue;
			}
			
			EsifUp_PollParticipant(upPtr);
			EsifUp_PutRef(upPtr);
		}
		esif_ccb_sleep_msec(g_ufpollPeriod);
	}

	return 0;
}


static void EsifUfPollExit(esif_thread_t *ufpollThread)
{
	CMD_OUT("Stopping Upper Framework Polling...\n");
	atomic_set(&g_ufpollQuit, 1);
	esif_ccb_thread_join(ufpollThread);
	CMD_OUT("Upper Framework Polling Stopped\n");
}

/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/

/* Add participant in participant manager */
eEsifError EsifUpPm_RegisterParticipant(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr,
	UInt8 *upInstancePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	EsifUpManagerEntryPtr entryPtr = NULL;
	UInt8 i = 0;
	Bool isUppMgrLocked = ESIF_FALSE;

	/* Validate parameters */
	if ((NULL == metadataPtr) || (NULL == upInstancePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*upInstancePtr = ESIF_INSTANCE_INVALID;
	
	/*
	 * Check if a participant has already been created, but was then removed.
	 * In that case, just re-enable the participant.
	 */
	entryPtr = EsifUpPm_GetParticipantEntryFromMetadata(origin, metadataPtr);
	
	if (NULL != entryPtr) {
		/* Lock manager */
		esif_ccb_write_lock(&g_uppMgr.fLock);
		isUppMgrLocked = ESIF_TRUE;

		if (entryPtr->fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) {
			goto exit;
		}

		upPtr = entryPtr->fUpPtr;
		ESIF_ASSERT(upPtr != NULL);

		rc = EsifUp_GetRef(upPtr);
		if (ESIF_OK != rc) {
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		rc = EsifUp_ReInitializeParticipant(upPtr, origin, metadataPtr);

		if (ESIF_OK != rc) {
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		*upInstancePtr = EsifUp_GetInstance(upPtr);

		entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
		g_uppMgr.fEntryCount++;
		esif_ccb_write_unlock(&g_uppMgr.fLock);
		isUppMgrLocked = ESIF_FALSE;
	}
	else {
		esif_ccb_write_lock(&g_uppMgr.fLock);
		isUppMgrLocked = ESIF_TRUE;

		/*
		 *  Find available slot in participant manager table.  Simple Table Lookup For Now.
		 *  Scan table and find first empty slot.  Empty slot indicated by AVAILABLE state.
		 */
		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			if (ESIF_PM_PARTICIPANT_STATE_AVAILABLE == g_uppMgr.fEntries[i].fState) {
				break;
			}
		}

		/* If no available slots return */
		if (i >= MAX_PARTICIPANT_ENTRY) {
			ESIF_TRACE_ERROR("No available slot in participant manager\n");
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		rc = EsifUp_CreateParticipant(origin, i, metadataPtr, &upPtr);
		if ((rc != ESIF_OK) || (upPtr == NULL)) {
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		rc = EsifUp_GetRef(upPtr);
		if (rc != ESIF_OK) {
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		g_uppMgr.fEntries[i].fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
		g_uppMgr.fEntries[i].fUpPtr = upPtr;
		g_uppMgr.fEntryCount++;

		*upInstancePtr = i;

		esif_ccb_write_unlock(&g_uppMgr.fLock);
		isUppMgrLocked = ESIF_FALSE;
	}

	/* Perform initialization that requires primitive support */
	EsifUp_DspReadyInit(upPtr);

	/* Now offer this participant to each running application */
	rc = EsifAppMgrCreateParticipantInAllApps(upPtr);

exit:
	if (isUppMgrLocked == ESIF_TRUE) {
		/* Unlock manager */
		esif_ccb_write_unlock(&g_uppMgr.fLock);
	}

	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	return rc;
}


static eEsifError ESIF_CALLCONV EsifUpPm_EventCallback(
	void *contextPtr,
	UInt8 upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	struct esif_ipc_event_data_create_participant *creationDataPtr = NULL;
	UInt8 newInstance = ESIF_INSTANCE_INVALID;

	UNREFERENCED_PARAMETER(contextPtr);
	UNREFERENCED_PARAMETER(domainId);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	switch(fpcEventPtr->esif_event){
	case ESIF_EVENT_PARTICIPANT_CREATE:

		if (NULL == eventDataPtr) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		if (eventDataPtr->data_len < sizeof(*creationDataPtr)) {
			rc = 	ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		creationDataPtr = (struct esif_ipc_event_data_create_participant *) eventDataPtr->buf_ptr;

		if (EsifUpPm_DoesAvailableParticipantExistByName(creationDataPtr->name)) {
			ESIF_TRACE_WARN("Participant %s has already existed in UF\n", creationDataPtr->name);
			goto exit;
		}

		rc = EsifUpPm_RegisterParticipant(eParticipantOriginLF, creationDataPtr, &newInstance);
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Fail to add participant %s in participant manager\n", creationDataPtr->name);
			goto exit;
		}

		ESIF_TRACE_DEBUG("\nCreate new UF participant: %s, instance = %d\n", creationDataPtr->name, newInstance);
		break;

	case ESIF_EVENT_PARTICIPANT_SUSPEND:
		if (upInstance != ESIF_INSTANCE_LF) {
			ESIF_TRACE_INFO("Suspending Participant: %d\n", upInstance);
			rc = EsifUpPm_UnregisterParticipant(eParticipantOriginLF, upInstance);
		}
		break;

	case ESIF_EVENT_PARTICIPANT_RESUME:
		if (upInstance != ESIF_INSTANCE_LF) {
			ESIF_TRACE_INFO("Reregistering Participant: %d\n", upInstance);
			rc = EsifUpPm_ResumeParticipant(upInstance);
		}
		break;

	case ESIF_EVENT_PARTICIPANT_UNREGISTER: 
			ESIF_TRACE_INFO("Unregistering Participant: %d\n", upInstance);
			rc = EsifUpPm_UnregisterParticipant(eParticipantOriginLF, upInstance);
		break;

	default:
		break;
	}

exit:
	return rc;
}

eEsifError EsifUFPollStart(int pollInterval)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	if (pollInterval >= ESIF_UFPOLL_PERIOD_MIN) {
		g_ufpollPeriod = pollInterval;
	}

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);
		
		if (NULL == upPtr) {
			continue;
		}
		
		EsifUp_RegisterParticipantForPolling(upPtr);

		EsifUp_PutRef(upPtr);
	}

	if (!EsifUFPollStarted()) {
		rc = esif_ccb_thread_create(&g_ufpollThread, EsifUfPollWorkerThread, NULL);
	}
	return rc;
}

void EsifUFPollStop()
{
	if (EsifUFPollStarted()) {
		UInt8 i = 0;
		
		EsifUfPollExit(&g_ufpollThread);

		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);
			
			if (NULL == upPtr) {
				continue;
			}

			EsifUp_UnRegisterParticipantForPolling(upPtr);

			EsifUp_PutRef(upPtr);
		}
	}
}

Bool EsifUFPollStarted()
{
	return ((Bool)atomic_read(&g_ufpollQuit) == 0);
}

/* Unregister Upper Participant Instance */
eEsifError EsifUpPm_UnregisterParticipant(
	const eEsifParticipantOrigin origin,
	const UInt8 upInstance
	)
{
	eEsifError rc    = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;

	UNREFERENCED_PARAMETER(origin);

	if (upInstance >= MAX_PARTICIPANT_ENTRY) {
		ESIF_TRACE_ERROR("Instance id %d is out of range\n", upInstance);
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entryPtr = &g_uppMgr.fEntries[upInstance];
	upPtr = entryPtr->fUpPtr;
	if ((NULL != upPtr) && (entryPtr->fState > ESIF_PM_PARTICIPANT_STATE_REMOVED)) {

		EsifUp_SuspendParticipant(upPtr);

		entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_REMOVED;
		g_uppMgr.fEntryCount--;

	} else {
		upPtr = NULL;
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);

	if (NULL != upPtr) {
		rc = EsifAppMgrDestroyParticipantInAllApps(upPtr);
	}

	ESIF_TRACE_INFO("Unregistered participant, instant id = %d\n", upInstance);
exit:
	return rc;
}


/* Resume upper participant instance that existed previously */
eEsifError EsifUpPm_ResumeParticipant(
	const UInt8 upInstance
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;

	/* Should never have to re-register Participant 0 */
	if (0 == upInstance) {
		goto exit;
	}

	if (upInstance >= MAX_PARTICIPANT_ENTRY) {
		ESIF_TRACE_ERROR("Instance id %d is out of range\n", upInstance);
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entryPtr = &g_uppMgr.fEntries[upInstance];
	upPtr = entryPtr->fUpPtr;
	if ((NULL != upPtr) && (entryPtr->fState < ESIF_PM_PARTICIPANT_STATE_CREATED)) {
		entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
		g_uppMgr.fEntryCount++;

		/*
			* Get reference on participant before pass it to other function
			* Make sure the participant is not destroyed before the function returns
			*/
		rc = EsifUp_GetRef(upPtr);
		if (rc != ESIF_OK) {
			upPtr = NULL;
		}
	}
	else {
		upPtr = NULL;
	}
	esif_ccb_write_unlock(&g_uppMgr.fLock);

	if (NULL != upPtr) {
		EsifUp_ResumeParticipant(upPtr);
		rc = EsifAppMgrCreateParticipantInAllApps(upPtr);
	}

	ESIF_TRACE_INFO("Reregistered participant, instant id = %d\n", upInstance);
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


/*
 * Get By Instance From ID
 * the caller should call EsifUp_PutRef to release reference on participant when done with it
 */
EsifUpPtr EsifUpPm_GetAvailableParticipantByInstance(
	const UInt8 upInstance
	)
{
	EsifUpPtr upPtr = NULL;
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("Instance %d\n", upInstance);

	if (upInstance >= MAX_PARTICIPANT_ENTRY) {
		ESIF_TRACE_ERROR("Instance id %d is out of range\n", upInstance);
		ESIF_ASSERT(ESIF_FALSE);
		goto exit;
	}

	/* Lock manager */
	esif_ccb_read_lock(&g_uppMgr.fLock);

	if (g_uppMgr.fEntries[upInstance].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) {
		upPtr = g_uppMgr.fEntries[upInstance].fUpPtr;
		if (upPtr != NULL) {
			rc = EsifUp_GetRef(upPtr);
			if (rc != ESIF_OK) {
				ESIF_TRACE_INFO("Unable to acquire reference on participant\n");
				upPtr = NULL;
			}
		}
	}
	/* Unlock Manager */
	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	return upPtr;
}

/* Check if a participant already exists by the HID */
Bool EsifUpPm_DoesAvailableParticipantExistByHID(
	char *participantHID
	)
{
	Bool bRet = ESIF_FALSE;
	EsifUpPtr upPtr = NULL;
	EsifUpDataPtr metaPtr = NULL;
	UInt8 i;

	if (NULL == participantHID) {
		ESIF_TRACE_ERROR("The participant HID pointer is NULL\n");
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);

		if (NULL == upPtr) {
			continue;
		}

		metaPtr = EsifUp_GetMetadata(upPtr);
		if (NULL == metaPtr) {
			continue;
		}
		if ((g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) && !esif_ccb_strcmp(metaPtr->fAcpiDevice, participantHID)) {
			bRet = ESIF_TRUE;
			break;
		}

		EsifUp_PutRef(upPtr);
		upPtr = NULL;
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	return bRet;
}



/* Check if a participant already exists by the name */
Bool EsifUpPm_DoesAvailableParticipantExistByName (
	char *participantName
	)
{
	Bool bRet = ESIF_FALSE;
	EsifUpPtr upPtr = NULL;
	UInt8 i;

	if (NULL == participantName) {
		ESIF_TRACE_ERROR("The participant name pointer is NULL\n");
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);

		if (NULL == upPtr) {
			continue;
		}

		if ((g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) && !esif_ccb_strcmp(EsifUp_GetName(upPtr), participantName)) {
			bRet = ESIF_TRUE;
			break;
		}

		EsifUp_PutRef(upPtr);
		upPtr = NULL;
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	
	return bRet;
}

/* the caller should call EsifUp_PutRef to release reference on participant when done with it */
EsifUpPtr EsifUpPm_GetAvailableParticipantByName (
	char *participantName
	)
{
	EsifUpPtr upPtr = NULL;
	UInt8 i;

	if (NULL == participantName) {
		ESIF_TRACE_ERROR("The participant name pointer is NULL\n");
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(i);

		if (NULL == upPtr) {
			continue;
		}

		if (!esif_ccb_stricmp(participantName, EsifUp_GetName(upPtr))) {
			break;
		}

		EsifUp_PutRef(upPtr);
		upPtr = NULL;
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	return upPtr;
}


static EsifUpManagerEntryPtr EsifUpPm_GetParticipantEntryFromMetadata(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	)
{
	EsifUpManagerEntryPtr entryPtr = NULL;
	char *participantName = "";
	UInt8 i;

	/* Validate parameters */
	if (NULL == metadataPtr) {
		ESIF_TRACE_ERROR("The meta data pointer is NULL\n");
		goto exit;
	}

	switch (origin) {
	case eParticipantOriginLF:
		participantName = ((struct esif_ipc_event_data_create_participant *)metadataPtr)->name;
		break;

	case eParticipantOriginUF:
		participantName = ((EsifParticipantIfacePtr)metadataPtr)->name;
		break;

	default:
		goto exit;
		break;
	}

	esif_ccb_write_lock(&g_uppMgr.fLock);
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entryPtr = &g_uppMgr.fEntries[i];

		if (NULL != entryPtr->fUpPtr) {
			if (!strcmp(EsifUp_GetName(entryPtr->fUpPtr), participantName)) {
				break;
			}
		}
		entryPtr = NULL;
	}
	esif_ccb_write_unlock(&g_uppMgr.fLock);

exit:
	return entryPtr;
}


/* Map a participant handle to  */
eEsifError EsifUpPm_MapLpidToParticipantInstance(
	const UInt8 lpInstance,
	UInt8 *upInstancePtr
	)
{
	eEsifError rc    = ESIF_E_INVALID_HANDLE;
	UInt8 i = 0;

	/* Validate parameters */
	if (NULL == upInstancePtr) {
		ESIF_TRACE_ERROR("The participant handle pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (g_uppMgr.fEntries[i].fUpPtr && (EsifUp_GetLpInstance(g_uppMgr.fEntries[i].fUpPtr) == lpInstance)) {
			break;
		}
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);

	if (i >= MAX_PARTICIPANT_ENTRY) {
		goto exit;
	}

	*upInstancePtr = i;
	rc = ESIF_OK;

exit:
	return rc;
}


/*
 * Used to iterate through the available participants.
 * First call EsifUpPm_InitIterator to initialize the iterator.
 * Next, call EsifUpPm_GetNextUp using the iterator.  Repeat until
 * EsifUpPm_GetNextUp fails. The call will release the reference of the
 * participant from the previous call.  If you stop iteration part way through
 * all participants, the caller is responsible for releasing the reference on
 * the last participant returned.  Iteration is complete when
 * ESIF_E_ITERATOR_DONE is returned.
 */
eEsifError EsifUpPm_InitIterator(
	UfPmIteratorPtr iteratorPtr
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == iteratorPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = UF_PM_ITERATOR_MARKER;
exit:
	return rc;
}


/* See EsifUpPm_InitIterator for usage */
eEsifError EsifUpPm_GetNextUp(
	UfPmIteratorPtr iteratorPtr,
	EsifUpPtr *upPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr nextUpPtr = NULL;
	UInt8 i;

	if ((NULL == upPtr) || (NULL == iteratorPtr)) {
		ESIF_TRACE_WARN("Parameter is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if ((iteratorPtr->marker != UF_PM_ITERATOR_MARKER) ||
		(iteratorPtr->handle >= MAX_PARTICIPANT_ENTRY)) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (iteratorPtr->ref_taken) {
		iteratorPtr->handle++;
		EsifUp_PutRef(iteratorPtr->upPtr);
		iteratorPtr->upPtr = NULL;
		iteratorPtr->ref_taken = ESIF_FALSE;
	}

	for (i = iteratorPtr->handle; i < MAX_PARTICIPANT_ENTRY; i++) {
		nextUpPtr = EsifUpPm_GetAvailableParticipantByInstance(i);
		if (nextUpPtr != NULL) {
			iteratorPtr->handle = i;
			iteratorPtr->upPtr = nextUpPtr;
			iteratorPtr->ref_taken = ESIF_TRUE;
			break;
		}
	}

	*upPtr = nextUpPtr;

	if (NULL == nextUpPtr) {
		rc = ESIF_E_ITERATION_DONE;
	}
exit:
	return rc;
}

	
/* Initialize manager */
eEsifError EsifUpPm_Init(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	/* Initialize Lock */
	esif_ccb_lock_init(&g_uppMgr.fLock);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


/* Exit manager */
void EsifUpPm_Exit(void)
{
	ESIF_TRACE_ENTRY_INFO();

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, NULL);

	/* Clean up resources */
	EsifUpPm_DestroyParticipants();

	/* Uninitialize Lock */
	esif_ccb_lock_uninit(&g_uppMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


/* This should only be called when shutting down */
static eEsifError EsifUpPm_DestroyParticipants(void)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	UInt8 i = 0;

	esif_ccb_write_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entryPtr = &g_uppMgr.fEntries[i];

		if (NULL != entryPtr->fUpPtr) {

			// This will be cleaned up when the reference counting code is brought in
			esif_ccb_write_unlock(&g_uppMgr.fLock);
			EsifUpPm_UnregisterParticipant(entryPtr->fUpPtr->fOrigin, i);
			esif_ccb_write_lock(&g_uppMgr.fLock);

			EsifUp_DestroyParticipant(entryPtr->fUpPtr);
		}
		entryPtr->fUpPtr = NULL;
		entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_AVAILABLE;
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);

	ESIF_TRACE_INFO("The participants are destroyed in ESIF UF participant manager\n");
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
