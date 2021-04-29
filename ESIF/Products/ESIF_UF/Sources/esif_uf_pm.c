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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PARTICIPANT

/* ESIF */
#include "esif_uf.h"		/* Upper Framework */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_dsp.h"		/* Device Support Package */
#include "esif_uf_eventmgr.h"
#include "esif_participant.h"
#include "esif_uf_ccb_thermalapi.h"
#include "esif_uf_ccb_imp_spec.h"
#include "esif_uf_handlemgr.h"
#include "esif_command.h"
#include "esif_ccb_string.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define ESIF_PARTICIPANT0_INDEX	0


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
static void EsifUpPm_SuspendLfParticipants();


/*
 * ===========================================================================
 * The followins functions are participant "friend" functions
 * ===========================================================================
 */
eEsifError EsifUp_CreateParticipant(
	const eEsifParticipantOrigin origin,
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

static eEsifError EsifUpPm_SuspendParticipant(const esif_handle_t upInstance);
static eEsifError EsifUpPm_ResumeParticipant(const esif_handle_t upInstance);

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

eEsifError EsifUp_StartParticipantSlowPoll(
	EsifUpPtr self
	);

eEsifError EsifUp_StopParticipantSlowPoll(
	EsifUpPtr self
	);

eEsifError EsifUp_ReevaluateParticipantCaps(
	EsifUpPtr self
	);

void EsifUp_SetInstance(
	EsifUpPtr self,
	esif_handle_t upInstance
	);

Bool EsifUp_IsPreferredParticipant(
	EsifUpPtr upPtr,
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
);


static EsifUpPtr EsifUpPm_GetAvailableParticipantByIndex(
	const UInt8 index
);

static EsifUpPtr EsifUpPm_GetAvailableParticipantByInstanceLocked(
	const esif_handle_t upInstance
);

static EsifUpPtr EsifUpPm_GetAvailableParticipantByIndexLocked(
	const UInt8 index
);

static EsifUpManagerEntryPtr EsifUpPm_GetEntryByInstanceLocked(
	const esif_handle_t upInstance
);

/*
 * ===========================================================================
 * PRIVATE
 * ===========================================================================
 */
static EsifUpManagerEntryPtr EsifUpPm_GetParticipantEntryFromMetadataLocked(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	);

static eEsifError EsifUpPm_DestroyParticipants(void);

static eEsifError ESIF_CALLCONV EsifUpPm_EventCallback(
	esif_context_t context,
	esif_handle_t upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

static eEsifError EsifUpPm_ActionChangeHandler(
	EsifDataPtr eventDataPtr
	);

static void EsifUpPm_SuspendDynamicUfParticipants();
static void EsifUpPm_ResumeDynamicUfParticipants();


static void *ESIF_CALLCONV EsifUfPollWorkerThread(void *ptr)
{
	UInt8 i = 0;
	
	UNREFERENCED_PARAMETER(ptr);

	atomic_set(&g_ufpollQuit, 0);

	CMD_OUT("Starting Upper Framework Polling... \n");

	/* check temperature */
	while (!atomic_read(&g_ufpollQuit)) {
		
		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByIndex(i);
			
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

/* Enable Participant Activity Logging for the the given Participant for supported Domain Types */
eEsifError EsifUpPm_ParticipantActivityLoggingEnable(EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;

	if (upPtr) {
		UpDomainIterator udIter = { 0 };
		EsifUpDomainPtr domainPtr = NULL;
		eEsifError iterRc = ESIF_OK;
		rc = ESIF_OK;

		iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
		if (ESIF_OK == iterRc) {

			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			while (ESIF_OK == iterRc) {

				// Found a Valid Domain. Enable Participant Activity Logging for supported domainTypes.
				if (domainPtr) {
					switch (domainPtr->domainType) {
					case ESIF_DOMAIN_TYPE_PROCESSOR:
					case ESIF_DOMAIN_TYPE_GRAPHICS:
					case ESIF_DOMAIN_TYPE_FAN:
					case ESIF_DOMAIN_TYPE_WIRELESS:
					case ESIF_DOMAIN_TYPE_MULTIFUNCTION:
					case ESIF_DOMAIN_TYPE_DISPLAY:
					case ESIF_DOMAIN_TYPE_BATTERYCHARGER:
					case ESIF_DOMAIN_TYPE_WWAN:
					case ESIF_DOMAIN_TYPE_POWER:
					case ESIF_DOMAIN_TYPE_CHIPSET:
					{
						UInt32 capMask = EsifUp_GetDomainCapabilityMask(domainPtr);
						EsifData capData = { ESIF_DATA_UINT32, &capMask, sizeof(capMask), sizeof(capMask) };

						EsifEventMgr_SignalEvent(domainPtr->participantId,
							domainPtr->domain,
							ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
							&capData
						);
						break;
					}
					default:
						break;
					}
				}
				iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			}

			if (ESIF_E_ITERATION_DONE != iterRc) {
				EsifUp_PutRef(upPtr);
			}
		}
	}
	return rc;
}

/* Add participant in participant manager */
eEsifError EsifUpPm_RegisterParticipant(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr,
	esif_handle_t *upInstancePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	EsifUpPtr newUpPtr = NULL;
	EsifUpPtr tempUpPtr = NULL;
	EsifUpManagerEntryPtr entryPtr = NULL;
	UInt8 i = 0;
	Bool isUppMgrLocked = ESIF_FALSE;
	Bool isPreferredParticipant = ESIF_FALSE;
	esif_handle_t newUpInstance = ESIF_INVALID_HANDLE;

	/* Validate parameters */
	if ((NULL == metadataPtr) || (NULL == upInstancePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*upInstancePtr = ESIF_INVALID_HANDLE;

	esif_ccb_write_lock(&g_uppMgr.fLock);
	isUppMgrLocked = ESIF_TRUE;
	
	/*
	 * Check if a participant has already been created, but was then removed.
	 * In that case, just re-enable the participant.
	 */
	entryPtr = EsifUpPm_GetParticipantEntryFromMetadataLocked(origin, metadataPtr);

	if (NULL != entryPtr) {

		tempUpPtr = entryPtr->fUpPtr;

		isPreferredParticipant = EsifUp_IsPreferredParticipant(tempUpPtr, origin, metadataPtr);
		if (!isPreferredParticipant) {

			if (entryPtr->fState > ESIF_PM_PARTICIPANT_STATE_LF_REMOVED) {
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

			entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
			g_uppMgr.fEntryCount++;

			esif_ccb_write_unlock(&g_uppMgr.fLock);
			isUppMgrLocked = ESIF_FALSE;
		}
		else {
			rc = EsifUp_CreateParticipant(origin, metadataPtr, &newUpPtr);
			if ((rc != ESIF_OK) || (newUpPtr == NULL)) {
				rc = ESIF_E_NO_CREATE;
				goto exit;
			}
			EsifUp_SetInstance(newUpPtr, EsifUp_GetInstance(tempUpPtr));

			rc = EsifUp_GetRef(newUpPtr);
			if (rc != ESIF_OK) {
				EsifUp_DestroyParticipant(newUpPtr);
				rc = ESIF_E_NO_CREATE;
				goto exit;
			}

			EsifUp_SuspendParticipant(tempUpPtr);

			if (entryPtr->fState != ESIF_PM_PARTICIPANT_STATE_CREATED) {
				entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
				g_uppMgr.fEntryCount++;
			}

			entryPtr->fUpPtr = newUpPtr;

			esif_ccb_write_unlock(&g_uppMgr.fLock);
			isUppMgrLocked = ESIF_FALSE;

			EsifAppMgr_DestroyParticipantInAllApps(tempUpPtr);
			EsifUp_DestroyParticipant(tempUpPtr);

			upPtr = newUpPtr;
		}
	}
	else {

		rc = EsifUp_CreateParticipant(origin, metadataPtr, &upPtr);
		if ((rc != ESIF_OK) || (upPtr == NULL)) {
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		/*
		 *  Find available slot in participant manager table.  Simple Table Lookup For Now.
		 *  Scan table and find first empty slot.  Empty slot indicated by AVAILABLE state.
		 */
		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			if (ESIF_PM_PARTICIPANT_STATE_AVAILABLE == g_uppMgr.fEntries[i].fState) {
				if ((i != ESIF_PARTICIPANT0_INDEX) || (EsifUp_IsPrimaryParticipant(upPtr))) {
					break;
				}
			}
		}

		/* If no available slots return */
		if (i >= MAX_PARTICIPANT_ENTRY) {
			ESIF_TRACE_ERROR("Unable to add participant, the maximum number of participants has been reached.\n");
			EsifUp_DestroyParticipant(upPtr);
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		if (EsifUp_IsPrimaryParticipant(upPtr)) {
			newUpInstance = ESIF_HANDLE_PRIMARY_PARTICIPANT;
		}
		else {
			rc = EsifHandleMgr_GetNextHandle(&newUpInstance);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("Unable to allocate partcipant handle\n");
				EsifUp_DestroyParticipant(upPtr);
				/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
				upPtr = NULL;
				rc = ESIF_E_NO_CREATE;
				goto exit;
			}
		}
		EsifUp_SetInstance(upPtr, newUpInstance);

		rc = EsifUp_GetRef(upPtr);
		if (rc != ESIF_OK) {
			EsifUp_DestroyParticipant(upPtr);
			/* clear upPtr since we don't need to call EsifUp_PutRef in the end */
			upPtr = NULL;
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		g_uppMgr.fEntries[i].fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
		g_uppMgr.fEntries[i].fUpPtr = upPtr;
		g_uppMgr.fEntryCount++;

		esif_ccb_write_unlock(&g_uppMgr.fLock);
		isUppMgrLocked = ESIF_FALSE;
	}

	*upInstancePtr = EsifUp_GetInstance(upPtr);

	/* Perform initialization that requires primitive support */
	EsifUp_DspReadyInit(upPtr);

	/* Now offer this participant to each running application */
	EsifAppMgr_CreateParticipantInAllApps(upPtr);

	/* Enable this participant for thermal API (Windows) */
	EsifThermalApi_ParticipantCreate(upPtr);

	/* Enable participant activity logging if participant supports it */
	EsifUpPm_ParticipantActivityLoggingEnable(upPtr);

	EsifEventMgr_SignalEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PARTICIPANT_CREATE_COMPLETE, NULL);
exit:
	if (isUppMgrLocked == ESIF_TRUE) {
		/* Unlock manager */
		esif_ccb_write_unlock(&g_uppMgr.fLock);
	}

	EsifUp_PutRef(upPtr);

	return rc;
}


static eEsifError ESIF_CALLCONV EsifUpPm_EventCallback(
	esif_context_t context,
	esif_handle_t upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	struct esif_ipc_event_data_create_participant *creationDataPtr = NULL;
	esif_handle_t newInstance = ESIF_INVALID_HANDLE;

	UNREFERENCED_PARAMETER(context);
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

		rc = EsifUpPm_RegisterParticipant(eParticipantOriginLF, creationDataPtr, &newInstance);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Failed to add participant %s to participant manager; %s(%u)", creationDataPtr->name, esif_rc_str(rc), rc);
			goto exit;
		}

		ESIF_TRACE_INFO("\nCreate new UF participant: %s, instance = %d\n", creationDataPtr->name, newInstance);
		break;

	case ESIF_EVENT_PARTICIPANT_SUSPEND:
		if (upInstance != ESIF_HANDLE_PRIMARY_PARTICIPANT) {
			ESIF_TRACE_INFO("Suspending Participant: " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));
			rc = EsifUpPm_SuspendParticipant(upInstance);
		}
		else { /* Use Participant 0 suspend as an indicator of Sx entry */
			EsifUpPm_SuspendDynamicUfParticipants();
		}
		break;

	case ESIF_EVENT_PARTICIPANT_RESUME:
		if (upInstance != ESIF_HANDLE_PRIMARY_PARTICIPANT) {
			ESIF_TRACE_INFO("Reregistering Participant: " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));
			rc = EsifUpPm_ResumeParticipant(upInstance);
		}
		else { /* Use Participant 0 suspend as an indicator of Sx entry */
			EsifUpPm_ResumeDynamicUfParticipants();
		}
		break;

	case ESIF_EVENT_PARTICIPANT_UNREGISTER: 
			ESIF_TRACE_INFO("Unregistering Participant: " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));
			rc = EsifUpPm_DestroyParticipantByInstance(upInstance);
		break;

	case ESIF_EVENT_LF_UNLOADED:
		EsifUpPm_SuspendLfParticipants();
		break;

	case ESIF_EVENT_ACTION_LOADED:
	case ESIF_EVENT_ACTION_UNLOADED:
			EsifUpPm_ActionChangeHandler(eventDataPtr);
		break;

	case ESIF_EVENT_DISPLAY_OFF:
			EsifUpPm_StartAllParticipantsSlowPoll();
		break;
	case ESIF_EVENT_DISPLAY_ON:
			EsifUpPm_StopAllParticipantsSlowPoll();
		break;

	default:
		break;
	}

exit:
	return rc;
}

static eEsifError EsifUpPm_ActionChangeHandler(
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;
	EsifUpPtr upPtr = NULL;
	UInt32 actionType = 0;

	if ((NULL == eventDataPtr) ||
		(NULL == eventDataPtr->buf_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if ((eventDataPtr->buf_len < sizeof(actionType)) ||
		(eventDataPtr->data_len < sizeof(actionType))) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	actionType = *((UInt32 *)eventDataPtr->buf_ptr);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {

		upPtr = EsifUpPm_GetAvailableParticipantByIndex(i);
		if (NULL == upPtr) {
			continue;
		}

		if(EsifUp_IsActionInDsp(upPtr, actionType)) {
			EsifUp_ReevaluateParticipantCaps(upPtr);
		}

		EsifUp_PutRef(upPtr);
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
		EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByIndex(i);
		
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
			EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByIndex(i);
			
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

/* Set all polling participants' sample period value explicitely*/
eEsifError EsifUpPm_StartAllParticipantsSlowPoll()
{
	EsifUpPtr upPtr = NULL;
	UfPmIterator upIter = { 0 };
	eEsifError rc = ESIF_OK;
	eEsifError iteratorRc = ESIF_OK;

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc == ESIF_OK) {
		iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		while (ESIF_OK == iteratorRc) {
			if (NULL == upPtr) {
				iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				continue;
			}
			
			EsifUp_StartParticipantSlowPoll(upPtr);

			iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		EsifUp_PutRef(upPtr);
	}

	return rc;
}

/* Reset all polling participants' sample period back to their _TSP value */
eEsifError EsifUpPm_StopAllParticipantsSlowPoll()
{
	EsifUpPtr upPtr = NULL;
	UfPmIterator upIter = { 0 };
	eEsifError rc = ESIF_OK;
	eEsifError iteratorRc = ESIF_OK;

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc == ESIF_OK) {
		iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		while (ESIF_OK == iteratorRc) {
			if (NULL == upPtr) {
				iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				continue;
			}

			EsifUp_StopParticipantSlowPoll(upPtr);

			iteratorRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		EsifUp_PutRef(upPtr);
	}

	return rc;
}


/* Resume upper participant instance that existed previously */
static eEsifError EsifUpPm_ResumeParticipant(
	const esif_handle_t upInstance
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;

	/* Should never have to re-register Participant 0 */
	if (EsifUpPm_IsPrimaryParticipantId(upInstance)) {
		goto exit;
	}

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entryPtr = EsifUpPm_GetEntryByInstanceLocked(upInstance);
	if (NULL == entryPtr) {
		rc = ESIF_E_NOT_FOUND;
		esif_ccb_write_unlock(&g_uppMgr.fLock);
		goto exit;
	}

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
		rc = EsifAppMgr_CreateParticipantInAllApps(upPtr);
	}

	ESIF_TRACE_INFO("Reregistered participant, instant id = " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


/* Resumes all upper participants instances (except primary) that exist */
eEsifError EsifUpPm_ResumeParticipants()
{
	UInt8 i = 0;
	EsifUpPtr upPtr = NULL;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;

	ESIF_TRACE_INFO("Resuming all participants\n");

	for (i = ESIF_PARTICIPANT0_INDEX + 1; i < MAX_PARTICIPANT_ENTRY; i++) {

		esif_ccb_write_lock(&g_uppMgr.fLock);
		upPtr = g_uppMgr.fEntries[i].fUpPtr;
		participantId = EsifUp_GetInstance(upPtr);
		esif_ccb_write_unlock(&g_uppMgr.fLock);

		if (upPtr != NULL) {
			EsifUpPm_ResumeParticipant(participantId);
		}
	}

	ESIF_TRACE_INFO("Resumption of participants complete\n");
	return ESIF_OK;
}


/* Suspend Upper Participant Instance */
static eEsifError EsifUpPm_SuspendParticipant(
	const esif_handle_t upInstance
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entryPtr = EsifUpPm_GetEntryByInstanceLocked(upInstance);
	if (NULL == entryPtr) {
		esif_ccb_write_unlock(&g_uppMgr.fLock);
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	upPtr = entryPtr->fUpPtr;
	if ((NULL != upPtr) && (entryPtr->fState > ESIF_PM_PARTICIPANT_STATE_REMOVED)) {

		EsifUp_SuspendParticipant(upPtr);

		entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_REMOVED;
		g_uppMgr.fEntryCount--;

		/*
		* Get reference on participant before passing it to other functions
		* to make sure the participant is valid when passed in
		*/
		rc = EsifUp_GetRef(upPtr);
		if (rc != ESIF_OK) {
			rc = ESIF_OK;
			upPtr = NULL;
		}
	}
	else {
		upPtr = NULL;
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);

	if (NULL != upPtr) {
		rc = EsifAppMgr_DestroyParticipantInAllApps(upPtr);
	}

	ESIF_TRACE_INFO("Suspended participant, instant id = " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));
exit:
	EsifUp_PutRef(upPtr);
	return rc;
}


static void EsifUpPm_SuspendDynamicUfParticipants()
{
	UInt8 i = 0;
	EsifUpPtr upPtr = NULL;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;

	ESIF_TRACE_INFO("Suspending all dynamic participants\n");

	for (i = ESIF_PARTICIPANT0_INDEX + 1; i < MAX_PARTICIPANT_ENTRY; i++) {

		esif_ccb_read_lock(&g_uppMgr.fLock);
		upPtr = g_uppMgr.fEntries[i].fUpPtr;

		if (upPtr && (eParticipantOriginUF == upPtr->fOrigin) && (ESIF_PARTICIPANT_ENUM_CONJURE == upPtr->fMetadata.fEnumerator)) {
			participantId = EsifUp_GetInstance(upPtr);
		}
		esif_ccb_read_unlock(&g_uppMgr.fLock);

		if (participantId != ESIF_INVALID_HANDLE) {
			ESIF_TRACE_INFO("Suspending dynamic participant " ESIF_HANDLE_FMT "\n", participantId);
			EsifEventMgr_SignalEvent(participantId, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL);
		}
	}

	ESIF_TRACE_INFO("Suspension of all dynamic participants complete\n");
	return;
}


static void EsifUpPm_ResumeDynamicUfParticipants()
{
	UInt8 i = 0;
	EsifUpPtr upPtr = NULL;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;

	ESIF_TRACE_INFO("Resuming all dynamic participants\n");

	for (i = ESIF_PARTICIPANT0_INDEX + 1; i < MAX_PARTICIPANT_ENTRY; i++) {

		esif_ccb_read_lock(&g_uppMgr.fLock);
		upPtr = g_uppMgr.fEntries[i].fUpPtr;

		if (upPtr && (eParticipantOriginUF == upPtr->fOrigin) && (ESIF_PARTICIPANT_ENUM_CONJURE == upPtr->fMetadata.fEnumerator)) {
			participantId = EsifUp_GetInstance(upPtr);
		}
		esif_ccb_read_unlock(&g_uppMgr.fLock);

		if (participantId != ESIF_INVALID_HANDLE) {
			ESIF_TRACE_INFO("Rsuming dynamic participant " ESIF_HANDLE_FMT "\n", participantId);
			EsifEventMgr_SignalEvent(participantId, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PARTICIPANT_RESUME, NULL);
		}
	}

	ESIF_TRACE_INFO("Resumption of all dynamic participants complete\n");
	return;
}


static void EsifUpPm_SuspendLfParticipants()
{
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;
	UInt8 i = 0;

	esif_ccb_write_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entryPtr = &g_uppMgr.fEntries[i];
		upPtr = entryPtr->fUpPtr;
		if (upPtr && (eParticipantOriginLF == upPtr->fOrigin)) {
			EsifEventMgr_SignalEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL);
		}
		if (ESIF_PARTICIPANT0_INDEX == i) {
			entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_LF_REMOVED;
		}
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);
	return;
}



// Removes a Conjured Kernel Participant
esif_error_t EsifUpPm_DestroyConjuredLfParticipant(esif_string name)
{
	eEsifError rc = ESIF_OK;
	struct esif_command_participant_destroy *reqDataPtr = NULL;
	UInt32 dataLen = sizeof(*reqDataPtr);
	struct esif_ipc *ipcPtr = NULL;
	struct esif_ipc_command *cmdPtr = NULL;

	if (NULL == name) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ipcPtr = esif_ipc_alloc_command(&cmdPtr, dataLen);
	if ((NULL == ipcPtr) || (NULL == cmdPtr)) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	//
	// Initialize the command portion of the data
	//
	cmdPtr->type = ESIF_COMMAND_TYPE_PARTICIPANT_DESTROY;
	cmdPtr->req_data_type = ESIF_DATA_STRUCTURE;
	cmdPtr->req_data_offset = 0;
	cmdPtr->req_data_len = dataLen;
	cmdPtr->rsp_data_type = ESIF_DATA_VOID;
	cmdPtr->rsp_data_offset = 0;
	cmdPtr->rsp_data_len = 0;

	//
	// Initialize the data payload to create the participant
	//
	reqDataPtr = (struct esif_command_participant_destroy *)(cmdPtr + 1);
	esif_ccb_strcpy(reqDataPtr->name, name, sizeof(reqDataPtr->name));

	ESIF_TRACE_DEBUG("Deleting kernel participant %s.\n", name);

	rc = ipc_execute(ipcPtr);
	if (rc != ESIF_OK || cmdPtr->return_code != ESIF_OK) {
		rc = (rc != ESIF_OK ? rc : cmdPtr->return_code);
		ESIF_TRACE_DEBUG("Failure deleting kernel participant %s; err = %s(%d)\n",
			name, esif_rc_str(rc), rc);
	}
exit:
	esif_ipc_free(ipcPtr);
	return rc;
}



/*
 * Get By Instance From ID (May be handle or index; depending on current code state)
 * the caller should call EsifUp_PutRef to release reference on participant when done with it
 */
EsifUpPtr EsifUpPm_GetAvailableParticipantByInstance(
	const esif_handle_t upInstance
	)
{
	EsifUpPtr upPtr = NULL;

	/* Lock manager */
	esif_ccb_read_lock(&g_uppMgr.fLock);

	upPtr = EsifUpPm_GetAvailableParticipantByInstanceLocked(upInstance);

	/* Unlock Manager */
	esif_ccb_read_unlock(&g_uppMgr.fLock);
	return upPtr;
}


/*
* Get participant by index
* NOTE:  The caller should call EsifUp_PutRef to release reference on participant when done with it
*/
static EsifUpPtr EsifUpPm_GetAvailableParticipantByIndex(
	const UInt8 index
	)
{
	EsifUpPtr upPtr = NULL;

	esif_ccb_read_lock(&g_uppMgr.fLock);

	upPtr = EsifUpPm_GetAvailableParticipantByIndexLocked(index);

	esif_ccb_read_unlock(&g_uppMgr.fLock);
	return upPtr;
}


/*
* Get participant by index (locked version - lock already held)
* NOTE:  The caller should call EsifUp_PutRef to release reference on participant when done with it
*/
static EsifUpPtr EsifUpPm_GetAvailableParticipantByIndexLocked(
	const UInt8 index
	)
{
	EsifUpPtr upPtr = NULL;
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("Index %d\n", index);

	if (index >= MAX_PARTICIPANT_ENTRY) {
		ESIF_TRACE_ERROR("Instance id %d is out of range\n", index);
		ESIF_ASSERT(ESIF_FALSE);
		goto exit;
	}

	if (g_uppMgr.fEntries[index].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) {
		upPtr = g_uppMgr.fEntries[index].fUpPtr;
		if (upPtr != NULL) {
			rc = EsifUp_GetRef(upPtr);
			if (rc != ESIF_OK) {
				ESIF_TRACE_INFO("Unable to acquire reference on participant\n");
				upPtr = NULL;
			}
		}
	}
exit:
	return upPtr;
}


/*
* Get PM entry by handle (locked version - lock already held)
*/
static EsifUpManagerEntryPtr EsifUpPm_GetEntryByInstanceLocked(
	const esif_handle_t upInstance
	)
{
	EsifUpManagerEntryPtr entryPtr = NULL;
	size_t index = 0;

	for (index = 0; index < MAX_PARTICIPANT_ENTRY; index++) {
		entryPtr = &g_uppMgr.fEntries[index];
		if (entryPtr != NULL) {
			if (upInstance == EsifUp_GetInstance(entryPtr->fUpPtr)) {
				break;
			}
		}
		entryPtr = NULL;
	}
	return entryPtr;
}


/*
* Get participant by handle (locked version - lock already held)
* NOTE:  The caller should call EsifUp_PutRef to release reference on participant when done with it
*/
static EsifUpPtr EsifUpPm_GetAvailableParticipantByInstanceLocked(
	const esif_handle_t upInstance
)
{
	EsifUpPtr upPtr = NULL;
	eEsifError rc = ESIF_OK;
	size_t index = 0;
	esif_handle_t localInstance = upInstance;
	esif_handle_t curUpInstance = ESIF_INVALID_HANDLE;

	if (EsifUpPm_IsPrimaryParticipantId(upInstance)) {
		localInstance = ESIF_HANDLE_PRIMARY_PARTICIPANT;
	}
	for (index = 0; index < MAX_PARTICIPANT_ENTRY; index++) {
		if (g_uppMgr.fEntries[index].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) {
			upPtr = g_uppMgr.fEntries[index].fUpPtr;
			if (upPtr != NULL) {

				curUpInstance = EsifUp_GetInstance(upPtr);
				if (localInstance == curUpInstance) {
					rc = EsifUp_GetRef(upPtr);
					if (rc != ESIF_OK) {
						ESIF_TRACE_INFO("Unable to acquire reference on participant\n");
						upPtr = NULL;
					}
					break;
				}
			}
			upPtr = NULL;
		}
	}
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
		upPtr = EsifUpPm_GetAvailableParticipantByIndexLocked(i);

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

/* Check if a participant already exists by the device path */
Bool EsifUpPm_DoesAvailableParticipantExistByDevicePath(
	char *participantDevicePath
)
{
	Bool bRet = ESIF_FALSE;
	EsifUpPtr upPtr = NULL;
	EsifUpDataPtr metaPtr = NULL;
	UInt8 i;

	if (NULL == participantDevicePath) {
		ESIF_TRACE_ERROR("The participant device path pointer is NULL\n");
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		upPtr = EsifUpPm_GetAvailableParticipantByIndexLocked(i);

		if (NULL == upPtr) {
			continue;
		}

		metaPtr = EsifUp_GetMetadata(upPtr);
		if (NULL == metaPtr) {
			continue;
		}
		if ((g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_STATE_REMOVED) && !esif_ccb_stricmp(metaPtr->fDevicePath, participantDevicePath)) {
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
		upPtr = EsifUpPm_GetAvailableParticipantByIndexLocked(i);

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
		upPtr = EsifUpPm_GetAvailableParticipantByIndexLocked(i);

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


static EsifUpManagerEntryPtr EsifUpPm_GetParticipantEntryFromMetadataLocked(
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
		// If the participant is a DPTFZ, then it will only match the primary participant
		if (((struct esif_ipc_event_data_create_participant *)metadataPtr)->flags & ESIF_FLAG_DPTFZ) {
			if (g_uppMgr.fEntries[ESIF_PARTICIPANT0_INDEX].fUpPtr != NULL) {
				entryPtr = &g_uppMgr.fEntries[ESIF_PARTICIPANT0_INDEX];
			}
			goto exit;
		}

		participantName = ((struct esif_ipc_event_data_create_participant *)metadataPtr)->name;
		break;

	case eParticipantOriginUF:
		// If the participant is a DPTFZ, then it will only match the primary participant
		if (((EsifParticipantIfacePtr)metadataPtr)->flags & ESIF_FLAG_DPTFZ) {
			if (g_uppMgr.fEntries[ESIF_PARTICIPANT0_INDEX].fUpPtr != NULL) {
				entryPtr = &g_uppMgr.fEntries[ESIF_PARTICIPANT0_INDEX];
			}
			goto exit;
		}

		participantName = ((EsifParticipantIfacePtr)metadataPtr)->name;
		break;

	default:
		goto exit;
		break;
	}

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entryPtr = &g_uppMgr.fEntries[i];

		if (NULL != entryPtr->fUpPtr) {
			if (!strcmp(EsifUp_GetName(entryPtr->fUpPtr), participantName)) {
				break;
			}
		}
		entryPtr = NULL;
	}
exit:
	return entryPtr;
}


/* Map a participant handle to  */
eEsifError EsifUpPm_MapLpidToParticipantInstance(
	const UInt8 lpInstance,
	esif_handle_t *upInstancePtr
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

	*upInstancePtr = EsifUp_GetInstance(g_uppMgr.fEntries[i].fUpPtr);
	rc = ESIF_OK;

exit:
	return rc;
}


Bool EsifUpPm_IsActionUsedByParticipants(
	enum esif_action_type type
	)
{
	Bool isUsed = ESIF_FALSE;
	EsifUpPtr upPtr = NULL;
	UInt8 i;

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		upPtr = g_uppMgr.fEntries[i].fUpPtr;
		if (NULL == upPtr) {
			continue;
		}

		isUsed = EsifUp_IsActionInDsp(upPtr, type);
		if (isUsed) {
			break;
		}
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);

	return isUsed;
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
		(iteratorPtr->index >= MAX_PARTICIPANT_ENTRY)) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (iteratorPtr->ref_taken) {
		iteratorPtr->index++;
		EsifUp_PutRef(iteratorPtr->upPtr);
		iteratorPtr->upPtr = NULL;
		iteratorPtr->ref_taken = ESIF_FALSE;
	}

	for (i = iteratorPtr->index; i < MAX_PARTICIPANT_ENTRY; i++) {
		nextUpPtr = EsifUpPm_GetAvailableParticipantByIndex(i);
		if (nextUpPtr != NULL) {
			iteratorPtr->index = i;
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

UInt8 EsifUpPm_ParticipantCount(void)
{
	UInt8 result = 0;
	esif_ccb_read_lock(&g_uppMgr.fLock);
	result = g_uppMgr.fEntryCount;
	esif_ccb_read_unlock(&g_uppMgr.fLock);
	return result;
}

/* Initialize manager */
eEsifError EsifUpPm_Init(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	/* Initialize Lock */
	esif_ccb_lock_init(&g_uppMgr.fLock);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_ACTION_LOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_ACTION_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_DISPLAY_OFF, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_DISPLAY_ON, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_BATTERY_COUNT_NOTIFICATION, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_LF_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);

	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


/* Exit manager */
void EsifUpPm_Exit(void)
{
	ESIF_TRACE_ENTRY_INFO();

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_ACTION_LOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_ACTION_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DISPLAY_OFF, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DISPLAY_ON, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_BATTERY_COUNT_NOTIFICATION, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_LF_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifUpPm_EventCallback, 0);

	/* Clean up resources */
	EsifUpPm_DestroyParticipants();

	/* Uninitialize Lock */
	esif_ccb_lock_uninit(&g_uppMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


// Destroy Particicpant by Name if participantName is non-NULL or ALL Participants if NULL
eEsifError EsifUpPm_DestroyParticipant(char *participantName)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;
	UInt8 i = 0;
	esif_handle_t upInstance = ESIF_INVALID_HANDLE;
	Bool isConjuredLfParticipant = ESIF_FALSE;

	esif_ccb_write_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entryPtr = &g_uppMgr.fEntries[i];

		// Destroy Participant if Name=NULL or the name matches
		if ((participantName == NULL) || (entryPtr->fUpPtr && esif_ccb_stricmp(participantName, entryPtr->fUpPtr->fMetadata.fName) == 0)) {
			rc = ESIF_OK;
			
			if (NULL != entryPtr->fUpPtr) {

				upInstance = EsifUp_GetInstance(entryPtr->fUpPtr);


				/* Remove conjured participants from the LF also */
				isConjuredLfParticipant = ESIF_FALSE;
				if ((ESIF_PARTICIPANT_ENUM_CONJURE == entryPtr->fUpPtr->fMetadata.fEnumerator) && 
					(EsifUp_GetLpInstance(entryPtr->fUpPtr) != ESIF_INSTANCE_INVALID)) {
					isConjuredLfParticipant = ESIF_TRUE;
				}

				// This will be cleaned up when the reference counting code is brought in
				esif_ccb_write_unlock(&g_uppMgr.fLock);

				/* Remove conjured participants from the LF also */
				if (isConjuredLfParticipant) {
					/* Best effort; don't check return as we want to continue as there may not be an LF participant with the name */
					EsifUpPm_DestroyConjuredLfParticipant(participantName);
				}

				EsifUpPm_SuspendParticipant(upInstance);
				EsifEventMgr_SignalEvent(upInstance, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_PARTICIPANT_UNREGISTER_COMPLETE, NULL);
				ESIF_TRACE_INFO("Unregistered participant, instant id = " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(upInstance));

				esif_ccb_write_lock(&g_uppMgr.fLock);

				upPtr = entryPtr->fUpPtr;
				entryPtr->fUpPtr = NULL;
				entryPtr->fState = ESIF_PM_PARTICIPANT_STATE_AVAILABLE;

				esif_ccb_write_unlock(&g_uppMgr.fLock);
				EsifUp_DestroyParticipant(upPtr);
				esif_ccb_write_lock(&g_uppMgr.fLock);
			}
		}
	}

	ESIF_TRACE_INFO("%s %s destroyed in ESIF UF participant manager\n",
		(participantName ? "Participant" : "All"),
		(participantName ? participantName : "Participants"));
	esif_ccb_write_unlock(&g_uppMgr.fLock);

	return rc;
}

/* This should only be called when shutting down */
static eEsifError EsifUpPm_DestroyParticipants(void)
{
	return EsifUpPm_DestroyParticipant(NULL);
}

//
// Unregister Upper Participant Instance
// !!!WARNING!!! No references may be held on the participant when this
// function is called or a deadlock will occur
//
eEsifError EsifUpPm_DestroyParticipantByInstance(
	const esif_handle_t upInstance
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entryPtr = NULL;
	EsifUpPtr upPtr = NULL;
	char name[ESIF_NAME_LEN];

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entryPtr = EsifUpPm_GetEntryByInstanceLocked(upInstance);
	if (NULL == entryPtr) {
		rc = ESIF_E_NOT_FOUND;
		esif_ccb_write_unlock(&g_uppMgr.fLock);
		goto exit;
	}

	upPtr = entryPtr->fUpPtr;
	rc = EsifUp_GetRef(upPtr);
	if (rc != ESIF_OK) {
		upPtr = NULL;
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);

	if (upPtr) {
		// Must copy name and release reference or else the destroy below will deadlock
		esif_ccb_strcpy(name, EsifUp_GetName(upPtr), sizeof(name));
		EsifUp_PutRef(upPtr);

		rc = EsifUpPm_DestroyParticipant(name);
	}
exit:
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
