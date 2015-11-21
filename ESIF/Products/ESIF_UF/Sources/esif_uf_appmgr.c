/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
	if (rc!= ESIF_OK) {
		goto exit;
	}

	rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == rc) {
		metaPtr = EsifUp_GetMetadata(upPtr);
		if ((metaPtr != NULL) && (metaPtr->fAcpiType == domainType)) {
				participantId = EsifUp_GetInstance(upPtr);
				found = ESIF_TRUE;
				break;
		}
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
exit:
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
eEsifError EsifAppMgrCreateParticipantInAllApps(const EsifUpPtr upPtr)
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
eEsifError EsifAppMgrDestroyParticipantInAllApps(const EsifUpPtr upPtr)
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


eEsifError EsifAppMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_appMgr.fLock);

	EsifAppInit();

	g_appMgr.GetAppFromName = GetAppFromName;
	g_appMgr.GetPrompt = GetPrompt;

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAppMgr_EventCallback, NULL);

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

	EsifAppExit();
	ESIF_TRACE_DEBUG("Exit Action Manager (APPMGR)");

	esif_ccb_read_lock(&g_appMgr.fLock);
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &g_appMgr.fEntries[i];

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
