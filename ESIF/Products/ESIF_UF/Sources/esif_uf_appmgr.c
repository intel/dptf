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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_APP

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_uf_eventmgr.h"
#include "esif_uf_primitive.h"


//
// GENERAL DEFINITIONS
//
#define ESIF_UF_APPMGR_QUEUE_TIMEOUT ESIF_QUEUE_TIMEOUT_INFINITE /* No timeout */
#define ESIF_UF_APPMGR_QUEUE_SIZE 0xFFFFFFFF
#define ESIF_UF_APPMGR_QUEUE_NAME "UfAppMgrPartQueue"
#define ESIF_UF_APPMGR_QUEUE_DELAY_TIME 1000 /* ms */


//
// TYPE DECLARATIONS
// 
typedef struct EsifAppMgrPartQueueItem_s {
	EsifUpPtr upPtr;
	Bool isCreate;
} EsifAppMgrPartQueueItem, *EsifAppMgrPartQueueItemPtr;


//
// GLOBAL DEFINITIONS
// 
EsifAppMgr g_appMgr = {0};

//
// FRIEND OBJECTS AND FUNCTIONS
//
extern esif_handle_t g_dst;
extern char *g_dstName;

eEsifError EsifApp_Create(
	const EsifString appName,
	EsifAppPtr *appPtr
);

void EsifApp_Destroy(
	EsifAppPtr self
);

eEsifError EsifApp_GetRef(EsifAppPtr self);
void EsifApp_PutRef(EsifAppPtr self);

eEsifError EsifApp_Start(EsifAppPtr self);
eEsifError EsifApp_Stop(EsifAppPtr self);

eEsifError EsifApp_CreateParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	);

eEsifError EsifApp_DestroyParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	);

Bool EsifApp_IsAppName(
	EsifAppPtr self,
	const EsifString name
);

Bool EsifApp_IsAppHandle(
	EsifAppPtr self,
	const esif_handle_t handle
);


//
// PRIVATE FUNCTION PROTOTYPES
//
static eEsifError ESIF_CALLCONV EsifAppMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

//
// Takes an additional reference on the app which prevents the app from being
// destroyed until all references are released.
//
static EsifAppPtr EsifAppMgr_GetAppN(
	size_t index,
	Bool isSvcRequest
);

static eEsifError EsifAppMgr_CreateEntry(
	const EsifString appName,
	EsifAppMgrEntryPtr *entryPtr
);
static void EsifAppMgr_DestroyEntry(EsifAppMgrEntryPtr entryPtr);

static eEsifError EsifAppMgr_GetRefLocked(
	EsifAppMgrEntryPtr entryPtr,
	Bool isSvcRef
);

//
// IMPLEMENTATION FUNCTIONS
//

//
// WARNINGS:
// 1. The caller is responsible for calling EsifAppMgr_PutRef on the returned
// pointer when done using the app
// 2. May only be called by the "service" functions for the app to
// reference itself
//
EsifAppPtr EsifAppMgr_GetAppFromHandle(const esif_handle_t appHandle)
{
	EsifAppPtr appPtr = NULL;
	u8 i = 0;

	if (ESIF_INVALID_HANDLE == appHandle) {
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		appPtr = EsifAppMgr_GetAppN(i, ESIF_TRUE);

		if (EsifApp_IsAppHandle(appPtr, appHandle)) {
			break;
		}
		EsifAppMgr_PutRef(appPtr);
		appPtr = NULL;
	}
exit:
	return appPtr;
}


//
// WARNING:  The caller is responsible for calling EsifAppMgr_PutRef on the
// returned pointer when done using the app
//
EsifAppPtr EsifAppMgr_GetAppFromName(
	EsifString lib_name
	)
{
	EsifAppPtr appPtr = NULL;
	u8 i = 0;

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		appPtr = EsifAppMgr_GetAppN(i, ESIF_FALSE);

		if (EsifApp_IsAppName(appPtr, lib_name)) {
			break;
		}
		EsifAppMgr_PutRef(appPtr);
		appPtr = NULL;
	}
	return appPtr;
}


