/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Friends */
extern int g_dst;
EsifAppMgr g_appMgr = {0};

static eEsifError ESIF_CALLCONV EsifAppMgr_EventCallback(
	void *contextPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);



EsifAppPtr GetAppFromHandle(const void *appHandle)
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &g_appMgr.fEntries[i];

		if (NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}

		if (a_app_ptr->fHandle == appHandle) {
			return a_app_ptr;
		}
	}
	return NULL;
}


static EsifAppPtr GetAppFromName(
	EsifAppMgr *THIS,
	EsifString lib_name
	)
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &THIS->fEntries[i];

		if (NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}

		if (!esif_ccb_stricmp(a_app_ptr->fLibNamePtr, lib_name)) {
			return a_app_ptr;
		}
	}
	return NULL;
}


static eEsifError GetPrompt(
	EsifAppMgr *THIS,
	EsifDataPtr promptPtr
	)
{
	enum esif_rc rc = ESIF_OK;
	EsifAppPtr a_app_ptr = THIS->fSelectedAppPtr;
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(promptPtr->buf_len, (esif_string)promptPtr->buf_ptr, "esif(%u)->", g_dst);
	} else {
		ESIF_DATA(data_prompt, ESIF_DATA_STRING, promptPtr->buf_ptr, promptPtr->buf_len);
		rc = a_app_ptr->fInterface.fAppGetPromptFuncPtr(a_app_ptr->fHandle, &data_prompt);
	}
	return rc;
}


eEsifError EsifAppsEventByDomainType(
	enum esif_domain_type domainType,
	eEsifEventType eventType,
	EsifDataPtr eventData
	)
{
	eEsifError rc = ESIF_OK;
	u8 found = ESIF_FALSE;
	UInt8 participantId = 0;
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;
	EsifUpDataPtr metaPtr = NULL;

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc == ESIF_OK) {
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	while (ESIF_OK == rc) {
		metaPtr = EsifUp_GetMetadata(upPtr);
		if ((metaPtr != NULL) && (metaPtr->fAcpiType == domainType)) {
				participantId = EsifUp_GetInstance(upPtr);
				found = ESIF_TRUE;
				break;
		}
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	EsifUp_PutRef(upPtr);
	if (ESIF_FALSE == found) {
		rc = ESIF_E_NOT_FOUND;
	} else {
		rc = EsifEventMgr_SignalEvent(participantId, EVENT_MGR_DOMAIN_NA, eventType, eventData);
	}
	return rc;
}


/* Event handler for events targeted at ALL apps without app registration */
static eEsifError ESIF_CALLCONV EsifAppMgr_EventCallback(
	void *contextPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	UInt8 i = 0;

	UNREFERENCED_PARAMETER(contextPtr);
	UNREFERENCED_PARAMETER(domainId);
	UNREFERENCED_PARAMETER(eventDataPtr);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Only handle at the app level
	if (participantId != 0) {
		goto exit;
	}

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED:
	{
		EsifData responseData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };
		rc = EsifExecutePrimitive(ESIF_INSTANCE_LF, GET_CONFIG, "D0", 255, NULL, &responseData);
		esif_ccb_free(responseData.buf_ptr);
	}
		break;
	default:
		break;
	}

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		appPtr = &g_appMgr.fEntries[i];
		if (NULL == appPtr->fHandle) {
			continue;
		}

		switch (fpcEventPtr->esif_event)
		{
		case ESIF_EVENT_PARTICIPANT_SUSPEND:
			ESIF_TRACE_INFO("System suspend event received\n");
			if (NULL != appPtr->fInterface.fAppSuspendFuncPtr) {
				appPtr->fInterface.fAppSuspendFuncPtr(appPtr->fHandle);
			}
			break;

		case ESIF_EVENT_PARTICIPANT_RESUME:
			ESIF_TRACE_INFO("System resume event received\n");
			if (NULL != appPtr->fInterface.fAppResumeFuncPtr) {
				appPtr->fInterface.fAppResumeFuncPtr(appPtr->fHandle);
			}
			break;

		default:
			break;
		}
	}

exit:
	return rc;
}