//
// Takes an additional reference on the app which prevents the app from being
// destroyed until all references are released.
//
static EsifAppPtr EsifAppMgr_GetAppN(
	size_t index,
	Bool isSvcRequest
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppMgrEntryPtr entryPtr = NULL;
	EsifAppPtr appPtr = NULL;

	if (index < (sizeof(g_appMgr.fEntries) / sizeof(*g_appMgr.fEntries))) {
		esif_ccb_write_lock(&g_appMgr.fLock);

		entryPtr = g_appMgr.fEntries[index];
		if (entryPtr) {
			rc = EsifAppMgr_GetRefLocked(entryPtr, isSvcRequest);
			if (ESIF_OK == rc) {
				appPtr = entryPtr->appPtr;
			}
		}
		esif_ccb_write_unlock(&g_appMgr.fLock);
	}
	return appPtr;
}


static eEsifError EsifAppMgr_GetRefLocked(
	EsifAppMgrEntryPtr entryPtr,
	Bool isSvcRequest
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == entryPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (entryPtr->svcRefOnly  && !isSvcRequest) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	rc = EsifApp_GetRef(entryPtr->appPtr);
exit:
	return rc;
}


/*
* NOTE:  Must be called when done using any app pointer obtained from AppMgr
* functions.
*/
void EsifAppMgr_PutRef(
	EsifAppPtr appPtr
	)
{
	EsifApp_PutRef(appPtr);
}


/*
* Used to iterate through the available apps.
* First call AppMgr_InitIterator to initialize the iterator.
* Next, call AppMgr_GetNextApp using the iterator.  Repeat until
* AppMgr_GetNextApp fails. The call will release the reference of the
* app from the previous call.  If you stop iteration part way through
* all apps, the caller is responsible for releasing the reference on
* the last app returned.  Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError AppMgr_InitIterator(
	AppMgrIteratorPtr iteratorPtr
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == iteratorPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = APPMGR_ITERATOR_MARKER;
exit:
	return rc;
}


/* See AppMgr_InitIterator for usage */
eEsifError AppMgr_GetNextApp(
	AppMgrIteratorPtr iteratorPtr,
	EsifAppPtr *appPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr nextAppPtr = NULL;
	size_t i;

	if ((NULL == appPtr) || (NULL == iteratorPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if ((iteratorPtr->marker != APPMGR_ITERATOR_MARKER) ||
		(iteratorPtr->index >= ESIF_MAX_APPS)) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (iteratorPtr->refTaken) {
		iteratorPtr->index++;
		EsifAppMgr_PutRef(iteratorPtr->appPtr);
		iteratorPtr->appPtr = NULL;
		iteratorPtr->refTaken = ESIF_FALSE;
	}

	for (i = iteratorPtr->index; i < ESIF_MAX_APPS; i++) {
		nextAppPtr = EsifAppMgr_GetAppN(i, ESIF_FALSE);
		if (nextAppPtr != NULL) {
			iteratorPtr->index = i;
			iteratorPtr->appPtr = nextAppPtr;
			iteratorPtr->refTaken = ESIF_TRUE;
			break;
		}
	}

	*appPtr = nextAppPtr;

	if (NULL == nextAppPtr) {
		rc = ESIF_E_ITERATION_DONE;
	}
exit:
	return rc;
}


eEsifError EsifAppMgr_GetPrompt(
	EsifDataPtr promptPtr
	)
{
	enum esif_rc rc = ESIF_OK;
	EsifAppPtr appPtr = g_appMgr.fSelectedAppPtr;
	EsifUpPtr upPtr = NULL;
	esif_handle_t target = g_dst;

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(target);
	if (NULL == upPtr) {
		target = ESIF_HANDLE_PRIMARY_PARTICIPANT;
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(target);
	}
	if (upPtr != NULL) {
		g_dst = target;
		esif_ccb_free(g_dstName);
		g_dstName = esif_ccb_strdup(EsifUp_GetName(upPtr));
		EsifUp_PutRef(upPtr);
	}

	if (g_dstName != NULL) {
		if (NULL == appPtr) {
			esif_ccb_sprintf(promptPtr->buf_len, (esif_string)promptPtr->buf_ptr, "ipf(%s)->", g_dstName);
		}
		else {
			esif_ccb_sprintf(promptPtr->buf_len, (esif_string)promptPtr->buf_ptr, "%s(%s)->", appPtr->fAppNamePtr, g_dstName);
		}
	}
	else {
		esif_ccb_sprintf(promptPtr->buf_len, (esif_string)promptPtr->buf_ptr, "esif(UNK)->");
	}
	return rc;
}


/* Event handler for events targeted at ALL apps without app registration */
static eEsifError ESIF_CALLCONV EsifAppMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	UInt8 i = 0;

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
	case ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED:
	{
		EsifData responseData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };
		rc = EsifExecutePrimitive(ESIF_HANDLE_PRIMARY_PARTICIPANT, GET_CONFIG, "D0", 255, NULL, &responseData);
		esif_ccb_free(responseData.buf_ptr);
	}
		break;
	default:
		break;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		
		appPtr = EsifAppMgr_GetAppN(i, ESIF_FALSE);

		if (NULL == appPtr) {
			continue;
		}

		switch (fpcEventPtr->esif_event)
		{
		case ESIF_EVENT_PARTICIPANT_SUSPEND:
			ESIF_TRACE_INFO("System suspend event received\n");
			EsifApp_SuspendApp(appPtr);
			break;

		case ESIF_EVENT_PARTICIPANT_RESUME:
			ESIF_TRACE_INFO("System resume event received\n");
			EsifApp_ResumeApp(appPtr);
			break;

		default:
			break;
		}
		EsifAppMgr_PutRef(appPtr);
	}
exit:
	return rc;
}


/* Removes a participant from each running application */
static eEsifError EsifAppMgr_EnquePartChange(
	const EsifUpPtr upPtr,
	Bool isCreate
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppMgrPartQueueItemPtr queueItemPtr = NULL;
	Bool refTaken = ESIF_FALSE;

	if (NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	queueItemPtr = esif_ccb_malloc(sizeof(*queueItemPtr));
	if (NULL == queueItemPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/*
	* Take a reference on the participant so that the pointer is valid when
	* dequeued.  The reference must be released after it is dequeued and used.
	*/
	rc = EsifUp_GetRef(upPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
	refTaken = ESIF_TRUE;

	queueItemPtr->upPtr = upPtr;
	queueItemPtr->isCreate = isCreate;

	esif_ccb_write_lock(&g_appMgr.fLock);

	if (NULL == g_appMgr.partQueuePtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto lockExit;
	}

	rc = esif_queue_enqueue(g_appMgr.partQueuePtr, queueItemPtr);
lockExit:
	esif_ccb_write_unlock(&g_appMgr.fLock);
exit:
	ESIF_TRACE_DEBUG("Participant-app state request queued(%d): rc = %d\n", isCreate, rc);
	if (rc != ESIF_OK) {
		if (refTaken) {
			EsifUp_PutRef(upPtr);
		}
		esif_ccb_free(queueItemPtr);
	}
	return rc;
}


/* Creates the participant in each running application */
eEsifError EsifAppMgr_CreateParticipantInAllApps(const EsifUpPtr upPtr)
{
	return EsifAppMgr_EnquePartChange(upPtr, ESIF_TRUE);
}


/* Creates the participant in each running application */
static eEsifError EsifAppMgr_CreateParticipantInAllAppsPriv(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	UInt8 i;

	if (NULL == upPtr) {
		ESIF_TRACE_ERROR("The prticipant data pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Now offer this participant to each running application */
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		appPtr = EsifAppMgr_GetAppN(i, ESIF_FALSE);

		if ((appPtr != NULL) && !EsifUp_IsPrimaryParticipant(upPtr)) {
			EsifApp_CreateParticipant(appPtr, upPtr);
		}
		EsifAppMgr_PutRef(appPtr);
	}
exit:
	return rc;
}


/* Removes a participant from each running application */
eEsifError EsifAppMgr_DestroyParticipantInAllApps(const EsifUpPtr upPtr)
{
	return EsifAppMgr_EnquePartChange(upPtr, ESIF_FALSE);
}


/* Removes a participant from each running application */
static eEsifError EsifAppMgr_DestroyParticipantInAllAppsPriv(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	UInt8 i;

	if (NULL == upPtr) {
		ESIF_TRACE_ERROR("The participant data pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		appPtr = EsifAppMgr_GetAppN(i, ESIF_FALSE);

		EsifApp_DestroyParticipant(appPtr, upPtr);

		EsifAppMgr_PutRef(appPtr);
	}
exit:
	return rc;
}


eEsifError EsifAppMgr_AppStart(const EsifString appName)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifAppMgrEntryPtr entryPtr = NULL;
	EsifAppMgrEntryPtr newEntryPtr = NULL;
	EsifAppPtr appPtr = NULL;
	EsifAppPtr newAppPtr = NULL;
	int j = 0;
	int availableIndex = ESIF_MAX_APPS;

	if (!g_appMgr.isInitialized) {  // Check if app creation is blocked
		goto exit;
	}

	if (NULL == appName) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_INFO("Creating app %s\n", appName);

	esif_ccb_write_lock(&g_appMgr.fLock);
	g_appMgr.creationRefCount++;

	/*
	* Keep Entries sorted by name and Search for an available slot and verify an app with the same name is not already running
	*/
	for (j = 0; j < ESIF_MAX_APPS; j++) {
		entryPtr = g_appMgr.fEntries[j];

		if (NULL == entryPtr) {	
			if (ESIF_MAX_APPS == availableIndex) {
				availableIndex = j;
			}
			continue;
		}
		appPtr = entryPtr->appPtr;
		if (EsifApp_IsAppName(appPtr, appName)) {
			rc = EsifAppMgr_GetRefLocked(entryPtr, ESIF_FALSE);
			if (ESIF_OK == rc) {
				EsifAppMgr_PutRef(appPtr);
				rc = ESIF_E_APP_ALREADY_STARTED;
				goto lockExit;
			}
		}
	}

	if (availableIndex >= ESIF_MAX_APPS) {
		rc = ESIF_E_MAXIMUM_CAPACITY_REACHED;
		goto lockExit;
	}

	rc = EsifAppMgr_CreateEntry(appName, &newEntryPtr);
	if (rc != ESIF_OK) {
		goto lockExit;
	}
	ESIF_ASSERT(newEntryPtr != NULL);
	newAppPtr = newEntryPtr->appPtr;

	/*
	* Take an extra reference on the app so that it can't be destroyed while
	* creation completes
	*/
	rc = EsifApp_GetRef(newAppPtr);
	if (rc != ESIF_OK) {
		EsifAppMgr_DestroyEntry(newEntryPtr); // Can destroy with lock held; app not started yet
		goto lockExit;
	}

	g_appMgr.fEntries[availableIndex] = newEntryPtr;
	g_appMgr.fEntryCount++;

	/* Start without locks held */
	esif_ccb_write_unlock(&g_appMgr.fLock);

	rc = EsifApp_Start(newAppPtr);

	EsifAppMgr_PutRef(newAppPtr); /* Release our extra reference */

	esif_ccb_write_lock(&g_appMgr.fLock);
	if ((rc != ESIF_OK) &&
	    (rc != ESIF_E_APP_ALREADY_STARTED) &&
		(rc != ESIF_I_INIT_PAUSED)) {

		ESIF_TRACE_DEBUG("Failure creating app %s\n", appName);
		
		/* If the app has NOT already been destroyed and is not in the process of being stopped; destroy it */
		if ((g_appMgr.fEntries[availableIndex] == newEntryPtr) && !newEntryPtr->svcRefOnly) {
			g_appMgr.fEntries[availableIndex] = NULL;
			g_appMgr.fEntryCount--;
			g_appMgr.creationRefCount--;
			esif_ccb_write_unlock(&g_appMgr.fLock);
			EsifAppMgr_DestroyEntry(newEntryPtr);
			goto exit;
		}
	}
lockExit:
	g_appMgr.creationRefCount--;
	ESIF_TRACE_INFO("Done creating app %s; Exit code = %s(%d)\n", appName, esif_rc_str(rc), rc);
	esif_ccb_write_unlock(&g_appMgr.fLock);
	esif_queue_signal_event(g_appMgr.partQueuePtr);
exit:
	return rc;
}


eEsifError EsifAppMgr_AppStop(const EsifString appName)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifAppMgrEntryPtr entryPtr = NULL;
	EsifAppPtr appPtr = NULL;
	u8 i = 0;

	if (NULL == appName) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		entryPtr = g_appMgr.fEntries[i];
		if (entryPtr && !entryPtr->svcRefOnly && EsifApp_IsAppName(entryPtr->appPtr, appName)) {
			entryPtr->svcRefOnly = ESIF_TRUE; // Force rejection of any other stop requests
			appPtr = entryPtr->appPtr;
			break;
		}
		entryPtr = NULL;
	}
	if (NULL == appPtr) {
		rc = ESIF_E_NOT_FOUND;
		esif_ccb_write_unlock(&g_appMgr.fLock); 
		goto exit;
	}
	esif_ccb_write_unlock(&g_appMgr.fLock);

	// If this is a Restartable App, Stop all Child Non-Restartable Apps with the same Libname
	if (appPtr->isRestartable) {
		esif_ccb_write_lock(&g_appMgr.fLock);
		for (int j = 0; j < ESIF_MAX_APPS; j++) {
			if (j != i && g_appMgr.fEntries[j]) {
				EsifAppPtr childAppPtr = g_appMgr.fEntries[j]->appPtr;
				if (childAppPtr && childAppPtr->isRestartable == ESIF_FALSE && esif_ccb_stricmp(childAppPtr->fLibNamePtr, entryPtr->appPtr->fLibNamePtr) == 0) {
					char childAppName[ESIF_NAME_LEN] = { 0 };
					esif_ccb_strcpy(childAppName, childAppPtr->fAppNamePtr, sizeof(childAppName));

					esif_ccb_write_unlock(&g_appMgr.fLock);
					EsifAppMgr_AppStop(childAppName);
					esif_ccb_write_lock(&g_appMgr.fLock);
				}
			}
		}
		esif_ccb_write_unlock(&g_appMgr.fLock);
	}

	EsifApp_Stop(appPtr);

	// If app is still present, remove and destroy it
	esif_ccb_write_lock(&g_appMgr.fLock);
	if (g_appMgr.fEntries[i] != entryPtr) {
		entryPtr = NULL; // Prevent destruction if already removed
	}
	else {
		g_appMgr.fEntries[i] = NULL;
		g_appMgr.fEntryCount--;
	}
	esif_ccb_write_unlock(&g_appMgr.fLock); 

	rc = ESIF_OK;
exit:
	EsifAppMgr_DestroyEntry(entryPtr);
	return rc;
}


eEsifError EsifAppMgr_AppRestartAll()
{
	eEsifError rc = ESIF_OK;
	EsifString loadedApps[ESIF_MAX_APPS] = { 0 };
	int appCount = 0;
	int j = 0;

	esif_ccb_write_lock(&g_appMgr.fLock);

	for (j = 0; j < ESIF_MAX_APPS; j++) {
		if ((g_appMgr.fEntries[j] != NULL) && EsifApp_IsRestartable(g_appMgr.fEntries[j]->appPtr)) {
			loadedApps[appCount] = EsifApp_CopyAppFullName(g_appMgr.fEntries[j]->appPtr);
			if (loadedApps[appCount] == NULL) {
				rc = ESIF_E_NO_MEMORY;
				break;
			}
			appCount++;
		}
	}
	esif_ccb_write_unlock(&g_appMgr.fLock);

	// Stop all Restartable Apps, which also stops all Non-Restartable Apps
	for (j = 0; j < appCount; j++) {
		ESIF_TRACE_INFO("Stopping App %s ...", loadedApps[j]);
		rc = EsifAppMgr_AppStop(loadedApps[j]);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("Error Stopping App %s - %s (%d) ...", loadedApps[j], esif_rc_str(rc), rc);
		}
	}

	// Start all Restartable Apps, which does not start any Non-Restartable Apps
	for (j = 0; j < appCount; j++) {
		ESIF_TRACE_INFO("Restarting App %s ...", loadedApps[j]);
		rc = EsifAppMgr_AppStart(loadedApps[j]);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("Error Restarting App %s - %s (%d) ...", loadedApps[j], esif_rc_str(rc), rc);
		}
		esif_ccb_free(loadedApps[j]);
	}

	return rc;
}