/* Creates the participant in each running application */
eEsifError EsifAppMgr_CreateParticipantInAllApps(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr app_ptr = NULL;
	UInt8 i;

	if (NULL == upPtr) {
		ESIF_TRACE_ERROR("The prticipant data pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Now offer this participant to each running application */
	esif_ccb_read_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		app_ptr = &g_appMgr.fEntries[i];
		if (NULL != app_ptr->fHandle) {
			EsifAppCreateParticipant(app_ptr, upPtr);
		}
	}
	esif_ccb_read_unlock(&g_appMgr.fLock);
exit:
	return rc;
}


/* Removes a participant from each running application */
eEsifError EsifAppMgr_DestroyParticipantInAllApps(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr app_ptr = NULL;
	UInt8 i;

	if (NULL == upPtr) {
		ESIF_TRACE_ERROR("The participant data pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_read_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		app_ptr = &g_appMgr.fEntries[i];

		if (NULL != app_ptr->fHandle) {
			EsifAppDestroyParticipant(app_ptr, upPtr);
		}
	}

	esif_ccb_read_unlock(&g_appMgr.fLock);
exit:
	return rc;
}

static eEsifError EsifAppMgr_AppStart_WLock(const EsifString appName)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifAppPtr a_app_ptr = NULL;
	int j = 0;

	// Check whether the App is already started
	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, appName);
	if (NULL == a_app_ptr) {
		// Find next available App slot or return error if Max Apps reached
		for (j = 0; j < ESIF_MAX_APPS; j++) {
			if (NULL == g_appMgr.fEntries[j].fLibNamePtr) {
				break;
			}
		}
		if (ESIF_MAX_APPS == j) {
			rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
			goto exit;
		}

		a_app_ptr = &g_appMgr.fEntries[j];
		a_app_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(appName);
		g_appMgr.fEntryCount++;
	}

	rc = EsifAppStart(a_app_ptr);

	 if ((rc != ESIF_OK) &&
		 (rc != ESIF_E_APP_ALREADY_STARTED) &&
		 (rc != ESIF_I_INIT_PAUSED)) {
		esif_ccb_free(a_app_ptr->fLibNamePtr);
		memset(a_app_ptr, 0, sizeof(*a_app_ptr));
		g_appMgr.fEntryCount--;
	 }
exit:
	return rc;
}

static eEsifError EsifAppMgr_AppStop_WLock(const EsifString appName)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifAppPtr a_app_ptr = NULL;

	a_app_ptr = g_appMgr.GetAppFromName(&g_appMgr, appName);
	if (NULL == a_app_ptr) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	rc = EsifAppStop(a_app_ptr);

	if (ESIF_OK == rc) {
		g_appMgr.fEntryCount--;
	}

exit:
	return rc;
}

static eEsifError EsifAppMgr_AppRestart_WLock(const EsifString appName)
{
	eEsifError rc = EsifAppMgr_AppStop_WLock(appName);
	if (rc == ESIF_OK) {
		rc = EsifAppMgr_AppStart_WLock(appName);
	}
	return rc;
}

eEsifError EsifAppMgr_AppStart(const EsifString appName)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_write_lock(&g_appMgr.fLock);
	rc = EsifAppMgr_AppStart_WLock(appName);
	esif_ccb_write_unlock(&g_appMgr.fLock);
	return rc;
}

eEsifError EsifAppMgr_AppStop(const EsifString appName)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_write_lock(&g_appMgr.fLock);
	rc = EsifAppMgr_AppStop_WLock(appName);
	esif_ccb_write_unlock(&g_appMgr.fLock);
	return rc;
}

eEsifError EsifAppMgr_AppRestart(const EsifString appName)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_write_lock(&g_appMgr.fLock);
	rc = EsifAppMgr_AppRestart_WLock(appName);
	esif_ccb_write_unlock(&g_appMgr.fLock);
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
		if (NULL != g_appMgr.fEntries[j].fLibNamePtr) {
			loadedApps[appCount] = esif_ccb_strdup(g_appMgr.fEntries[j].fLibNamePtr);
			if (loadedApps[appCount] == NULL) {
				rc = ESIF_E_NO_MEMORY;
				break;
			}
			appCount++;
		}
	}

	for (j = 0; j < appCount; j++) {
		ESIF_TRACE_INFO("Restarting App %s ...", loadedApps[j]);
		rc = EsifAppMgr_AppRestart_WLock(loadedApps[j]);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("Error Restarting App %s - %s (%d) ...", loadedApps[j], esif_rc_str(rc), rc);
		}
		esif_ccb_free(loadedApps[j]);
	}

	esif_ccb_write_unlock(&g_appMgr.fLock);
	return rc;
}

eEsifError EsifAppMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_appMgr.fLock);

	g_appMgr.GetAppFromName = GetAppFromName;
	g_appMgr.GetPrompt = GetPrompt;

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);		

	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifAppMgrExit()
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PRIMARY_PARTICIPANT_ARRIVED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);

	ESIF_TRACE_DEBUG("Exit Action Manager (APPMGR)");

	esif_ccb_read_lock(&g_appMgr.fLock);
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &g_appMgr.fEntries[i];

		/* Release any reference to participants taken when init was paused */
		EsifUp_PutRef(a_app_ptr->upPtr);	
		a_app_ptr->upPtr = NULL;
		a_app_ptr->iteratorValid = ESIF_FALSE;

		// Attempt to gracefully shutdown App before forcing library unload
		if (a_app_ptr->fLibNamePtr != NULL) {
			EsifAppStop(a_app_ptr);
		}
		if (a_app_ptr->fLibNamePtr != NULL) {
			esif_ccb_library_unload(a_app_ptr->fLibHandle);
			esif_ccb_free(a_app_ptr->fLibNamePtr);
			esif_ccb_memset(a_app_ptr, 0, sizeof(*a_app_ptr));
		}
	}
	esif_ccb_read_unlock(&g_appMgr.fLock);

	esif_ccb_lock_uninit(&g_appMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