eEsifError EsifAppMgr_AppRename(
	const EsifString appName,
	const EsifString newName
)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(appName);
	EsifAppPtr newPtr = EsifAppMgr_GetAppFromName(newName);

	// Close Orphan Client App if one exists with the same name that uses the same library (ipfsrv)
	if (appPtr && newPtr && !newPtr->isRestartable 
		&& appPtr->fLibNamePtr && newPtr->fLibNamePtr && esif_ccb_stricmp(appPtr->fLibNamePtr, newPtr->fLibNamePtr) == 0
		&& newPtr->appCreationDone && !newPtr->markedForDelete) {

		char intro_buf[1024] = {0};
		EsifData data_intro = { ESIF_DATA_STRING, intro_buf, sizeof(intro_buf), 0 };
		EsifString intro = newPtr->fAppIntroPtr;
		newPtr->fAppIntroPtr = NULL;
		esif_ccb_free(intro);

		// Call GetIntro to determine if we're still talking to a valid IPF Client Session
		rc = EsifApp_GetIntro(newPtr, &data_intro);
		if (rc == ESIF_E_INVALID_HANDLE) {
			EsifAppMgr_PutRef(newPtr);
			rc = EsifAppMgr_AppStop(newName);
			if (rc != ESIF_OK) {
				EsifAppMgr_PutRef(appPtr);
				appPtr = NULL;
			}
			newPtr = NULL;
		}
	}

	if (newPtr) {
		rc = ESIF_E_APP_ALREADY_STARTED;
	}
	else if (appPtr) {
		EsifString newNamePtr = esif_ccb_strdup(newName);
		if (newNamePtr == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			esif_ccb_write_lock(&g_appMgr.fLock);
			esif_ccb_write_lock(&appPtr->objLock);

			EsifString oldNamePtr = appPtr->fAppNamePtr;
			appPtr->fAppNamePtr = newNamePtr;

			ESIF_TRACE_INFO("Renaming app %s to %s\n", oldNamePtr, newNamePtr);

			esif_ccb_write_unlock(&appPtr->objLock);
			esif_ccb_write_unlock(&g_appMgr.fLock);

			esif_ccb_free(oldNamePtr);
			rc = ESIF_OK;
		}
	}
	EsifAppMgr_PutRef(appPtr);
	EsifAppMgr_PutRef(newPtr);
	return rc;
}

static eEsifError EsifAppMgr_CreateEntry(
	const EsifString appName,
	EsifAppMgrEntryPtr *entryPtr
)
{
	eEsifError rc = ESIF_OK;
	EsifAppMgrEntryPtr newEntryPtr = NULL;

	ESIF_ASSERT(entryPtr != NULL);

	newEntryPtr = (EsifAppMgrEntryPtr)esif_ccb_malloc(sizeof(*newEntryPtr));
	if (NULL == newEntryPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = EsifApp_Create(appName, &newEntryPtr->appPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	if (NULL == newEntryPtr->appPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	newEntryPtr->svcRefOnly = ESIF_FALSE;
	*entryPtr = newEntryPtr;
exit:
	if (rc != ESIF_OK) {
		EsifAppMgr_DestroyEntry(newEntryPtr);
	}
	ESIF_TRACE_VERBOSE("Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


static void EsifAppMgr_DestroyEntry(
	EsifAppMgrEntryPtr entryPtr
	)
{
	if (NULL == entryPtr) {
		goto exit;
	}
	EsifApp_Destroy(entryPtr->appPtr);
exit:
	esif_ccb_free(entryPtr);
	return;
}


static void *ESIF_CALLCONV EsifAppMgr_PartQueueThread(void *ctxPtr)
{
	EsifAppMgrPartQueueItemPtr queueItemPtr = NULL;
	Bool isProcessing = ESIF_FALSE;

	UNREFERENCED_PARAMETER(ctxPtr);

	while (!g_appMgr.partQueueExitFlag) {
		queueItemPtr = esif_queue_pull(g_appMgr.partQueuePtr);

		if (NULL == queueItemPtr) {
			continue;
		}

		esif_ccb_write_lock(&g_appMgr.fLock);
		isProcessing = (g_appMgr.creationRefCount != 0) ? ESIF_TRUE : ESIF_FALSE;
		esif_ccb_write_unlock(&g_appMgr.fLock);

		/*
		* If and app is being created, re-queue the participant change until
		* after it is done creating the app
		*/
		if (isProcessing) {
			ESIF_TRACE_DEBUG("Requeuing participant-app state request\n");
			esif_ccb_sleep_msec(ESIF_UF_APPMGR_QUEUE_DELAY_TIME);
			esif_queue_requeue(g_appMgr.partQueuePtr, queueItemPtr);
			continue;
		}

		if (queueItemPtr->isCreate) {
			ESIF_TRACE_DEBUG("Creating participant in all apps\n");
			EsifAppMgr_CreateParticipantInAllAppsPriv(queueItemPtr->upPtr);
			ESIF_TRACE_DEBUG("Participant created in all apps\n");
		}
		else {
			ESIF_TRACE_DEBUG("Destroying participant in all apps\n");
			EsifAppMgr_DestroyParticipantInAllAppsPriv(queueItemPtr->upPtr);
			ESIF_TRACE_DEBUG("Participant destroyed in all apps\n");
		}

		EsifUp_PutRef(queueItemPtr->upPtr);
		esif_ccb_free(queueItemPtr);
	}
	return 0;
}


static void EsifAppMgr_QueueDestroyCallback(void *ctxPtr)
{
	EsifAppMgrPartQueueItemPtr queueItemPtr = (EsifAppMgrPartQueueItemPtr)ctxPtr;

	if (queueItemPtr != NULL) {
		EsifUp_PutRef(queueItemPtr->upPtr);
		esif_ccb_free(queueItemPtr);
	}
}


eEsifError EsifAppMgr_Init(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_appMgr.fLock);

	g_appMgr.partQueuePtr = esif_queue_create(ESIF_UF_APPMGR_QUEUE_SIZE, ESIF_UF_APPMGR_QUEUE_NAME, ESIF_UF_APPMGR_QUEUE_TIMEOUT);
	if (NULL == g_appMgr.partQueuePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = esif_ccb_thread_create(&g_appMgr.partQueueThread, EsifAppMgr_PartQueueThread, NULL);
	if (rc != ESIF_OK) {
		goto exit;
	}
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


eEsifError EsifAppMgr_Start(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);

	g_appMgr.isInitialized = ESIF_TRUE;

	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifAppMgr_Stop(void)
{
	u8 i = 0;
	EsifAppMgrEntryPtr entryPtr = NULL;
	EsifQueuePtr partQueuePtr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	g_appMgr.isInitialized = ESIF_FALSE;  // Prevent creation of any new apps

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, 0);

	g_appMgr.partQueueExitFlag = ESIF_TRUE;
	esif_queue_signal_event(g_appMgr.partQueuePtr);
	esif_ccb_thread_join(&g_appMgr.partQueueThread);

	esif_ccb_write_lock(&g_appMgr.fLock);
	partQueuePtr = g_appMgr.partQueuePtr;
	g_appMgr.partQueuePtr = NULL;
	esif_ccb_write_unlock(&g_appMgr.fLock);

	esif_queue_destroy(partQueuePtr, EsifAppMgr_QueueDestroyCallback);

	esif_ccb_write_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		entryPtr = g_appMgr.fEntries[i];

		if (entryPtr && entryPtr->appPtr && entryPtr->appPtr->fAppNamePtr) {
			esif_ccb_write_unlock(&g_appMgr.fLock);
			EsifAppMgr_AppStop(entryPtr->appPtr->fAppNamePtr);
			esif_ccb_write_lock(&g_appMgr.fLock);
		}
	}
	esif_ccb_write_unlock(&g_appMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


void EsifAppMgr_Exit(void)
{
	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_uninit(&g_appMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
